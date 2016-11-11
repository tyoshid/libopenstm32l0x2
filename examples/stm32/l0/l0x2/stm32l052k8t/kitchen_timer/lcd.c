/*
 * lcd.c
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

#include <gpio.h>
#include <spi.h>

#include "kitchen_timer.h"

/*
 * AQM1248A-RN
 * Graphic LCD (128x48)
 *
 * /CS - PA1, RS - PA4, /RESET - PA6
 */
/*
 * SITRONIX ST7565R
 * 65 x 132 Dot Matrix LCD Controller/Driver
 * datasheet Ver 1.5  2006/03/10
 */
/* Command */
#define LCD_DISPLAY_OFF			0xae
#define LCD_DISPLAY_ON			0xaf
#define LCD_DISPLAY_START_LINE_SET	0x40
#define LCD_PAGE_ADDRESS_SET		0xb0
#define LCD_COLUMN_ADDRESS_SET_UPPER	0x10
#define LCD_COLUMN_ADDRESS_SET_LOWER	0x00
#define LCD_ADC_NORMAL			0xa0
#define LCD_ADC_REVERSE			0xa1
#define LCD_DISPLAY_NORMAL		0xa6
#define LCD_DISPLAY_REVERSE		0xa7
#define LCD_DISPLAY_ALL_POINTS_NORMAL	0xa4
#define LCD_DISPLAY_ALL_POINTS_ON	0xa5
#define LCD_BIAS_SET_1_9		0xa2
#define LCD_BIAS_SET_1_7		0xa3
#define LCD_RESET			0xe2
#define LCD_COMMON_OUTPUT_NORMAL	0xc0
#define LCD_COMMON_OUTPUT_REVERSE	0xc8
#define LCD_POWER_CONTROL_SET		0x28
#define LCD_INTERNAL_RESISTOR_RATIO_SET	0x20
#define LCD_ELECTRONIC_VOLUME_MODE_SET	0x81
#define LCD_STATIC_INDICATOR_OFF	0xac
#define LCD_STATIC_INDICATOR_ON		0xad
#define LCD_BOOSTER_RATIO_SET		0xf8
#define LCD_NOP				0xe3

/* Power Control Set */
#define LCD_BOOSTER			(1 << 2)
#define LCD_VOLTAGE_REGULATOR		(1 << 1)
#define LCD_VOLTAGE_FOLLOWER		(1 << 0)

/* Static Indicator */
#define LCD_INDICATOR_OFF		0
#define LCD_INDICATOR_BLINK_1		1
#define LCD_INDICATOR_BLINK_0_5		2
#define LCD_INDICATOR_ON		3

#define LCD_TPOWERON	5 /* Power up time: 5 ms */
#define LCD_TRW		1 /* Reset "L" pulse width: 1 ms (1.0 us.min) */
#define LCD_TR		1 /* Reset time: 1 ms (1.0 us.max) */

void lcd_init(void)
{
	int i;
	int j;
	
	delay_ms(LCD_TPOWERON);
	gpio_clear(GPIO_PA6);
	delay_ms(LCD_TRW);
	gpio_set(GPIO_PA6);
	delay_ms(LCD_TR);

	gpio_clear(GPIO_PA1);
	gpio_clear(GPIO_PA4);
	spi_send_blocking(SPI1, LCD_DISPLAY_OFF);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_ADC_NORMAL);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_COMMON_OUTPUT_REVERSE);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_BIAS_SET_1_7);
	spi_recv_blocking(SPI1);

	spi_send_blocking(SPI1, LCD_POWER_CONTROL_SET | LCD_BOOSTER);
	spi_recv_blocking(SPI1);
	delay_ms(2);
	spi_send_blocking(SPI1, LCD_POWER_CONTROL_SET | LCD_BOOSTER |
			  LCD_VOLTAGE_REGULATOR);
	spi_recv_blocking(SPI1);
	delay_ms(2);
	spi_send_blocking(SPI1, LCD_POWER_CONTROL_SET | LCD_BOOSTER |
			  LCD_VOLTAGE_REGULATOR | LCD_VOLTAGE_FOLLOWER);
	spi_recv_blocking(SPI1);

	spi_send_blocking(SPI1, LCD_INTERNAL_RESISTOR_RATIO_SET | 3);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_ELECTRONIC_VOLUME_MODE_SET);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, 0x1c);	/* Electronic Volume Register Set */
	spi_recv_blocking(SPI1);

	spi_send_blocking(SPI1, LCD_DISPLAY_ALL_POINTS_NORMAL);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_DISPLAY_START_LINE_SET | 0);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_DISPLAY_NORMAL);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_DISPLAY_ON);
	spi_recv_blocking(SPI1);

	for (i = 0; i < 6; i++) {
		gpio_clear(GPIO_PA4);
		spi_send_blocking(SPI1, LCD_PAGE_ADDRESS_SET | i);
		spi_recv_blocking(SPI1);
		spi_send_blocking(SPI1, LCD_COLUMN_ADDRESS_SET_UPPER | 0);
		spi_recv_blocking(SPI1);
		spi_send_blocking(SPI1, LCD_COLUMN_ADDRESS_SET_LOWER | 0);
		spi_recv_blocking(SPI1);
		gpio_set(GPIO_PA4);
		for (j = 0; j < 128; j++) {
			spi_send_blocking(SPI1, 0);
			spi_recv_blocking(SPI1);
		}
	}
}

void lcd_write(int page, int column, const char *buf, int width, int height)
{
	int i;
	int j;
	
	for (i = 0; i < height / 8; i++) {
		gpio_clear(GPIO_PA4);
		spi_send_blocking(SPI1, LCD_PAGE_ADDRESS_SET | (page++ & 0xf));
		spi_recv_blocking(SPI1);
		spi_send_blocking(SPI1, LCD_COLUMN_ADDRESS_SET_UPPER |
				  ((column >> 4) & 0xf));
		spi_recv_blocking(SPI1);
		spi_send_blocking(SPI1, LCD_COLUMN_ADDRESS_SET_LOWER |
				  (column & 0xf));
		spi_recv_blocking(SPI1);
		gpio_set(GPIO_PA4);
		for (j = 0; j < width; j++) {
			spi_send_blocking(SPI1, *buf++);
			spi_recv_blocking(SPI1);
		}
	}
}

void lcd_sleep(void)
{
	gpio_clear(GPIO_PA4);
	spi_send_blocking(SPI1, LCD_DISPLAY_OFF);
	spi_recv_blocking(SPI1);
	spi_send_blocking(SPI1, LCD_DISPLAY_ALL_POINTS_ON);
	spi_recv_blocking(SPI1);
	gpio_set(GPIO_PA1);
}
