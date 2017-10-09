/*
 * kitchen_timer.h
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

#define HZ			100   /* SysTick frequency */
#define FALARM			880   /* Alarm sound */
#define MAXBCOUNT		4     /* Button tick count */
#define ALARMINTERVAL		(HZ / 16)
#define POWERDOWN		(3 * 60 * HZ) /* Timeout */
#define ALARMTOOLONG		(POWERDOWN / ALARMINTERVAL)

/* Button flag */
enum {
	RIGHT = (1 << 0),
	DOWN = (1 << 1),
	LEFT = (1 << 2),
	ENTER = (1 << 3),
	UP = (1 << 4)
};

void delay_ms(int ms);
void lcd_init(void);
void lcd_write(int page, int column, const char *buf, int width, int height);
void lcd_sleep(void);
void timer_init(void);
void timer(void);
