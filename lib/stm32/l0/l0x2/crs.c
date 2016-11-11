/*
 * CRS functions
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

#include <mmio.h>
#include <memorymap.h>
#include <crs.h>

void crs_config(int reload, int felim, int div, enum crs_src src)
{
	int i;

	for (i = 0; i < 8; i++) {
		if (div == 1 << i)
			break;
	}
	if (i >= 8)
		return;
	MMIO32(CRS_CFGR) = src | i << 24 | (felim & 0xff) << 16 |
		(reload & 0xffff);
}

void crs_enable(void)
{
	MMIO32(CRS_CR) |= CRS_CR_CEN;
}

void crs_disable(void)
{
	MMIO32(CRS_CR) &= ~CRS_CR_CEN;
}

void crs_enable_autotrim(void)
{
	MMIO32(CRS_CR) |= CRS_CR_AUTOTRIMEN;
}

void crs_disable_autotrim(void)
{
	MMIO32(CRS_CR) &= ~CRS_CR_AUTOTRIMEN;
}

void crs_software_sync(void)
{
	MMIO32(CRS_CR) |= CRS_CR_SWSYNC;
}

void crs_set_trim(int trim)
{
	int r;

	r = MMIO32(CRS_CR);
	r &= ~(CRS_CR_TRIM5 | CRS_CR_TRIM4 | CRS_CR_TRIM3 | CRS_CR_TRIM2 |
	       CRS_CR_TRIM1 | CRS_CR_TRIM0);
	MMIO32(CRS_CR) = r | (trim & 0x3f) << 8;
}

void crs_enable_interrupt(int interrupt)
{
	MMIO32(CRS_CR) |= interrupt & (CRS_CR_ESYNCIE | CRS_CR_ERRIE |
				       CRS_CR_SYNCWARNIE | CRS_CR_SYNCOKIE);
}

void crs_disable_interrupt(int interrupt)
{
	MMIO32(CRS_CR) &= ~(interrupt & (CRS_CR_ESYNCIE | CRS_CR_ERRIE |
					 CRS_CR_SYNCWARNIE | CRS_CR_SYNCOKIE));
}

int crs_get_interrupt_mask(int interrupt)
{
	return MMIO32(CRS_CR) & interrupt;
}

int crs_get_interrupt_status(int interrupt)
{
	return MMIO32(CRS_ISR) & interrupt;
}

void crs_clear_interrupt(int interrupt)
{
	MMIO32(CRS_ICR) = interrupt & (CRS_ICR_ESYNCC | CRS_ICR_ERRC |
				       CRS_ICR_SYNCWARNC | CRS_ICR_SYNCOKC);
}
