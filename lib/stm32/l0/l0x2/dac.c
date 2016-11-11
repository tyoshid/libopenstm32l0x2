/*
 * DAC functions
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
#include <dac.h>

void dac_set_mode(int mode)
{
	MMIO32(DAC_CR) = mode & 0x3fff3fff;
}

void dac_enable(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) |= DAC_CR_EN1;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) |= DAC_CR_EN2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) |= DAC_CR_EN1 | DAC_CR_EN2;
		break;
	default:
		break;
	}
}

void dac_disable(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) &= ~DAC_CR_EN1;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) &= ~DAC_CR_EN2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) &= ~(DAC_CR_EN1 | DAC_CR_EN2);
		break;
	default:
		break;
	}
}

void dac_enable_dma(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) |= DAC_CR_DMAEN1;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) |= DAC_CR_DMAEN2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) |= DAC_CR_DMAEN1 | DAC_CR_DMAEN2;
		break;
	default:
		break;
	}
}

void dac_disable_dma(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) &= ~DAC_CR_DMAEN1;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) &= ~DAC_CR_DMAEN2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) &= ~(DAC_CR_DMAEN1 | DAC_CR_DMAEN2);
		break;
	default:
		break;
	}
}

void dac_enable_noise_wave_generation(enum dac_channel channel)
{
	int r;

	switch (channel) {
	case DAC_CH1:
		r = MMIO32(DAC_CR);
		r &= ~DAC_CR_WAVE11;
		MMIO32(DAC_CR) = r | DAC_CR_WAVE10;
		break;
	case DAC_CH2:
		r = MMIO32(DAC_CR);
		r &= ~DAC_CR_WAVE21;
		MMIO32(DAC_CR) = r | DAC_CR_WAVE20;
		break;
	case DAC_DUAL:
		r = MMIO32(DAC_CR);
		r &= ~(DAC_CR_WAVE11 | DAC_CR_WAVE21);
		MMIO32(DAC_CR) = r | DAC_CR_WAVE10 | DAC_CR_WAVE20;
		break;
	default:
		break;
	}
}

void dac_enable_triangle_wave_generation(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) |= DAC_CR_WAVE11;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) |= DAC_CR_WAVE21;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) |= DAC_CR_WAVE11 | DAC_CR_WAVE21;
		break;
	default:
		break;
	}
}

void dac_disable_wave_generation(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) &= ~(DAC_CR_WAVE11 | DAC_CR_WAVE10);
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) &= ~(DAC_CR_WAVE21 | DAC_CR_WAVE20);
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) &= ~(DAC_CR_WAVE11 | DAC_CR_WAVE10 |
				    DAC_CR_WAVE21 | DAC_CR_WAVE20);
		break;
	default:
		break;
	}
}

void dac_enable_trigger(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) |= DAC_CR_TEN1;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) |= DAC_CR_TEN2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) |= DAC_CR_TEN1 | DAC_CR_TEN2;
		break;
	default:
		break;
	}
}

void dac_disable_trigger(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_CR) &= ~DAC_CR_TEN1;
		break;
	case DAC_CH2:
		MMIO32(DAC_CR) &= ~DAC_CR_TEN2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_CR) &= ~(DAC_CR_TEN1 | DAC_CR_TEN2);
		break;
	default:
		break;
	}
}

void dac_set_data(enum dac_format format, enum dac_channel channel, int data)
{
	switch (format) {
	case DAC_8R:
		switch (channel) {
		case DAC_CH1:
			MMIO32(DAC_DHR8R1) = data;
			break;
		case DAC_CH2:
			MMIO32(DAC_DHR8R2) = data;
			break;
		case DAC_DUAL:
			MMIO32(DAC_DHR8RD) = data;
			break;
		default:
			break;
		}
		break;
	case DAC_12L:
		switch (channel) {
		case DAC_CH1:
			MMIO32(DAC_DHR12L1) = data;
			break;
		case DAC_CH2:
			MMIO32(DAC_DHR12L2) = data;
			break;
		case DAC_DUAL:
			MMIO32(DAC_DHR12LD) = data;
			break;
		default:
			break;
		}
		break;
	case DAC_12R:
		switch (channel) {
		case DAC_CH1:
			MMIO32(DAC_DHR12R1) = data;
			break;
		case DAC_CH2:
			MMIO32(DAC_DHR12R2) = data;
			break;
		case DAC_DUAL:
			MMIO32(DAC_DHR12RD) = data;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void dac_software_trigger(enum dac_channel channel)
{
	switch (channel) {
	case DAC_CH1:
		MMIO32(DAC_SWTRIGR) = DAC_SWTRIGR_SWTRIG1;
		break;
	case DAC_CH2:
		MMIO32(DAC_SWTRIGR) = DAC_SWTRIGR_SWTRIG2;
		break;
	case DAC_DUAL:
		MMIO32(DAC_SWTRIGR) = DAC_SWTRIGR_SWTRIG1 |
			DAC_SWTRIGR_SWTRIG2;
		break;
	default:
		break;
	}
}

void dac_enable_interrupt(int interrupt)
{
	MMIO32(DAC_CR) |= interrupt & (DAC_CR_DMAUDRIE1 | DAC_CR_DMAUDRIE2);
}

void dac_disable_interrupt(int interrupt)
{
	MMIO32(DAC_CR) &= ~(interrupt & (DAC_CR_DMAUDRIE1 | DAC_CR_DMAUDRIE2));
}

int dac_get_interrupt_mask(int interrupt)
{
	return MMIO32(DAC_CR) & interrupt;
}

int dac_get_interrupt_status(int interrupt)
{
	return MMIO32(DAC_SR) & interrupt;
}

void dac_clear_interrupt(int interrupt)
{
	MMIO32(DAC_SR) = interrupt & (DAC_SR_DMAUDR1 | DAC_SR_DMAUDR2);
}
