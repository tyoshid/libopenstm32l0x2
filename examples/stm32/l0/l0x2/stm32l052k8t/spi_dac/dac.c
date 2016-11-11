/*
 * dac.c
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
#include <math.h>

#include <spi.h>
#include <tim.h>
#include <dma.h>
#include <memorymap.h>

#include "spi_dac.h"

/*
 * Microchip MCP4922
 * 12-Bit Dual Voltage Output Digital-to-Analog Converter with SPI Interface
 */
#define DAC_A		0x3000
#define DAC_B		0xb000
#define MAXDATA		512

static int sine[MAXDATA];
static int autoreload[7];
static int dma_data[MAXDATA * 2];

void dac_init(void)
{
	int i;
	
	for (i = 0; i < MAXDATA; i++)
		sine[i] = round(sin(2. * M_PI / MAXDATA * i) * 2047. + 2048.);

	autoreload[0] = round(TIMX_CLK_APB2 / 130.812783 / MAXDATA);
	autoreload[1] = round(TIMX_CLK_APB2 / 146.832384 / MAXDATA);
	autoreload[2] = round(TIMX_CLK_APB2 / 164.813778 / MAXDATA);
	autoreload[3] = round(TIMX_CLK_APB2 / 174.614116 / MAXDATA);
	autoreload[4] = round(TIMX_CLK_APB2 / 195.997718 / MAXDATA);
	autoreload[5] = round(TIMX_CLK_APB2 / 220.000000 / MAXDATA);
	autoreload[6] = round(TIMX_CLK_APB2 / 246.941651 / MAXDATA);
}

static void dac_dma_enable(int ch, int s)
{
	int i;
	
	for (i = 0; i < s; i++) {
		if (ch == CHANNEL_A) {
			dma_data[i] = sine[MAXDATA / s * i] | DAC_A;
		} else if (ch == CHANNEL_B){
			dma_data[i] = sine[MAXDATA / s * i] | DAC_B;
		} else {
			dma_data[i * 2] = sine[MAXDATA / s * i] | DAC_A;
			dma_data[i * 2 + 1] = sine[MAXDATA / s * i] | DAC_B;
		}
	}

	dma_setup_channel(DMA_CH2, (int)dma_data, SPI1_DR,
			  ch == CHANNEL_A_B ? (s * 2) : s,
			  DMA_M_32BIT | DMA_P_32BIT | DMA_M_INC | DMA_M_TO_P |
			  DMA_CIRCULAR | DMA_ENABLE);
}

void dac_tone(int ch, int pitch, int octave, int duration)
{
	if (ch == CHANNEL_A_B)
		tim_enable_counter(TIM22);
	dac_dma_enable(ch, MAXDATA >> (octave - 3));
	tim_setup_counter(TIM21, 1, autoreload[pitch]);
	tim_set_capture_compare_value(TIM21_CC1, autoreload[pitch] - 4);
	tim_enable_counter(TIM21);
	delay_ms(1000 * duration);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	tim_disable_counter(TIM21);
	dma_disable(DMA_CH2);
	if (ch == CHANNEL_A_B)
		tim_disable_counter(TIM22);
}
