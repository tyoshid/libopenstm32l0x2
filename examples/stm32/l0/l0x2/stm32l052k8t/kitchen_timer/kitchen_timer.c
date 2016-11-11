/*
 * kitchen_timer - A kitchen timer
 *
 * kitchen_timer.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of kitchen_timer.
 *
 * kitchen_timer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * kitchen_timer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with kitchen_timer.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <gpio.h>
#include <stk.h>
#include <vector.h>
#include <spi.h>
#include <tim.h>
#include <syscfg.h>
#include <comp.h>

#include "kitchen_timer.h"

#define HCLK			65536 /* AHB clock frequency */
#define TIMX_CLK_APB1		65536 /* TIM2 and TIM6 clock frequency */

volatile unsigned int jiffies;
volatile int button;

/*
 * Clock
 * SYSCLK	65.536 kHz
 * HCLK		65.536 kHz (AHB)
 * PCLK1	65.536 kHz (APB1)
 * PCLK2	65.536 kHz (APB2)
 */
static void clock_setup(void)
{
	rcc_set_msi_range(RCC_MSI_65KHZ);
	rcc_enable_clock(RCC_PWR);
	pwr_set_vos(PWR_1_2V);
}

/*
 * GPIO
 * PA0: TIM2_CH1	Piezoelectric sounder
 * PA1: output		AQM1248 /CS
 * PA2: TIM2_CH3	Piezoelectric sounder
 * (PA3: analog		COMP2 input+)
 * PA4: output		AQM1248 RS
 * PA5: SPI1_SCK	AQM1248 SCLK
 * PA6: output		AQM1248 /RESET
 * PA7: SPI1_MOSI	AQM1248 SDI
 * PB3: input		SW R
 * PB4: input		SW D
 * PB5: input		SW L
 * PB6: input		SW C
 * PB7: input		SW U
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_AF2, 0, GPIO_PA(0, 2));
	gpio_set(GPIO_PA(1, 6));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(1, 4, 6));
	gpio_config(GPIO_AF0, 0, GPIO_PA(5, 7));

	rcc_enable_clock(RCC_GPIOB);
	gpio_config(GPIO_INPUT, 0, GPIO_PB(3, 4, 5, 6, 7));
}

/* STK: interrupt: HZ Hz */
static void stk_setup(void)
{
	stk_init((HCLK * 2 / 8 + HZ) / (HZ * 2), STK_INT);
}

/* SPI1: SCK: 32.768 kHz */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 2, 8, SPI_NSS_OUTPUT | SPI_CPOL | SPI_CPHA |
			SPI_ENABLE);
}

/*
 * TIM
 * TIM2: CK_CNT = FALARM Hz
 *   OC1: PWM mode 1, duty 50%
 *   OC3: PWM mode 2, duty 50%
 * TIM6: CK_CNT = 2 kHz, one-pulse mode
 */
static void tim_setup(void)
{
	int c;
	
	rcc_enable_clock(RCC_TIM2);
	c = TIMX_CLK_APB1 / FALARM;
	tim_setup_counter(TIM2, 1, c);
	tim_set_capture_compare_mode(TIM2_CC1, TIM_OC_PWM1 | TIM_CC_ENABLE);
	tim_set_capture_compare_value(TIM2_CC1, c / 2);
	tim_set_capture_compare_mode(TIM2_CC3, TIM_OC_PWM2 | TIM_CC_ENABLE);
	tim_set_capture_compare_value(TIM2_CC3, c / 2);

	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000);
}

/* COMP2: input+: PA3, input-: VREFINT, slow speed */
static void comp_setup(void)
{
	rcc_enable_clock(RCC_SYSCFG);
	syscfg_enable_vrefint(SYSCFG_VREFINT_COMP);
	comp_init(COMP2, COMP_PA3, COMP_VREFINT, COMP_ENABLE);
}

void systick_isr(void)
{
	int i;
	static const struct {
		int port;
		int flag;
	} b[5] = {
		{GPIO_PB3, RIGHT},
		{GPIO_PB4, DOWN},
		{GPIO_PB5, LEFT},
		{GPIO_PB6, CENTER},
		{GPIO_PB7, UP}
	};
	static int low[5];
	static int high[5];
	static bool push[5];

	stk_countflag();
	jiffies++;
	for (i = 0; i < 5; i++) {
		if (gpio_get(b[i].port)) {
			if (high[i] < MAXBCOUNT) {
				high[i]++;
				if (high[i] >= MAXBCOUNT && push[i])
					push[i] = false;
			}
			low[i] = 0;
		} else {
			if (low[i] < MAXBCOUNT) {
				low[i]++;
				if (low[i] >= MAXBCOUNT && !push[i]) {
					push[i] = true;
					button |= b[i].flag;
				}
			}
			high[i] = 0;
		}
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

int main(void)
{
	clock_setup();
	gpio_setup();
	stk_setup();
	spi_setup();
	tim_setup();
	comp_setup();

	lcd_init();
	timer_init();
	stk_enable_counter();
	
	timer();

	return 0;
}
