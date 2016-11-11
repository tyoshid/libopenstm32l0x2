/*
 * comp - Control an LED with a potentiometer.
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <tim.h>
#include <rng.h>
#include <dac.h>
#include <comp.h>
#include <nvic.h>
#include <vector.h>
#include <exti.h>

#define TIMX_CLK_APB2	32000000 /* TIM22 clock frequency */
/*
 * DAC settling time: tSETTLING.max = 12 us
 * COMP2 propagation delay: td_fast.max = 4 us
 */
#define T_SETTLING	20	/* 20 us */

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
 * PA0: COM1_OUT	LED1
 * (PA1: analog		COMP1_INP)
 * PA2: output		LED2
 * (PA3: analog		COMP2_INP)
 * PA4: analog		DAC_OUT
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_AF7, 0, GPIO_PA0);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA2);
	gpio_config(GPIO_ANALOG, 0, GPIO_PA4);
}

/* TIM22: CK_CNT = 2 MHz */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM22);
	tim_enable_one_pulse_mode(TIM22);
	tim_disable_update_interrupt_on_any(TIM22);
	tim_load_prescaler(TIM22, TIMX_CLK_APB2 / 2000000);
}

/* Delay: 1 - 32768 us */
static void delay_us(int us)
{
	tim_set_autoreload(TIM22, us << 1);
	tim_enable_counter(TIM22);
	while (!tim_get_interrupt_status(TIM22, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM22, TIM_UPDATE);
}

static void rng_setup(void)
{
	rcc_enable_clock(RCC_RNG);
	rng_enable();
}

static void dac_setup(void)
{
	rcc_enable_clock(RCC_DAC);
	dac_set_mode(DAC_CH1_ENABLE);
	delay_us(DAC_T_WAKEUP);
	dac_set_data(DAC_12R, DAC_CH1, rng_get_data());
}

/*
 * COMP
 * COMP1: input+: PA1, input-: DAC_OUT1, output: LED1
 * COMP2: input+: PA3, input-: DAC_OUT1, output: EXTI22
 */
static void comp_setup(void)
{
	rcc_enable_clock(RCC_SYSCFG);
	comp_init(COMP1, COMP_PA1, COMP_DAC_OUT1, COMP_ENABLE);
	comp_init(COMP2, COMP_PA3, COMP_DAC_OUT1, COMP_FAST | COMP_ENABLE);
	delay_us(COMP2_T_START_FAST);
}

static void exti_setup(void)
{
	delay_us(T_SETTLING);
	nvic_enable_irq(NVIC_ADC_COMP);
	exti_set_trigger(EXTI_COMP2_OUTPUT, EXTI_BOTH);
	exti_enable_interrupt(EXTI_COMP2_OUTPUT);
}

/* COMP2 interrupt: blink LED2. */
void adc_comp_isr()
{
	if (exti_get_interrupt_status(EXTI_COMP2_OUTPUT)) {
		dac_set_data(DAC_12R, DAC_CH1, rng_get_data());
		gpio_toggle(GPIO_PA2);
		delay_us(T_SETTLING);
		exti_clear_interrupt(EXTI_COMP2_OUTPUT);
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	rng_setup();
	dac_setup();
	comp_setup();
	exti_setup();
	
	while (1)
		;

	return 0;
}
