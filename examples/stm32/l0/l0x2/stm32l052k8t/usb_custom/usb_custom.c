/*
 * usb_custom - Make USB custom device requests.
 *
 * usb_custom.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_custom.
 *
 * usb_custom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_custom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_custom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdalign.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <syscfg.h>
#include <crs.h>
#include <gpio.h>
#include <tim.h>
#include <usbdevfs.h>

#include "usb_custom.h"

#define TIMX_CLK_APB1		32000000 /* TIM6 clock frequency */
#define BUFFER_TABLE_ADDRESS	0
#define PACKET_MEMORY_START	(BUFFER_TABLE_ADDRESS + \
				 USBDEVFS_BUFFER_TABLE_SIZE)

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
 * PA0: output	LED1
 * PA1: output	LED2
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(0, 1));
}

/* TIM6: CK_CNT = 2 MHz, one-pulse mode */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000000);
}

/* USB: power-up */
static void usbdevfs_setup(void)
{
	rcc_set_usb_rng_clock(RCC_HSI48);
	rcc_enable_clock(RCC_USB);
	usbdevfs_disable_power_down();
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

int main(void)
{
	int offset;
	int s;
	int ep_id;
	int t;
	int len;
	alignas(2) static char buf[MAXPACKETSIZE0];

	clock_setup();
	crs_setup();
	gpio_setup();
	tim_setup();
	usbdevfs_setup();

	delay_us(USBDEVFS_T_STARTUP);
	usbdevfs_disable_usb_reset();
	usbdevfs_clear_interrupt(USBDEVFS_PMAOVR | USBDEVFS_ERR |
				 USBDEVFS_WKUP | USBDEVFS_SUSP |
				 USBDEVFS_RESET | USBDEVFS_SOF |
				 USBDEVFS_ESOF | USBDEVFS_L1REQ);
	usbdevfs_set_buffer_table_address(BUFFER_TABLE_ADDRESS);
	offset = usbdevfs_assign_packet_memory_tx(0, PACKET_MEMORY_START,
						  MAXPACKETSIZE0);
	usbdevfs_assign_packet_memory_rx(0, offset, MAXPACKETSIZE0);
	usbdevfs_enable_dp_pull_up();
	
	while (1) {
		s = usbdevfs_get_interrupt_status(USBDEVFS_CTR |
						  USBDEVFS_RESET |
						  USBDEVFS_DIR |
						  USBDEVFS_EP_ID);
		if (s & USBDEVFS_CTR) {
			ep_id = s & USBDEVFS_EP_ID;
			t = usbdevfs_get_ep_status(ep_id, USBDEVFS_RX |
						   USBDEVFS_SETUP |
						   USBDEVFS_TX);
			if (t & USBDEVFS_RX) {
				len = usbdevfs_read(ep_id, buf, sizeof(buf));
				if (t & USBDEVFS_SETUP)
					control_setup(buf, len);
				else
					control_rx();
			}
			if (t & USBDEVFS_TX)
				control_tx();
		}
		if (s & USBDEVFS_RESET) {
			usbdevfs_setup_endpoint(0, USBDEVFS_CONTROL, 0);
			usbdevfs_set_device_address(0);
			control_reset();
			usbdevfs_clear_interrupt(USBDEVFS_RESET);
		}
	}

	return 0;
}
