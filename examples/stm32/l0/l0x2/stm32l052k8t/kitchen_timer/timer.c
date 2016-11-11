/*
 * timer.c
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

#include <tim.h>
#include <scb.h>
#include <pwr.h>
#include <comp.h>

#include "kitchen_timer.h"

extern volatile unsigned int jiffies;
extern volatile int button;
extern const char large_font[];
extern const char small_font[];
extern const char alarm_mark[];
extern const char battery_mark[];
extern const char underline[];
extern const char allzero[];

/*
 * t[0] - Second units
 * t[1] - Second tens
 * t[2] - Minute units
 * t[3] - Minute tens
 */
static int t[4];
static int cp = 2;			/* Cursor position */
static bool battery_warning;
static enum state {
	IDLE,
	COUNT,
	TIMEOUT
} state;
static unsigned int expire = POWERDOWN;

static void digit(int n)
{
	if (n == 3)
		lcd_write(0, 0, &large_font[t[3] * 32 * 32 / 8], 32, 32);
	else if (n == 2)
		lcd_write(0, 32, &large_font[t[2] * 32 * 32 / 8], 32, 32);
	else if (n == 1)
		lcd_write(1, 74, &small_font[t[1] * 16 * 16 / 8], 16, 16);
	else if (n == 0)
		lcd_write(1, 100, &small_font[t[0] * 16 * 16 / 8], 16, 16);
}

static void clear(void)
{
	int i;

	for (i = 3; i >= 0; i--) {
		t[i] = 0;
		digit(i);
	}
}

static void cursor(bool on)
{
	if (on)
		lcd_write(4, 32 * (3 - cp), underline, 32, 8);
	else if (battery_warning && cp < 2)
		lcd_write(4, 32 * (3 - cp),
			  &battery_mark[32 * (1 - cp)], 32, 8);
	else
		lcd_write(4, 32 * (3 - cp), allzero, 32, 8);
}

static void right(void)
{
	if (cp) {
		cursor(false);
		cp--;
		cursor(true);
	}
}

static void left(void)
{
	if (cp < 3) {
		cursor(false);
		cp++;
		cursor(true);
	}
}

static void up(void)
{
	if (t[cp] < ((cp & 1) ? 5 : 9)) {
		t[cp]++;
		digit(cp);
	}
}

static void down(void)
{
	if (t[cp]) {
		t[cp]--;
		digit(cp);
	}
}

static void count_down(void)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (t[i]) {
			t[i]--;
			break;
		}
		t[i] = (i & 1) ? 5 : 9;
	}
	for (i = i >= 4 ? 3 : i; i >= 0; i--)
		digit(i);
}

static void alarm(bool on)
{
	static bool mark;
	static bool sound;
	
	if (on) {
		if (!mark) {
			lcd_write(4, 0, alarm_mark, 64, 16);
			mark = true;
		}
		if (!sound) {
			tim_enable_counter(TIM2);
			sound = true;
		} else {
			tim_disable_counter(TIM2);
			sound = false;
		}
	} else {
		if (mark) {
			lcd_write(4, 0, allzero, 64, 16);
			mark = false;
		}
		if (sound) {
			tim_disable_counter(TIM2);
			sound = false;
		}
	}
}

static void powerdown(void)
{
	lcd_sleep();

	scb_set_sleep(SCB_SLEEP_DEEP);
	pwr_set_mode(PWR_STANDBY);
	__asm__ ("wfi");
}

static void battery(void)
{
	if (comp_get_status(COMP2)) {
		if (battery_warning) {
			lcd_write(4, 64, allzero, 64, 16);
			battery_warning = false;
		}
	} else {
		if (!battery_warning) {
			lcd_write(4, 64, battery_mark, 64, 16);
			battery_warning = true;
		}
	}
}

void timer_init(void)
{
	clear();
	battery();
	cursor(true);
}

static void idle(void)
{
	int i;
	static const struct {
		int bit;
		void (*func)(void);
	} b[] = {
		{RIGHT, right},
		{LEFT, left},
		{UP, up},
		{DOWN, down},
		{CENTER, 0}
	};
	
	if (!button) {
		if (jiffies >= expire)
			powerdown();
		return;
	}
	
	for (i = 0; i < 5; i++) {
		if (button & b[i].bit) {
			button &= ~b[i].bit;
			if (b[i].func) {
				expire = jiffies + POWERDOWN;
				(*b[i].func)();
			} else if (t[0] || t[1] || t[2] || t[3]) {
				state = COUNT;
				expire = jiffies + HZ;
				cursor(false);
			} else {
				state = TIMEOUT;
				expire = jiffies + ALARMINTERVAL;
				cursor(false);
				alarm(true);
			}
		}
	}
}

static void count(void)
{
	if (button) {
		state = IDLE;
		expire = jiffies + POWERDOWN;
		clear();
		cursor(true);
		button = 0;
		return;
	}
	if (jiffies >= expire) {
		count_down();
		if (t[0] || t[1] || t[2] || t[3]) {
			expire += HZ;
		} else {
			state = TIMEOUT;
			expire = jiffies + ALARMINTERVAL;
			alarm(true);
		}
	}
}

static void timeout(void)
{
	static int alarmtime;

	if (button) {
		state = IDLE;
		expire = jiffies + POWERDOWN;
		alarm(false);
		alarmtime = 0;
		cursor(true);
		button = 0;
		return;
	}
	if (jiffies >= expire) {
		if (++alarmtime >= ALARMTOOLONG)
			powerdown();
		alarm(true);
		expire += ALARMINTERVAL;
	}
}

void timer(void)
{
	while (1) {
		if (state == IDLE)
			idle();
		else if (state == COUNT)
			count();
		else if (state == TIMEOUT)
			timeout();
	}
}
