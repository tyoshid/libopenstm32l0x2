/*
 * ultrasonic_sensor - Use the HC-SR04 ultrasonic ranging sensor.
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
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

#define TIMX_CLK_APB1	32000000 /* TIM2 and TIM6 clock frequency */
#define TIMX_CLK_APB2	32000000 /* TIM21 and TIM22 clock frequency */

volatile int count;

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
 *	   A
 *	   -
 *	F| G |B
 *	   -
 *	E| D |C
 *	   -   .DP
 */
/*
 * GPIO
 * PA0: output		B
 * PA1: output		F
 * PA2: output		A
 * PA3: output		E
 * PA4: output		D
 * PA5: output		DP
 * PA6: output		C
 * PA7: output		G
 * PB0: output		DIG. 4
 * PB3: TIM2_CH2	HC-SR04 Echo
 * PB4: TIM22_CH1	HC-SR04 Trig
 * PB5: output		DIG. 3
 * PB6: output		DIG. 2
 * PB7: output		DIG. 1
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_set(GPIO_PA(0, 1, 2, 3, 4, 5, 6, 7));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(0, 1, 2, 3, 4, 5, 6, 7));
	
	rcc_enable_clock(RCC_GPIOB);
	gpio_set(GPIO_PB(0, 5, 6, 7));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PB(0, 5, 6, 7));
	gpio_config(GPIO_AF2, 0, GPIO_PB3);
	gpio_config(GPIO_AF4, 0, GPIO_PB4);
}

/*
 * TIM
 * TIM6: CK_CNT = 1 MHz, frequency = 200 Hz
 * TIM21: CK_CNT = 2 kHz
 * TIM22: frequency = 100 kHz (10 us), one-pulse mode
 *   OC1: PWM mode 2, compare = 1
 * TIM2: CK_CNT = 2 MHz, reset mode - TI2FP2
 *   IC1: TI2FP1 falling edge
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	nvic_enable_irq(NVIC_TIM6_DAC);
	tim_setup_counter(TIM6, TIMX_CLK_APB1 / 1000000, 1000000 / 200);
	tim_clear_interrupt(TIM6, TIM_UPDATE);
	tim_enable_interrupt(TIM6, TIM_UPDATE);
	tim_enable_counter(TIM6);

	rcc_enable_clock(RCC_TIM21);
	tim_enable_one_pulse_mode(TIM21);
	tim_disable_update_interrupt_on_any(TIM21);
	tim_load_prescaler(TIM21, TIMX_CLK_APB2 / 2000);

	rcc_enable_clock(RCC_TIM22);
	tim_enable_one_pulse_mode(TIM22);
	tim_setup_counter(TIM22, 1, TIMX_CLK_APB2 / 100000);
	tim_set_capture_compare_value(TIM22_CC1, 1);
	tim_set_capture_compare_mode(TIM22_CC1, TIM_CC_OUTPUT | TIM_OC_PWM2 |
				     TIM_CC_ENABLE);

	rcc_enable_clock(RCC_TIM2);
	nvic_enable_irq(NVIC_TIM2);
	tim_setup_counter(TIM2, TIMX_CLK_APB1 / 2000000, 0xffff);
	tim_set_slave_mode(TIM2, TIM_TRGI_RESET | TIM_TI2FP2);
	tim_set_capture_compare_mode(TIM2_CC1, TIM_CC_INPUT2 |
				     TIM_IC_FALLING |TIM_CC_ENABLE);
	tim_enable_interrupt(TIM2, TIM_CC1);
	tim_enable_counter(TIM2);
}

void tim6_dac_isr(void)
{
	static int dig;
	int n;
	int i;
	static const int num[] = {0xa0, 0xbe, 0x62, 0x2a, 0x3c,
				  0x29, 0x21, 0xba, 0x20, 0x28};
	static const int gpio[] = {GPIO_PB7, GPIO_PB6, GPIO_PB5, GPIO_PB0};
	
	if (tim_get_interrupt_status(TIM6, TIM_UPDATE)) {
		gpio_set(gpio[0] | gpio[1] | gpio[2] | gpio[3]);
		n = count;
		for (i = 0; i < 3 - dig; i++)
			n /= 10;
		if (n > 0 || dig == 3) {
			n %= 10;
			gpio_write_port(GPIOA, num[n]);
			gpio_clear(gpio[dig]);
		}
		dig = dig < 3 ? dig + 1 : 0;
		tim_clear_interrupt(TIM6, TIM_UPDATE);
	}
}

void tim2_isr(void)
{
	int n;
	
	if (tim_get_interrupt_status(TIM2, TIM_CC1)) {
		n = tim_get_capture_compare_value(TIM2_CC1);
		count = (n + 58) / (58 * 2);
		tim_clear_interrupt(TIM2, TIM_CC1);
	}
}

/* Delay: 1 - 32768 ms */
static void delay_ms(int ms)
{
	tim_set_autoreload(TIM21, ms << 1);
	tim_enable_counter(TIM21);
	while (!tim_get_interrupt_status(TIM21, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM21, TIM_UPDATE);
}

int main(void)
{
	clock_setup();
	gpio_setup();
	tim_setup();

	while (1) {
		delay_ms(1000);
		count = 0;
		tim_enable_counter(TIM22);
	}

	return 0;
}
