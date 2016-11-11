/*
 * camera.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of ethernet_camera.
 *
 * ethernet_camera is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ethernet_camera is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ethernet_camera.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>
#include <stdbool.h>

#include <usart.h>

#include "ethernet_camera.h"

/*
 * adafruit
 * TTL Serial JPEG Camera with NTSC Video #397
 *
 * PTC08 serial camera module
 *
 * VC0706 (VC0703 ?)
 * Digital Video Processor
 */

/* Command */
#define CAMERA_GEN_VERSION		0x11
#define CAMERA_SET_SERIAL_NUMBER	0x21
#define CAMERA_SET_PORT			0x24
#define CAMERA_SYSTEM_RESET		0x26
#define CAMERA_READ_DATA		0x30
#define CAMERA_WRITE_DATA		0x31
#define CAMERA_READ_FBUF		0x32
#define CAMERA_WRITE_FBUF		0x33
#define CAMERA_GET_FBUF_LEN		0x34
#define CAMERA_SET_FBUF_LEN		0x35
#define CAMERA_FBUF_CTRL		0x36
#define CAMERA_COMM_MOTION_CTRL		0x37
#define CAMERA_COMM_MOTION_STATUS	0x38
#define CAMERA_COMM_MOTION_DETECTED	0x39
#define CAMERA_MIRROR_CTRL		0x3a
#define CAMERA_MIRROR_STATUS		0x3b
#define CAMERA_COLOR_CTRL		0x3c
#define CAMERA_COLOR_STATUS		0x3d
#define CAMERA_POWER_SAVE_CTRL		0x3e
#define CAMERA_POWER_SAVE_STATUS	0x3f
#define CAMERA_AE_CTRL			0x40
#define CAMERA_AE_STATUS		0x41
#define CAMERA_MOTION_CTRL		0x42
#define CAMERA_MOTION_STATUS		0x43
#define CAMERA_TV_OUT_CTRL		0x44
#define CAMERA_OSD_ADD_CHAR		0x45
#define CAMERA_DOWNSIZE_CTRL		0x54
#define CAMERA_DOWNSIZE_STATUS		0x55
#define CAMERA_GET_FLASH_SIZE		0x60
#define CAMERA_ERASE_FLASH_SECTOR	0x61
#define CAMERA_ERASE_FLASH_ALL		0x62
#define CAMERA_READ_LOGO		0x70
#define CAMERA_SET_BITMAP		0x71
#define CAMERA_BATCH_WRITE		0x80

/* Status */
#define CAMERA_STATUS_OK		0
#define CAMERA_STATUS_RECEIVE_ERROR	1
#define CAMERA_STATUS_DATA_LENGTH_ERROR	2
#define CAMERA_STATUS_DATA_FORMAT_ERROR	3
#define CAMERA_STATUS_CAN_NOT_EXECUTE	4
#define CAMERA_STATUS_EXEC_ERROR	5
#define CAMERA_STATUS_INVALID_RETURN	10
#define CAMERA_STATUS_TIMEOUT		11

#define CAMERA_SERIAL_NUMBER		0

/* FBUF_CTRL: control flag */
#define CAMERA_STOP_CURRENT_FRAME	0
#define CAMERA_STOP_NEXT_FRAME		1
#define CAMERA_RESUME_FRAME		3 /* ??? */

/* GET_FBUF_LEN: FBUF type */
/* READ_FBUF: FBUF type */
#define CAMERA_CURRENT_FRAME		0
#define CAMERA_NEXT_FRAME		1

#define CAMERA_COMPRESSION_DEFAULT	0x36
#define CAMERA_T_POWERUP		2500 /* Power-up time: 2500 ms */
#define TIMEOUT				1000000
#define BUFSIZE				32

enum camera_image_size {
	CAMERA_640X480 = 0x00,
	CAMERA_320X240 = 0x11,
	CAMERA_160X120 = 0x22
};

/*
 * Communication Protocol
 *
 * Receive command format:
 * 'V' + {Serial number} + {Command} + {Data length} + {Data (0 - 16 bytes)}
 *
 * Return command format:
 * 'v' + {Serial number} + {Command} + {Status} + {Data length} +
 * {Data (0 - 16 bytes)}
 */

static char ret_buf[21];

static void camera_send_command(const char *command)
{
	int i;
	
	for (i = 0; i < *(command + 3) + 4; i++)
		usart_send_blocking(USART2, *(command + i));
}

static int camera_recv(void)
{
	int i;
	
	for (i = 0; i < TIMEOUT; i++) {
		if (usart_get_interrupt_status(USART2, USART_RXNE)) 
			break;
	}
	if (i >= TIMEOUT)
		return -1;
	return usart_recv(USART2);
}

static int camera_recv_return(const char *command)
{
	int r;
	int i;
	
	r = camera_recv();
	if (r < 0)
		return -CAMERA_STATUS_TIMEOUT;
	ret_buf[0] = r;
	if (ret_buf[0] != 'v')
		return -CAMERA_STATUS_INVALID_RETURN;

	r = camera_recv();
	if (r < 0)
		return -CAMERA_STATUS_TIMEOUT;
	ret_buf[1] = r;
	if (ret_buf[1] != *(command + 1))
		return -CAMERA_STATUS_INVALID_RETURN;

	r = camera_recv();
	if (r < 0)
		return -CAMERA_STATUS_TIMEOUT;
	ret_buf[2] = r;
	if (ret_buf[2] != *(command + 2))
		return -CAMERA_STATUS_INVALID_RETURN;

	r = camera_recv();
	if (r < 0)
		return -CAMERA_STATUS_TIMEOUT;
	ret_buf[3] = r;

	r = camera_recv();
	if (r < 0)
		return -CAMERA_STATUS_TIMEOUT;
	ret_buf[4] = r;
	if (ret_buf[4] < 0 || ret_buf[4] > 16)
		return -CAMERA_STATUS_INVALID_RETURN;

	for (i = 0; i < ret_buf[4]; i++) {
		r = camera_recv();
		if (r < 0)
			return -CAMERA_STATUS_TIMEOUT;
		ret_buf[5 + i] = r;
	}

	return -ret_buf[3];
}

static int camera_reset(void)
{
	int s;
	int r;
	int i;
	static char buf[BUFSIZE];
	static const char system_reset[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_SYSTEM_RESET,
		0
	};

	camera_send_command(system_reset);
	s = camera_recv_return(system_reset);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;

	i = 0;
	while (1) {
		r = camera_recv();
		if (r < 0)
			return -CAMERA_STATUS_TIMEOUT;
		if (i < BUFSIZE - 1)
			buf[i++] = r;
		if (r == '\n') {
			buf[i] = '\0';
			if (strcmp(buf, "Init end\r\n") == 0)
				break;
			i = 0;
		}
	}
	return 0;
}

static int camera_get_image_size(void)
{
	int s;
	static char get_image_size[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_READ_DATA,
		4,
		4,			/* I2C EEPROM */
		1,			/* Bytes */
		0x00, 0x19		/* Register address */
	};

	camera_send_command(get_image_size);
	s = camera_recv_return(get_image_size);
	if (s)
		return s;
	if (ret_buf[4] != 1)
		return -CAMERA_STATUS_INVALID_RETURN;
	return ret_buf[5];
}

static int camera_set_image_size(enum camera_image_size size)
{
	int s;
	static char set_image_size[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_WRITE_DATA,
		5,
		4,			/* I2C EEPROM */
		1,			/* Bytes */
		0x00, 0x19,		/* Register address */
		0x00			/* Data */
	};

	set_image_size[8] = size;
	camera_send_command(set_image_size);
	s = camera_recv_return(set_image_size);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;
	return 0;
}

static int camera_set_compression(int factor)
{
	int s;
	static char set_compression[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_WRITE_DATA,
		5,
		1,			/* Chip register */
		1,			/* Bytes */
		0x12, 0x04,		/* Register address */
		0x36			/* Data */
	};

	set_compression[8] = factor;
	camera_send_command(set_compression);
	s = camera_recv_return(set_compression);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;
	return 0;
}

int camera_fbuf_ctrl_stop(void)
{
	int s;
	static const char fbuf_ctrl_stop[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_FBUF_CTRL,
		1,
		CAMERA_STOP_CURRENT_FRAME
	};

	camera_send_command(fbuf_ctrl_stop);
	s = camera_recv_return(fbuf_ctrl_stop);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;
	return 0;
}

int camera_fbuf_ctrl_resume(void)
{
	int s;
	static const char fbuf_ctrl_resume[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_FBUF_CTRL,
		1,
		CAMERA_RESUME_FRAME
	};
	
	camera_send_command(fbuf_ctrl_resume);
	s = camera_recv_return(fbuf_ctrl_resume);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;
	return 0;
}

int camera_get_fbuf_len(void)
{
	int s;
	static const char get_fbuf_len[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_GET_FBUF_LEN,
		1,
		CAMERA_CURRENT_FRAME
	};

	camera_send_command(get_fbuf_len);
	s = camera_recv_return(get_fbuf_len);
	if (s)
		return s;
	if (ret_buf[4] != 4)
		return -CAMERA_STATUS_INVALID_RETURN;
	return ret_buf[5] << 24 | (ret_buf[6] & 0xff) << 16 |
		(ret_buf[7] & 0xff) << 8 | (ret_buf[8] & 0xff);
}

int camera_read_fbuf(int addr, int nbyte, char *buf)
{
	int s;
	int i;
	int r;
	static char read_fbuf[] = {
		'V',
		CAMERA_SERIAL_NUMBER,
		CAMERA_READ_FBUF,
		12,
		CAMERA_CURRENT_FRAME,
		0x0a,		    /* ??? */
		0, 0, 0, 0,	    /* Starting addresss */
		0, 0, 0, 0,	    /* Data length */
		0x00, 0x0a	    /* Delay (10) */
	};

	read_fbuf[6] = addr >> 24;
	read_fbuf[7] = addr >> 16;
	read_fbuf[8] = addr >> 8;
	read_fbuf[9] = addr;
	read_fbuf[10] = nbyte >> 24;
	read_fbuf[11] = nbyte >> 16;
	read_fbuf[12] = nbyte >> 8;
	read_fbuf[13] = nbyte;
	camera_send_command(read_fbuf);
	s = camera_recv_return(read_fbuf);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;
	for (i = 0; i < nbyte; i++) {
		r = camera_recv();
		if (r < 0)
			return -CAMERA_STATUS_TIMEOUT;
		*buf++ = r;
	}
	s = camera_recv_return(read_fbuf);
	if (s)
		return s;
	if (ret_buf[4])
		return -CAMERA_STATUS_INVALID_RETURN;
	return nbyte;
}

int camera_init(void)
{
	int s;

	delay_ms(CAMERA_T_POWERUP);
	s = camera_get_image_size();
	if (s < 0)
		return s;
	if (s != CAMERA_640X480) {
		s = camera_set_image_size(CAMERA_640X480);
		if (s)
			return s;
	}
	s = camera_reset();
	if (s)
		return s;
	return camera_set_compression(CAMERA_COMPRESSION_DEFAULT);
}
