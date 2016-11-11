/*
 * ethernet_camera - A network still camera.
 *
 * ethernet_camera.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of ethernet_camera.
 *
 * ethernet_camera is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ethernet_camera is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ethernet_camera.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <usart.h>
#include <tim.h>
#include <i2c.h>
#include <spi.h>
#include <nvic.h>
#include <vector.h>
#include <exti.h>
#include <dma.h>

#include <syscall.h>

#include "w5200.h"
#include "ethernet_camera.h"

#define FCK			32000000 /* USART clock frequency */
#define TIMX_CLK_APB1		32000000 /* TIM6 clock frequency */

char netinfo[18] = {
	0, 0, 0, 0,		/* Gateway address */
	255, 255, 255, 0,	/* Subnet mask */
	0, 0, 0, 0, 0, 0,	/* MAC address */
	192, 168, 2, 2,		/* IP address */
};

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
 * PA3: input		WIZ820io nINT
 * PA4: SPI1_NSS	WIZ820io nSS
 * PA5: SPI1_SCK	WIZ820io SCLK
 * PA6: SPI1_MISO	WIZ820io MISO
 * PA7: SPI1_MOSI	WIZ820io MOSI
 * PA9: USART1_TX	Debug
 * PA14: USART2_TX	PTC08 RX
 * PA15: USART2_RX	PTC08 TX
 * PB0: output		WIZ820io nRESET
 * PB1: output		WIZ820io PWDN
 * PB5: output		AT24MAC402 WP
 * PB6: I2C1_SCL	AT24MAC402 SCL
 * PB7: I2C1_SDA	AT24MAC302 SDA
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_set(GPIO_PA(0, 1));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(0, 1));
	gpio_config(GPIO_INPUT, 0, GPIO_PA3);
	gpio_config(GPIO_AF0, GPIO_35MHZ, GPIO_PA(4, 5, 7));
	gpio_config(GPIO_AF0, GPIO_PULLUP, GPIO_PA6);
#ifdef DEBUG
	gpio_config(GPIO_AF4, 0, GPIO_PA9);
#endif
	gpio_config(GPIO_AF4, 0, GPIO_PA(14, 15));

	rcc_enable_clock(RCC_GPIOB);
	gpio_clear(GPIO_PB(0, 1));
	gpio_set(GPIO_PB5);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PB(0, 1, 5));
	gpio_config(GPIO_AF1, GPIO_OPENDRAIN | GPIO_2MHZ, GPIO_PB(6, 7));
}

/*
 * USART
 * (USART1: 115200 8N1 (debug))
 * USART2: 38400 8N1 (camera)
 */
static void usart_setup(void)
{
#ifdef DEBUG
	rcc_enable_clock(RCC_USART1);
	usart_init(USART1, FCK, 115200, 8, USART_STOP_1, USART_PARITY_NONE,
		   USART_FLOW_NONE, USART_TX);
#endif
	rcc_enable_clock(RCC_USART2);
	usart_init(USART2, FCK, 38400, 8, USART_STOP_1, USART_PARITY_NONE,
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

/* SPI1: SCK: 32 MHz / 2 = 16 MHz, DMA */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 2, 8, SPI_NSS_OUTPUT | SPI_DMA_TX_RX |
			SPI_ENABLE);
}

/* EXTI3: falling edge trigger */
static void exti_setup(void)
{
	nvic_enable_irq(NVIC_EXTI2_3);
	exti_set_trigger(EXTI3, EXTI_FALLING);
	exti_enable_interrupt(EXTI3);
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

#ifdef DEBUG
int _write(int file, char *ptr, int len)
{
	int i;

	if (file == 1) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n')
				usart_send_blocking(USART1, '\r');
			usart_send_blocking(USART1, ptr[i]);
		}
		return i;
	}

	errno = EIO;
	return -1;
}
#endif

/* Delay: 1 - 32768 ms */
void delay_ms(int ms)
{
	tim_set_autoreload(TIM6, ms << 1);
	tim_enable_counter(TIM6);
	while (!tim_get_interrupt_status(TIM6, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM6, TIM_UPDATE);
}

void exti2_3_isr(void)
{
	if (exti_get_interrupt_status(EXTI3)) {
		exti_clear_interrupt(EXTI3);
		w5200_isr();
	}
}

int main(void)
{
#ifdef DEBUG
	int i;
#endif
	
	clock_setup();
	gpio_setup();
	usart_setup();
	tim_setup();
	i2c_setup();
	exti_setup();
	dma_setup();

#ifdef DEBUG
	puts("ethernet_camera");
#endif
	w5200_enable();
	spi_setup();
	
	if (rom_read_eui48(&netinfo[8]))
		goto i2c_error;
#ifdef DEBUG
	printf("Gateway ");
	for (i = 0; i < 4; i++)
		printf("%u%c", (unsigned char)netinfo[i],
		       i == 3 ? '\n' : '.');
	printf("inet ");
	for (i = 0; i < 4; i++)
		printf("%u%c", (unsigned char)netinfo[14 + i],
		       i == 3 ? ' ' : '.');
	printf(" netmask ");
	for (i = 0; i < 4; i++)
		printf("%u%c", (unsigned char)netinfo[4 + i],
		       i == 3 ? '\n' : '.');
	printf("ether ");
	for (i = 0; i < 6; i++)
		printf("%02x%c", (unsigned char)netinfo[8 + i],
		       i == 5 ? '\n' : ':');
#endif

#ifdef DEBUG	
	puts("Initialize camera.");
#endif
	if (camera_init())
		goto camera_error;

	w5200_start();

i2c_error:
	while (1) {
		gpio_toggle(GPIO_PA1);
		delay_ms(250);
	}

camera_error:
	gpio_clear(GPIO_PA1);
	while (1)
		;

	return 0;
}
