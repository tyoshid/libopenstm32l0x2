/*
 * usb_radio - An FM radio using USB isochronous transfers.
 *
 * usb_radio.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_radio.
 *
 * usb_radio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_radio.  If not, see <http://www.gnu.org/licenses/>.
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
#include <tim.h>
#include <memorymap.h>
#include <dma.h>
#include <nvic.h>
#include <vector.h>
#include <i2c.h>
#include <exti.h>
#include <spi.h>
#include <usart.h>
#include <usbdevfs.h>
#include <scb.h>
#include <stk.h>

#include <syscall.h>

#include "usb_radio.h"

#define FCK			32000000 /* USART clock frequency */
#define TIMX_CLK_APB1		32000000 /* TIM6 clock frequency */
#define TIMX_CLK_APB2		32000000 /* TIM21 and TIM22 clock frequency */
#define HCLK			32000000
#define TXBUFSIZE		32	 /* Tx buffer size (power of 2) */
#define RXBUFSIZE		32	 /* Rx buffer size (power of 2) */
#define DEFAULT_RADIO_FREQ	8130	 /* 81.3 MHz (J-WAVE) */
#define BUFFER_TABLE_ADDRESS	0
#define PACKET_MEMORY_START	(BUFFER_TABLE_ADDRESS + \
				 USBDEVFS_BUFFER_TABLE_SIZE)
#define MAXBUF			10
#define BUFSIZE			(SAMPLING_FREQ / (1000 / MAXBUF) * \
				 SAMPLING_BYTES * SAMPLING_CHANNELS)

alignas(2) static char buf[BUFSIZE];
static volatile bool send_start;
static int tx_buf[TXBUFSIZE];
static volatile int tx_len;
static int rx_buf[RXBUFSIZE];
static volatile int rx_len;
volatile unsigned int send_packet;
volatile unsigned int send_len;
volatile unsigned int tim2_enable;
volatile unsigned int overrun;
volatile int spi_status;
volatile int usbdevfs_status;
volatile bool si4737_int;
static volatile bool si4737_dclk;
static bool si4737_dout;

volatile int apb2clk = TIMX_CLK_APB2;
volatile int scount;
volatile int frame;
volatile int hclk;

/*
 * Clock
 * PLL: 16 MHz * 4 / 2 = 32 MHz
 * SYSCLK	32 MHz
 * HCLK		32 MHz (AHB)
 * PCLK1	32 MHz (APB1)
 * PCLK2	32 MHz (APB2)
 * LSE -> MCO
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

	pwr_disable_backup_write_protection();
	rcc_enable_osc(RCC_LSE);
	rcc_setup_mco(RCC_LSE, 1);

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
 * PA1: output		Si4737 /RST
 * PA2: TIM21_CH1	TIM2_ETR
 * (PA3: TIM21_CH2	Si4737 DFS)
 * PA4: input		Si4737 /INT
 * PA5: SPI1_SCK
 * (PA6: TIM22_CH1	Si4737 DCLK, SPI1_SCK)
 * PA7: SPI1_MOSI	Si4737 DOUT
 * (PA8: MCO		Si4737 RCLK)
 * PA9: USART1_TX
 * PA10: USART1_RX
 * PA15: input		JP2
 * PB0: output		LED1
 * PB1: output		LED2
 * PB5: output		LM4861 SHUTDOWN
 * PB6: I2C1_SCL	Si4737 SCLK
 * PB7: I2C1_SDA	Si4737 SDIO
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_AF5, 0, GPIO_PA0);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA1);
	gpio_config(GPIO_AF0, 0, GPIO_PA2);
	gpio_config(GPIO_INPUT, GPIO_PULLDOWN, GPIO_PA4);
	gpio_config(GPIO_AF0, GPIO_PULLDOWN, GPIO_PA(5, 7));
	gpio_config(GPIO_AF4, 0, GPIO_PA9);
	gpio_config(GPIO_AF4, GPIO_PULLUP, GPIO_PA10);
	gpio_config(GPIO_INPUT, GPIO_PULLUP, GPIO_PA15);

	rcc_enable_clock(RCC_GPIOB);
	gpio_set(GPIO_PB(0, 1, 5));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PB(0, 1, 5));
	gpio_config(GPIO_AF1, GPIO_OPENDRAIN | GPIO_2MHZ, GPIO_PB(6, 7));
}

/*
 * TIM
 * TIM6: CK_CNT = 2 kHz, one-pulse mode
 * TIM2: one-pulse mode, external clock mode2, /ETR, count = 441, TRGO: enable,
 *       interrupt: update
 *
 * DCLK = 32 MHz / 44.1 kHz / (16 * 2) = 22
 *
 * TIM21: DFS, gated mode, count = 32 MHz / 44.1 kHz = 725, TRGO: OC1REF
 *        (counter = 725 - 1 > DCLK * 16 * 2)
 *   OC1: PWM mode 1 (TIMx_CNT < TIMx_CCR1: active), compare = DCLK * 16 * 2
 *   OC2: PWM mode 1 (TIMx_CNT < TIMx_CCR1: active), compare = DCLK
 * TIM22: DCLK, gated mode, count = DCLK
 *   OC1: PWM mode 2 (TIMx_CNT < TIMx_CCR1: inactive), compare = DCLK / 2
 */
static void tim_setup(void)
{
	int dcount;
	
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000);

	rcc_enable_clock(RCC_TIM2);
	nvic_enable_irq(NVIC_TIM2);
	tim_enable_one_pulse_mode(TIM2);
	tim_set_slave_mode(TIM2, TIM_EXTERNAL_CLOCK_MODE2 | TIM_ETR_INVERTED);
	tim_setup_counter(TIM2, 1, SAMPLING_FREQ / (1000 / MAXBUF));
	tim_set_master_mode(TIM2, TIM_TRGO_ENABLE);
	tim_clear_interrupt(TIM2, TIM_UPDATE);
	tim_enable_interrupt(TIM2, TIM_UPDATE);

	scount = apb2clk / SAMPLING_FREQ;
	dcount = scount / (SAMPLING_BITS * SAMPLING_CHANNELS);
	if (dcount * SAMPLING_BITS * SAMPLING_CHANNELS >= scount - 8)
		dcount--;
	
	rcc_enable_clock(RCC_TIM21);
	tim_set_slave_mode(TIM21, TIM_TRGI_GATED | TIM_ITR0);
	tim_setup_counter(TIM21, 1, scount);
	tim_set_counter(TIM21, scount - 1);
	tim_set_capture_compare_value(TIM21_CC1, dcount * SAMPLING_BITS *
				      SAMPLING_CHANNELS);
	tim_set_capture_compare_mode(TIM21_CC1, TIM_CC_OUTPUT | TIM_OC_PWM1 |
				     TIM_CC_ENABLE);
	tim_set_capture_compare_value(TIM21_CC2, dcount);
	tim_set_capture_compare_mode(TIM21_CC2, TIM_CC_OUTPUT | TIM_OC_PWM1 |
				     TIM_CC_ENABLE);
	tim_set_master_mode(TIM21, TIM_TRGO_OC1REF);
	tim_enable_counter(TIM21);

	rcc_enable_clock(RCC_TIM22);
	tim_set_slave_mode(TIM22, TIM_TRGI_GATED | TIM_ITR0);
	tim_setup_counter(TIM22, 1, dcount);
	tim_set_capture_compare_value(TIM22_CC1, dcount / 2);
	tim_set_capture_compare_mode(TIM22_CC1, TIM_CC_OUTPUT | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);
	tim_enable_counter(TIM22);
}

/*
 * DMA
 * CH2: SPI1_RX, SPI1_DR->buf, BUFSIZE, 16-bit, circular
 */
static void dma_setup(void)
{
	rcc_enable_clock(RCC_DMA);
	dma_set_request(DMA_CH2_SPI1_RX);
	dma_setup_channel(DMA_CH2, (int)buf, SPI1_DR, BUFSIZE / 2,
			  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC |
			  DMA_CIRCULAR | DMA_ENABLE);
}

/*
 * I2C1: master
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

/* EXTI4: falling edge trigger, interrupt */
void exti_setup(void)
{
	nvic_enable_irq(NVIC_EXTI4_15);
	exti_set_trigger(EXTI4, EXTI_FALLING);
}

/* SPI1: slave, 16-bit, receive-only, DMA */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_slave(SPI1, 16, SPI_RXONLY | SPI_NSS_LOW);
	spi_enable_dma(SPI1, SPI_DMA_RX);
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

/* USB: power-up, interrupt */
static void usbdevfs_setup(void)
{
	rcc_set_usb_rng_clock(RCC_HSI48);
	rcc_enable_clock(RCC_USB);
	nvic_enable_irq(NVIC_USB);
	usbdevfs_disable_power_down();
}

void tim2_isr(void)
{
	int dcount;

	if (tim_get_interrupt_status(TIM2, TIM_UPDATE)) {
		if (frame > SAMPLING_CYCLE) {
			apb2clk -= TIMX_CLK_APB2 / 100;
			scount = apb2clk / SAMPLING_FREQ;
			dcount = scount / (SAMPLING_BITS * SAMPLING_CHANNELS);
			if (dcount * SAMPLING_BITS * SAMPLING_CHANNELS >=
			    scount - 8)
				dcount--;
			tim_disable_counter(TIM22);
			tim_disable_capture_compare(TIM21_CC2);
			tim_setup_counter(TIM21, 1, scount);
			tim_set_counter(TIM21, scount - 1);
			tim_set_capture_compare_value(TIM21_CC1,
						      dcount * SAMPLING_BITS *
						      SAMPLING_CHANNELS);
			tim_set_capture_compare_value(TIM21_CC2, dcount);
			tim_enable_capture_compare(TIM21_CC2);
			tim_setup_counter(TIM22, 1, dcount);
			tim_set_capture_compare_value(TIM22_CC1, dcount / 2);
			tim_enable_counter(TIM22);
		}
		frame = 0;
		tim_clear_interrupt(TIM2, TIM_UPDATE);
	}
}

/* Delay: 1 - 32768 ms */
void delay_ms(int ms)
{
	tim_set_autoreload(TIM6, ms << 1);
	tim_enable_counter(TIM6);
	while (!tim_get_interrupt_status(TIM6, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM6, TIM_UPDATE);
}

void exti4_15_isr(void)
{
	int m;
	int s;
	
	m = exti_get_interrupt_mask(EXTI4);
	s = exti_get_interrupt_status(EXTI4);
	if (m & s & EXTI4) {
		si4737_int = true;
		exti_clear_interrupt(EXTI4);
	}
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

static void isochronous(void)
{
	int t;
	int m;
	int n;
	int i;
	static const int length[MAXBUF] = {
		AS_SIZE_MAX, AS_SIZE, AS_SIZE, AS_SIZE, AS_SIZE, AS_SIZE,
		AS_SIZE, AS_SIZE, AS_SIZE, AS_SIZE
	};
	int wi;
	static int ri;
	static unsigned int count;
	int r;
	
	t = usbdevfs_get_ep_status(1, USBDEVFS_TX | USBDEVFS_TX_DATA1);
	usbdevfs_clear_ep_interrupt(1);
	if (!(t & USBDEVFS_TX))
		return;
	m = BUFSIZE - dma_get_number_of_data(DMA_CH2) * 2;
	n = 0;
	for (i = 0; i < MAXBUF; i++) {
		n += length[i];
		if (m < n)
			break;
	}
	wi = i >= MAXBUF ? 0 : i;
	if (send_start) {
		ri = (wi >= MAXBUF / 2) ? (wi - MAXBUF / 2) :
			(wi + MAXBUF / 2);
		send_start = false;
	}
	if (si4737_dout && ri == wi) {
		ri = (wi >= MAXBUF / 2) ? (wi - MAXBUF / 2) :
			(wi + MAXBUF / 2);
		overrun++;
	}
	n = 0;
	for (i = 0; i < ri; i++)
		n += length[i];
	if (t & USBDEVFS_TX_DATA1)
		usbdevfs_write0(1, &buf[n], length[ri]);
	else
		usbdevfs_write1(1, &buf[n], length[ri]);
	send_packet++;
	send_len += length[ri];
	ri = (ri < MAXBUF - 1) ? (ri + 1) : 0;
	count++;
	if ((count & 127) == 0) {
		if (count & 128)
			gpio_clear(GPIO_PB0);
		else
			gpio_set(GPIO_PB0);
	}
	r = spi_get_interrupt_status(SPI1, SPI_OVR);
	if (r) {
		spi_status = r;
		gpio_clear(GPIO_PB1);
	}
}

void usb_isr(void)
{
	int s;
	int ep_id;
	int n;
	
	s = usbdevfs_get_interrupt_status(USBDEVFS_CTR | USBDEVFS_PMAOVR |
					  USBDEVFS_ERR | USBDEVFS_WKUP |
					  USBDEVFS_SUSP | USBDEVFS_RESET |
					  USBDEVFS_SOF | USBDEVFS_ESOF |
					  USBDEVFS_L1REQ |
					  USBDEVFS_DIR | USBDEVFS_EP_ID);
	usbdevfs_status |= s & (USBDEVFS_CTR | USBDEVFS_PMAOVR | USBDEVFS_ERR |
				USBDEVFS_WKUP | USBDEVFS_SUSP | USBDEVFS_SOF |
				USBDEVFS_ESOF | USBDEVFS_L1REQ);
	if (s & USBDEVFS_CTR) {
		ep_id = s & USBDEVFS_EP_ID;
		if (ep_id == 0)
			control();
		else if (ep_id == 1)
			isochronous();
	}
	if (s & USBDEVFS_RESET) {
		usbdevfs_status = 0;
		usbdevfs_setup_endpoint(0, USBDEVFS_CONTROL, 0);
		usbdevfs_setup_endpoint(1, USBDEVFS_ISOCHRONOUS, AS_ENUM);
		usbdevfs_set_device_address(0);
		control_reset();
		usbdevfs_clear_interrupt(USBDEVFS_RESET);
	}
	if (s & USBDEVFS_SOF) {
		nvic_disable_irq(NVIC_TIM2);
		if (frame == 0) {
			tim_set_counter(TIM22, 0);
			tim_set_counter(TIM21, scount - 1);
			tim_enable_counter(TIM2);
			if (!si4737_dclk)
				si4737_dclk = true;
			tim2_enable++;

			stk_disable_counter();
			n = stk_get_counter();
			stk_init(0x1000000, STK_HCLK | STK_ENABLE);
			hclk = hclk ? (0xffffff - n) * 1000 / SAMPLING_CYCLE :
				HCLK;
		}
		frame++;
		nvic_enable_irq(NVIC_TIM2);
		usbdevfs_clear_interrupt(USBDEVFS_SOF);
	}
}

int main(void)
{
	extern enum ep_state endpoint_state[];
	int offset;
	bool digital;
	bool si4737_error = false;

	clock_setup();
	crs_setup();
	gpio_setup();
	tim_setup();
	dma_setup();
	i2c_setup();
	exti_setup();
	spi_setup();
	usart_setup();
	usbdevfs_setup();
	scb_set_sleep(SCB_SLEEP_DEEP);
	pwr_set_mode(PWR_STOP);

	digital = gpio_get(GPIO_PA15);
	if (si4737_init(DEFAULT_RADIO_FREQ, digital))
		si4737_error = true;
	if (digital) {
		puts("Digital output");
		printf("sampling: %2d", SAMPLING_FREQ / 1000);
		if (SAMPLING_FREQ % 1000)
			printf(".%d", SAMPLING_FREQ % 1000 / 100);
		printf(" kHz  %2d bits\n", SAMPLING_BITS);
	} else {
		puts("Analog output");
		gpio_clear(GPIO_PB5);
	}
	usbdevfs_disable_usb_reset();
	usbdevfs_clear_interrupt(USBDEVFS_PMAOVR | USBDEVFS_ERR |
				 USBDEVFS_WKUP | USBDEVFS_SUSP |
				 USBDEVFS_RESET | USBDEVFS_SOF |
				 USBDEVFS_ESOF | USBDEVFS_L1REQ);
	usbdevfs_set_buffer_table_address(BUFFER_TABLE_ADDRESS);
	offset = usbdevfs_assign_packet_memory_tx(0, PACKET_MEMORY_START,
						  MAXPACKETSIZE0);
	offset = usbdevfs_assign_packet_memory_rx(0, offset, MAXPACKETSIZE0);
	offset = usbdevfs_assign_packet_memory_tx0(1, offset, AS_SIZE_MAX);
	usbdevfs_assign_packet_memory_tx1(1, offset, AS_SIZE_MAX);
	command_init();
	if (digital) {
		usbdevfs_enable_dp_pull_up();
		usbdevfs_enable_interrupt(USBDEVFS_CTR | USBDEVFS_RESET |
					  USBDEVFS_SOF);
	}
	while (1) {
		if (rx_len)
			command_line();
		if (endpoint_state[1] == EP_STATE_ENABLE &&
		    !si4737_dout && si4737_dclk && !si4737_error) {
			if (si4737_enable_digital_output())
				si4737_error = true;
			si4737_dout = true;
			send_start = true;
		}
	}
		
	return 0;
}
