/*
 * command.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of spi_sd.
 *
 * spi_sd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * spi_sd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with spi_sd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>

#include "spi_sd.h"

#define DELIMITER	" /.,;:-"
#define MAXTOKEN	4
#define BUFSIZE		32

/* Disk parameter */
extern unsigned int boot_sector;
extern unsigned int fsinfo;
extern unsigned int root_dir;
extern unsigned int reserved_sector;
extern unsigned int total_sector;
extern unsigned int fat_size;
extern unsigned int fat_num;
extern int cluster_size;

void dump(unsigned char *d, int size)
{
	int n;
	int a;
	int i;

	a = 0;
	while (size > 0) {
		if (size > 16)
			n = 16;
		else
			n = size;
		printf("%04x ", a);
		for (i = 0; i < n; i++)
			printf("%02x ", *(d + i));
		for (i = n; i < 17; i++)
			printf("   ");
		for (i = 0; i < n; i++)
			printf("%c", (*(d + i) < ' ' || *(d + i) > '~') ? '.' :
			       *(d + i));
		printf("\n");
		a += n;
		size -= n;
		d += n;
	}
}

static int sector(int n, char **p, unsigned int *s)
{
	char *endp;
	
	if (n != 2) {
		puts("Usage: sector <sector_num>");
		return -1;
	}

	*s = strtoul(p[1], &endp, 10);
	if (*endp != '\0' || *s >= total_sector) {
		printf("Illegal logical sector number: %s (0 - %u)\n", p[1],
		       total_sector - 1);
		return -1;
	}

	return 0;
}

static int cluster(int n, char **p, unsigned int *s)
{
	unsigned int c;
	char *endp;

	if (n != 2) {
		puts("Usage: cluster <cluster_num>");
		return -1;
	}

	c = strtoul(p[1], &endp, 10);
	if (*endp != '\0' || c < 2 ||
	    c > (total_sector - reserved_sector - fat_size * fat_num) /
	    cluster_size + 1) {
		printf("Illegal logical cluster number: %s (2 - %u)\n", p[1],
		       (total_sector - reserved_sector - fat_size * fat_num) /
		       cluster_size + 1);
		return -1;
	}

	*s = reserved_sector + fat_size * fat_num + (c - 2) * cluster_size;
	printf("Logical sector number = %u - %u\n", *s, *s + cluster_size - 1);
	return 0;
}

static void help(void)
{
	puts("Usage:");
	puts("  sector <sector_num>        Dump the logical sector");
	puts("  cluster <cluster_num>      Dump the first sector");
	puts("  next                       Dump the next sector");
	puts("  help                       Print this message");
}

void get_line(char buf[])
{
	int c;
	int i;
	
	i = 0;
	do {
		c = getchar();
		if (isprint(c)) {
			putchar(c);
			if (i < BUFSIZE - 1)
				buf[i++] = c;
		} else {
			switch (c) {
			case '\b':
				if (i > 0) {
					printf("\b \b");
					i--;
				}
				break;
			case '\n':
				putchar('\n');
				buf[i] = '\0';
				break;
			default:
				break;
			}
		}
	} while (c != '\n');
}

void command(void)
{
	unsigned int s;
	static char buf[BUFSIZE];
	int i;
	static char *p[MAXTOKEN];
	int n;
	static unsigned char d[SECTOR_SIZE + 2];
	
	puts("spi_sd");
	if (sd_init())
		return;
	if (sd_read_sector(0, d))
		return;
	if (proc_mbr(d))
		return;
	if (sd_read_sector(boot_sector, d))
		return;
	if (proc_vbr(d))
		return;
	if (fat(d))
		return;
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);
	s = 0;
	while (1) {
		printf("spi_sd> ");
		get_line(buf);
		for (i = 0; i < MAXTOKEN; i++) {
			p[i] = strtok(i ? NULL : buf, DELIMITER);
			if (p[i] == NULL)
				break;
		}
		n = i;
		if (n == 0)
			continue;
		if (strncasecmp(p[0], "sector", strlen(p[0])) == 0) {
			if (sector(n, p, &s) == 0) {
				if (sd_read_sector(boot_sector + s, d))
					return;
				dump(d, SECTOR_SIZE);
			}
		} else if (strncasecmp(p[0], "cluster", strlen(p[0])) == 0) {
			if (cluster(n, p, &s) == 0) {
				if (sd_read_sector(boot_sector + s, d))
					return;
				dump(d, SECTOR_SIZE);
			}
		} else if (strncasecmp(p[0], "next", strlen(p[0])) == 0) {
			if (n == 1) {
				s++;
				if (s > total_sector)
					s = 0;
				printf("Logical sector number: %u\n", s);
				if (sd_read_sector(boot_sector + s, d))
					return;
				dump(d, SECTOR_SIZE);
			} else {
				puts("Usage: next");
			}
		} else if (strncasecmp(p[0], "help", strlen(p[0])) == 0 ||
			   strcmp(p[0], "?") == 0) {
			help();
		} else {
			puts("unknown command");
		}
	}
}
