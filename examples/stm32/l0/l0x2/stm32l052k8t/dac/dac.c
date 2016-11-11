/*
 * dac - Generate sine-wave, noise, triangle-wave.
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
#include <math.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <nvic.h>
#include <vector.h>
#include <tim.h>
#include <dma.h>
#include <memorymap.h>
#include <dac.h>

#define TIMX_CLK_APB1	32000000 /* TIM6 clock frequency */
#define TIMX_CLK_APB2	32000000 /* TIM21 and TIM22 clock frequency */
#define MAXDATA		2048
/* Pitch */
enum {
	PITCH_C,
	PITCH_D,
	PITCH_E,
	PITCH_F,
	PITCH_G,
	PITCH_A,
	PITCH_B
};

short sine[MAXDATA];
int autoreload[7];
volatile int inc;
volatile bool stop;
short dma_data[MAXDATA / 2];

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
 * PA0: output	LED1
 * PA1: output	LED2
 * PA4: analog	DAC_OUT
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(0, 1));
	gpio_config(GPIO_ANALOG, 0, GPIO_PA4);
}

/*
 * TIM
 * TIM6:
 * TIM21: TRGO: Update
 * TIM22: CK_CNT = 2 MHz
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	nvic_enable_irq(NVIC_TIM6_DAC);

	rcc_enable_clock(RCC_TIM21);
	tim_set_master_mode(TIM21, TIM_TRGO_UPDATE);
	
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

/* DMA CH2: DAC CH1 */
static void dma_setup(void)
{
	rcc_enable_clock(RCC_DMA);
	dma_set_request(DMA_CH2_DAC_CH1);
}

static void dac_setup(void)
{
	rcc_enable_clock(RCC_DAC);
	dac_set_mode(DAC_CH1_ENABLE);
	delay_us(DAC_T_WAKEUP);
}

/* TIM6 interrupt */
void tim6_dac_isr(void)
{
	static int di;
	
	if (tim_get_interrupt_mask(TIM6, TIM_UPDATE) &&
	    tim_get_interrupt_status(TIM6, TIM_UPDATE)) {
		tim_clear_interrupt(TIM6, TIM_UPDATE);
		dac_set_data(DAC_12R, DAC_CH1, sine[di]);
		di += inc;
		if (di >= MAXDATA) {
			di = 0;
			if (stop) {
				tim_disable_counter(TIM6);
				stop = false;
			}
		}
	}
}

static void tone_sin_int(int pitch, int octave, int duration)
{
	int i;
	
	inc = 1 << (pitch >= 2 ? octave : octave - 1);
	tim_setup_counter(TIM6, 1, autoreload[pitch]);
	tim_clear_interrupt(TIM6, TIM_UPDATE);
	tim_enable_interrupt(TIM6, TIM_UPDATE);
	tim_enable_counter(TIM6);
	for (i = 0; i < 100 * duration; i++)
		delay_us(10000);
	stop = true;
	while (stop)
		;
	tim_disable_interrupt(TIM6, TIM_UPDATE);
}

static void dac_dma_enable(int s)
{
	int i;

	if (s == MAXDATA) {
		dma_setup_channel(DMA_CH2, (int)sine, DAC_DHR12R1, s,
				  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC |
				  DMA_M_TO_P | DMA_CIRCULAR | DMA_ENABLE);
	} else {
		for (i = 0; i < s; i++)
			dma_data[i] = sine[MAXDATA / s * i];
		dma_setup_channel(DMA_CH2, (int)dma_data, DAC_DHR12R1, s,
				  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC |
				  DMA_M_TO_P | DMA_CIRCULAR | DMA_ENABLE);
	}
}
	
static void tone_sin_dma(int pitch, int octave, int duration)
{
	int i;

	dac_dma_enable(MAXDATA >> (pitch >= 2 ? octave : (octave - 1)));
	tim_set_master_mode(TIM6, TIM_TRGO_UPDATE);
	tim_setup_counter(TIM6, 1, autoreload[pitch]);
	dac_set_mode(DAC_CH1_TIM6_TRGO | DAC_CH1_DMA | DAC_CH1_ENABLE);
	tim_enable_counter(TIM6);
	for (i = 0; i < 100 * duration; i++)
		delay_us(10000);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	tim_disable_counter(TIM6);
	dac_set_mode(DAC_CH1_ENABLE);
	tim_set_master_mode(TIM6, TIM_TRGO_RESET);
	dma_disable(DMA_CH2);
}
		
static void tone_noise(int fs, int mamp, int duration)
{
	int i;
	
	tim_setup_counter(TIM21, 1, TIMX_CLK_APB2 / fs);
	dac_set_mode(DAC_CH1_NOISE(mamp) | DAC_CH1_TIM21_TRGO |
		     DAC_CH1_ENABLE);
	dac_set_data(DAC_12R, DAC_CH1, 0);
	tim_enable_counter(TIM21);
	for (i = 0; i < 100 * duration; i++)
		delay_us(10000);
	tim_disable_counter(TIM21);
	dac_set_mode(DAC_CH1_ENABLE);
}

static void tone_triangle(int fout, int mamp, int duration)
{
	int i;
	
	tim_setup_counter(TIM21, 1,
			  (TIMX_CLK_APB2 + (fout * (1 << (mamp + 1)))) /
			  (fout * (1 << (mamp + 2))));
	dac_set_mode(DAC_CH1_TRIANGLE(mamp) | DAC_CH1_TIM21_TRGO |
		     DAC_CH1_ENABLE);
	dac_set_data(DAC_12R, DAC_CH1, 0);
	tim_enable_counter(TIM21);
	for (i = 0; i < 100 * duration; i++)
		delay_us(10000);
	tim_disable_counter(TIM21);
	dac_set_mode(DAC_CH1_ENABLE);
}

/*
 * Sample rate: 40 kHz < fs < 80 kHz
 *	(MAXDATA = 2048)
 *	autoreload[0] - 67 kHz
 *	autoreload[1] - 75 kHz
 *	autoreload[2] - 42 kHz
 *	autoreload[3] - 45 kHz
 *	autoreload[4] - 50 kHz
 *	autoreload[5] - 56 kHz
 *	autoreload[6] - 63 kHz
 */
int main(void)
{
	int i;
	int pitch;
	int octave;
	
	clock_setup();
	gpio_setup();
	tim_setup();
	dma_setup();
	dac_setup();

	for (i = 0; i < MAXDATA; i++)
		sine[i] = round(sin(2. * M_PI / MAXDATA * i) * 2047. + 2048.);

	autoreload[0] = round(TIMX_CLK_APB1 / 32.703196 / MAXDATA);
	autoreload[1] = round(TIMX_CLK_APB1 / 36.708096 / MAXDATA);
	autoreload[2] = round(TIMX_CLK_APB1 / 41.203445 / (MAXDATA / 2.));
	autoreload[3] = round(TIMX_CLK_APB1 / 43.653529 / (MAXDATA / 2.));
	autoreload[4] = round(TIMX_CLK_APB1 / 48.999429 / (MAXDATA / 2.));
	autoreload[5] = round(TIMX_CLK_APB1 / 55.000000 / (MAXDATA / 2.));
	autoreload[6] = round(TIMX_CLK_APB1 / 61.735413 / (MAXDATA / 2.));

	tone_sin_int(PITCH_A, 4, 10);
	for (i = 0; i < 100; i++)
		delay_us(10000);

	while (1) {
		for (octave = 1; octave <= 9; octave++) {
			for (pitch = PITCH_C; pitch <= PITCH_B; pitch++) {
				gpio_toggle(GPIO_PA0);
				tone_sin_int(pitch, octave, 3);
			}
		}
		for (i = 0; i < 100; i++)
			delay_us(10000);

		gpio_toggle(GPIO_PA1);
		for (octave = 1; octave <= 9; octave++) {
			for (pitch = PITCH_C; pitch <= PITCH_B; pitch++) {
				gpio_toggle(GPIO_PA0);
				tone_sin_dma(pitch, octave, 3);
			}
		}
		for (i = 0; i < 100; i++)
			delay_us(10000);

		gpio_toggle(GPIO_PA(0, 1));
		for (i = 11; i >= 0; i--) {
			gpio_toggle(GPIO_PA1);
			tone_noise(40000, i, 3);
		}
		
		gpio_toggle(GPIO_PA0);
		for (i = 11; i >= 0; i--) {
			gpio_toggle(GPIO_PA1);
			tone_triangle(440, i, 3);
		}
	}

	return 0;
}
