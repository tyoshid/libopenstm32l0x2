/*
 * spi_sd - Read microSD card (FAT32 format) information.
 *
 * spi_sd.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of spi_sd.
 *
 * spi_sd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * spi_sd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with spi_sd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <nvic.h>
#include <vector.h>
#include <usart.h>
#include <tim.h>
#include <dma.h>
#include <spi.h>
#include <crc.h>

#include <syscall.h>

#include "spi_sd.h"

#define FCK				32000000 /* USART clock frequency */
#define TIMX_CLK_APB1			32000000 /* TIM6 clock frequency */
#define TXBUFSIZE			32 /* Tx buffer size (power of 2) */
#define RXBUFSIZE			32 /* Rx buffer size (power of 2) */

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
}

/*
 * GPIO
 * PA0: output		LED1
 * PA1: output		LED2
 * PA5: SPI1_SCK	SCLK
 * PA6: SPI1_MISO	DO
 * PA7: SPI1_MOSI	DI
 * PA9: USART1_TX
 * PA10: USART1_RX
 * PB0: output		CS
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_set(GPIO_PA(0, 1));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(0, 1));
	gpio_config(GPIO_AF0, GPIO_35MHZ, GPIO_PA(5, 7));
	gpio_config(GPIO_AF0, GPIO_PULLUP, GPIO_PA6);
	gpio_config(GPIO_AF4, 0, GPIO_PA(9, 10));

	rcc_enable_clock(RCC_GPIOB);
	gpio_set(GPIO_PB0);
	gpio_config(GPIO_OUTPUT, GPIO_35MHZ, GPIO_PB0);
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

/* TIM6: CK_CNT = 2 kHz, one-pulse mode */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000);
}

/*
 * DMA
 * CH2: SPI1_RX
 * CH3: SPI1_TX
 */
static void dma_setup(void)
{
	rcc_enable_clock(RCC_DMA);
	dma_set_request(DMA_CH2_SPI1_RX | DMA_CH3_SPI1_TX);
}

/* SPI1: SCK: 32 MHz / 128 = 250 kHz */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 128, 8, SPI_NSS_OUTPUT | SPI_ENABLE);
}

/* CRC: CRC-7 */
static void crc_setup(void)
{
	rcc_enable_clock(RCC_CRC);
	crc_set_mode(0x09, CRC_POLYSIZE_7);
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

/* Delay: 1 - 32768 ms */
void delay_ms(int ms)
{
	tim_set_autoreload(TIM6, ms << 1);
	tim_enable_counter(TIM6);
	while (!tim_get_interrupt_status(TIM6, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM6, TIM_UPDATE);
}

int main(void)
{
	clock_setup();
	gpio_setup();
	usart_setup();
	tim_setup();
	dma_setup();
	spi_setup();
	crc_setup();

	command();
	
	while (1) {
		gpio_toggle(GPIO_PA1);
		delay_ms(500);
	}

	return 0;
}
