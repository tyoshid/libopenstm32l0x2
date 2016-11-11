/*
 * usb_cdcacm.h
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

#define TIMX_CLK_APB1			32000000 /* TIM6 clock frequency */
#define TIMX_CLK_APB2			32000000 /* TIM21,22 clock frequency */
#define FCK				32000000 /* USART clock frequency */
#define ESCAPE_CHAR			'+'
#define ESCAPE_CODE_GUARD_TIME		50 /* 50 * 20 ms = 1 s */
#define CONFIGURATION_VALUE		1
#define MAXPACKETSIZE0			64
#define MAX_DATA_OUT			64
#define MAX_DATA_IN			64
#define MAXINTERFACE			2
#define MAXENDPOINT			4
#define INTERFACE_COMM			0
#define NOTIFICATION_ENUM		0x81
#define NOTIFICATION_INDEX		1
#define NOTIFICATION_SIZE		16
#define NOTIFICATION_INTERVAL		128
#define INTERFACE_DATA			1
#define DATA_RX_ENUM			0x02
#define DATA_RX_INDEX			2
#define DATA_TX_ENUM			0x82
#define DATA_TX_INDEX			3
#define DATA_SIZE			64
#define DATA_INTERVAL			1
#define DATABUFSIZE			(DATA_SIZE + 4)
#define MAX_STRING_INDEX		3

enum ep_state {
	EP_STATE_DISABLE,
	EP_STATE_ENABLE,
	EP_STATE_HALT
};
enum control_stage {
	CONTROL_STAGE_SETUP,
	CONTROL_STAGE_DATA,
	CONTROL_STAGE_STATUS
};
enum cmd_state {
	ONLINE_DATA_STATE,
	ESCAPE_STATE1,
	ESCAPE_STATE2,
	ESCAPE_STATE3,
	COMMAND_STATE,
	EXIT_STATE
};
enum v25ter_result {
	RESULT_OK = 0,
	RESULT_CONNECT = 1,
	RESULT_ERROR = 4,
	RESULT_NONE = -1
};

void notify_serial_state(int usart_status);
bool notify_available(void);
int notify(char **data);
void bulk_rx(void);
void set_serial_number(int mcu, int *unique);
void control_setup(void);
void control_rx(void);
void control_tx(void);
void control_reset(void);
