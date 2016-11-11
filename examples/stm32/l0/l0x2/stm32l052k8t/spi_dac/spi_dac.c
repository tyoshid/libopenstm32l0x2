/*
 * spi_dac - Generate sine-wave.
 *
 * spi_dac.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of spi_dac.
 *
 * spi_dac is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * spi_dac is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with spi_dac.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <spi.h>
#include <tim.h>
#include <dma.h>

#include "spi_dac.h"

#define TIMX_CLK_APB1	32000000 /* TIM6 clock frequency */

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
 * PA0: TIM2_ETR
 * PA1: TIM2_IC2
 * PA2: TIM21_OC1	TIM2_IC1
 * PA3: TIM2_OC4	MCP4922 /CS
 * PA4: TIM22_ETR
 * PA5: SPI1_SCK	MCP4922 SCK, TIM22_ETR, TIM2_ETR
 * PA6: TIM22_OC1	TIM2_IC2
 * PA7: SPI1_MOSI	MCP4922 SDI
 * PA13: output		LED2
 * PA14: output		LED1
 * PA15: TIM2_IC1
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
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(13, 14));
	gpio_set(GPIO_PA(13, 14));
	gpio_config(GPIO_AF5, 0, GPIO_PA15);
}

/* SPI1: SCK: 32 MHz / 4 = 8 MHz */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 4, 16, SPI_NSS_OUTPUT | SPI_ENABLE);
}

/*
 * TIM
 * TIM6: CK_CNT = 2 kHz, one-pulse mode
 * TIM21:
 *   OC1: PWM mode 2 (TIMx_CNT < TIMx_CCR1: inactive)
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
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000);

	rcc_enable_clock(RCC_TIM21);
	tim_set_capture_compare_value(TIM21_CC1, 1);
	tim_set_capture_compare_mode(TIM21_CC1, TIM_CC_OUTPUT | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);

	rcc_enable_clock(RCC_TIM22);
	tim_set_slave_mode(TIM22, TIM_EXTERNAL_CLOCK_MODE1 | TIM_ETRF);
	tim_setup_counter(TIM22, 1, 32);
	tim_set_capture_compare_value(TIM22_CC1, 16);
	tim_set_capture_compare_mode(TIM22_CC1, TIM_CC_OUTPUT | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);

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

/* DMA CH2: TIM2_UP */
static void dma_setup(void)
{
	rcc_enable_clock(RCC_DMA);
	dma_set_request(DMA_CH2_TIM2_UP);
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
	int ch;
	int octave;
	int pitch;
	
	clock_setup();
	gpio_setup();
	spi_setup();
	tim_setup();
	dma_setup();

	dac_init();

	gpio_clear(GPIO_PA14);
	dac_tone(CHANNEL_A, PITCH_A, 4, 10);
	gpio_set(GPIO_PA14);
	delay_ms(1000);

	gpio_clear(GPIO_PA13);
	dac_tone(CHANNEL_B, PITCH_A, 4, 10);
	gpio_set(GPIO_PA13);
	delay_ms(1000);
	
	while (1) {
		for (ch = CHANNEL_A; ch <= CHANNEL_A_B; ch++) {
			for (octave = 3; octave <= 6; octave++) {
				for (pitch = PITCH_C; pitch <= PITCH_B;
				     pitch++) {
					if (ch == CHANNEL_A)
						gpio_toggle(GPIO_PA14);
					else if (ch == CHANNEL_B)
						gpio_toggle(GPIO_PA13);
					else
						gpio_toggle(GPIO_PA(14, 13));
					dac_tone(ch, pitch, octave, 3);
				}
			}
			delay_ms(1000);
		}
	}

	return 0;
}
