/*
 * function.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "monitor.h"

int function(char *p)
{
	char *argv[MAXARG];
	int i;
	int j;
	extern const struct functab ftab[];
	
	argv[0] = p;
	for (i = 1; i < MAXARG; i++) {
		argv[i] = strtok(NULL, DELIMITER);
		if (argv[i] == NULL)
			break;
	}
	for (j = 0; ftab[j].prefix; j++) {
		if (strncmp(p, ftab[j].prefix, strlen(ftab[j].prefix)) == 0)
			return (*ftab[j].f)(i, argv);
	}
	printf("Unkown function: %s\n", p);
	return -1;
}

int convert_arg(int argc, char *argv[], int a[])
{
	int i;
	char *endp;

	for (i = 1; i < MAXARG; i++)
		a[i] = 0;
	for (i = 1; i < argc; i++) {
		a[i] = strtoul(argv[i], &endp, 0);
		if (*endp != '\0') {
			printf("Invalid argument: %s\n", argv[i]);
			return -1;
		}
	}
	return 0;
}

int fname_cmp(char *s, const char *p[])
{
	int i;

	for (i = 0; p[i]; i++) {
		if (strcmp(s, p[i]) == 0)
			break;
	}
	return i;
}
