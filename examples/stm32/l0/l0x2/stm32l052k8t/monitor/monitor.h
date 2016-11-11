/*
 * monitor.h
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of monitor.
 *
 * monitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * monitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with monitor.  If not, see <http://www.gnu.org/licenses/>.
 */

#define VERSION		"0.1"	/* Software version number */
#define MAXARG		16	/* Argument number */
#define NHIST		8	/* History size */
#define BUFSIZE		64	/* Line buffer size */
#define DELIMITER	" ,()"	/* Separator characters */

struct functab {
	char *prefix;
	int (*f)(int argc, char *argv[]);
};

void command(void);
int indirection(void);
int function(char *p);
