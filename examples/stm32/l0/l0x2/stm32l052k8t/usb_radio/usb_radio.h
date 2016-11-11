/*
 * usb_radio.h
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_radio.
 *
 * usb_radio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_radio.  If not, see <http://www.gnu.org/licenses/>.
 */

#define SAMPLING_FREQ		44100
#define SAMPLING_CYCLE		10
#define SAMPLING_BITS		16
#define SAMPLING_BYTES		(SAMPLING_BITS <= 8 ? 1 : \
				 (SAMPLING_BITS <= 16 ? 2 : 4))
#define SAMPLING_CHANNELS	2
#define CONFIGURATION_VALUE	1
#define MAXPACKETSIZE0		64
#define MAX_DATA_IN		64
#define MAXINTERFACE		2
#define MAXENDPOINT		2
#define INTERFACE_AC		0 /* Audio Control Interface */
#define INTERFACE_AS		1 /* Audio Stream Interface */
#define AS_ENUM			0x81
#define AS_SIZE			(SAMPLING_FREQ / 1000 * SAMPLING_BYTES * \
				 SAMPLING_CHANNELS)
#define AS_SIZE_MAX		(((SAMPLING_FREQ % 1000) ? \
				  (SAMPLING_FREQ / 1000 + 1) : \
				  (SAMPLING_FREQ / 1000)) * SAMPLING_BYTES * \
				 SAMPLING_CHANNELS)
#define AS_INTERVAL		1
#define MAX_STRING_INDEX	2
#define INPUT_TERMINAL_ID	1
#define FEATURE_UNIT_ID		2
#define OUTPUT_TERMINAL_ID	3

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

void delay_ms(int ms);
int si4737_init(int freq, bool digital);
int si4737_revision(void);
int si4737_tune(int freq);
int si4737_rsq(void);
int si4737_enable_digital_output(void);
void command_init(void);
void command_line(void);
void control_setup(void);
void control_rx(void);
void control_tx(void);
void control_reset(void);
