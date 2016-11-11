/*
 * usb_speaker.h
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_speaker.
 *
 * usb_speaker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_speaker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_speaker.  If not, see <http://www.gnu.org/licenses/>.
 */

#define SAMPLING_FREQ		44100
#define SAMPLING_CYCLE		10
#define SAMPLING_BITS		12
#define SAMPLING_BYTES		(SAMPLING_BITS <= 8 ? 1 : \
				 (SAMPLING_BITS <= 16 ? 2 : 4))
#define SAMPLING_CHANNELS	2
#define CONFIGURATION_VALUE	1
#define MAXPACKETSIZE0		64
#define MAX_DATA_OUT		2
#define MAX_DATA_IN		64
#define MAXINTERFACE		2
#define MAXENDPOINT		2
#define INTERFACE_AC		0 /* Audio Control Interface */
#define INTERFACE_AS		1 /* Audio Stream Interface */
#define AS_ENUM			1
#define AS_SIZE			(SAMPLING_FREQ / 1000 * SAMPLING_BYTES * \
				 SAMPLING_CHANNELS)
#define AS_SIZE_MAX		(((SAMPLING_FREQ % 1000) ? \
				  (SAMPLING_FREQ / 1000 + 2) : \
				  (SAMPLING_FREQ / 1000 + 1)) * \
				 SAMPLING_BYTES * SAMPLING_CHANNELS)
#define AS_INTERVAL		1
#define MAX_STRING_INDEX	2
#define INPUT_TERMINAL_ID	1
#define FEATURE_UNIT_ID		2
#define OUTPUT_TERMINAL_ID	3
#define MIN_VOLUME		(-62 * 256)
#define MAX_VOLUME		(0 * 256)
#define RES_VOLUME		(1 * 256)
#define DEFAULT_VOLUME		(-10 * 256)
#define MUTE_VOLUME		(-63 * 256)

enum control_stage {
	CONTROL_STAGE_SETUP,
	CONTROL_STAGE_DATA,
	CONTROL_STAGE_STATUS
};

void ds1881_write(const char *buf, int nbyte);
void command(void);
void control_setup(void);
void control_rx(void);
void control_tx(void);
void control_reset(void);
