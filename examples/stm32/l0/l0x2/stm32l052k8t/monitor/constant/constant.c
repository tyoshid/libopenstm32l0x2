/*
 * constant - Display the constants defined in libopenstm32l0x2.
 *
 * constant.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of constant.
 *
 * constant is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * constant is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with constant.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "symtab.h"

void memorymap(char *category);
void ppb(char *category);
void flash(char *category);
void crc(char *category);
void fw(char *category);
void pwr(char *category);
void rcc(char *category);
void crs(char *category);
void gpio(char *category);
void syscfg(char *category);
void dma(char *category);
void nvic(char *category);
void vector(char *category);
void exti(char *category);
void adc(char *category);
void dac(char *category);
void comp(char *category);
void tsc(char *category);
void aes(char *category);
void rng(char *category);
void tim(char *category);
void lptim(char *category);
void iwdg(char *category);
void wwdg(char *category);
void rtc(char *category);
void i2c(char *category);
void usart(char *category);
void lpuart(char *category);
void spi(char *category);
void usbdevfs(char *category);
void dbg(char *category);
void misc(char *category);
void stk(char *category);
void scb(char *category);
void mpu(char *category);

extern struct symtab enumconst[];

static struct {
	char *fname;
	void (*f)(char *category);
} ftab[] = {
	{"memorymap", memorymap},
	{"ppb", ppb},
	{"flash", flash},
	{"crc", crc},
	{"fw", fw},
	{"pwr", pwr},
	{"rcc", rcc},
	{"crs", crs},
	{"gpio", gpio},
	{"syscfg", syscfg},
	{"dma", dma},
	{"nvic", nvic},
	{"vector", vector},
	{"exti", exti},
	{"adc", adc},
	{"dac", dac},
	{"comp", comp},
	{"tsc", tsc},
	{"aes", aes},
	{"rng", rng},
	{"tim", tim},
	{"lptim", lptim},
	{"iwdg", iwdg},
	{"wwdg", wwdg},
	{"rtc", rtc},
	{"i2c", i2c},
	{"usart", usart},
	{"lpuart", lpuart},
	{"spi", spi},
	{"usbdevfs", usbdevfs},
	{"dbg", dbg},
	{"misc", misc},
	{"stk", stk},
	{"scb", scb},
	{"mpu", mpu},
	{NULL, NULL}
};

void help(char *prog)
{
	int i;
	
	printf("Usage: %s <name> [<name> ...]\n", prog);
	printf("       %s -c <category1> [<category2>]\n", prog);
	printf("\n");
	printf("category1:\n");
	for (i = 0; ftab[i].fname; i++)
		printf("\t%s\n", ftab[i].fname);
}

void search_symtab(int offset, int argc, char *argv[])
{
	int i;
	int j;
	unsigned int v;	

	v = 0;
	for (i = offset; i < argc; i++) {
		for (j = 0; enumconst[j].name; j++) {
			if (strcasecmp(argv[i], enumconst[j].name) == 0) {
				v |= enumconst[j].value;
				if (i > offset)
					printf("| ");
				printf("0x%08x ", enumconst[j].value);
				break;
			}
		}
		if (enumconst[j].name == NULL) {
			printf("Unkown constant: %s\n", argv[i]);
			return;
		}
	}
	if (argc - offset > 1)
		printf("= 0x%08x ", v);
	printf("%d\n", v);
}

int main(int argc, char *argv[])
{
	char *category;
	int i;
	
	if (argc < 2) {
		help(argv[0]);
	} else if (strcmp(argv[1], "-c") == 0) {
		if (argc < 3) {
			help(argv[0]);
			return 0;
		}
		category = (argc >= 4) ? argv[3] : "?";

		for (i = 0; ftab[i].fname; i++) {
			if (strcmp(argv[2], ftab[i].fname) == 0) {
				(*ftab[i].f)(category);
				break;
			}
		}
		if (!ftab[i].fname)
			help(argv[0]);
	} else {
		search_symtab(1, argc, argv);
	}
	
	return 0;
}
