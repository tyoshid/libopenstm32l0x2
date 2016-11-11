/*
 * spi_dac.h
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

#define TIMX_CLK_APB2	32000000 /* TIM21 clock frequency */

/* Channel */
enum {
	CHANNEL_A,
	CHANNEL_B,
	CHANNEL_A_B
};

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

void delay_ms(int ms);
void dac_init(void);
void dac_tone(int ch, int pitch, int octave, int duration);
