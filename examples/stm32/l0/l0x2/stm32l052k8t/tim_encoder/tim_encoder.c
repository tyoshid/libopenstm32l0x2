/*
 * tim_encoder - Use an encoder to fade three LEDs.
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
#include <nvic.h>
#include <vector.h>
#include <tim.h>
#include <exti.h>

#define TIMX_CLK_APB2	32000000 /* TIM21 and TIM22 clock frequency */
/*
 * SparkFun Rotary Encoder - Illuminated (RGB)
 * Top-up EC12PLRGBSDVBF-D-25K-24-24C-61
 */
#define MAXCOUNT	(24 * 4) /* Pulses per rotation */

volatile int led = 4;

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
 * PA0: TIM2_CH1	encoder
 * PA1: TIM2_CH2	encoder
 * PA3: TIM21_CH2	LED R
 * PA6: TIM22_CH1	LED G
 * PA7: TIM22_CH2	LED B
 * PA4: input		SW
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_AF2, 0, GPIO_PA(0, 1));
	gpio_config(GPIO_AF0, GPIO_OPENDRAIN, GPIO_PA3);
	gpio_config(GPIO_AF5, GPIO_OPENDRAIN, GPIO_PA(6, 7));
	gpio_config(GPIO_INPUT, 0, GPIO_PA4);
}

/*
 * TIM
 * TIM21: CK_CNT = 4.8 kHz, count = 96
 *   OC2: PWM mode 2
 * TIM22: CK_CNT = 4.8 kHz, count = 96
 *   OC1: PWM mode 2
 *   OC2: PWM mode 2
 * TIM2: Encoder mode 3
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM21);
	nvic_enable_irq(NVIC_TIM21);
	tim_setup_counter(TIM21, TIMX_CLK_APB2 / (MAXCOUNT * 50), MAXCOUNT);
	tim_set_capture_compare_mode(TIM21_CC2, TIM_OC_PRELOAD | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);
	tim_clear_interrupt(TIM21, TIM_UPDATE);
	tim_enable_interrupt(TIM21, TIM_UPDATE);
	tim_enable_counter(TIM21);

	rcc_enable_clock(RCC_TIM22);
	tim_setup_counter(TIM22, TIMX_CLK_APB2 / (MAXCOUNT * 50), MAXCOUNT);
	tim_set_capture_compare_mode(TIM22_CC1, TIM_OC_PRELOAD | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);
	tim_set_capture_compare_mode(TIM22_CC2, TIM_OC_PRELOAD | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);
	tim_enable_counter(TIM22);

	rcc_enable_clock(RCC_TIM2);
	tim_setup_counter(TIM2, 1, MAXCOUNT);
	tim_set_slave_mode(TIM2, TIM_ENCODER_MODE3);
	tim_set_capture_compare_mode(TIM2_CC1, TIM_CC_INPUT1);
	tim_set_capture_compare_mode(TIM2_CC2, TIM_CC_INPUT1);
	tim_enable_counter(TIM2);
}

/* EXTI4: rising edge trigger */
static void exti_setup(void)
{
	nvic_enable_irq(NVIC_EXTI4_15);
	exti_set_trigger(EXTI4, EXTI_RISING);
	exti_enable_interrupt(EXTI4);
}

/*
 * TIM21 interrupt
 *   Indicate the LED if button is pushed.
 *   Make the LED brighter or dimmer.
 */
void tim21_isr(void)
{
	int c;
	int d;
	static int count[3];

	if (tim_get_interrupt_status(TIM21, TIM_UPDATE)) {
		c = led;
		count[c & 3] = tim_get_counter(TIM2);

		d = c < 4 ? (c == 0 ? MAXCOUNT - 1 : 0) : count[0];
		tim_set_capture_compare_value(TIM21_CC2, d);

		d = c < 4 ? (c == 1 ? MAXCOUNT - 1 : 0) : count[1];
		tim_set_capture_compare_value(TIM22_CC1, d);

		d = c < 4 ? (c == 2 ? MAXCOUNT - 1 : 0) : count[2];
		tim_set_capture_compare_value(TIM22_CC2, d);

		tim_clear_interrupt(TIM21, TIM_UPDATE);
	}
}

void exti4_15_isr(void)
{
	if (exti_get_interrupt_status(EXTI4)) {
		switch (exti_get_trigger(EXTI4)) {
		case EXTI_RISING:
			led = (led & 3) >= 2 ? 0 : ((led & 3) + 1);
			exti_set_trigger(EXTI4, EXTI_FALLING);
			break;
		case EXTI_FALLING:
			led |= 4;
			exti_set_trigger(EXTI4, EXTI_RISING);
			break;
		default:
			break;
		}

		exti_clear_interrupt(EXTI4);
	}
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();
	exti_setup();

	while (1)
		;

	return 0;
}
