/*
 * TSC functions
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

#include <stdint.h>
#include <stdbool.h>

#include <mmio.h>
#include <memorymap.h>
#include <tsc.h>

void tsc_enable_channel(enum tsc_group group, int channel, int sampling)
{
	MMIO32(TSC_IOSCR) |= (sampling & 0xf) << (group * 4);
	MMIO32(TSC_IOCCR) |= (channel & 0xf) << (group * 4);
	MMIO32(TSC_IOGCSR) |= (1 << group) & 0xff;
}

void tsc_disable_channel(enum tsc_group group, int channel, int sampling)
{
	int s;
	int c;
	
	s = MMIO32(TSC_IOSCR);
	s &= ~((sampling & 0xf) << (group * 4));
	MMIO32(TSC_IOSCR) = s;
	c = MMIO32(TSC_IOCCR);
	c &= ~((channel & 0xf) << (group * 4));
	MMIO32(TSC_IOCCR) = c;
	if (!(s & (0xf << (group * 4))) || !(c & (0xf << (group * 4))))
		MMIO32(TSC_IOGCSR) &= ~((1 << group) & 0xff);
}

void tsc_set_charge_transfer_pulse(int prescaler, int high, int low)
{
	int i;
	int r;
	
	for (i = 0; i < 8; i++) {
		if (prescaler == 1 << i)
			break;
	}
	if (i >= 8)
		return;
	if (high < 1 || high > 16)
		return;
	if (low < 1 || low > 16)
		return;
	r = MMIO32(TSC_CR);
	r &= ~(TSC_CR_CTPH3 | TSC_CR_CTPH2 | TSC_CR_CTPH1 | TSC_CR_CTPH0 |
	       TSC_CR_CTPL3 | TSC_CR_CTPL2 | TSC_CR_CTPL1 | TSC_CR_CTPL0 |
	       TSC_CR_PGPSC2 | TSC_CR_PGPSC1 | TSC_CR_PGPSC0);
	MMIO32(TSC_CR) = r | (high - 1) << 28 | (low - 1) << 24 | i << 12;
}

void tsc_set_max_count_value(int count)
{
	int i;
	int r;
	
	for (i = 0; i < 7; i++) {
		if (count == (1 << (8 + i)) - 1)
			break;
	}
	if (i >= 7)
		return;
	r = MMIO32(TSC_CR);
	r &= ~(TSC_CR_MCV2 | TSC_CR_MCV1 | TSC_CR_MCV0);
	MMIO32(TSC_CR) = r | i << 5;
}

void tsc_enable_spread_spectrum(int prescaler, int deviation)
{
	int r;
	
	if (prescaler < 1 || prescaler > 2)
		return;
	if (deviation < 1 || deviation > 128)
		return;
	r = MMIO32(TSC_CR);
	r &= ~(TSC_CR_SSD6 | TSC_CR_SSD5 | TSC_CR_SSD4 | TSC_CR_SSD3 |
	       TSC_CR_SSD2 | TSC_CR_SSD1 | TSC_CR_SSD0);
	if (prescaler == 2)
		r |= TSC_CR_SSPSC;
	else
		r &= ~TSC_CR_SSPSC;
	MMIO32(TSC_CR) = r | (deviation - 1) << 17 | TSC_CR_SSE;
}

void tsc_disable_spread_spectrum(void)
{
	MMIO32(TSC_CR) &= ~TSC_CR_SSE;
}

void tsc_enable_sync(bool rising)
{
	int r;

	r = MMIO32(TSC_CR);
	if (rising)
		r |= TSC_CR_SYNCPOL;
	else
		r &= ~TSC_CR_SYNCPOL;
	MMIO32(TSC_CR) = r | TSC_CR_AM;
}

void tsc_disable_sync(void)
{
	MMIO32(TSC_CR) &= ~TSC_CR_AM;
}

void tsc_enable_io_default_input(void)
{
	MMIO32(TSC_CR) |= TSC_CR_IODEF;
}

void tsc_disable_io_default_input(void)
{
	MMIO32(TSC_CR) &= ~TSC_CR_IODEF;
}

void tsc_enable(void)
{
	MMIO32(TSC_CR) |= TSC_CR_TSCE;
}

void tsc_disable(void)
{
	MMIO32(TSC_CR) &= ~TSC_CR_TSCE;
}

void tsc_start(void)
{
	MMIO32(TSC_CR) |= TSC_CR_START;
}

void tsc_init(int prescaler, int high, int low, int max_count,
	      int sspsc, int ssd, int option)
{
	int i;
	int r;
	
	for (i = 0; i < 8; i++) {
		if (prescaler == 1 << i)
			break;
	}
	if (i >= 8)
		return;
	if (high < 1 || high > 16)
		return;
	if (low < 1 || low > 16)
		return;
	r = (high - 1) << 28 | (low - 1) << 24 | i << 12;
	for (i = 0; i < 7; i++) {
		if (max_count == (1 << (8 + i)) - 1)
			break;
	}
	if (i >= 7)
		return;
	r |= i << 5;
	if (sspsc == 2)
		r |= TSC_CR_SSPSC;
	if (ssd >= 1 && ssd <= 128)
		r |= (ssd - 1) << 17;
	r |= option & (TSC_CR_SSE | TSC_CR_IODEF | TSC_CR_SYNCPOL | TSC_CR_AM |
		       TSC_CR_START | TSC_CR_TSCE);
	MMIO32(TSC_CR) = r;
}

void tsc_enable_interrupt(int interrupt)
{
	MMIO32(TSC_IER) |= interrupt & (TSC_IER_MCEIE | TSC_IER_EOAIE);
}

void tsc_disable_interrupt(int interrupt)
{
	MMIO32(TSC_IER) &= ~(interrupt & (TSC_IER_MCEIE | TSC_IER_EOAIE));
}

int tsc_get_interrupt_mask(int interrupt)
{
	return MMIO32(TSC_IER) & interrupt;
}

int tsc_get_interrupt_status(int interrupt)
{
	return MMIO32(TSC_ISR) & interrupt;
}

void tsc_clear_interrupt(int interrupt)
{
	MMIO32(TSC_ICR) = interrupt & (TSC_ICR_MCEIC | TSC_ICR_EOAIC);
}

int tsc_get_status(enum tsc_group group)
{
	if (group == TSC_ALL)
		return MMIO32(TSC_IOGCSR);
	else
		return MMIO32(TSC_IOGCSR) & (0x10000 << group);
}

int tsc_get_counter(enum tsc_group group)
{
	if (group == TSC_ALL)
		return 0;
	else
		return MMIO32(TSC_IOGCR(group + 1));
}

void tsc_enable_hysteresis(int g_io)
{
	MMIO32(TSC_IOHCR) |= g_io;
}

void tsc_disable_hysteresis(int g_io)
{
	MMIO32(TSC_IOHCR) &= ~g_io;
}

void tsc_close_analog_switch(int g_io)
{
	MMIO32(TSC_IOASCR) |= g_io;
}

void tsc_open_analog_switch(int g_io)
{
	MMIO32(TSC_IOASCR) &= ~g_io;
}
