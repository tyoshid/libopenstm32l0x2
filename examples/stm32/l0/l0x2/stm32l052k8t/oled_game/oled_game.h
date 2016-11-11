/*
 * oled_game.h
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of oled_game.
 *
 * oled_game is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * oled_game is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with oled_game.  If not, see <http://www.gnu.org/licenses/>.
 */

#define MAXX				64
#define MAXY				48
#define REFRATE				60
#define FSOUND				480
#define NWAVE				(FSOUND / REFRATE)		    
#define WAVEDATA			64
#define MAXDATA				(WAVEDATA * NWAVE)

void oled_init(void);
void oled_point(int x, int y, int color);
void oled_line_y(int x, int y, int length, int color);
void oled_string(int page, int column, char *str);
void sound_init(void);
void sound_restart(void);
void sound_beep(bool g);
void sound_pop(void);
void sound_stop(void);
void sound_make_data(void);
void game_init(void);
void game(void);
void delay_us(int us);
