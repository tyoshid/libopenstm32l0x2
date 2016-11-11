/*
 * game.c
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

#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include <rng.h>
#include <tim.h>

#include "oled_game.h"

#define O_X				1.0 /* Offset (wall thickness) */
#define O_Y				1.0
#define RACKET_SIZE			4
#define MAXBALLCOUNT			5
#define TIMEOUT_BALL			(REFRATE * 3)
#define TIMEOUT_GAME			(REFRATE * 10)

static double b_x = MAXX / 2.;	/* Ball */
static double b_y = MAXY / 2.;
static double bs_x;		/* Speed */
static double bs_y;
static int r_y = MAXY / 2;	/* Racket */
static int bcount;
static int timeout = TIMEOUT_BALL;
static int score;

/* bs_x = -0.5, bs_y = -0.5 - 0.5 */
static void set_ball_speed(void)
{
	int r;
	double d;
	int i;
	
	r = rng_get_data();
	d = 0.;
	for (i = 0; i < 31; i++) {
		if (r & (1 << (30 - i)))
			d += pow(0.5, i + 1);
	}

	bs_x = -0.5;
	if (r < 0)
		bs_y = -0.5 * d;
	else
		bs_y = 0.5 * d;
}

static void ball_y(void)
{
	b_y += bs_y;
	if (b_y < O_Y) {
		b_y = O_Y * 2. - b_y;
		bs_y = -bs_y;
		sound_pop();
		score++;
	} else if (b_y > MAXY - 1 - O_Y) {
		b_y = (MAXY - 1 - O_Y ) * 2. - b_y;
		bs_y = -bs_y;
		sound_pop();
		score++;
	}
}

static void ball_x(void)
{
	double prev;
	int y;
	int r;
	
	prev = b_x;
	b_x += bs_x;
	if (b_x < O_X) {
		b_x = O_X * 2. - b_x;
		bs_x = -bs_x;
		sound_pop();
		score++;
	} else if (prev <= MAXX - 1 - O_X && b_x > MAXX - 1 - O_X) {
		y = round(b_y);
		if (y >= r_y && y < r_y + RACKET_SIZE) {
			b_x = (MAXX - 1 - O_X ) * 2. - b_x;
			bs_x = -bs_x;
			/* Random factor */
			r = rng_get_data();
			if (r > 1073741824 && bs_y < 0.4)
				bs_y += 0.1;
			else if (r < -1073741824 && bs_y > -0.4)
				bs_y -= 0.1;
			sound_pop();
			score++;
		}
	}
}

static int ball(bool rewrite)
{
	if (rewrite) {
		oled_point(round(b_x), round(b_y), 0);
		ball_y();
		ball_x();
	}
	if (round(b_x) > MAXX - 1)
		return -1;
	oled_point(round(b_x), round(b_y), 1);
	return 0;
}

static void racket_write(int color)
{
	if (r_y < 0) {
		if (r_y + RACKET_SIZE > 0)
			oled_line_y(MAXX - 1, 0, r_y + RACKET_SIZE, color);
	} else if (r_y + RACKET_SIZE > MAXY) {
		if (r_y < MAXY)
			oled_line_y(MAXX - 1, r_y, MAXY - r_y, color);
	} else {
		oled_line_y(MAXX - 1, r_y, RACKET_SIZE, color);
	}
}

static void racket(bool rewrite)
{
	if (rewrite) {
		racket_write(0);
		r_y = tim_get_counter(TIM22);
	}
	racket_write(1);
}

void game_init(void)
{
	oled_init();
	racket(false);
}

static void game_timeout(void)
{
	char buf[11];
	
	if (timeout == TIMEOUT_BALL && !bcount) {
		bcount = MAXBALLCOUNT;
		score = 0;
		oled_string(2, 2, "PLAY!    ");
		sprintf(buf, "BALL    %u", bcount);
		oled_string(3, 2, buf);
	}
	--timeout;
	if (!timeout) {
		oled_string(2, 2, "         ");
		oled_string(3, 2, "         ");
		b_x = MAXX / 2.;
		b_y = MAXY / 2.;
		set_ball_speed();
		ball(false);
	}
}

static void game_ball_out(void)
{
	char buf[11];

	sound_stop();
	--bcount;
	if (!bcount) {
		timeout = TIMEOUT_GAME;
		sound_beep(true);
		oled_string(2, 2, "GAME OVER");
	} else {
		timeout = TIMEOUT_BALL;
		sound_beep(false);
		sprintf(buf, "BALL    %u", bcount);
		oled_string(2, 2, buf);
	}
	if (score >= 0 && score < 1000) {
		sprintf(buf, "SCORE %03u", score);
		oled_string(3, 2, buf);
	} else {
		oled_string(3, 2, "SCORE ???");
	}
}

void game(void)
{
	racket(true);
	if (timeout) {
		game_timeout();
	} else {
		if (ball(true))
			game_ball_out();
	}
}
