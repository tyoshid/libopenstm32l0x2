/*
 * rtc_lcd.h
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

/* RTC clock: 32.768 kHz / 128 / 256 = 1 Hz */
#define PREDIV_A		(128 - 1)
#define PREDIV_S		(256 - 1)

#define PROMPT			"rtc_lcd> "

void delay_us(int us);
void delay_ms(int ms);
void eeprom_write(int *addr, int data);
void lcd_init(void);
void lcd_date(int date);
void lcd_time(int time);
void mcp_write_wiper(int w);
void command(void);
