/*
 * sound.c - 12-bit DAC sound
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
#include <math.h>

#include <dma.h>
#include <memorymap.h>
#include <dac.h>
#include <tim.h>

#include "oled_game.h"

#define BEEP_BALL			(REFRATE / 2)
#define BEEP_GAME			(REFRATE * 2)
#define MAXPOPCOUNT			12

static int beep;		/* Beep counter */
static int pop;			/* Pop counter */
static short sine[WAVEDATA];	/* Sine wave data */
static short sbuf[2][MAXDATA];	/* Sound buffer */
static int curbuf;		/* Current sound buffer (0 or 1) */

static void square_wave(int b)
{
	int x;
	int j;
	int i;

	x = 0;
	for (j = 0; j < NWAVE; j++) {
		for (i = 0; i < WAVEDATA / 8; i++)
			sbuf[b][x++] = 2048;
		for (i = 0; i < WAVEDATA / 4; i++)
			sbuf[b][x++] = 4095;
		for (i = 0; i < WAVEDATA / 4; i++)
			sbuf[b][x++] = 2048;
		for (i = 0; i < WAVEDATA / 4; i++)
			sbuf[b][x++] = 0;
		for (i = 0; i < WAVEDATA / 8; i++)
			sbuf[b][x++] = 2048;
	}
}

static void sine_wave(int b, int shift)
{
	int j;
	int i;
	
	for (j = 0; j < NWAVE; j++)
		for (i = 0; i < WAVEDATA; i++)
			sbuf[b][j * WAVEDATA + i] = (sine[i] >> shift) + 2048;
}

static void silence(int b)
{
	int i;
	
	for (i = 0; i < MAXDATA; i++)
		sbuf[b][i] = 2048;
}

void sound_init(void)
{
	int i;
	
	for (i = 0; i < WAVEDATA; i++)
		sine[i] = round(sin(2. * M_PI / WAVEDATA * i) * 2047.);
	silence(0);
	silence(1);
	dma_setup_channel(DMA_CH2, (int)sbuf[0], DAC_DHR12R1, MAXDATA,
			  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC | DMA_M_TO_P |
			  DMA_COMPLETE | DMA_ENABLE);
	tim_enable_counter(TIM21);
}

void sound_restart(void)
{
	dma_disable(DMA_CH2);

	curbuf ^= 1;
	dma_setup_channel(DMA_CH2, (int)sbuf[curbuf], DAC_DHR12R1, MAXDATA,
			  DMA_M_16BIT | DMA_P_16BIT | DMA_M_INC | DMA_M_TO_P |
			  DMA_COMPLETE | DMA_ENABLE);
}

void sound_beep(bool g)
{
	if (g)
		beep = BEEP_GAME;
	else
		beep = BEEP_BALL;
}

void sound_pop(void)
{
	pop = MAXPOPCOUNT;
}

void sound_stop(void)
{
	pop = 0;
}

void sound_make_data(void)
{
	int b;
	
	b = curbuf ^ 1;
	if (beep) {
		square_wave(b);
		--beep;
	} else if (pop) {
		sine_wave(b, MAXPOPCOUNT - pop);
		--pop;
	} else {
		silence(b);
	}
}
