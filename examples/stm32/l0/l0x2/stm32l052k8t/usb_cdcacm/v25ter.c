/*
 * v25ter.c
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include <usart.h>

#include "usb_cdcacm.h"

#define CMDSIZE 64

enum v25ter_state {
	V25TER_STATE_NONE,
	V25TER_STATE_PREFIX,
	V25TER_STATE_BODY
};
static enum v25ter_state state;
static enum v25ter_state state_encap;

/* DTE-DCE local flow control (+IFC) */
static enum v25ter_result ifc(char *cmd, char **info)
{
	static bool rts_enable;
	static bool cts_enable;

	if (strcmp(cmd, "+IFC?") == 0) {
		if (rts_enable) {
			if (cts_enable) {
				*info = "\r\n+IFC:2,2\r\nOK\r\n";
				return RESULT_OK;
			} else {
				*info = "\r\n+IFC:2,0\r\nOK\r\n";
				return RESULT_OK;
			}
		} else {
			if (cts_enable) {
				*info = "\r\n+IFC:0,2\r\nOK\r\n";
				return RESULT_OK;
			} else {
				*info = "\r\n+IFC:0,0\r\nOK\r\n";
				return RESULT_OK;
			}
		}
	} else if (strcmp(cmd, "+IFC=") == 0) {
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=?") == 0) {
		*info = "\r\n+IFC:(0,2),(0,2)\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=0") == 0) {
		rts_enable = false;
		cts_enable = false;
		usart_set_flow_control(USART2, USART_FLOW_NONE);
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=2") == 0) {
		rts_enable = true;
		cts_enable = true;
		usart_set_flow_control(USART2, USART_RTS_CTS);
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=0,0") == 0) {
		rts_enable = false;
		cts_enable = false;
		usart_set_flow_control(USART2, USART_FLOW_NONE);
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=0,2") == 0) {
		rts_enable = false;
		cts_enable = true;
		usart_set_flow_control(USART2, USART_FLOW_CTS);
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=2,0") == 0) {
		rts_enable = true;
		cts_enable = false;
		usart_set_flow_control(USART2, USART_FLOW_RTS);
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (strcmp(cmd, "+IFC=2,2") == 0) {
		rts_enable = true;
		cts_enable = true;
		usart_set_flow_control(USART2, USART_RTS_CTS);
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	}
	*info = "\r\nERROR\r\n";
	return RESULT_ERROR;
}

/* Escape code guard time (S12) */
static enum v25ter_result s12(char *cmd, char **info)
{
	extern int escape_code_guard_time;
	static char buf[16];
	char *p;
	int  n;
	int i;
	
	if (strcmp(cmd, "S12?") == 0) {
		sprintf(buf, "\r\nS12:%03d\r\nOK\r\n", escape_code_guard_time);
		*info = buf;
		return RESULT_OK;
	} else if (strcmp(cmd, "S12=?") == 0) {
		*info = "\r\nS12:0-255\r\nOK\r\n";
		return RESULT_OK;
	} else if (strncmp(cmd, "S12=", 4) == 0) {
		p = cmd + 4;
		n = 0;
		for (i = 0; i < 4; i++) {
			if (!isdigit(*p))
				break;
			n = n * 10 + (*p - '0');
			p++;
		}
		if (i > 0 && n < 256 && *p == '\0') {
			escape_code_guard_time = n;
			*info = "\r\nOK\r\n";
			return RESULT_OK;
		}
	}
	*info = "\r\nERROR\r\n";
	return RESULT_ERROR;
}

static enum v25ter_result command(char *cmd, char **info)
{
	char buf[CMDSIZE];
	int i;

	i = 0;
	while (*cmd) {
		if (!isspace((int)*cmd))
			buf[i++] = toupper((int)*cmd);
		cmd++;
	}
	buf[i] = '\0';
	if (buf[0] == '\0') {
		*info = "\r\nOK\r\n";
		return RESULT_OK;
	} else if (buf[0] == 'O') {
		*info = "\r\nCONNECT\r\n";
		return RESULT_CONNECT;
	} else if (strncmp(buf, "+IFC", 3) == 0) {
		return ifc(buf, info);
	} else if (strncmp(buf, "S12", 3) == 0) {
		return s12(buf, info);
	}
	*info = "\r\nERROR\r\n";
	return RESULT_ERROR;
}

/*
 * A command line is made up of three elements: the prefix, the body,
 * and the terminate character.
 */
enum v25ter_result v25ter_input(int c, char *echo, char **info)
{
	static char prev_cmd[CMDSIZE];
	static char cmd[CMDSIZE];
	static char *cmd_p;

	switch (state) {
	case V25TER_STATE_NONE:
		*echo++ = c;
		*echo = '\0';
		if (toupper(c) == 'A')
			state = V25TER_STATE_PREFIX;
		break;
	case V25TER_STATE_PREFIX:
		*echo++ = c;
		*echo = '\0';
		if (c == '/') {
			state = V25TER_STATE_NONE;
			return command(prev_cmd, info);
		} else if (toupper(c) == 'T') {
			cmd_p = cmd;
			state = V25TER_STATE_BODY;
		} else {
			state = V25TER_STATE_NONE;
		}
		break;
	case V25TER_STATE_BODY:
		if (c == '\r') {
			*echo = '\0';
			*cmd_p = '\0';
			strcpy(prev_cmd, cmd);
			state = V25TER_STATE_NONE;
			return command(cmd, info);
		} else {
			if (c == '\b') {
				if (cmd_p != cmd) {
					*echo++ = '\b';
					*echo++ = ' ';
					*echo++ = '\b';
					*echo = '\0';
					cmd_p--;
				} else {
					*echo = '\0';
				}
			} else {
				if (cmd_p < cmd + CMDSIZE - 1) {
					*echo++ = c;
					*echo = '\0';
					*cmd_p++ = c;
				} else {
					*echo = '\0';
				}
			}
		}
		break;
	default:
		break;
	}
	return RESULT_NONE;
}

enum v25ter_result v25ter_input_encap(int c, char **info)
{
	static char prev_cmd[CMDSIZE];
	static char cmd[CMDSIZE];
	static char *cmd_p;

	switch (state_encap) {
	case V25TER_STATE_NONE:
		if (toupper(c) == 'A')
			state_encap = V25TER_STATE_PREFIX;
		break;
	case V25TER_STATE_PREFIX:
		if (c == '/') {
			state_encap = V25TER_STATE_NONE;
			return command(prev_cmd, info);
		} else if (toupper(c) == 'T') {
			cmd_p = cmd;
			state_encap = V25TER_STATE_BODY;
		} else {
			state_encap = V25TER_STATE_NONE;
		}
		break;
	case V25TER_STATE_BODY:
		if (c == '\r') {
			*cmd_p = '\0';
			strcpy(prev_cmd, cmd);
			state_encap = V25TER_STATE_NONE;
			return command(cmd, info);
		} else {
			if (c == '\b') {
				if (cmd_p != cmd) {
					cmd_p--;
				}
			} else {
				if (cmd_p < cmd + CMDSIZE - 1) {
					*cmd_p++ = c;
				}
			}
		}
		break;
	default:
		break;
	}
	return RESULT_NONE;
}

void v25ter_reset(void)
{
	state = V25TER_STATE_NONE;
	state_encap = V25TER_STATE_NONE;
}
