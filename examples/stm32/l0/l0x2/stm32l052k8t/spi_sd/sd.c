/*
 * sd.c
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

#include <gpio.h>
#include <spi.h>
#include <dma.h>
#include <memorymap.h>
#include <crc.h>

#include "spi_sd.h"

/* CS (neg true) - PB0 */

/* SD R1 */
#define SD_R1_PARAMETER_ERROR		(1 << 6)
#define SD_R1_ADDRESS_ERROR		(1 << 5)
#define SD_R1_ERASE_SEQUENCE_ERROR	(1 << 4)
#define SD_R1_COM_CRC_ERROR		(1 << 3)
#define SD_R1_ILLEGAL_COMMAND		(1 << 2)
#define SD_R1_ERASE_RESET		(1 << 1)
#define SD_R1_IN_IDLE_STATE		(1 << 0)

/* SD OCR (Operation conditions register) */
#define SD_OCR_BUSY			(1 << 31)
#define SD_OCR_CCS			(1 << 30)
#define SD_OCR_UHS_II			(1 << 29)
#define SD_OCR_S18A			(1 << 24)
#define SD_OCR_3_5			(1 << 23)
#define SD_OCR_3_4			(1 << 22)
#define SD_OCR_3_3			(1 << 21)
#define SD_OCR_3_2			(1 << 20)
#define SD_OCR_3_1			(1 << 19)
#define SD_OCR_3_0			(1 << 18)
#define SD_OCR_2_9			(1 << 17)
#define SD_OCR_2_8			(1 << 16)
#define SD_OCR_2_7			(1 << 15)

/* SD commands */
const unsigned char sd_cmd0[] = {0x40, 0x00, 0x00, 0x00, 0x00, 0x95};
const unsigned char sd_cmd8[] = {0x48, 0x00, 0x00, 0x01, 0x23, 0x87};
const unsigned char sd_cmd58[] = {0x7a, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char sd_cmd55[] = {0x77, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char sd_acmd41[] = {0x69, 0x40, 0x00, 0x00, 0x00, 0x01};
const unsigned char sd_cmd9[] = {0x49, 0x00, 0x00, 0x00, 0x00, 0x01};
const unsigned char sd_cmd10[] = {0x4a, 0x00, 0x00, 0x00, 0x00, 0x01};

static int sd_command(const unsigned char *cmd, unsigned char *r, int size)
{
	int i;
	
	gpio_clear(GPIO_PA0);
	gpio_clear(GPIO_PB0);
	for (i = 0; i < 6; i++) {
		spi_send_blocking(SPI1, *cmd++);
		spi_recv_blocking(SPI1);
	}
	do {
		spi_send_blocking(SPI1, 0xff);
		*r = spi_recv_blocking(SPI1);
	} while (*r & 0x80);
	if (*r & (SD_R1_ILLEGAL_COMMAND | SD_R1_COM_CRC_ERROR)) {
		gpio_set(GPIO_PA0);
		return *r;
	}
	r++;
	for (i = 1; i < size; i++) {
		spi_send_blocking(SPI1, 0xff);
		*r++ = spi_recv_blocking(SPI1);
	}
	gpio_set(GPIO_PB0);
	gpio_set(GPIO_PA0);
	return 0;
}

static int sd_read_block(const unsigned char *cmd, unsigned char *d, int size)
{
	int i;
	int r;
	static const int dummy = 0xffffffff;
	
	gpio_clear(GPIO_PA0);
	gpio_clear(GPIO_PB0);
	for (i = 0; i < 6; i++) {
		spi_send_blocking(SPI1, *cmd++);
		spi_recv_blocking(SPI1);
	}
	do {
		spi_send_blocking(SPI1, 0xff);
		r = spi_recv_blocking(SPI1);
	} while (r & 0x80);
	if (r) {
		gpio_set(GPIO_PB0);
		gpio_set(GPIO_PA0);
		return r << 8;
	}
	do {
		spi_send_blocking(SPI1, 0xff);
		r = spi_recv_blocking(SPI1);
	} while (r == 0xff);
	if (r != 0xfe) {
		gpio_set(GPIO_PB0);
		gpio_set(GPIO_PA0);
		return r;
	}
	dma_setup_channel(DMA_CH2, (int)d, SPI1_DR, size,
			  DMA_M_8BIT | DMA_P_8BIT | DMA_M_INC | DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)&dummy, SPI1_DR, size,
			  DMA_M_8BIT | DMA_P_8BIT | DMA_M_TO_P | DMA_ENABLE);
	spi_enable_dma(SPI1, SPI_DMA_TX_RX);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	gpio_set(GPIO_PB0);
	spi_disable_dma(SPI1, SPI_DMA_TX_RX);
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
	gpio_set(GPIO_PA0);
	return 0;
}

int sd_init(void)
{
	int i;
	unsigned char r[5];
	bool ver1 = false;
	static unsigned char d[SECTOR_SIZE + 2];
	int s;
	
	delay_ms(36);

	for (i = 0; i < 10; i++) {
		spi_send_blocking(SPI1, 0xff);
		spi_recv_blocking(SPI1);
	}

	puts("CMD0");
	sd_command(sd_cmd0, r, 1);
	printf("R1 = 0x%02x\n", r[0]);
	if (r[0] != SD_R1_IN_IDLE_STATE)
		return -1;

	puts("CMD8");
	if (sd_command(sd_cmd8, r, 5)) {
		printf("R1 = 0x%02x\n", r[0]);
		if (r[0] & SD_R1_ILLEGAL_COMMAND)
			ver1 = true;
		else
			return -1;
	} else {
		printf("R7 = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		       r[0], r[1], r[2], r[3], r[4]);
		if (r[0] != SD_R1_IN_IDLE_STATE || r[3] != sd_cmd8[3] ||
		    r[4] != sd_cmd8[4])
			return -1;
	}

	puts("CMD58");
	sd_command(sd_cmd58, r, 5);
	printf("R3 = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", r[0], r[1], r[2],
	       r[3], r[4]);
	if (r[0] != SD_R1_IN_IDLE_STATE ||
	    ((r[2] << 16 | r[3] << 8) &
	     (SD_OCR_2_7 | SD_OCR_2_8 | SD_OCR_2_9 |
	      SD_OCR_3_0 | SD_OCR_3_1 | SD_OCR_3_2 |
	      SD_OCR_3_3 | SD_OCR_3_4 | SD_OCR_3_5)) !=
	    (SD_OCR_2_7 | SD_OCR_2_8 | SD_OCR_2_9 |
	     SD_OCR_3_0 | SD_OCR_3_1 | SD_OCR_3_2 |
	     SD_OCR_3_3 | SD_OCR_3_4 | SD_OCR_3_5))
		return -1;

	do {
		puts("CMD55");
		sd_command(sd_cmd55, r, 1);
		printf("R1 = 0x%02x\n", r[0]);
		if (r[0] & ~SD_R1_IN_IDLE_STATE)
			return -1;
	
		puts("ACMD41");
		sd_command(sd_acmd41, r, 1);
		printf("R1 = 0x%02x\n", r[0]);
		if (r[0] & ~SD_R1_IN_IDLE_STATE)
			return -1;
	} while (r[0] & SD_R1_IN_IDLE_STATE);

	if (ver1) {
		puts("Ver 1.X SDSC");
	} else {
		puts("CMD58");
		sd_command(sd_cmd58, r, 5);
		printf("R3 = 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		       r[0], r[1], r[2], r[3], r[4]);
		if (r[0] || !((r[1] << 24) & SD_OCR_BUSY))
			return -1;

		if ((r[1] << 24) & SD_OCR_CCS)
			puts("SDHC or SDXC");
		else
			puts("SDSC");
	}

	/* SCK: 32 MHz / 2 = 16 MHz */
	spi_init_master(SPI1, 2, 8, SPI_NSS_OUTPUT | SPI_ENABLE);

	puts("CMD9");
	s = sd_read_block(sd_cmd9, d, 16);
	if (s) {
		printf("0x%04x\n", s);
		return -1;
	}
	for (i = 0; i < 16; i++)
		printf("0x%02x ", d[i]);
	printf("\n");
	if (d[15] != ((crc_calc(0, (char *)d, 15) << 1) | 1))
		return -1;
	print_csd(d);
	
	puts("CMD10");
	s = sd_read_block(sd_cmd10, d, 16);
	if (s) {
		printf("0x%04x\n", s);
		return -1;
	}
	for (i = 0; i < 16; i++)
		printf("0x%02x ", d[i]);
	printf("\n");
	if (d[15] != ((crc_calc(0, (char *)d, 15) << 1) | 1))
		return -1;
	print_cid(d);

	puts("");
	crc_set_mode(0x1021, CRC_POLYSIZE_16);
	return 0;
}

int sd_read_sector(unsigned int sector, unsigned char *d)
{
	unsigned char c[6];
	int s;
	
	c[0] = 0x51;
	c[1] = sector >> 24;
	c[2] = sector >> 16;
	c[3] = sector >> 8;
	c[4] = sector;
	c[5] = 1;
	s = sd_read_block(c, d, SECTOR_SIZE + 2);
	if (s) {
		printf("CMD17 failed: 0x%04x\n", s);
		return -1;
	}
	if (crc_calc(0, (char *)d, SECTOR_SIZE + 2)) {
		puts("CMD17 data CRC error");
		return -1;
	}
	return 0;
}
