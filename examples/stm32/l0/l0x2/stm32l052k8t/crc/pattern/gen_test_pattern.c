/*
 * gen_test_pattern
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "types.h"

/* linux-4.0.5 */
#include "crc32.h"
#include "crc16.h"
#include "crc-ccitt.h"
#include "crc8.h"
#include "crc7.h"

#include "crc.h"

#define DATALEN	1024

int main()
{
	int i;
	struct timeval tv;
	unsigned char buf[DATALEN];
	int j;
	struct test_result result[TESTLEN];
	DECLARE_CRC8_TABLE(crc8_table);

	printf("/* This file was generated automatically -"
	       "don't edit it. */\n\n");
	printf("#include \"crc.h\"\n\n");
	
	/* Random data */
	if (gettimeofday(&tv, NULL)) {
		perror("gettimeofday failed");
		return 1;
	}

	srandom(tv.tv_usec);

	for (i = 0; i < DATALEN; i++)
		buf[i] = random();

	printf("const char test_data[%d] = {\n", DATALEN);
	for (i = 0; i < DATALEN; i += 8) {
		printf("\t");
		for (j = 0; j < 8; j++) {
			printf("0x%02x,", buf[i + j]);
			if (j < 7)
				printf(" ");
			else
				printf("\n");
		}
	}
	printf("};\n\n");

/*
 * CRC-32(BE)
 *
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
 * +--------+---------+-----------+---------+-----------+---------+---+
 * |        |         |           |         |           |         |   |
 * |        v         v           v         v           v         v   |
 * +->|0|->(+)->|1|->(+)->|2|3|->(+)->|4|->(+)->|5|6|->(+)->|7|->(+)-+|
 *                                                                   ||
 * +-----------------------------------------------------------------+|
 * |          +----------+----------+-------------------+-------------+
 * |          |          |          |                   |             |
 * |          v          v          v                   v             |
 * +->|8|9|->(+)->|10|->(+)->|11|->(+)->|12|13|14|15|->(+)-+          |
 *                                                         |          |
 * +-------------------------------------------------------+          |
 * |                        +----------+----------------+-------------+
 * |                        |          |                |             |
 * |                        v          v                v             |
 * +->|16|17|18|19|20|21|->(+)->|22|->(+)->|23|24|25|->(+)-+          |
 *                                                         |          |
 *                                           +-------------+          |
 *                                           |                        |
 *                                           +->|26|27|28|29|30|31|->(+)<- Data
 *                                                                    MSB first
 */
	for (i = 0; i < TESTLEN; i++) {
		result[i].init = random() & 0xffff;
		result[i].init = (result[i].init << 16) | (random() & 0xffff);
		result[i].offset = random() & 0x3f;
		result[i].length = random() & 0x1ff;
		result[i].result = crc32_be(result[i].init,
					    buf + result[i].offset,
					    result[i].length);
	}

	printf("const struct test_result crc32_be_result[%d] = {\n", TESTLEN);
	for (i = 0; i < TESTLEN; i++) {
		printf("\t{0x%08x, ", result[i].init);
		printf("%2d, ", result[i].offset);
		printf("%4d, ", result[i].length);
		printf("0x%08x},\n", result[i].result);
	}
	printf("};\n\n");

/*
 * CRC-32(LE)
 *
 * x^32+x^26+x^23+x^22+x^16+x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x+1
 * +---------+----------+-------------+----------+-------------+----------+---+
 * |         |          |             |          |             |          |   |
 * |         v          v             v          v             v          v   |
 * +->|31|->(+)->|30|->(+)->|29|28|->(+)->|27|->(+)->|26|25|->(+)->|24|->(+)-+|
 *                                                                           ||
 * +-------------------------------------------------------------------------+|
 * |            +----------+----------+-------------------+-------------------+
 * |            |          |          |                   |                   |
 * |            v          v          v                   v                   |
 * +->|23|22|->(+)->|21|->(+)->|20|->(+)->|19|18|17|16|->(+)-+                |
 *                                                           |                |
 * +---------------------------------------------------------+                |
 * |                        +---------+-------------+-----------+-------------+
 * |                        |         |             |           |
 * |                        v         v             v           |
 * +->|15|14|13|12|11|10|->(+)->|9|->(+)->|8|7|6|->(+)-+        |
 *                                                     |        |
 *                                           +---------+        |
 *                                           |                  |
 *                                           +->|5|4|3|2|1|0|->(+)<- Data
 *                                                                   LSB first
 */
	for (i = 0; i < TESTLEN; i++) {
		result[i].init = random() & 0xffff;
		result[i].init = (result[i].init << 16) | (random() & 0xffff);
		result[i].offset = random() & 0x3f;
		result[i].length = random() & 0x1ff;
		result[i].result = crc32_le(result[i].init,
					    buf + result[i].offset,
					    result[i].length);
	}

	printf("const struct test_result crc32_le_result[%d] = {\n", TESTLEN);
	for (i = 0; i < TESTLEN; i++) {
		printf("\t{0x%08x, ", result[i].init);
		printf("%2d, ", result[i].offset);
		printf("%4d, ", result[i].length);
		printf("0x%08x},\n", result[i].result);
	}
	printf("};\n\n");

/*
 * CRC16
 *
 * x^16+x^15+x^2+1
 *  +------------+-------------------------------------+---------+
 *  |            |                                     |         |
 *  |            v                                     v         |
 *  +->|15|14|->(+)->|13|12|11|10|9|8|7|6|5|4|3|2|1|->(+)->|0|->(+)<- Data
 *                                                                    LSB first
 */
	for (i = 0; i < TESTLEN; i++) {
		result[i].init = random() & 0xffff;
		result[i].offset = random() & 0x3f;
		result[i].length = random() & 0x1ff;
		result[i].result = crc16(result[i].init,
					 buf + result[i].offset,
					 result[i].length);
	}

	printf("const struct test_result crc16_result[%d] = {\n", TESTLEN);
	for (i = 0; i < TESTLEN; i++) {
		printf("\t{0x%08x, ", result[i].init);
		printf("%2d, ", result[i].offset);
		printf("%4d, ", result[i].length);
		printf("0x%08x},\n", result[i].result);
	}
	printf("};\n\n");

/*
 * CRC_CCITT
 *
 * x^16+x^12+x^5+1
 *  +---------------------+----------------------+---------------+
 *  |                     |                      |               |
 *  |                     v                      v               |
 *  +->|15|14|13|12|11|->(+)->|10|9|8|7|6|5|4|->(+)->|3|2|1|0|->(+)<- Data
 *                                                                    LSB first
 */
	for (i = 0; i < TESTLEN; i++) {
		result[i].init = random() & 0xffff;
		result[i].offset = random() & 0x3f;
		result[i].length = random() & 0x1ff;
		result[i].result = crc_ccitt(result[i].init,
					     buf + result[i].offset,
					     result[i].length);
	}

	printf("const struct test_result crc_ccitt_result[%d] = {\n", TESTLEN);
	for (i = 0; i < TESTLEN; i++) {
		printf("\t{0x%08x, ", result[i].init);
		printf("%2d, ", result[i].offset);
		printf("%4d, ", result[i].length);
		printf("0x%08x},\n", result[i].result);
	}
	printf("};\n\n");

/*
 * CRC8-Dallas/Maxim
 *
 * x^8+x^5+x^4+1
 *  +--------------+---------+---------+-----------+
 *  |              |         |         |           |
 *  |              v         v         v           |
 *  +->|7|6|5|4|->(+)->|3|->(+)->|2|->(+)->|1|0|->(+)<- Data  LSB first
 */
	crc8_populate_lsb(crc8_table, 0x8c);
	for (i = 0; i < TESTLEN; i++) {
		result[i].init = random() & 0xff;
		result[i].offset = random() & 0x3f;
		result[i].length = random() & 0x1ff;
		result[i].result = crc8(crc8_table,
					buf + result[i].offset,
					result[i].length,
					result[i].init);
	}

	printf("const struct test_result crc8_result[%d] = {\n", TESTLEN);
	for (i = 0; i < TESTLEN; i++) {
		printf("\t{0x%08x, ", result[i].init);
		printf("%2d, ", result[i].offset);
		printf("%4d, ", result[i].length);
		printf("0x%08x},\n", result[i].result);
	}
	printf("};\n\n");

/*
 * CRC7
 *
 * x^7+x^3+1
 *  +------------+---------------+
 *  |            |               |
 *  |            v               |
 *  +->|1|2|3|->(+)->|4|5|6|7|->(+)<- Data  MSB first
 */
	for (i = 0; i < TESTLEN; i++) {
		result[i].init = random() & 0x7f;
		result[i].offset = random() & 0x3f;
		result[i].length = random() & 0x1ff;
		result[i].result = crc7_be(result[i].init << 1,
					   buf + result[i].offset,
					   result[i].length) >> 1;
	}

	printf("const struct test_result crc7_result[%d] = {\n", TESTLEN);
	for (i = 0; i < TESTLEN; i++) {
		printf("\t{0x%08x, ", result[i].init);
		printf("%2d, ", result[i].offset);
		printf("%4d, ", result[i].length);
		printf("0x%08x},\n", result[i].result);
	}
	printf("};\n\n");

	return 0;
}
