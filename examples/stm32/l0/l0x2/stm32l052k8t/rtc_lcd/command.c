/*
 * command.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of rtc_lcd.
 *
 * rtc_lcd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rtc_lcd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rtc_lcd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>

#include <gpio.h>
#include <tim.h>
#include <nvic.h>
#include <vector.h>
#include <rtc.h>

#include "rtc_lcd.h"

#define MAXTOKEN		10
#define BUFSIZE			64
#define DELIMITER		" /.,;:"

/* EEPROM address */
extern int *rom_contrast;
extern int *rom_bright;
extern int *rom_calib;

/* Day of week */
const char week_day_name[8][4] = {
	"???",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"Sun"
};

/* Month */
const char month_name[13][4] = {
	"???",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

/* Alarm */
int alarm_a;
int alarm_b;
bool alarm_a_enable;
bool alarm_b_enable;
extern volatile bool alarm_a_flag;
extern volatile bool alarm_b_flag;

static void print_date(int date, int time, int ss)
{
	unsigned int u;

	printf("%s ", week_day_name[(date >> 13) & 7]);
	u = ((date >> 12) & 1) * 10 + ((date >> 8) & 15);
	if (u <= 12)
		printf("%s ", month_name[u]);
	u = ((date >> 4) & 3) * 10 + (date & 15);
	printf("%2u ", u);
	u = ((time >> 20) & 3) * 10 + ((time >> 16) & 15);
	printf("%02u:", u);
	u = ((time >> 12) & 7) * 10 + ((time >> 8) & 15);
	printf("%02u:", u);
	u = ((time >> 4) & 7) * 10 + (time & 15);
	printf("%02u", u);
	if (ss < 0) {
		printf(" ");
	} else if (ss > PREDIV_S) {
		printf(".?????? ");
	} else {
		u = ((PREDIV_S - ss) * 1000000 + (PREDIV_S + 1) / 2) /
			(PREDIV_S + 1);
		printf(".%06u ", u);
	}
	u = ((date >> 20) & 15) * 10 + ((date >> 16) & 15);
	printf("20%02u\n", u);
}

static int str_date(char **p, int *date, int *time)
{
	int i;
	int d;
	int t;
	char *endp;
	
	for (i = 1; i <= 7; i++) {
		if (strncasecmp(p[1], week_day_name[i], 3) == 0)
			break;
	}
	if (i > 7)
		return 1;
	d = i << 13;
	for (i = 1; i <= 12; i++) {
		if (strncasecmp(p[2], month_name[i], 3) == 0)
			break;
	}
	if (i > 12)
		return 2;
	d |= (i / 10) << 12;
	d |= (i % 10) << 8;
	i = strtol(p[3], &endp, 10);
	if (*endp)
		return 3;
	if (i <= 0 || i > 31)
		return 3;
	d |= (i / 10) << 4;
	d |= i % 10;
	i = strtol(p[4], &endp, 10);
	if (*endp)
		return 4;
	if (i < 0 || i >= 24)
		return 4;
	t = (i / 10) << 20;
	t |= (i % 10) << 16;
	i = strtol(p[5], &endp, 10);
	if (*endp)
		return 5;
	if (i < 0 || i >= 60)
		return 5;
	t |= (i / 10) << 12;
	t |= (i % 10) << 8;
	i = strtol(p[6], &endp, 10);
	if (*endp)
		return 6;
	if (i < 0 || i >= 60)
		return 6;
	t |= (i / 10) << 4;
	t |= i % 10;
	i = strtol(p[8], &endp, 10);
	if (*endp)
		return 8;
	if (i < 2000 || i > 2099)
		return 8;
	d |= (i / 10 % 10) << 20;
	d |= (i % 10) << 16;
	*date = d;
	*time = t;
	return 0;
}

static void date(int n, char **p)
{
	int date;
	int time;
	int ss;
	int r;

	if (n != 1 && n != 9) {
		puts("Invalid argument: date [<day_of_week> <month> dd \\");
		puts("                        hh:mm:ss <not_used> 20yy]");
		return;
	}
	if (n == 9 && (r = str_date(p, &date, &time))) {
		printf("Invalid value: %s ", p[r]);
		switch (r) {
		case 1:
			puts("(Mon - Sun)");
			break;
		case 2:
			puts("(Jan - Dec)");
			break;
		case 3:
			puts("(1 - 31)");
			break;
		case 4:
			puts("(0 - 23)");
			break;
		case 5:
			puts("(0 - 59)");
			break;
		case 6:
			puts("(0 - 59)");
			break;
		case 8:
			puts("(2000 - 2099)");
			break;
		default:
			break;
		}
		return;
	}
	
	nvic_disable_irq(NVIC_RTC);
	if (n == 9) {
		rtc_unlock();
		rtc_init(PREDIV_A, PREDIV_S, date, time, false, false);
		rtc_lock();
	}
	rtc_get_calendar(&date, &time, &ss);
	nvic_enable_irq(NVIC_RTC);
	print_date(date, time, ss);
}

static void contrast(int n, char **p)
{
	int i;
	char *endp;
	
	if (n == 1) {
		printf("%d\n", *rom_contrast);
	} else if (n == 2) {
		i = strtol(p[1], &endp, 10);
		if (*endp) {
			printf("Invalid value: %s (0 - 128)\n", p[1]);
			return;
		}
		if (i < 0 || i > 128) {
			printf("Invalid value: %d (0 - 128)\n", i);
			return;
		}
		mcp_write_wiper(i);
		eeprom_write(rom_contrast, i);
		printf("%d\n", i);
	} else {
		puts("Invalid argument: contrast [<value>]");
	}
}

static void bright(int n, char **p)
{
	int i;
	char *endp;
	
	if (n == 1) {
		printf("%d\n", *rom_bright);
	} else if (n == 2) {
		i = strtol(p[1], &endp, 10);
		if (*endp) {
			printf("Invalid value: %s (0 - 255)\n", p[1]);
			return;
		}
		if (i < 0 || i > 255) {
			printf("Invalid value: %d (0 - 255)\n", i);
			return;
		}
		tim_set_capture_compare_value(TIM2_CC1, i);
		eeprom_write(rom_bright, i);
		printf("%d\n", i);
	} else {
		puts("Invalid argument: bright [<value>]");
	}
}

static void print_alarm(bool enable, int alarm, int ss, bool flag)
{
	unsigned int u;

	if (enable)
		printf("Enable  ");
	else
		printf("Disable ");
	if (alarm & (1 << 31)) {
		printf("  * ");
	} else {
		if (alarm & (1 << 30)) {
			u = (alarm >> 24) & 7;
			printf("%s ", week_day_name[u]);
		} else {
			u = ((alarm >> 28) & 3) * 10 + ((alarm >> 24) & 15);
			printf(" %2u ", u);
		}
	}
	if (alarm & (1 << 23)) {
		printf(" *:");
	} else {
		u = ((alarm >> 20) & 3) * 10 + ((alarm >> 16) & 15);
		printf("%02u:", u);
	}
	if (alarm & (1 << 15)) {
		printf(" *:");
	} else {
		u = ((alarm >> 12) & 7) * 10 + ((alarm >> 8) & 15);
		printf("%02u:", u);
	}
	if (alarm & (1 << 7)) {
		printf(" * ");
	} else {
		u = ((alarm >> 4) & 7) * 10 + (alarm & 15);
		printf("%02u ", u);
	}
	if (ss >= 0) {
		u = ss & 0x7fff;
		printf("0x%04x(%5u)/", u, u);
		u = (1 << ((ss >> 24) & 15)) - 1;
		printf("0x%04x", u);
	}
	if (flag)
		printf(" ### ALARM ###");
	printf("\n");
}

static int str_alarm(char **p, int *time, int *ss)
{
	int i;
	int t;
	char *endp;
	int s;

	if (*p[2] == '*') {
		t = (1 << 31);
	} else if (isdigit((int)*p[2])) {
		i = atoi(p[2]);
		if (i <= 0 || i > 31)
			return 2;
		t = (i / 10) << 28;
		t |= (i % 10) << 24;
	} else {
		for (i = 1; i < 8; i++) {
			if (strncasecmp(p[2], week_day_name[i], 3) == 0)
				break;
		}
		if (i >= 8)
			return 2;
		t = (1 << 30);
		t |= i << 24;
	}
	if (*p[3] == '*') {
		t |= (1 << 23);
	} else {
		i = strtoul(p[3], &endp, 10);
		if (*endp != '\0')
			return 3;
		if (i < 0 || i >= 24)
			return 3;
		t |= (i / 10) << 20;
		t |= (i % 10) << 16;
	}
	if (*p[4] == '*') {
		t |= (1 << 15);
	} else {
		i = strtoul(p[4], &endp, 10);
		if (*endp != '\0')
			return 4;
		if (i < 0 || i >= 60)
			return 4;
		t |= (i / 10) << 12;
		t |= (i % 10) << 8;
	}
	if (*p[5] == '*') {
		t |= (1 << 7);
	} else {
		i = strtoul(p[5], &endp, 10);
		if (*endp != '\0')
			return 5;
		if (i < 0 || i >= 60)
			return 5;
		t |= (i / 10) << 4;
		t |= i % 10;
	}
	*time = t;
	if (ss) {
		i = strtoul(p[6], &endp, 0);
		if (*endp != '\0')
			return -1;
		if (i < 0 || i > 0x7fff)
			return 6;
		s = i;
		i = strtoul(p[7], &endp, 0);
		if (*endp != '\0')
			return -1;
		if (i < 0 || i > 15)
			return 7;
		s |= (i << 24);
		*ss = s;
	}
	return 0;
}

static void print_alarm_a(void)
{
	printf("A: ");
	print_alarm(alarm_a_enable, alarm_a, -1, alarm_a_flag);
}

static void print_alarm_b(void)
{
	printf("B: ");
	print_alarm(alarm_b_enable, alarm_b, -1, alarm_b_flag);
}

static void clear_alarm_a(void)
{
	if (alarm_a_flag) {
		alarm_a_flag = false;
		gpio_set(GPIO_PB0);
	}
}

static void clear_alarm_b(void)
{
	if (alarm_b_flag) {
		alarm_b_flag = false;
		gpio_set(GPIO_PB1);
	}
}

static void disable_alarm_a(void)
{
	rtc_unlock();
	rtc_disable_alarm_a();
	rtc_lock();
}

static void disable_alarm_b(void)
{
	rtc_unlock();
	rtc_disable_alarm_b();
	rtc_lock();
}

static void set_alarm_a(int time, int ss)
{
	rtc_unlock();
	rtc_disable_alarm_a();
	rtc_enable_alarm_a(time, ss);
	rtc_lock();
}

static void set_alarm_b(int time, int ss)
{
	rtc_unlock();
	rtc_disable_alarm_b();
	rtc_enable_alarm_b(time, ss);
	rtc_lock();
}

static void alarm(int n, char **p)
{
	bool a;
	bool e;
	int r;
	int time = 0;

	a = false;
	if (n > 1) {
		if (strcasecmp(p[1], "a") == 0) {
			a = true;
		} else if (strcasecmp(p[1], "b") != 0) {
			printf("Invalid value: %s (A or B)\n", p[1]);
			return;
		}
	}
	e = false;
	if (n == 3) {
		if (strncasecmp(p[2], "enable", strlen(p[2])) == 0) {
			e = true;
		} else if (strncasecmp(p[2], "disable", strlen(p[2])) != 0) {
			printf("Invalid value: %s (enable or disable)\n",
			       p[2]);
			return;
		}
	}
	if ((n > 3 && n < 6) || n > 6) {
		puts("Invalid argument: alarm [{a | b} [{{<day_of_week> | "
		     "dd} \\");
		puts("                  hh:mm:ss} | {enable | disable}]]");
		return;
	}
	if (n == 6 && (r = str_alarm(p, &time, NULL))) {
		printf("Invalid value: %s ", p[r]);
		switch (r) {
		case 2:
			puts("(*, (Mon - Sun), or (1 - 31))");
			break;
		case 3:
			puts("(* or (0 - 23))");
			break;
		case 4:
			puts("(* or (0 - 59))");
			break;
		case 5:
			puts("(* or (0 - 59))");
			break;
		default:
			break;
		}
		return;
	}
	nvic_disable_irq(NVIC_RTC);
	if (n == 3) {
		if (a) {
			if (e) {
				set_alarm_a(alarm_a, 0);
				alarm_a_enable = true;
			} else {
				disable_alarm_a();
				clear_alarm_a();
				alarm_a_enable = false;
			}
		} else {
			if (e) {
				set_alarm_b(alarm_b, 0);
				alarm_b_enable = true;
			} else {
				disable_alarm_b();
				clear_alarm_b();
				alarm_b_enable = false;
			}
		}
	} else if (n == 6) {
		if (a) {
			set_alarm_a(time, 0);
			alarm_a = time;
			alarm_a_enable = true;
		} else {
			set_alarm_b(time, 0);
			alarm_b = time;
			alarm_b_enable = true;
		}
	}
	nvic_enable_irq(NVIC_RTC);
	if (n == 1) {
		print_alarm_a();
		print_alarm_b();
	} else if (a) {
		print_alarm_a();
	} else {
		print_alarm_b();
	}
}

static void sync(int n, char **p)
{
	int date;
	int time;
	int ss;
	char *endp;
	int rs;
	int ru;
	int rss;
	int s;
	int d;
	
	nvic_disable_irq(NVIC_RTC);
	rtc_get_calendar(&date, &time, &ss);
	nvic_enable_irq(NVIC_RTC);
	if (ss > PREDIV_S) {
		puts("A shift operation is now in progress.");
		return;
	}
	if (n != 3) {
		puts("Invalid argument: "
		     "synchronize <second>.<micro second>");
		return;
	}
	rs = strtol(p[1], &endp, 10);
	if (*endp) {
		printf("Invalid value: %s (0 - 59)\n", p[1]);
		return;
	}
	if (rs < 0 || rs > 59) {
		printf("Invalid value: %d (0 - 59)\n", rs);
		return;
	}
	ru = strtol(p[2], &endp, 10);
	if (*endp) {
		printf("Invalid value: %s (0 - 999999)\n", p[2]);
		return;
	}
	if (ru < 0 || ru > 999999) {
		printf("Invalid value: %d (0 - 999999)\n", ru);
		return;
	}
	rss = PREDIV_S - ru * (PREDIV_S + 1) / 1000000;
	s = ((time >> 4) & 7) * 10 + (time & 15);
	d = (rs - s) * (PREDIV_S + 1) + ss - rss;
	if (d) {
		nvic_disable_irq(NVIC_RTC);
		rtc_unlock();
		if (d >= PREDIV_S + 1)
			rtc_synchronize(1, 0);
		else if (d > 0)
			rtc_synchronize(1, PREDIV_S + 1 - d);
		else if (d > -(PREDIV_S + 1))
			rtc_synchronize(0, -d);
		else
			rtc_synchronize(0, PREDIV_S + 1);
		rtc_lock();
		nvic_enable_irq(NVIC_RTC);
	}
	printf("%+d (1/%u second)\n", d, PREDIV_S + 1);
}

static void rtcclk(int n, char **p)
{
	int i;
	char *endp;
	
	if (n != 1 && n != 2) {
		puts("Invalid argument: rtcclk [<add/sub>]");
		return;
	}
	if (n == 1) {
		printf("%+d pulse / 32-second cycle (clock %u Hz)\n",
		       *rom_calib, (PREDIV_A + 1) * (PREDIV_S + 1));
		return;
	}
	i = strtol(p[1], &endp, 10);
	if (*endp) {
		printf("Invalid value: %s (-511 to 512)\n", p[1]);
		return;
	}
	if (i < -511 || i > 512) {
		printf("Invalid value: %d (-511 to 512)\n", i);
		return;
	}
	nvic_disable_irq(NVIC_RTC);
	rtc_unlock();
	if (i <= 0)
		rtc_set_smooth_digital_calibration(0, -i, RTC_32S);
	else
		rtc_set_smooth_digital_calibration(1, 512 - i, RTC_32S);
	rtc_lock();
	nvic_enable_irq(NVIC_RTC);
	eeprom_write(rom_calib, i);
	printf("%+d pulse / 32-second cycle (clock %u Hz)\n", i,
	       (PREDIV_A + 1) * (PREDIV_S + 1));
}

static void help(void)
{
	puts("Usage:");
	puts("  date                             Print date and time");
	puts("  date <day_of_week> <month> dd \\");
	puts("        hh:mm:ss <not_used> 20yy   Set date and time");
	puts("  contrast                         Print LCD contrast");
	puts("  contrast <value>                 Set LCD contrast");
	puts("  bright                           Print LCD brightness");
	puts("  bright <value>                   Set LCD brightness");
	puts("  alarm [a | b]                    Print alarm status");
	puts("  alarm {a | b} {<day_of_week> | dd} \\");
	puts("         hh:mm:ss                  Set alarm");
	puts("  alarm {a | b} disable            Disable alarm");
	puts("  alarm {a | b} enable             Re-enable alarm");
	puts("  synchronize ss.uuuuuu            Synchronize");
	puts("  rtcclk                           Print calibration value");
	puts("  rtcclk <add/sub>                 Set calibration value");
	puts("  help                             Print this message");
}

static void get_line(char buf[])
{
	int i;
	int c;

	i = 0;
	do {
		c = getchar();
		if (isprint(c)) {
			putchar(c);
			if (i < BUFSIZE - 1)
				buf[i++] = c;
		} else {
			switch (c) {
			case '\b':
				if (i > 0) {
					printf("\b \b");
					--i;
				}
				break;
			case '\n':
				putchar('\n');
				buf[i] = '\0';
				break;
			default:
				break;
			}
		}
	} while (c != '\n');
}

void command(void)
{
	static char buf[BUFSIZE];
	int i;
	static char *p[MAXTOKEN];

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	while (1) {
		printf(PROMPT);
		get_line(buf);
		for (i = 0; i < MAXTOKEN; i++) {
			p[i] = strtok(i ? NULL : buf, DELIMITER);
			if (p[i] == NULL)
				break;
		}
		if (i == 0)
			continue;
		if (strncasecmp(p[0], "date", strlen(p[0])) == 0)
			date(i, p);
		else if (strncasecmp(p[0], "contrast", strlen(p[0])) == 0)
			contrast(i, p);
		else if (strncasecmp(p[0], "bright", strlen(p[0])) == 0)
			bright(i, p);
		else if (strncasecmp(p[0], "alarm", strlen(p[0])) == 0)
			alarm(i, p);
		else if (strncasecmp(p[0], "synchronize", strlen(p[0])) == 0)
			sync(i, p);
		else if (strncasecmp(p[0], "rtcclk", strlen(p[0])) == 0)
			rtcclk(i, p);
		else if (strncasecmp(p[0], "help", strlen(p[0])) == 0 ||
			 strcmp(p[0], "?") == 0)
			help();
		else
			puts("unknown command");
	}
}
