/*
 * usb_speaker - Use USB isochronous transfers.
 *
 * usb_speaker.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_speaker.
 *
 * usb_speaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_speaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_speaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdalign.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <syscfg.h>
#include <crs.h>
#include <gpio.h>
#include <spi.h>
#include <tim.h>
#include <nvic.h>
#include <vector.h>
#include <dma.h>
#include <memorymap.h>
#include <usbdevfs.h>
#include <i2c.h>
#include <usart.h>

#include <syscall.h>

#include "usb_speaker.h"

#define FCK			32000000 /* USART clock frequency */
#define TIMX_CLK_APB1		32000000 /* TIM6 clock frequency */
#define TIMX_CLK_APB2		32000000 /* TIM21 clock frequency */
#define AUTORELOAD		(TIMX_CLK_APB2 / SAMPLING_FREQ)	/* TIM21 */
#define BUFFER_TABLE_ADDRESS	0
#define PACKET_MEMORY_START	(BUFFER_TABLE_ADDRESS + \
				 USBDEVFS_BUFFER_TABLE_SIZE)
#define DAC_CONFIG		0xb0003000 /* MCP4822 config bits: B + A */
#define T_POWERUP		1000 /* DS1881 power-up time: 1 ms */
#define MAXBUF			4 /* Buffer queue size (power of 2) */
#define TXBUFSIZE		32 /* Tx buffer size (power of 2) */
#define RXBUFSIZE		32 /* Rx buffer size (power of 2) */
/*
 * Maxim/Dallas DS1881
 * Dual NV Audio Taper Digital Potentiometer
 */
#define DS1881_SLA		0x50

static unsigned int buf[MAXBUF][AS_SIZE_MAX / 4];
static int length[MAXBUF];	/* half-word count */
static volatile int wi;		/* write index */
static volatile int ri;		/* read index */
static volatile bool busy;
static volatile bool sof;
volatile unsigned int recv_packet;
volatile unsigned int recv_len;
volatile int max_recv_len;
volatile int min_recv_len = AS_SIZE_MAX;
volatile unsigned int send_packet;
volatile unsigned int send_len;
volatile unsigned int overrun;
static volatile int sof_count;
volatile int max_sof_count;
volatile unsigned int sync_error;
volatile int i2c_status;
/* Tx buffer */
static volatile int tx_buf[TXBUFSIZE];
static volatile int tx_len;
/* Rx buffer */
static volatile int rx_buf[RXBUFSIZE];
static volatile int rx_len;

/*
 * Clock
 * PLL: 16 MHz * 4 / 2 = 32 MHz
 * SYSCLK	32 MHz
 * HCLK		32 MHz (AHB)
 * PCLK1	32 MHz (APB1)
 * PCLK2	32 MHz (APB2)
 * HSI48 -> USB
 */
static void clock_setup(void)
{
	rcc_enable_clock(RCC_PWR);
	pwr_set_vos(PWR_1_8V);

	rcc_enable_osc(RCC_HSI16);
	rcc_setup_pll(RCC_HSI16, 4, 2);
	rcc_enable_osc(RCC_PLL);
	flash_config_access(1, FLASH_PREFETCH | FLASH_PREREAD);
	rcc_set_sysclk_source(RCC_PLL);
	rcc_disable_osc(RCC_MSI);

	rcc_enable_clock(RCC_SYSCFG);
	syscfg_enable_vrefint(SYSCFG_VREFINT_HSI48);
	rcc_enable_osc(RCC_HSI48);
}

/* CRS: automatic triming */
static void crs_setup(void)
{
	rcc_enable_clock(RCC_CRS);
	crs_enable_autotrim();
	crs_enable();
}

/*
 * GPIO
 * PA0: TIM2_ETR
 * PA1: TIM2_IC2
 * PA2: TIM21_OC1	TIM2_IC1
 * PA3: TIM2_OC4	MCP4822 /CS
 * PA4: TIM22_ETR
 * PA5: SPI1_SCK	MCP4822 SCK, TIM22_ETR, TIM2_ETR
 * PA6: TIM22_OC1	TIM2_IC2
 * PA7: SPI1_MOSI	MCP4822 SDI
 * PA9: USART1_TX
 * PA10: USART1_RX
 * PA15: TIM2_IC1
 * PB0: output		LED1
 * PB1: output		LED2
 * PB6: I2C1_SCL	DS1881 SCL
 * PB7: I2C1_SDA	DS1881 SDA
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_AF5, 0, GPIO_PA0);
	gpio_config(GPIO_AF2, 0, GPIO_PA1);
	gpio_config(GPIO_AF0, GPIO_10MHZ, GPIO_PA2);
	gpio_config(GPIO_AF2, GPIO_10MHZ, GPIO_PA3);
	gpio_config(GPIO_AF5, 0, GPIO_PA4);
	gpio_config(GPIO_AF0, GPIO_10MHZ, GPIO_PA(5, 7));
	gpio_config(GPIO_AF5, GPIO_10MHZ, GPIO_PA6);
	gpio_config(GPIO_AF4, 0, GPIO_PA9);
	gpio_config(GPIO_AF4, GPIO_PULLUP, GPIO_PA10);
	gpio_config(GPIO_AF5, 0, GPIO_PA15);

	rcc_enable_clock(RCC_GPIOB);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PB(0, 1));
	gpio_set(GPIO_PB(0, 1));
	gpio_config(GPIO_AF1, GPIO_OPENDRAIN | GPIO_2MHZ, GPIO_PB(6, 7));
}

/* SPI1: master, SCK: 32 MHz / 4 = 8 MHz, 16-bit */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 4, 16, SPI_NSS_OUTPUT | SPI_ENABLE);
}

/*
 * TIM
 * TIM6: CK_CNT = 2 MHz, one-pulse mode
 * TIM21: frequency = SAMPLING_FREQ
 *   OC1:  PWM mode 2 (TIMx_CNT < TIMx_CCR1: inactive),
 *         compare = AUTORELOAD - 4
 * TIM22: external clock mode 1, count = 32
 *   OC1: PWM mode 2 (TIMx_CNT < TIMx_CCR1: inactive), compare = 16
 * TIM2: external clock mode 2, reset mode, count > 16, DMA: update
 *   IC1, IC2: XOR
 *   OC4: PWM mode 1 (TIMx_CNT < TIMx_CCR4: active), active low, compare = 16
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000000);

	rcc_enable_clock(RCC_TIM21);
	tim_setup_counter(TIM21, 1, AUTORELOAD);
	tim_set_capture_compare_value(TIM21_CC1, AUTORELOAD - 4);
	tim_set_capture_compare_mode(TIM21_CC1, TIM_CC_OUTPUT | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);

	rcc_enable_clock(RCC_TIM22);
	tim_set_slave_mode(TIM22, TIM_EXTERNAL_CLOCK_MODE1 | TIM_ETRF);
	tim_setup_counter(TIM22, 1, 32);
	tim_set_capture_compare_value(TIM22_CC1, 16);
	tim_set_capture_compare_mode(TIM22_CC1, TIM_CC_OUTPUT | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);
	tim_enable_counter(TIM22);

	rcc_enable_clock(RCC_TIM2);
	tim_enable_ti1_ch123_xor(TIM2);
	tim_set_slave_mode(TIM2, TIM_EXTERNAL_CLOCK_MODE2 | TIM_TRGI_RESET |
			   TIM_TI1FP1);
	tim_setup_counter(TIM2, 1, 256);
	tim_set_counter(TIM2, 16);
	tim_set_capture_compare_value(TIM2_CC4, 16);
	tim_set_capture_compare_mode(TIM2_CC4, TIM_CC_OUTPUT | TIM_OC_PWM1 |
				     TIM_OC_ACTIVE_LOW | TIM_CC_ENABLE);
	tim_enable_dma(TIM2, TIM_DMA_UPDATE);
	tim_enable_counter(TIM2);
}

/* DMA CH2: TIM2_UP, interrupt */
static void dma_setup(void)
{
	rcc_enable_clock(RCC_DMA);
	nvic_enable_irq(NVIC_DMA_CH2_3);
	dma_set_request(DMA_CH2_TIM2_UP);
}

/* USB: power-up, interrupt */
static void usbdevfs_setup(void)
{
	rcc_set_usb_rng_clock(RCC_HSI48);
	rcc_enable_clock(RCC_USB);
	nvic_enable_irq(NVIC_USB);
	usbdevfs_disable_power_down();
}

/*
 * I2C
 * Fast-mode
 * tPRESC = 32 MHz / 4 = 8 MHz = 125 ns
 * scl_hi = (tHIGH - tSYNC2(min)) / tPRESC
 *          (1 us - tAF(min) - 2 * tI2CCLK) / 125 ns = 8
 * scl_lo = (tLOW - tSYNC1(min)) / tPRESC
 *          (1.5 us - tAF(min) - 2 * tI2CCLK) / 125 ns = 12
 */
static void i2c_setup(void)
{
	rcc_enable_clock(RCC_I2C1);
	i2c_init_master(I2C1, 4, 8, 12, I2C_ENABLE);
}

/* USART1: 115200 8N1, interrupt */
static void usart_setup(void)
{
	rcc_enable_clock(RCC_USART1);
	nvic_enable_irq(NVIC_USART1);
	usart_enable_interrupt(USART1, USART_RXNE);
	usart_init(USART1, FCK, 115200, 8, USART_STOP_1, USART_PARITY_NONE,
		   USART_FLOW_NONE, USART_TX_RX);
}

/* Delay: 1 - 32768 us */
static void delay_us(int us)
{
	tim_set_autoreload(TIM6, us << 1);
	tim_enable_counter(TIM6);
	while (!tim_get_interrupt_status(TIM6, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM6, TIM_UPDATE);
}

void dma_ch2_3_isr(void)
{
	if (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		return;
	dma_disable(DMA_CH2);
	send_packet++;
	send_len += length[ri] * 2;
	length[ri] = 0;
	ri = (ri + 1) & (MAXBUF - 1);
	if (sof && length[ri]) {
		dma_setup_channel(DMA_CH2, (int)buf[ri], SPI1_DR, length[ri],
				  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC |
				  DMA_M_TO_P | DMA_COMPLETE | DMA_ENABLE);
	} else {
		tim_disable_counter(TIM21);
		busy = false;
	}
	if (sof) {
		sof = false;
		sof_count++;
		if (sof_count > max_sof_count)
			max_sof_count = sof_count;
		if (sof_count > SAMPLING_CYCLE * 3) {
			gpio_clear(GPIO_PB1);
			sync_error++;
			sof_count = 0;
		}
	}
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
}

void usart1_isr(void)
{
	int m;
	int s;
	static int rx_wp;
	static int tx_rp;

	m = usart_get_interrupt_mask(USART1, USART_RXNE | USART_TXE);
	s = usart_get_interrupt_status(USART1, USART_RXNE | USART_TXE);

	if (m & s & USART_RXNE) {
		if (rx_len < RXBUFSIZE) {
			rx_buf[rx_wp++] = usart_recv(USART1);
			rx_wp &= RXBUFSIZE - 1;
			rx_len++;
		} else {
			usart_flush_receive_data(USART1);
		}
	}

	if (m & s & USART_TXE) {
		if (tx_len) {
			usart_send(USART1, tx_buf[tx_rp++]);
			tx_rp &= TXBUFSIZE - 1;
			tx_len--;
	        } else {
			usart_disable_interrupt(USART1, USART_TXE);
		}
	}
}

static void tx_write_blocking(int data)
{
	static int tx_wp;

	while (tx_len == TXBUFSIZE)
		;
	usart_disable_interrupt(USART1, USART_TXE);
	tx_buf[tx_wp++] = data;
	tx_wp &= TXBUFSIZE - 1;
	tx_len++;
	usart_enable_interrupt(USART1, USART_TXE);
}

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n')
				tx_write_blocking('\r');
			tx_write_blocking(ptr[i]);
		}
		return i;
	}

	errno = EIO;
	return -1;
}

int _read(int file, char *ptr, int len)
{
	int i;
	static int rx_rp;

	if (file == STDIN_FILENO) {
		for (i = 0; i < len; i++) {
			while (rx_len == 0)
				;
			usart_disable_interrupt(USART1, USART_RXNE);
			ptr[i] = rx_buf[rx_rp++];
			rx_rp &= RXBUFSIZE - 1;
			rx_len--;
			usart_enable_interrupt(USART1, USART_RXNE);
			if (ptr[i] == '\r')
				ptr[i] = '\n';
		}
		return i;
	}

	errno = EIO;
	return -1;
}

static void control(void)
{
	int t;

	t = usbdevfs_get_ep_status(0, USBDEVFS_RX | USBDEVFS_SETUP |
				   USBDEVFS_TX);
	if (t & USBDEVFS_RX) {
		if (t & USBDEVFS_SETUP)
			control_setup();
		else
			control_rx();
	}
	if (t & USBDEVFS_TX)
		control_tx();
}

/*
 * Audio Data Format
 *
 * USB
 *   L       R       ...
 * | 16bit | 16bit | ...
 * PCM -32768,32767
 *
 * DAC
 *   DACA                  DACB                  ...
 * |A/B|-|GA|SHDN| 12bit |A/B|-|GA|SHDN| 12bit | ...
 * 0,0xfff
 */
static void isochronous(void)
{
	int t;
	int n;
	int i;
	static unsigned int count;

	t = usbdevfs_get_ep_status(1, USBDEVFS_RX | USBDEVFS_RX_DATA1);
	if (!(t & USBDEVFS_RX))
		n = 0;
	else if (t & USBDEVFS_RX_DATA1)
		n = usbdevfs_read0(1, (char *)buf[wi], AS_SIZE_MAX);
	else
		n = usbdevfs_read1(1, (char *)buf[wi], AS_SIZE_MAX);
	if (length[wi]) {
		gpio_clear(GPIO_PB1);
		overrun++;
	} else if (n > 4) {
		for (i = 0; i < n / 4; i++)
			buf[wi][i] = (((buf[wi][i] >> 4) & 0x0fff0fff) ^
				      0x08000800) | DAC_CONFIG;
		length[wi] = n / 2;
		wi = (wi + 1) & (MAXBUF - 1);
		count++;
		if ((count & 127) == 0) {
			if (count & 128)
				gpio_clear(GPIO_PB0);
			else
				gpio_set(GPIO_PB0);
		}
	}
	recv_packet++;
	recv_len += n;
	if (n > max_recv_len)
		max_recv_len = n;
	if (n < min_recv_len)
		min_recv_len = n;
	usbdevfs_clear_ep_interrupt(1);
}

/* Start DMA. */
static void usb_sof(void)
{
	static int start;

	nvic_disable_irq(NVIC_DMA_CH2_3);
	if (busy) {
		sof = true;
	} else if (length[ri]) {
		dma_setup_channel(DMA_CH2, (int)buf[ri], SPI1_DR, length[ri],
				  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC |
				  DMA_M_TO_P | DMA_COMPLETE | DMA_ENABLE);
		if (length[ri] > AS_SIZE / 2)
			start = 0;
		if (start == 0)
			tim_set_counter(TIM21, AUTORELOAD - 5);
		else
			tim_set_counter(TIM21, AUTORELOAD * start /
					SAMPLING_CYCLE  - 5);
		start = (start < SAMPLING_CYCLE - 1) ? (start + 1) : 0;
		tim_enable_counter(TIM21);
		busy = true;
		sof_count = 0;
	}
	nvic_enable_irq(NVIC_DMA_CH2_3);
}

void usb_isr(void)
{
	int s;
	int ep_id;

	s = usbdevfs_get_interrupt_status(USBDEVFS_CTR | USBDEVFS_RESET |
					  USBDEVFS_SOF |
					  USBDEVFS_DIR | USBDEVFS_EP_ID);
	if (s & USBDEVFS_CTR) {
		ep_id = s & USBDEVFS_EP_ID;
		if (ep_id == 0)
			control();
		else if (ep_id == 1)
			isochronous();
	}
	if (s & USBDEVFS_RESET) {
		usbdevfs_setup_endpoint(0, USBDEVFS_CONTROL, 0);
		usbdevfs_setup_endpoint(1, USBDEVFS_ISOCHRONOUS, AS_ENUM);
		usbdevfs_set_device_address(0);
		control_reset();
		usbdevfs_clear_interrupt(USBDEVFS_RESET);
	}
	if (s & USBDEVFS_SOF) {
		usb_sof();
		usbdevfs_clear_interrupt(USBDEVFS_SOF);
	}
}

void ds1881_write(const char *buf, int nbyte)
{
	int r;
	int i;
	
	do {
		i2c_start(I2C1, I2C_AUTOEND | DS1881_SLA, nbyte);
		i2c_send_blocking(I2C1, *buf);
		r = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					     I2C_NACKF);
		if (r & (I2C_ARLO | I2C_BERR)) {
			gpio_clear(GPIO_PB1);
			i2c_status = r;
			return;
		}
		if (r & I2C_NACKF) {
			while (!i2c_get_interrupt_status(I2C1, I2C_STOPF))
				;
			i2c_clear_interrupt(I2C1, I2C_NACKF | I2C_STOPF);
		}
	} while (r & I2C_NACKF);

	for (i = 0; i < nbyte - 1; i++) {
		i2c_send_blocking(I2C1, *++buf);
		r = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					     I2C_NACKF);
		if (r) {
			gpio_clear(GPIO_PB1);
			i2c_status = r;
			return;
		}
	}
	
	while (!i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					 I2C_NACKF | I2C_STOPF))
		;
	r = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR | I2C_NACKF);
	if (r) {
		gpio_clear(GPIO_PB1);
		i2c_status = r;
		return;
	}
	i2c_clear_interrupt(I2C1, I2C_STOPF);
}

int main(void)
{
	static const char ds1881_config = 0x86; /* 63 positions */
	int offset;

	clock_setup();
	crs_setup();
	gpio_setup();
	spi_setup();
	tim_setup();
	dma_setup();
	usbdevfs_setup();
	i2c_setup();
	usart_setup();

	delay_us(T_POWERUP);
	ds1881_write(&ds1881_config, 1);
	
	usbdevfs_disable_usb_reset();
	usbdevfs_clear_interrupt(USBDEVFS_PMAOVR | USBDEVFS_ERR |
				 USBDEVFS_WKUP | USBDEVFS_SUSP |
				 USBDEVFS_RESET | USBDEVFS_SOF |
				 USBDEVFS_ESOF | USBDEVFS_L1REQ);
	usbdevfs_set_buffer_table_address(BUFFER_TABLE_ADDRESS);
	offset = usbdevfs_assign_packet_memory_tx(0, PACKET_MEMORY_START,
						  MAXPACKETSIZE0);
	offset = usbdevfs_assign_packet_memory_rx(0, offset, MAXPACKETSIZE0);
	offset = usbdevfs_assign_packet_memory_rx0(1, offset, AS_SIZE_MAX);
	usbdevfs_assign_packet_memory_rx1(1, offset, AS_SIZE_MAX);
	usbdevfs_enable_dp_pull_up();
	usbdevfs_enable_interrupt(USBDEVFS_CTR | USBDEVFS_RESET |
				  USBDEVFS_SOF);
	command();

	return 0;
}
