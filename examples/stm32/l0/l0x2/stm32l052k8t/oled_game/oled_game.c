/*
 * oled_game - A retro graphical game.
 *
 * oled_game.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of oled_game.
 *
 * oled_game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * oled_game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with oled_game.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <tim.h>
#include <spi.h>
#include <rng.h>
#include <nvic.h>
#include <vector.h>
#include <dma.h>
#include <dac.h>

#include "oled_game.h"

#define TIMX_CLK_APB1	32000000 /* TIM6 clock frequency */
#define TIMX_CLK_APB2	32000000 /* TIM21 clock frequency */
#define MAXCOUNT	(24 * 2) /* Rotary encoder */

/*
 * Clock
 * PLL: 16 MHz * 6 / 3 = 32 MHz
 * SYSCLK	32 MHz
 * HCLK		32 MHz (AHB)
 * PCLK1	32 MHz (APB1)
 * PCLK2	32 MHz (APB2)
 * PLL VCO / 2 = 48 MHz -> RNG
 */
static void clock_setup(void)
{
	rcc_enable_clock(RCC_PWR);
	pwr_set_vos(PWR_1_8V);

	rcc_enable_osc(RCC_HSI16);
	rcc_setup_pll(RCC_HSI16, 6, 3);
	rcc_enable_osc(RCC_PLL);
	flash_config_access(1, FLASH_PREFETCH | FLASH_PREREAD);
	rcc_set_sysclk_source(RCC_PLL);
	rcc_disable_osc(RCC_MSI);
}

/*
 * GPIO
 * PA2: output		OLED D/C#
 * PA3: output		OLED RES#
 * PA4: analog		DAC CH1
 * PA5: SPI1_SCK	OLED SCK
 * PA6: output		OLED CS#
 * PA7: SPI1_MOSI	OLED MOSI
 * PB4: TIM22_CH1	Rotary encoder
 * PB5: TIM22_CH2	Rotary encoder
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_set(GPIO_PA6);
	gpio_config(GPIO_OUTPUT, GPIO_10MHZ, GPIO_PA(2, 3, 6));
	gpio_config(GPIO_ANALOG, 0, GPIO_PA4);
	gpio_config(GPIO_AF0, GPIO_10MHZ, GPIO_PA(5, 7));

	rcc_enable_clock(RCC_GPIOB);
	gpio_config(GPIO_AF4, 0, GPIO_PB(4, 5));
}

/*
 * TIM
 * TIM21: DAC CH1 trigger: TRGO = (FSOUND * WAVEDATA) Hz
 * TIM22: Rotary encoder
 * TIM6: delay_us(): CK_CNT = 2 MHz, one-pulse mode
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM21);
	tim_set_master_mode(TIM21, TIM_TRGO_UPDATE);
	tim_set_autoreload(TIM21, TIMX_CLK_APB2 / FSOUND / WAVEDATA);

	rcc_enable_clock(RCC_TIM22);
	tim_set_capture_compare_mode(TIM22_CC1, TIM_CC_INPUT1 |
				     TIM_IC_NONINVERTED);
	tim_set_capture_compare_mode(TIM22_CC2, TIM_CC_INPUT1 |
				     TIM_IC_NONINVERTED);
	tim_set_slave_mode(TIM22, TIM_ENCODER_MODE2);
	tim_set_autoreload(TIM22, MAXCOUNT);
	tim_set_counter(TIM22, MAXCOUNT / 2);
	tim_enable_counter(TIM22);
	
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000000);
}

/* SPI1: SCK: 32 MHz / 4 = 8 MHz (SSD1306 tcycle.min = 100 ns (10 MHz)) */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 4, 8, SPI_NSS_OUTPUT | SPI_CPOL | SPI_CPHA |
			SPI_ENABLE);
}

static void rng_setup(void)
{
	rcc_enable_clock(RCC_RNG);
	rng_enable();
}

/* DMA CH2: DAC CH1, interrupt */
static void dma_setup(void)
{
	rcc_enable_clock(RCC_DMA);
	nvic_enable_irq(NVIC_DMA_CH2_3);
	dma_set_request(DMA_CH2_DAC_CH1);
}

/* Delay: 1 - 32768 us */
void delay_us(int us)
{
	tim_set_autoreload(TIM6, us << 1);
	tim_enable_counter(TIM6);
	while (!tim_get_interrupt_status(TIM6, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM6, TIM_UPDATE);
}

/* DAC CH1: Trigger: TIM21 TRGO, DMA */
static void dac_setup(void)
{
	rcc_enable_clock(RCC_DAC);
	dac_set_mode(DAC_CH1_TIM21_TRGO | DAC_CH1_DMA | DAC_CH1_ENABLE);
	delay_us(DAC_T_WAKEUP);
}

/*
 * DMA CH2 interrupt
 * Frequency: REFRATE Hz
 *   sound_restart() - restart DMA
 *   game() - move racket and ball
 *   sound_make_data() - new sound data
 */
void dma_ch2_3_isr(void)
{
	if (dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE)) {
		dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);

		sound_restart();
		game();
		sound_make_data();
	}
}

/*
 *   game_init() - enable OLED
 *   sound_init() - start DMA
 */
int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	spi_setup();
	rng_setup();
	dma_setup();
	dac_setup();

	game_init();
	sound_init();

	while (1)
		;
	
	return 0;
}
