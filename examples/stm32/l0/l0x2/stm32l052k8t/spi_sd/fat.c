/*
 * fat.c
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
#include <stdint.h>
#include <stdbool.h>

#include "spi_sd.h"

/* Disk parameter */
extern unsigned int boot_sector;
extern unsigned int fsinfo;
extern unsigned int root_dir;
extern unsigned int reserved_sector;
extern unsigned int total_sector;
extern unsigned int fat_size;
extern unsigned int fat_num;
extern int cluster_size;

struct fat_boot_fsinfo {
	uint32_t signature1;
	uint32_t reserved1[120];
	uint32_t signature2;
	uint32_t free_clusters;
	uint32_t next_cluster;
	uint32_t reserved2[3];
	uint32_t signature3;
};

struct msdos_dir_entry {
	uint8_t name[11];
	uint8_t attr;
	uint8_t lcase;
	uint8_t ctime_cs;
	uint16_t ctime;
	uint16_t cdate;
	uint16_t adate;
	uint16_t starthi;
	uint16_t time, date, start;
	uint32_t size;
};

struct msdos_dir_slot {
	uint8_t id;
	uint8_t name0_4[10];
	uint8_t attr;
	uint8_t reserved;
	uint8_t alias_checksum;
	uint8_t name5_10[12];
	uint16_t start;
	uint8_t name11_12[4];
};

static const char month[16][4] = {
	"?",
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
	"?", "?", "?"
};

static int print_fsinfo(unsigned char *d)
{
	struct fat_boot_fsinfo *p;

	puts("FS Information Sector:");
	p = (struct fat_boot_fsinfo *)d;
	if (p->signature3 != 0xaa550000 ||
	    p->signature1 != 0x41615252 || p->signature2 != 0x61417272) {
		dump(d, SECTOR_SIZE);
		return -1;
	}
	printf("\tLast known number of free data clusters = ");
	if (p->free_clusters == 0xffffffff)
		puts("unknown");
	else
		printf("%u\n", (unsigned int)p->free_clusters);
	printf("\tNumber of the most recently known to be allocated "
	       "data cluster = ");
	if (p->next_cluster == 0xffffffff)
		puts("unknown");
	else
		printf("%u\n", (unsigned int)p->next_cluster);
	return 0;
}

static unsigned char lfn_checksum(const unsigned char *name)
{
	int i;
	unsigned char sum = 0;

	for (i = 0; i < 11; i++)
		sum = ((sum & 1) << 7) + (sum >> 1) + *name++;
	return sum;
}

/* UTF-16 -> UTF-8 */
static int lfn_nputws(uint8_t *p, int n)
{
	int r;
	int i;
	int w;

	r = 0;
	for (i = 0; i < n; i++) {
		w = *(p + 1) << 8 | *p;
		if (w == 0 || w == 0xffff)
			return r;
		if (w <= 0x7f)
			putchar(w);
		else if (w >= 0x0800)
			printf("%c%c%c", 0xe0 | w >> 12,
			       0x80 | ((w >> 6) & 0x3f),
			       0x80 | (w & 0x3f));
		else
			r = -1;
		p += 2;
	}
	return r;
}

static void lfn_dump(uint8_t *p, int n)
{
	int i;
	int w;

	for (i = 0; i < n; i++) {
		w = *(p + 1) << 8 | *p;
		if (w == 0 || w == 0xffff)
			return;
		printf("%04x ", w);
		p += 2;
	}
}

static int print_dir(unsigned char *d)
{
	int i;
	struct msdos_dir_entry *p;
	struct msdos_dir_slot *q;
	int j;
	int r;
	static struct msdos_dir_slot slot[20];
	static int maxslot;
	static int nextslot;
	static int sum;

	q = (struct msdos_dir_slot *)d;
	for (i = 0; i < SECTOR_SIZE / (int)sizeof(struct msdos_dir_entry);
	     i++) {
		if (q->id == 0) {
			if (maxslot) {
				puts("LFN sequence error");
				maxslot = 0;
			}
			return -1;
		}
		if (q->id == 0xe5) {
			if (maxslot) {
				puts("LFN (file deleted ?)");
				maxslot = 0;
			}
			q++;
			continue;
		}
		if (q->attr == 0x0f) {
			if (q->id & 0x40) {
				if ((q->id & ~0x40) <= 20) {
					if (maxslot)
						puts("LFN sequence error");
					maxslot = q->id & ~0x40;
					nextslot = maxslot - 1;
					slot[nextslot] = *q;
					sum = q->alias_checksum;
				} else {
					if (maxslot)
						maxslot = 0;
					puts("LFN sequence error");
				}
			} else {
				if (!maxslot || q->id != nextslot ||
				    sum != q->alias_checksum) {
					puts("LFN sequence error");
					maxslot = 0;
				} else {
					nextslot--;
					slot[nextslot] = *q;
				}
			}
		} else {
			p = (struct msdos_dir_entry *)q;

			if (maxslot && nextslot == 0 &&
			    sum == lfn_checksum(p->name)) {
				r = 0;
				printf("\t\"");
				for (j = 0; j < maxslot; j++) {
					r |= lfn_nputws(slot[j].name0_4, 5);
					r |= lfn_nputws(slot[j].name5_10, 6);
					r |= lfn_nputws(slot[j].name11_12, 2);
				}
				printf("\"\n");
				if (r) {
					printf("\t");
					for (j = 0; j < maxslot; j++) {
						lfn_dump(slot[j].name0_4, 5);
						lfn_dump(slot[j].name5_10, 6);
						lfn_dump(slot[j].name11_12, 2);
					}
					printf("\n");
				}
				maxslot = 0;
			} else if (maxslot) {
				puts("LFN sequence error");

				maxslot = 0;
			}
			printf("\t\"%c", p->name[0] == 0x05 ? 0xe5 :
			       p->name[0]);
			for (j = 1; j < 8; j++)
				printf("%c", p->name[j]);
			if (p->name[8] != ' ')
				printf(".");
			for (j = 8; j < 11; j++)
				printf("%c", p->name[j]);
			if (p->name[8] != ' ')
				printf("\" ");
			else
				printf("\"  ");
			printf("0x%02x ", p->attr);
#if 0
			printf("0x%02x ", p->lcase);
			printf("%s %2u %u ", month[(p->cdate >> 5) & 0xf],
			       p->cdate & 0x1f, (p->cdate >> 9) + 1980);
			printf("%02u:%02u:%02u.%03u ", p->ctime >> 11,
			       (p->ctime >> 5) & 0x3f, (p->ctime & 0x1f) * 2,
			       p->ctime_cs * 5);
			printf("%s %2u %u ", month[(p->adate >> 5) & 0xf],
			       p->adate & 0x1f, (p->adate >> 9) + 1980);
#endif
			printf("%s %2u %u ", month[(p->date >> 5) & 0xf],
			       p->date & 0x1f, (p->date >> 9) + 1980);
			printf("%02u:%02u:%02u ", p->time >> 11,
			       (p->time >> 5) & 0x3f, (p->time & 0x1f) * 2);
			printf("%10u ", p->starthi << 16 | p->start);
			printf("%10u\n", (unsigned int)p->size);
		}
		q++;
	}
	return 0;
}

static void print_fat(unsigned char *d, int cluster, int size)
{
	int n;
	int i;
	static bool flag;

	while (size > 0) {
		n = size > 16 ? 16 : size;
		for (i = 0; i < n; i++) {
			if (*(d + i))
				break;
		}
		if (i >= n) {
			if (!flag) {
				flag = true;
				puts("\t........");
			}
		} else {
			flag = false;
			printf("\t%08x ", cluster);
			for (i = 0; i < n / 4; i++)
				printf(" %02x%02x%02x%02x", *(d + i * 4 + 3),
				       *(d + i * 4 + 2), *(d + i * 4 + 1),
				       *(d + i * 4));
			printf("\n");
		}
		cluster += n / 4;
		size -= n;
		d += n;
	}
}

int fat(unsigned char *d)
{
	unsigned int cluster;
	unsigned int sector;
	unsigned int *f;
	int i;
	
	if (sd_read_sector(boot_sector + fsinfo, d))
		return -1;
	if (print_fsinfo(d))
		return -1;

	puts("Root directory:");
	cluster = root_dir;
	do {
		sector = boot_sector + reserved_sector + fat_size * fat_num +
			(cluster - 2) * cluster_size;
		for (i = 0; i < cluster_size; i++) {
			if (sd_read_sector(sector + i, d))
				return -1;
			if (print_dir(d))
				break;
		}
		if (sd_read_sector(boot_sector + reserved_sector +
				   cluster * 4 / SECTOR_SIZE, d))
			return -1;
		f = (unsigned int *)d;
		cluster = *(f + (cluster & (SECTOR_SIZE / 4 - 1)));
	} while (cluster < 0x0ffffff7);

	puts("File Allocation Table:");
	for (i = 0; i < (int)fat_size; i++) {
		if (sd_read_sector(boot_sector + reserved_sector + i, d))
			return -1;
		print_fat(d, SECTOR_SIZE / 4 * i, SECTOR_SIZE);
	}
	return 0;
}
