/*
 * csd.c
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
#include <stdbool.h>

#include "spi_sd.h"

static const char time_value[48][4] = {
	"?", "1.0", "1.2", "1.3", "1.5", "2.0", "2.5", "3.0",
	"3.5", "4.0", "4.5", "5.0", "5.5", "6.0", "7.0", "8.0",
	"?", "10", "12", "13", "15", "20", "25", "30",
	"35", "40", "45", "50", "55", "60", "70", "80",
	"?", "100", "120", "130", "150", "200", "250", "300",
	"350", "400", "450", "500", "550", "600", "700", "800"
};

static const char taac_unit[3][3] = {
	"ns", "us", "ms"
};

static const char tran_speed_unit[8][7] = {
	"kbit/s", "Mbit/s", "Mbit/s", "Mbit/s",
	"?", "?", "?", "?"
};

static const char curr_min[8][7] = {
	"0.5 mA", "1 mA", "5 mA", "10 mA",
	"25 mA", "35 mA", "60 mA", "100 mA"
};

static const char curr_max[8][7] = {
	"1 mA", "5 mA", "10 mA", "25 mA",
	"35 mA", "45 mA", "80 mA", "200 mA"
};

static const char month[16][10] = {
	"?",
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
	"?", "?", "?"
};

static void print_csd_v1(unsigned char *d)
{
	int x;
	bool flag;
	int i;

	puts("\tCSD Version 1.0");
	printf("\tTAAC = %s %s\n",
	       time_value[(*(d + 1) >> 3 & 0xf) * ((*(d + 1) & 7) % 3 + 1)],
	       taac_unit[(*(d + 1) & 7) / 3]);
	printf("\tNSAC = %d clock\n", *(d + 2) * 100);
	x = *(d + 3) & 7;
	if (x == 0)
		x = (*(d + 3) >> 3 & 0xf) + 32;
	else if (x < 4)
		x = (*(d + 3) >> 3 & 0xf) + 16 * (x - 1);
	else
		x = 0;
	printf("\tTRAN_SPEED = %s %s\n", time_value[x],
	       tran_speed_unit[*(d + 3) & 7]);
	printf("\tCCC = ");
	flag = false;
	for (i = 0; i < 12; i++) {
		if ((*(d + 4) << 8 | *(d + 5)) & (0x10 << i)) {
			printf("%s%d", flag ? ", " : "class ", i);
			flag = true;
		}
	}
	printf(" (");
	for (i = 0; i < 12; i++)
		putchar(((*(d + 4) << 8 | *(d + 5)) & (0x8000 >> i)) ? '1' :
			'0');
	printf("b)\n");
	printf("\tREAD_BL_LEN = %d (%d Bytes)\n",
	       *(d + 5) & 0xf, 1 << (*(d + 5) & 0xf));
	printf("\tREAD_BL_PARTIAL = %c\n", (*(d + 6) & 0x80) ? '1' : '0');
	printf("\tWRITE_BLK_MISALIGN = %c\n", (*(d + 6) & 0x40) ? '1' : '0');
	printf("\tREAD_BLK_MISALIGN = %c\n", (*(d + 6) & 0x20) ? '1' : '0');
	printf("\tDSR_IMP = %c\n", (*(d + 6) & 0x10) ? '1' : '0');
	printf("\tC_SIZE = %d\n",
	       (*(d + 6) & 3) << 10 | *(d + 7) << 2 | *(d + 8) >> 6);
	printf("\tVDD_R_CURR_MIN = %s\n", curr_min[(*(d + 8) >> 3) & 7]);
	printf("\tVDD_R_CURR_MAX = %s\n", curr_max[*(d + 8) & 7]);
	printf("\tVDD_W_CURR_MIN = %s\n", curr_min[(*(d + 9) >> 5) & 7]);
	printf("\tVDD_W_CURR_MAX = %s\n", curr_max[(*(d + 9) >> 2) & 7]);
	printf("\tC_SIZE_MULT = %d\n", (*(d + 9) & 3) << 1 | *(d + 10) >> 7);
	printf("\tERASE_BLK_EN = %c\n", (*(d + 10) & 0x40) ? '1' : '0');
	printf("\tSECTOR_SIZE = %d\n",
	       (*(d + 10) & 0x3f) << 1 | *(d + 11) >> 7);
	printf("\tWP_GRP_SIZE = %d\n", *(d + 11) & 0x7f);
	printf("\tWP_GRP_ENABLE = %c\n", (*(d + 12) & 0x80) ? '1' : '0');
	printf("\tR2W_FACTOR = %d\n", *(d + 12) >> 2 & 7);
	printf("\tWRITE_BL_LEN = %d (%d Bytes)\n",
	       (*(d + 12) & 3) << 2 | *(d + 13) >> 6,
	       1 << ((*(d + 12) & 3) << 2 | *(d + 13) >> 6));
	printf("\tWRITE_BL_PARTIAL = %c\n", (*(d + 13) & 0x20) ? '1' : '0');
	printf("\tFILE_FORMAT_GRP = %c\n", (*(d + 14) & 0x80) ? '1' : '0');
	printf("\tCOPY = %c\n", (*(d + 14) & 0x40) ? '1' : '0');
	printf("\tPERM_WRITE_PROTECT = %c\n", (*(d + 14) & 0x20) ? '1' : '0');
	printf("\tTMP_WRITE_PROTECT = %c\n", (*(d + 14) & 0x10) ? '1' : '0');
	printf("\tFILE_FORMAT = %d\n", *(d + 14) >> 2 & 3);
}

static void print_csd_v2(unsigned char *d)
{
	int x;
	bool flag;
	int i;
	double w;

	puts("\tCSD Version 2.0");
	printf("\tTAAC = %s %s\n",
	       time_value[(*(d + 1) >> 3 & 0xf) * ((*(d + 1) & 7) % 3 + 1)],
	       taac_unit[(*(d + 1) & 7) / 3]);
	printf("\tNSAC = %d clock\n", *(d + 2) * 100);
	x = *(d + 3) & 7;
	if (x == 0)
		x = (*(d + 3) >> 3 & 0xf) + 32;
	else if (x < 4)
		x = (*(d + 3) >> 3 & 0xf) + 16 * (x - 1);
	else
		x = 0;
	printf("\tTRAN_SPEED = %s %s\n", time_value[x],
	       tran_speed_unit[*(d + 3) & 7]);
	printf("\tCCC = ");
	flag = false;
	for (i = 0; i < 12; i++) {
		if ((*(d + 4) << 8 | *(d + 5)) & (0x10 << i)) {
			printf("%s%d", flag ? ", " : "class ", i);
			flag = true;
		}
	}
	printf(" (");
	for (i = 0; i < 12; i++)
		putchar(((*(d + 4) << 8 | *(d + 5)) & (0x8000 >> i)) ? '1' :
			'0');
	printf("b)\n");
	printf("\tREAD_BL_LEN = %d (%d Bytes)\n",
	       *(d + 5) & 0xf, 1 << (*(d + 5) & 0xf));
	printf("\tREAD_BL_PARTIAL = %c\n", (*(d + 6) & 0x80) ? '1' : '0');
	printf("\tWRITE_BLK_MISALIGN = %c\n", (*(d + 6) & 0x40) ? '1' : '0');
	printf("\tREAD_BLK_MISALIGN = %c\n", (*(d + 6) & 0x20) ? '1' : '0');
	printf("\tDSR_IMP = %c\n", (*(d + 6) & 0x10) ? '1' : '0');
	printf("\tC_SIZE = %d ", *(d + 7) << 16 | *(d + 8) << 8 | *(d + 9));
	w = *(d + 7) << 16 | *(d + 8) << 8 | *(d + 9);
	printf("(%.1lf GB)\n", w * SECTOR_SIZE / 1024. / 1024.);
	printf("\tERASE_BLK_EN = %c\n", (*(d + 10) & 0x40) ? '1' : '0');
	printf("\tSECTOR_SIZE = %d\n",
	       (*(d + 10) & 0x3f) << 1 | *(d + 11) >> 7);
	printf("\tWP_GRP_SIZE = %d\n", *(d + 11) & 0x7f);
	printf("\tWP_GRP_ENABLE = %c\n", (*(d + 12) & 0x80) ? '1' : '0');
	printf("\tR2W_FACTOR = %d\n", *(d + 12) >> 2 & 7);
	printf("\tWRITE_BL_LEN = %d (%d Bytes)\n",
	       (*(d + 12) & 3) << 2 | *(d + 13) >> 6,
	       1 << ((*(d + 12) & 3) << 2 | *(d + 13) >> 6));
	printf("\tWRITE_BL_PARTIAL = %c\n", (*(d + 13) & 0x20) ? '1' : '0');
	printf("\tFILE_FORMAT_GRP = %c\n", (*(d + 14) & 0x80) ? '1' : '0');
	printf("\tCOPY = %c\n", (*(d + 14) & 0x40) ? '1' : '0');
	printf("\tPERM_WRITE_PROTECT = %c\n", (*(d + 14) & 0x20) ? '1' : '0');
	printf("\tTMP_WRITE_PROTECT = %c\n", (*(d + 14) & 0x10) ? '1' : '0');
	printf("\tFILE_FORMAT = %d\n", *(d + 14) >> 2 & 3);
}

void print_csd(unsigned char *d)
{
	puts("CSD (Card-specific data) register:");
	if (*d == 0)
		print_csd_v1(d);
	else if (*d == 0x40)
		print_csd_v2(d);
}

void print_cid(unsigned char *d)
{
	puts("CID (Card identification) register:");
	printf("\tMID = 0x%02x (%d)\n", *d, *d);
	printf("\tOID = \"%c%c\"\n", *(d + 1), *(d + 2));
	printf("\tPNM = \"%c%c%c%c%c\"\n", *(d + 3), *(d + 4), *(d + 5),
	       *(d + 6), *(d + 7));
	printf("\tPRV = %d.%d\n", *(d + 8) >> 4, *(d + 8) & 0xf);
	printf("\tPSN = 0x%02x%02x%02x%02x\n", *(d + 9), *(d + 10), *(d + 11),
	       *(d + 12));
	printf("\tMDT = %s %d\n", month[*(d + 14) & 0xf],
	       ((*(d + 13) & 0xf) << 4 | (*(d + 14) & 0xf0) >> 4) + 2000);
}
