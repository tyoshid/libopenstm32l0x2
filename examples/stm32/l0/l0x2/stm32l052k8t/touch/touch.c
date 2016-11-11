/*
 * touch - Use a touch sensor to blink an LED.
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

#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <usart.h>
#include <tsc.h>

#include <syscall.h>

#define FCK		32000000 /* USART clock frequency */
#define NCALIB		128
#define NSAMPLE		10

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
 * PA0: output		LED
 * PA2: TSC_G1_IO3
 * PA3: TSC_G1_IO4
 * PA9: USART1_TX
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA0);
	gpio_config(GPIO_AF3, GPIO_2MHZ, GPIO_PA(2, 3));
	gpio_config(GPIO_AF4, 0, GPIO_PA9);
}

/* USART1: 115200 8N1 */
static void usart_setup(void)
{
	rcc_enable_clock(RCC_USART1);
	usart_init(USART1, FCK, 115200, 8, USART_STOP_1, USART_PARITY_NONE,
		   USART_FLOW_NONE, USART_TX);
}

/*
 * TSC
 * PA2: CX
 * PA3: CS
 * 1 / 32 MHz * 2 * 16 = 1 us
 */
static void tsc_setup(void)
{
	rcc_enable_clock(RCC_TSC);
	tsc_enable_channel(TSC_G1, TSC_PA2, TSC_PA3);
	tsc_init(2, 16, 16, 16383, 0, 0, TSC_ENABLE);
}

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

static int get_count(void)
{
	int r;
	static int max;
	static int min = 0xffff;
	static int count;
	
	tsc_start();
	while (!(r = tsc_get_interrupt_status(TSC_EOA | TSC_MCE)))
		;
	if (r & TSC_MCE)
		puts("Max count error!");
	tsc_clear_interrupt(r);
	r = tsc_get_counter(TSC_G1);
	// printf("%d\n", r);
	if (r > max)
		max = r;
	if (r < min)
		min = r;
	if (count >= 0xffff) {
		printf("max = %d, min = %d\n", max, min);
		max = 0;
		min = 0xffff;
		count = 0;
	} else {
		count++;
	}
	return r;
}

int main(void)
{
	int i;
	int j;
	static int calib[NCALIB];
	double m;
	double s;
	double prev_m;
	double prev_s;
	double th;
	
	clock_setup();
	gpio_setup();
	usart_setup();
	tsc_setup();

	puts("touch");

	puts("Calibration");
	m = 0.;
	s = 0.;
	for (j = 0; j < 3; j++) {
		prev_m = m;
		prev_s = s;
		for (i = 0; i < NCALIB; i++)
			calib[i] = get_count();
		/* Mean value */
		m = 0.;
		for (i = 0; i < NCALIB; i++)
			m += calib[i];
		m /= NCALIB;
		/* Standard deviation */
		s = 0.;
		for (i = 0; i < NCALIB; i++)
			s += (calib[i] - m) * (calib[i] - m);
		s = sqrt(s / NCALIB);
		/* Consistency */
		if (m > prev_m + prev_s || m < prev_m - prev_s ||
		    prev_m > m + s || prev_m < m - s ||
		    s > m * 0.02) {
			j = 0;
			if (prev_m > 0.) {
				puts("Fail!");
			}
		}
		printf("m = %.1lf, s = %.1lf\n", m, s);
	}
	
	th = m * 0.9;
	printf("Mean value\t\t%8.1lf\n", m);
	printf("Standard deviation\t%8.1lf\n", s);
	printf("Threshold\t\t%8.1lf\n", th);
	
	gpio_set(GPIO_PA0);
	puts("Start");
	while (1) {
		m = 0.;
		for (i = 0; i < NSAMPLE; i++)
			m += get_count();
		m /= NSAMPLE;
		if (m < th)
			gpio_clear(GPIO_PA0);
		else
			gpio_set(GPIO_PA0);
		// printf("%lf\n", m);
	}
	
	return 0;
}
