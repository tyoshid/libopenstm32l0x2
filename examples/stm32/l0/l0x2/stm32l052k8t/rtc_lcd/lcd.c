/*
 * lcd.c - Liquid Crystal Display (LCD) driver
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

#include <gpio.h>

#include "rtc_lcd.h"

/* ADM1602K-NSA-FBS/3.3V (SparkFun LCD-09054) */
/* Sitronix ST7066 Dot Matrix LCD Controller/Driver */
#define LCD_MAXX		16
#define LCD_MAXY		2

/* ST7066 (HD44780) instructions */
#define LCD_CMD_CLEAR_DISPLAY	0x01

#define LCD_CMD_RETURN_HOME	0x02

#define LCD_CMD_ENTRY_MODE_SET	0x04
#define LCD_ENTRY_I_D		0x02
#define LCD_ENTRY_S		0x01

#define LCD_CMD_DISPLAY_CONTROL	0x08
#define LCD_DISPLAY_D		0x04
#define LCD_DISPLAY_C		0x02
#define LCD_DISPLAY_B		0x01

#define LCD_CMD_CURSOR_SHIFT	0x10
#define LCD_CURSOR_S_C		0x08
#define LCD_CURSOR_R_L		0x04

#define LCD_CMD_FUNCTION_SET	0x20
#define LCD_FUNCTION_DL		0x10
#define LCD_FUNCTION_N		0x08
#define LCD_FUNCTION_F		0x04

#define LCD_CMD_SET_CGRAM_ADDRESS	0x40

#define LCD_CMD_SET_DDRAM_ADDRESS	0x80

/* Execution time (usec) */
#define LCD_TIME_CMD_LONG	1520
#define LCD_TIME_CMD		37
#define LCD_TIME_DATA		43

/* Register select signal (PA2) */
#define LCD_RS_CMD		0
#define LCD_RS_DATA		4

/* LCD interface */
/*
 * GPIO PA 6   5   4   3 2   1  0
 *      |DB7|DB6|DB5|DB4|RS|R/W|E|
 */

/*
 * Write Operation
 * tsu1: address setup time (100 ns)
 * tw: enable pulse width  (300 ns)
 * tsu2: data setup time  (60 ns)
 * th1: address hold time (10 ns)
 * th2: data hold time (10 ns)
 * tc: enable cycle time (500 ns)
 * tc - tw = 500 - 300 = 200 ns
 */
static void lcd_write_4bit(int rs, int data)
{
	gpio_write_port(GPIOA, 0x78 | rs);
	gpio_write_port(GPIOA, (data & 0xf) << 3 | 1 | rs);
	gpio_write_port(GPIOA, (data & 0xf) << 3 | rs);
	gpio_write_port(GPIOA, 0x7e);
}

/* Command Write */
static void lcd_write_command(int command)
{
	lcd_write_4bit(LCD_RS_CMD, command >> 4);
	lcd_write_4bit(LCD_RS_CMD, command);
	if (command == LCD_CMD_CLEAR_DISPLAY || command == LCD_CMD_RETURN_HOME)
		delay_us(LCD_TIME_CMD_LONG);
	else
		delay_us(LCD_TIME_CMD);
}

/* Data Write */
static void lcd_write_data(int data)
{
	lcd_write_4bit(LCD_RS_DATA, data >> 4);
	lcd_write_4bit(LCD_RS_DATA, data);
	delay_us(LCD_TIME_DATA);
}

/* Initializing by Instruction */
void lcd_init(void)
{
	delay_ms(40);
	lcd_write_4bit(LCD_RS_CMD, 3);
	delay_ms(5);
	lcd_write_4bit(LCD_RS_CMD, 3);
	delay_us(100);
	lcd_write_4bit(LCD_RS_CMD, 3);
	delay_us(LCD_TIME_CMD);
	lcd_write_4bit(LCD_RS_CMD, 2);
	delay_us(LCD_TIME_CMD);
	lcd_write_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_N);
	lcd_write_command(LCD_CMD_DISPLAY_CONTROL);
	lcd_write_command(LCD_CMD_CLEAR_DISPLAY);
	lcd_write_command(LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_I_D);
	lcd_write_command(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_D);
}

/* Set cursor position */
static void lcd_set_cursor(int y, int x)
{
	lcd_write_command(LCD_CMD_SET_DDRAM_ADDRESS | ((y & 1) ? 0x40 : 0) |
			  ((y >> 1) * LCD_MAXX + x));
}

void lcd_date(int date)
{
	int i;
	int j;
	extern const char week_day_name[8][4];
	extern const char month_name[13][4];
	
	lcd_set_cursor(0, 0);

	i = (date >> 13) & 7;
	for (j = 0; j < 3; j++)
		lcd_write_data(week_day_name[i][j]);
	lcd_write_data(' ');
	
	i = ((date & (1 << 12)) ? 10 : 0) + ((date >> 8) & 15);
	for (j = 0; j < 3; j++)
		lcd_write_data(month_name[i][j]);
	lcd_write_data(' ');

	i = ((date >> 4) & 3) + '0';
	lcd_write_data(i);
	i = (date & 15) + '0';
	lcd_write_data(i);
	lcd_write_data(' ');

	lcd_write_data('2');
	lcd_write_data('0');
	i = ((date >> 20) & 15) + '0';
	lcd_write_data(i);
	i = ((date >> 16) & 15) + '0';
	lcd_write_data(i);
}

void lcd_time(int time)
{
	int i;

	lcd_set_cursor(1, 0);

	lcd_write_data(' ');
	lcd_write_data(' ');
	lcd_write_data(' ');

	i = ((time >> 20) & 3) + '0';
	lcd_write_data(i);
	i = ((time >> 16) & 15) + '0';
	lcd_write_data(i);
	lcd_write_data(':');

	i = ((time >> 12) & 7) + '0';
	lcd_write_data(i);
	i = ((time >> 8) & 15) + '0';
	lcd_write_data(i);
	lcd_write_data(':');

	i = ((time >> 4) & 7) + '0';
	lcd_write_data(i);
	i = (time & 15) + '0';
	lcd_write_data(i);
}
