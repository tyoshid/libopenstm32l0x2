/*
 * RNG functions
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
#include <rng.h>

void rng_enable(void)
{
	MMIO32(RNG_CR) |= RNG_CR_RNGEN;
}

void rng_disable(void)
{
	MMIO32(RNG_CR) &= ~RNG_CR_RNGEN;
}

int rng_get_data(void)
{
	return MMIO32(RNG_DR);
}

void rng_enable_interrupt(int interrupt)
{
	if (interrupt & (RNG_SEIS | RNG_CEIS))
		MMIO32(RNG_CR) |= RNG_CR_IE;
}

void rng_disable_interrupt(int interrupt)
{
	if (interrupt & (RNG_SEIS | RNG_CEIS))
		MMIO32(RNG_CR) &= ~RNG_CR_IE;
}

int rng_get_interrupt_mask(int interrupt)
{
	if (MMIO32(RNG_CR) & RNG_CR_IE)
		return interrupt & (RNG_SEIS | RNG_CEIS);
	else
		return 0;
}

int rng_get_interrupt_status(int interrupt)
{
	return MMIO32(RNG_SR) & interrupt;
}

void rng_clear_interrupt(int interrupt)
{
	MMIO32(RNG_SR) &= ~(interrupt & (RNG_SR_SEIS | RNG_SR_CEIS));
}
