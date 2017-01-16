/*
 * usb_cdcacm - A USB serial port.
 *
 * usb_cdcacm.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_cdcacm.
 *
 * usb_cdcacm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_cdcacm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_cdcacm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdalign.h>
#include <string.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <syscfg.h>
#include <crs.h>
#include <gpio.h>
#include <nvic.h>
#include <vector.h>
#include <usart.h>
#include <tim.h>
#include <usbdevfs.h>
#include <dbg.h>
#include <misc.h>

#include "usb_cdcacm.h"

#define TXBUFSIZE		32 /* power of 2 */
#define RXBUFSIZE		32 /* power of 2 */
#define BUFFER_TABLE_ADDRESS	0
#define PACKET_MEMORY_START	(BUFFER_TABLE_ADDRESS + \
				 USBDEVFS_BUFFER_TABLE_SIZE)

extern volatile enum cmd_state command_state;
extern char command_buf[];
extern volatile int command_len;
extern char escape_char;
extern int escape_code_guard_time;
extern char data_buf[];
extern volatile int data_wp;
extern volatile int data_rp;
extern volatile int data_len;
extern volatile enum ep_state endpoint_state[];
extern volatile int databits;
extern volatile bool parity_mark;

volatile int tx_buf[TXBUFSIZE];
volatile int tx_len;
volatile int rx_buf[RXBUFSIZE];
volatile int rx_len;
volatile bool notification_busy;
volatile bool bulk_rx_busy;
volatile bool bulk_tx_busy;
volatile bool suspended;

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
	rcc_set_msi_range(RCC_MSI_65KHZ);

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
 * PA0: USART2_CTS
 * PA1: USART2_RTS
 * PA2: USART2_TX
 * PA3: USART2_RX
 * PA4: output		LED1
 * PA5: output		LED2
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_AF4, GPIO_PULLDOWN, GPIO_PA0);
	gpio_config(GPIO_AF4, 0, GPIO_PA(1, 2));
	gpio_config(GPIO_AF4, GPIO_PULLUP, GPIO_PA3);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(4, 5));
}

/* USART2: interrupt */
static void usart_setup(void)
{
	rcc_enable_clock(RCC_USART2);
	nvic_enable_irq(NVIC_USART2);
	usart_enable_interrupt(USART2, USART_RXNE | USART_ORE | USART_NF |
			       USART_FE | USART_PE);
}

/*
 * TIM
 * TIM6: delay_us(): CK_CNT = 2 MHz, one-pulse mode
 * TIM21: Escape code guard time: CK_CNT = 1 kHz, one-pulse mode, interrupt
 * TIM22: SendBreak: one-pulse mode, interrupt
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000000);

	rcc_enable_clock(RCC_TIM21);
	nvic_enable_irq(NVIC_TIM21);
	tim_enable_one_pulse_mode(TIM21);
	tim_disable_update_interrupt_on_any(TIM21);
	tim_load_prescaler(TIM21, TIMX_CLK_APB2 / 1000);
	tim_enable_interrupt(TIM21, TIM_UPDATE);

	rcc_enable_clock(RCC_TIM22);
	nvic_enable_irq(NVIC_TIM22);
	tim_enable_one_pulse_mode(TIM22);
	tim_disable_update_interrupt_on_any(TIM22);
	tim_enable_interrupt(TIM22, TIM_UPDATE);
}

/* USB: power-up, interrupt */
static void usbdevfs_setup(void)
{
	rcc_set_usb_rng_clock(RCC_HSI48);
	rcc_enable_clock(RCC_USB);
	nvic_enable_irq(NVIC_USB);
	usbdevfs_disable_power_down();
}

void usart2_isr(void)
{
	int m;
	int s;
	int r;
	static int rx_wp;
	static int tx_rp;

	m = usart_get_interrupt_mask(USART2, USART_RXNE | USART_TXE |
				     USART_ORE | USART_NF | USART_FE |
				     USART_PE);
	s = usart_get_interrupt_status(USART2, USART_RXNE | USART_TXE |
				       USART_ORE | USART_NF | USART_FE |
				       USART_PE);
	if (m & s & USART_RXNE) {
		if (rx_len < RXBUFSIZE) {
			gpio_toggle(GPIO_PA4);
			r = usart_recv(USART2);
			rx_buf[rx_wp++] = r & ((1 << databits) - 1);
			rx_wp &= RXBUFSIZE - 1;
			rx_len++;
		} else {
			usart_flush_receive_data(USART2);
		}
	}
	if (m & s & USART_TXE) {
		if (tx_len) {
			gpio_toggle(GPIO_PA5);
			usart_send(USART2,
				   (tx_buf[tx_rp++] & ((1 << databits) -1)) |
				   (parity_mark ? (1 << databits) : 0));
			tx_rp &= TXBUFSIZE - 1;
			tx_len--;
		} else {
			usart_disable_interrupt(USART2, USART_TXE);
		}
	}
	if (m & s & (USART_ORE | USART_NF | USART_FE | USART_PE)) {
		notify_serial_state(m & s & (USART_ORE | USART_NF | USART_FE |
					     USART_PE));
		usart_clear_interrupt(USART2, m & s & (USART_ORE | USART_NF |
						       USART_FE | USART_PE));
	}
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

void tim21_isr(void)
{
	int i;
	
	if (tim_get_interrupt_mask(TIM21, TIM_UPDATE) &&
	    tim_get_interrupt_status(TIM21, TIM_UPDATE)) {
		switch (command_state) {
		case ESCAPE_STATE1:
			data_buf[data_wp++] = escape_char;
			if (data_wp >= DATABUFSIZE)
				data_wp = 0;
			data_len++;
			command_state = ONLINE_DATA_STATE;
			break;
		case ESCAPE_STATE2:
			for (i = 0; i < 2; i++) {
				data_buf[data_wp++] = escape_char;
				if (data_wp >= DATABUFSIZE)
					data_wp = 0;
				data_len++;
			}
			command_state = ONLINE_DATA_STATE;
			break;
		case ESCAPE_STATE3:
			command_state = COMMAND_STATE;
			strcpy(command_buf, "\r\nOK\r\n");
			command_len = 6;
			break;
		default:
			break;
		}
		tim_clear_interrupt(TIM21, TIM_UPDATE);
	}
}

void tim22_isr(void)
{
	if (tim_get_interrupt_mask(TIM22, TIM_UPDATE) &&
	    tim_get_interrupt_status(TIM22, TIM_UPDATE)) {
		gpio_set_mode(GPIO_ALTFUNC, GPIO_PA2);
		tim_clear_interrupt(TIM22, TIM_UPDATE);
	}
}

static void reset(void)
{
	usbdevfs_setup_endpoint(0, USBDEVFS_CONTROL, 0);
	usbdevfs_setup_endpoint(NOTIFICATION_INDEX, USBDEVFS_INTERRUPT,
				NOTIFICATION_ENUM);
	usbdevfs_setup_endpoint(DATA_RX_INDEX, USBDEVFS_BULK, DATA_RX_ENUM);
	usbdevfs_setup_endpoint(DATA_TX_INDEX, USBDEVFS_BULK, DATA_TX_ENUM);
	usbdevfs_set_device_address(0);
	control_reset();
	bulk_tx_busy = false;
	bulk_rx_busy = false;
	notification_busy = false;
}

static void suspend(void)
{
	usbdevfs_enable_suspend_mode();
	rcc_disable_clock(RCC_TIM22);
	rcc_disable_clock(RCC_TIM21);
	rcc_disable_clock(RCC_TIM6);
	rcc_disable_clock(RCC_USART2);
	gpio_set_mode(GPIO_ANALOG, GPIO_PA(0, 1, 2, 3, 4, 5));
	rcc_disable_clock(RCC_CRS);
	usbdevfs_enable_low_power_mode();
	rcc_enable_osc(RCC_MSI);
	rcc_set_sysclk_source(RCC_MSI);
	flash_config_access(0, 0);
	rcc_disable_osc(RCC_PLL);
	rcc_disable_osc(RCC_HSI16);
	pwr_set_vos(PWR_1_2V);
	rcc_disable_osc(RCC_HSI48);
	suspended = true;
}

static void resume(void)
{
	rcc_enable_osc(RCC_HSI48);
	pwr_set_vos(PWR_1_8V);
	rcc_enable_osc(RCC_HSI16);
	rcc_enable_osc(RCC_PLL);
	flash_config_access(1, FLASH_PREFETCH | FLASH_PREREAD);
	rcc_set_sysclk_source(RCC_PLL);
	rcc_disable_osc(RCC_MSI);
	usbdevfs_disable_suspend_mode();
	if (usbdevfs_get_frame_number() & 0x8000) {
		usbdevfs_enable_suspend_mode();
		usbdevfs_enable_low_power_mode();
		rcc_enable_osc(RCC_MSI);
		rcc_set_sysclk_source(RCC_MSI);
		flash_config_access(0, 0);
		rcc_disable_osc(RCC_PLL);
		rcc_disable_osc(RCC_HSI16);
		pwr_set_vos(PWR_1_2V);
		rcc_disable_osc(RCC_HSI48);
	} else {
		rcc_enable_clock(RCC_CRS);
		gpio_set_mode(GPIO_ALTFUNC, GPIO_PA(0, 1, 2, 3));
		gpio_set_mode(GPIO_OUTPUT, GPIO_PA(4, 5));
		rcc_enable_clock(RCC_USART2);
		rcc_enable_clock(RCC_TIM6);
		rcc_enable_clock(RCC_TIM21);
		rcc_enable_clock(RCC_TIM22);
		suspended = false;
	}
}

void usb_isr(void)
{
	int s;
	int ep_id;
	int t;

	s = usbdevfs_get_interrupt_status(USBDEVFS_CTR | USBDEVFS_WKUP |
					  USBDEVFS_SUSP | USBDEVFS_RESET |
					  USBDEVFS_DIR | USBDEVFS_EP_ID);
	if (s & USBDEVFS_CTR) {
		ep_id = s & USBDEVFS_EP_ID;
		if (ep_id == 0) {
			t = usbdevfs_get_ep_status(ep_id, USBDEVFS_RX |
						   USBDEVFS_SETUP |
						   USBDEVFS_TX);
			if (t & USBDEVFS_RX) {
				if (t & USBDEVFS_SETUP)
					control_setup();
				else
					control_rx();
			}
			if (t & USBDEVFS_TX)
				control_tx();
		} else if (ep_id == NOTIFICATION_INDEX) {
			notification_busy = false;
			usbdevfs_clear_ep_interrupt(ep_id);
		} else if (ep_id == DATA_RX_INDEX) {
			bulk_rx_busy = true;
			bulk_rx();
			if (data_len == 0 && command_len == 0 &&
			    endpoint_state[DATA_RX_INDEX] == EP_STATE_ENABLE &&
			    bulk_rx_busy) {
				bulk_rx_busy = false;
				usbdevfs_enable_ep_rx(DATA_RX_INDEX);
			}
			usbdevfs_clear_ep_interrupt(ep_id);
		} else if (ep_id == DATA_TX_INDEX) {
			bulk_tx_busy = false;
			usbdevfs_clear_ep_interrupt(ep_id);
		}
	}
	if (s & USBDEVFS_WKUP) {
		resume();
		usbdevfs_clear_interrupt(USBDEVFS_WKUP);
	}
	if (s & USBDEVFS_SUSP) {
		suspend();
		usbdevfs_clear_interrupt(USBDEVFS_SUSP);
	}
	if (s & USBDEVFS_RESET) {
		reset();
		usbdevfs_clear_interrupt(USBDEVFS_RESET);
	}
}

/* USART Rx -> Bulk Tx */
static void usart_rx(void)
{
	int i;
	alignas(2) static char buf[RXBUFSIZE];
	static int rx_rp;

	nvic_disable_irq(NVIC_USB);
	if (command_state == ONLINE_DATA_STATE &&
	    endpoint_state[DATA_TX_INDEX] == EP_STATE_ENABLE &&
	    !bulk_tx_busy && !suspended) {
		usart_disable_interrupt(USART2, USART_RXNE);
		for (i = 0; i < rx_len; i++) {
			buf[i] = rx_buf[rx_rp++] & ((1 << databits) - 1);
			rx_rp &= RXBUFSIZE - 1;
		}
		usbdevfs_write(DATA_TX_INDEX, buf, rx_len);
		rx_len = 0;
		bulk_tx_busy = true;
		usbdevfs_start_ep_tx(DATA_TX_INDEX);
		usart_enable_interrupt(USART2, USART_RXNE);
	}
	nvic_enable_irq(NVIC_USB);
}

/* Command mode: Bulk Rx -> Bulk Tx */
static void command_tx(void)
{
	nvic_disable_irq(NVIC_USB);
	if (endpoint_state[DATA_TX_INDEX] == EP_STATE_ENABLE &&
	    !bulk_tx_busy && !suspended) {
		usbdevfs_write(DATA_TX_INDEX, command_buf, command_len);
		command_len = 0;
		bulk_tx_busy = true;
		if (command_state == EXIT_STATE)
			command_state = ONLINE_DATA_STATE;
		usbdevfs_start_ep_tx(DATA_TX_INDEX);
	}
	if (endpoint_state[DATA_RX_INDEX] == EP_STATE_ENABLE && bulk_rx_busy) {
		bulk_rx_busy = false;
		usbdevfs_enable_ep_rx(DATA_RX_INDEX);
	}
	nvic_enable_irq(NVIC_USB);
}

/* Bulk Rx -> USART Tx */
static void usart_tx(void)
{
	static int tx_wp;

	nvic_disable_irq(NVIC_TIM21);
	nvic_disable_irq(NVIC_USB);
	usart_disable_interrupt(USART2, USART_TXE);
	while (tx_len < TXBUFSIZE && data_len) {
		tx_buf[tx_wp++] = data_buf[data_rp++];
		tx_wp &= TXBUFSIZE - 1;
		tx_len++;
		if (data_rp >= DATABUFSIZE)
			data_rp = 0;
		data_len--;
	}
	usart_enable_interrupt(USART2, USART_TXE);
	if (data_len == 0 &&
	    endpoint_state[DATA_RX_INDEX] == EP_STATE_ENABLE &&
	    bulk_rx_busy) {
		bulk_rx_busy = false;
		usbdevfs_enable_ep_rx(DATA_RX_INDEX);
	}
	nvic_enable_irq(NVIC_USB);
	nvic_enable_irq(NVIC_TIM21);
}

static void notify_tx(void)
{
	int len;
	char *p;

	nvic_disable_irq(NVIC_USART2);
	nvic_disable_irq(NVIC_USB);
	if (endpoint_state[NOTIFICATION_INDEX] == EP_STATE_ENABLE &&
	    !suspended) {
		len = notify(&p);
		usbdevfs_write(NOTIFICATION_INDEX, p, len);
		notification_busy = true;
		usbdevfs_start_ep_tx(NOTIFICATION_INDEX);
	}
	nvic_enable_irq(NVIC_USART2);
	nvic_enable_irq(NVIC_USB);
}

int main(void)
{
	int mcu;
	int unique[3];
	int offset;
	
	clock_setup();
	crs_setup();
	gpio_setup();
	usart_setup();
	tim_setup();
	usbdevfs_setup();

	mcu = dbg_get_mcu_id();
	misc_get_unique_id(unique);
	set_serial_number(mcu, unique);
	delay_us(USBDEVFS_T_STARTUP);
	usbdevfs_disable_usb_reset();
	usbdevfs_clear_interrupt(USBDEVFS_PMAOVR | USBDEVFS_ERR |
				 USBDEVFS_WKUP | USBDEVFS_SUSP |
				 USBDEVFS_RESET | USBDEVFS_SOF |
				 USBDEVFS_ESOF | USBDEVFS_L1REQ);
	usbdevfs_set_buffer_table_address(BUFFER_TABLE_ADDRESS);
	offset = usbdevfs_assign_packet_memory_tx(0, PACKET_MEMORY_START,
						  MAXPACKETSIZE0);
	offset = usbdevfs_assign_packet_memory_rx(0, offset, MAXPACKETSIZE0);
	offset = usbdevfs_assign_packet_memory_tx(NOTIFICATION_INDEX, offset,
						  NOTIFICATION_SIZE);
	offset = usbdevfs_assign_packet_memory_rx(DATA_RX_INDEX, offset,
						  DATA_SIZE);
	usbdevfs_assign_packet_memory_tx(DATA_TX_INDEX, offset, DATA_SIZE);
	usbdevfs_enable_dp_pull_up();
	usbdevfs_enable_interrupt(USBDEVFS_CTR | USBDEVFS_WKUP |
				  USBDEVFS_SUSP | USBDEVFS_RESET);

	while (1) {
		if (rx_len &&
		    command_state == ONLINE_DATA_STATE &&
		    endpoint_state[DATA_TX_INDEX] == EP_STATE_ENABLE &&
		    !bulk_tx_busy && !suspended) {
			usart_rx();
		}
		if (command_len &&
		    endpoint_state[DATA_TX_INDEX] == EP_STATE_ENABLE &&
		    !bulk_tx_busy && !suspended) {
			command_tx();
		}
		if (tx_len < TXBUFSIZE && data_len) {
			usart_tx();
		}
		if (notify_available() && !notification_busy &&
		    endpoint_state[NOTIFICATION_INDEX] == EP_STATE_ENABLE &&
		    !suspended) {
			notify_tx();
		}
		while (suspended)
			__asm__ ("wfi"); /* Sleep */
	}

	return 0;
}
