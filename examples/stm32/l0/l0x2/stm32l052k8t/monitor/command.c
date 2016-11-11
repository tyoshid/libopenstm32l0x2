/*
 * command.c
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
#include <ctype.h>
#include <string.h>

#include "monitor.h"

static int cp;			/* Character pointer */
static int lcp;			/* Last pointer */
static char history[NHIST][BUFSIZE]; /* History */
static int hlen[NHIST];
static int hwp;
static int hrp;
static char paste[BUFSIZE];	/* Paste buffer */
static int plen;

static void normal(char buf[], int c)
{
	int i;
	
	if (cp >= BUFSIZE - 1) {
		putchar('\a');
		return;
	}
	putchar(c);
	for (i = cp; i < lcp; i++) {
		if (i + 1 >= BUFSIZE - 1)
			break;
		putchar(buf[i]);
	}
	for (i = cp; i < lcp; i++) {
		if (i + 1 >= BUFSIZE - 1)
			break;
		putchar('\b');
	}
	for (i = lcp; i > cp; i--) {
		if (i < BUFSIZE - 1)
			buf[i] = buf[i - 1];
	}
	buf[cp++] = c;
	lcp++;
	if (lcp > BUFSIZE - 1)
		lcp = BUFSIZE - 1;
}

static void newline(char buf[])
{
	int i;
	
	putchar('\n');
	buf[lcp] = '\0';
	if (!lcp)
		return;
	if (hlen[hwp]) {
		hwp++;
		if (hwp >= NHIST)
			hwp = 0;
	}
	for (i = 0; i < lcp; i++)
		history[hwp][i] = buf[i];
	hlen[hwp] = lcp;
}

static void backspace(char buf[])
{
	int i;
	
	if (cp <= 0) {
		putchar('\a');
		return;
	}
	putchar('\b');
	for (i = cp; i < lcp; i++)
		putchar(buf[i]);
	printf(" \b");
	for (i = cp; i < lcp; i++)
		putchar('\b');
	for (i = cp; i < lcp; i++)
		buf[i - 1] = buf[i];
	cp--;
	lcp--;
}

static void ctrl_d(char buf[])
{
	int i;
	
	if (cp >= lcp) {
		putchar('\a');
		return;
	}
	for (i = cp + 1; i < lcp; i++)
		putchar(buf[i]);
	printf(" \b");
	for (i = cp + 1; i < lcp; i++)
		putchar('\b');
	for (i = cp + 1; i < lcp; i++)
		buf[i - 1] = buf[i];
	lcp--;
}

static void ctrl_b(void)
{
	if (cp <= 0) {
		putchar('\a');
		return;
	}
	putchar('\b');
	cp--;
}

static void ctrl_c(char buf[])
{
	putchar('\n');
	buf[0] = '\0';
	lcp = 0;
}

static void ctrl_e(char buf[])
{
	int i;
	
	if (cp >= lcp) {
		putchar('\a');
		return;
	}
	for (i = cp; i < lcp; i++)
		putchar(buf[i]);
	cp = lcp;
}

static void ctrl_f(char buf[])
{
	if (cp >= lcp) {
		putchar('\a');
		return;
	}
	putchar(buf[cp]);
	cp++;
}

static void ctrl_k(char buf[])
{
	int i;
	
	if (cp >= lcp) {
		putchar('\a');
		return;
	}
	for (i = cp; i < lcp; i++)
		putchar(' ');
	for (i = cp; i < lcp; i++)
		putchar('\b');
	for (i = cp; i < lcp; i++)
		paste[i - cp] = buf[i];
	plen = lcp - cp;
	lcp = cp;
}

static void ctrl_n(char buf[])
{
	int n;
	char *p;
	int i;
	
	if (hrp < 0) {
		putchar('\a');
		return;
	}
	if (hrp == hwp) {
		hrp = -1;
		n = 0;
		p = NULL;
	} else {
		if (++hrp >= NHIST)
			hrp = 0;
		n = hlen[hrp];
		p = history[hrp];
	}
	for (i = 0; i < cp; i++)
		putchar('\b');
	for (i = 0; i < n; i++)
		putchar(*(p + i));
	for (i = n; i < lcp; i++)
		putchar(' ');
	for (i = n; i < lcp; i++)
		putchar('\b');
	for (i = 0; i < n; i++)
		buf[i] = *(p + i);
	cp = lcp = n;
}

static void ctrl_p(char buf[])
{
	int i;
	
	if (hrp < 0) {
		if (!hlen[hwp]) {
			putchar('\a');
			return;
		}
	} else {
		i = hrp ? (hrp - 1) : (NHIST - 1);
		if (i == hwp || !hlen[i]) {
			putchar('\a');
			return;
		}
	}
	if (hrp < 0)
		hrp = hwp;
	else if (hrp == 0)
		hrp = NHIST - 1;
	else
		hrp--;
	for (i = 0; i < cp; i++)
		putchar('\b');
	for (i = 0; i < hlen[hrp]; i++)
		putchar(history[hrp][i]);
	for (i = hlen[hrp]; i < lcp; i++)
		putchar(' ');
	for (i = hlen[hrp]; i < lcp; i++)
		putchar('\b');
	for (i = 0; i < hlen[hrp]; i++)
		buf[i] = history[hrp][i];
	cp = lcp = hlen[hrp];
}

static void ctrl_y(char buf[])
{
	int i;

	if (cp >= BUFSIZE - 1) {
		putchar('\a');
		return;
	}
	for (i = 0; i < plen; i++) {
		if (cp + i >= BUFSIZE - 1)
			break;
		putchar(paste[i]);
	}
	for (i = cp; i < lcp; i++) {
		if (i + plen >= BUFSIZE - 1)
			break;
		putchar(buf[i]);
	}
	for (i = cp; i < lcp; i++) {
		if (i + plen >= BUFSIZE - 1)
			break;
		putchar('\b');
	}
	for (i = lcp; i > cp; i--) {
		if (i + plen - 1 < BUFSIZE - 1)
			buf[i + plen - 1] = buf[i - 1];
	}
	for (i = 0; i < plen; i++) {
		if (cp >= BUFSIZE - 1)
			break;
		buf[cp++] = paste[i];
	}
	lcp += plen;
	if (lcp > BUFSIZE - 1)
		lcp = BUFSIZE - 1;
}

static int get_line(char buf[])
{
	int c;
	
	cp = 0;
	lcp = 0;
	hrp = -1;
	do {
		c = getchar();
		if (isprint(c)) {
			normal(buf, c);
		} else {
			switch (c) {
			case '\n':
				newline(buf);
				break;
			case '\b':
				backspace(buf);
				break;
			case '\x7f':
			case 'D' - '@':
				ctrl_d(buf);
				break;
			case 'B' - '@':
				ctrl_b();
				break;
			case 'C' - '@':
				ctrl_c(buf);
				break;
			case 'E' - '@':
				ctrl_e(buf);
				break;
			case 'F' - '@':
				ctrl_f(buf);
				break;
			case 'K' - '@':
				ctrl_k(buf);
				break;
			case 'N' - '@':
				ctrl_n(buf);
				break;
			case 'P' - '@':
				ctrl_p(buf);
				break;
			case 'Y' - '@':
				ctrl_y(buf);
				break;
			default:
				break;
			}
		}
	} while (c != '\n' && c != ('C' - '@'));
	return lcp;
}

void command(void)
{
	static char buf[BUFSIZE];
	char *p;
	extern char end;

	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	printf("Libopenstm32l0x2 monitor version %s\n", VERSION);
	printf("BSS end = 0x%08x\n", (unsigned int)&end);
	while (1) {
		printf("mon> ");
		get_line(buf);
		p = strtok(buf, DELIMITER);
		if (p == NULL)
			continue;
		if (strcmp(p, "*") == 0)
			indirection();
		else
			function(p);
	}
}
