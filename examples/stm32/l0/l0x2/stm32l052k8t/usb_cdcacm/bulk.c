/*
 * bulk.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_cdcacm.
 *
 * usb_cdcacm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_cdcacm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_cdcacm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdalign.h>

#include <usbdevfs.h>
#include <tim.h>

#include "usb_cdcacm.h"

enum v25ter_result v25ter_input(int c, char *echo, char **info);

volatile enum cmd_state command_state;
alignas(2) char command_buf[DATA_SIZE];
volatile int command_len;
char escape_char = ESCAPE_CHAR;
int escape_code_guard_time;
alignas(2) char data_buf[DATABUFSIZE];
volatile int data_wp;
volatile int data_rp;
volatile int data_len;

static int check_escape(int c)
{
	int r;

	if (escape_code_guard_time <= 0)
		return 0;
	tim_disable_counter(TIM21);
	r = 0;
	switch (command_state) {
	case ONLINE_DATA_STATE:
		if (c == escape_char) {
			command_state = ESCAPE_STATE1;
			r = -1;
		}
		break;
	case ESCAPE_STATE1:
		if (c == escape_char) {
			command_state = ESCAPE_STATE2;
			r = -1;
		} else {
			command_state = ONLINE_DATA_STATE;
			r = 1;
		}
		break;
	case ESCAPE_STATE2:
		if (c == escape_char) {
			command_state = ESCAPE_STATE3;
			r = -1;
		} else {
			command_state = ONLINE_DATA_STATE;
			r = 2;
		}
		break;
	case ESCAPE_STATE3:
		command_state = ONLINE_DATA_STATE;
		r = 3;
	default:
		break;
	}
	if (r < 0) {
		if (command_state != ESCAPE_STATE2)
			tim_generate_event(TIM21, TIM_UPDATE);
		tim_set_autoreload(TIM21, (escape_code_guard_time << 4) +
				   (escape_code_guard_time << 2));
		tim_enable_counter(TIM21);
	}
	return r;
}

void bulk_rx(void)
{
	int len;
	char buf[DATA_SIZE];
	int i;
	enum v25ter_result result;
	char echo[8];
	char *info;
	int j;
	int r;
	
	len = usbdevfs_read(DATA_RX_INDEX, buf, DATA_SIZE);
	if (command_state == COMMAND_STATE) {
		for (i = 0; i < len; i++) {
			result = v25ter_input(buf[i], echo, &info);
			for (j = 0; echo[j] && command_len < DATA_SIZE; j++)
				command_buf[command_len++] = echo[j];
			if (result != RESULT_NONE) {
				while (*info && command_len < DATA_SIZE)
					command_buf[command_len++] = *info++;
				break;
			}
		}
		if (result == RESULT_CONNECT)
			command_state = EXIT_STATE;
	} else {
		for (i = 0; i < len; i++) {
			r = check_escape(buf[i]);
			for (j = 0; j < r; j++) {
				data_buf[data_wp++] = escape_char;
				if (data_wp >= DATABUFSIZE)
					data_wp = 0;
				data_len++;
			}
			if (r >= 0) {
				data_buf[data_wp++] = buf[i];
				if (data_wp >= DATABUFSIZE)
					data_wp = 0;
				data_len++;
			}
		}
	}
}

void bulk_reset(void)
{
	tim_disable_counter(TIM21);
	command_state = ONLINE_DATA_STATE;
	command_len = 0;
	data_len = 0;
	data_wp = 0;
	data_rp = 0;
}
