/*
 * si4737.c
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

#include <stdio.h>
#include <stdbool.h>

#include <gpio.h>
#include <i2c.h>
#include <exti.h>
#include <spi.h>

#include "usb_radio.h"
#include "si4737.h"

extern volatile bool si4737_int;
int i2c_status;
int si4737_status;
static int frequency;

static int put_command(char *cmd, int len)
{
	int i;
	int s;
	
	i2c_start(I2C1, I2C_AUTOEND | SI4737_SLA, len);
	for (i = 0; i < len; i++) {
		i2c_send_blocking(I2C1, *(cmd + i));
		s = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					     I2C_NACKF);
		if (s) {
			i2c_status |= s;
			gpio_clear(GPIO_PB1);
			printf("I2C error: 0x%08x\n", s);
			return s;
		}
	}
	while (!i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					 I2C_NACKF | I2C_STOPF))
		;
	s = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR | I2C_NACKF);
	if (s) {
		i2c_status |= s;
		gpio_clear(GPIO_PB1);
		printf("I2C error: 0x%08x\n", s);
		return s;
	}
	i2c_clear_interrupt(I2C1, I2C_STOPF);
	return 0;
}

static int get_response(char *resp, int len)
{
	int i;
	int r;
	int s;
	
	while (!si4737_int)
		;
	si4737_int = false;
	i2c_start(I2C1, I2C_AUTOEND | I2C_READ | SI4737_SLA, len);
	for (i = 0; i < len; i++) {
		r = i2c_recv_blocking(I2C1);
		if (resp)
			*(resp + i) = r;
		s = i2c_get_interrupt_status(I2C1, I2C_ARLO | I2C_BERR |
					     I2C_NACKF);
		if (s) {
			i2c_status |= s;
			gpio_clear(GPIO_PB1);
			printf("I2C error: 0x%08x\n", s);
			return s;
		}
		if (i == 0) {
			si4737_status = r;
			if ((r & SI4737_STATUS_ERR) ||
			    !(r & SI4737_STATUS_CTS)) {
				gpio_clear(GPIO_PB1);
				printf("Si4737 error: 0x%02x\n", r);
				return -1;
			}
		}
	}
	while (!i2c_get_interrupt_status(I2C1, I2C_BERR | I2C_STOPF))
		;
	s = i2c_get_interrupt_status(I2C1, I2C_BERR);
	if (s) {
		i2c_status |= s;
		gpio_clear(GPIO_PB1);
		printf("I2C error: 0x%08x\n", s);
		return s;
	}
	i2c_clear_interrupt(I2C1, I2C_STOPF);
	return 0;
}

static int power_up(int func, int opmode)
{
	static char cmd[] = {
		SI4737_POWER_UP,
		SI4737_POWER_UP_CTSIEN | SI4737_POWER_UP_GPO2OEN,
		0
	};
	
	cmd[1] |= func;
	cmd[2] = opmode;
	if (put_command(cmd, sizeof(cmd)))
		return -1;
	while (!gpio_get(GPIO_PA4))
		;
	exti_clear_interrupt(EXTI4);
	exti_enable_interrupt(EXTI4);
	if (get_response(NULL, 1))
		return -1;
	gpio_config(GPIO_AF0, 0, GPIO_PA8); /* MCO */
	delay_ms(500);		/* ? */
	return 0;
}

static int get_rev(char *revision)
{
	static const char cmd = SI4737_GET_REV;

	if (put_command((char *)&cmd, 1))
		return -1;
	if (get_response(revision, 9))
		return -1;
	return 0;
}

static int set_property(int property, int value)
{
	static char cmd[] = {
		SI4737_SET_PROPERTY, 0,
		0, 0,
		0, 0
	};

	cmd[2] = (property >> 8) & 0xff;
	cmd[3] = property & 0xff;
	cmd[4] = (value >> 8) & 0xff;
	cmd[5] = value & 0xff;
	if (put_command(cmd, sizeof(cmd)))
		return -1;
	if (get_response(NULL, 1))
		return -1;
	delay_ms(10);
	return 0;
}

static int fm_tune_freq(int freq)
{
	static char cmd[] = {
		SI4737_FM_TUNE_FREQ, 0,
		0, 0,
		0
	};
	static const char cmd1 = SI4737_GET_INT_STATUS;
	char r;
	
	cmd[2] = (freq >> 8) & 0xff;
	cmd[3] = freq & 0xff;
	if (put_command(cmd, sizeof(cmd)))
		return -1;
	if (get_response(NULL, 1))
		return -1;
	while (!si4737_int)
		;
	si4737_int = false;
	if (put_command((char *)&cmd1, 1))
		return -1;
	if (get_response(&r, 1))
		return -1;
	if (!(r & SI4737_STATUS_STCINT)) {
		puts("Si4737: STCINT status is not set.");
		gpio_clear(GPIO_PB1);
		return -1;
	}
	return 0;
}

static int fm_tune_status(char *status)
{
	static const char cmd[] = {
		SI4737_FM_TUNE_STATUS, SI4737_FM_TUNE_STATUS_INTACK
	};

	if (put_command((char *)cmd, sizeof(cmd)))
		return -1;
	if (get_response(status, 8))
		return -1;
	return 0;
}

static int fm_rsq_status(char *status)
{
	static const char cmd[] = {
		SI4737_FM_RSQ_STATUS, 0
	};

	if (put_command((char *)cmd, sizeof(cmd)))
		return -1;
	if (get_response(status, 8))
		return -1;
	return 0;
}

int si4737_init(int freq, bool digital)
{
	char r[16];
	
	delay_ms(1);		/* 250 us */
	gpio_set(GPIO_PA1);
	delay_ms(1);		/* 1 us */
	if (power_up(SI4737_POWER_UP_FUNC_FM_RECEIVE,
		     digital ? SI4737_POWER_UP_OPMODE_DIGITAL :
		     SI4737_POWER_UP_OPMODE_ANALOG))
		return -1;
	if (set_property(SI4737_GPO_IEN, SI4737_GPO_IEN_CTSIEN |
			 SI4737_GPO_IEN_STCIEN))
		return -1;

	/* Japan */
	if (set_property(SI4737_FM_DEEMPHASIS, SI4737_FM_DEEMPHASIS_50))
		return -1;

	if (digital) {
		if (set_property(SI4737_DIGITAL_OUTPUT_FORMAT,
				 SI4737_DIGITAL_OUTPUT_FORMAT_OMODE_FIRST |
				 SI4737_DIGITAL_OUTPUT_FORMAT_OSIZE_16))
			return -1;
		gpio_config(GPIO_AF5, GPIO_2MHZ, GPIO_PA6);
		gpio_config(GPIO_AF0, GPIO_2MHZ, GPIO_PA3);
		spi_enable(SPI1);
	}
	if (fm_tune_freq(freq))
		return -1;
	if (fm_tune_status(r))
		return -1;
	frequency = freq;
	return 0;
}

int si4737_revision(void)
{
	char r[16];

	if (get_rev(r))
		return -1;
	printf("Si47%02d-%c%c%c ", (unsigned char)r[1], r[8], r[2], r[3]);
	printf("FMRX %c.%c\n", r[6], r[7]);
	return 0;
}

int si4737_tune(int freq)
{
	char r[16];
	int f;

	if (fm_tune_freq(freq))
		return -1;
	if (fm_tune_status(r))
		return -1;
	f = (unsigned char)r[2] << 8 | (unsigned char)r[3];
	printf("%3d.%02d MHz  %3d dBuV\n", f / 100, f % 100, r[4]);
	frequency = freq;
	return 0;
}

int si4737_rsq(void)
{
	char r[16];

	if (fm_rsq_status(r))
		return -1;
	printf("%3d.%02d MHz  ", frequency / 100, frequency % 100);
	if (r[2] & 8)
		printf("<Soft Mute>");
	if (r[2] & 2)
		printf("<AFC Rail>");
	if (r[3] & 0x80)
		printf("<Pilot>");
	printf("\n");
	printf("Stereo Blend %3d%%, ", r[3] & 0x7f);
	printf("RSSI %3d dBuV, ", r[4]);	
	printf("SNR %3d dB, ", r[5]);	
	printf("Offset %+4d kHz\n", r[7]);
	return 0;
}

int si4737_enable_digital_output(void)
{
	if (set_property(SI4737_DIGITAL_OUTPUT_SAMPLE_RATE, SAMPLING_FREQ))
		return -1;
	return 0;
}
