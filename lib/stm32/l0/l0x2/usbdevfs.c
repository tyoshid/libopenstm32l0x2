/*
 * USB functions
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
#include <usbdevfs.h>

void usbdevfs_enable_usb_reset(void)
{
	MMIO32(USB_CNTR) |= USB_CNTR_FRES;
}

void usbdevfs_disable_usb_reset(void)
{
	MMIO32(USB_CNTR) &= ~USB_CNTR_FRES;
}

void usbdevfs_enable_power_down(void)
{
	MMIO32(USB_CNTR) |= USB_CNTR_PDWN;
}

void usbdevfs_disable_power_down(void)
{
	MMIO32(USB_CNTR) &= ~USB_CNTR_PDWN;
}

void usbdevfs_enable_low_power_mode(void)
{
	MMIO32(USB_CNTR) |= USB_CNTR_LP_MODE;
}

void usbdevfs_disable_low_power_mode(void)
{
	MMIO32(USB_CNTR) &= ~USB_CNTR_LP_MODE;
}

void usbdevfs_enable_suspend_mode(void)
{
	MMIO32(USB_CNTR) |= USB_CNTR_FSUSP;
}

void usbdevfs_disable_suspend_mode(void)
{
	MMIO32(USB_CNTR) &= ~USB_CNTR_FSUSP;
}

void usbdevfs_enable_resume(void)
{
	MMIO32(USB_CNTR) |= USB_CNTR_RESUME;
}

void usbdevfs_disable_resume(void)
{
	MMIO32(USB_CNTR) &= ~USB_CNTR_RESUME;
}

void usbdevfs_request_lpm_l1_resume(void)
{
	MMIO32(USB_CNTR) |= USB_CNTR_L1RESUME;
}

void usbdevfs_enable_interrupt(int interrupt)
{
	MMIO32(USB_CNTR) |= interrupt & (USB_CNTR_CTRM | USB_CNTR_PMAOVRM |
					 USB_CNTR_ERRM | USB_CNTR_WKUPM |
					 USB_CNTR_SUSPM | USB_CNTR_RESETM |
					 USB_CNTR_SOFM | USB_CNTR_ESOFM |
					 USB_CNTR_L1REQM);
}

void usbdevfs_disable_interrupt(int interrupt)
{
	MMIO32(USB_CNTR) &= ~(interrupt & (USB_CNTR_CTRM | USB_CNTR_PMAOVRM |
					   USB_CNTR_ERRM | USB_CNTR_WKUPM |
					   USB_CNTR_SUSPM | USB_CNTR_RESETM |
					   USB_CNTR_SOFM | USB_CNTR_ESOFM |
					   USB_CNTR_L1REQM));
}

int usbdevfs_get_interrupt_mask(int interrupt)
{
	return MMIO32(USB_CNTR) & interrupt;
}

int usbdevfs_get_interrupt_status(int interrupt)
{
	return MMIO32(USB_ISTR) & interrupt;
}

void usbdevfs_clear_interrupt(int interrupt)
{
	MMIO32(USB_ISTR) &= ~(interrupt & (USB_ISTR_PMAOVR | USB_ISTR_ERR |
					   USB_ISTR_WKUP | USB_ISTR_SUSP |
					   USB_ISTR_RESET | USB_ISTR_SOF |
					   USB_ISTR_ESOF | USB_ISTR_L1REQ));
}

int usbdevfs_get_frame_number(void)
{
	return MMIO32(USB_FNR);
}

void usbdevfs_set_device_address(int addr)
{
	MMIO32(USB_DADDR) = (addr | USB_DADDR_EF) & 0xff;
}

void usbdevfs_set_buffer_table_address(int offset)
{
	MMIO32(USB_BTABLE) = offset & 0xfff8;
}

void usbdevfs_enable_lpm(void)
{
	MMIO32(USB_LPMCSR) |= USB_LPMCSR_LPMEN;
}

void usbdevfs_disable_lpm(void)
{
	MMIO32(USB_LPMCSR) &= ~USB_LPMCSR_LPMEN;
}

void usbdevfs_enable_lpm_ack(void)
{
	MMIO32(USB_LPMCSR) |= USB_LPMCSR_LPMACK;
}

void usbdevfs_disable_lpm_ack(void)
{
	MMIO32(USB_LPMCSR) &= ~USB_LPMCSR_LPMACK;
}

int usbdevfs_get_lpm_status(void)
{
	return MMIO32(USB_LPMCSR);
}

void usbdevfs_enable_dp_pull_up(void)
{
	MMIO32(USB_BCDR) |= USB_BCDR_DPPU;
}

void usbdevfs_disable_dp_pull_up(void)
{
	MMIO32(USB_BCDR) &= ~USB_BCDR_DPPU;
}

void usbdevfs_set_bcd_mode(enum usbdevfs_bcd_mode mode)
{
	int r;

	r = MMIO32(USB_BCDR);
	r &= ~(USB_BCDR_SDEN | USB_BCDR_PDEN | USB_BCDR_DCDEN |
	       USB_BCDR_BCDEN);
	MMIO32(USB_BCDR) = r | mode;
}

int usbdevfs_get_bcd_status(int status)
{
	return MMIO32(USB_BCDR) & status;
}

int usbdevfs_assign_packet_memory_tx(int ep_id, int offset, int nbyte)
{
	offset = (offset + 1) & ~1;
	MMIO16(USB_ADDR_TX(ep_id)) = offset;
	return offset + ((nbyte + 1) & ~1);
}

int usbdevfs_assign_packet_memory_tx1(int ep_id, int offset, int nbyte)
{
	offset = (offset + 1) & ~1;
	MMIO16(USB_ADDR_RX(ep_id)) = offset;
	return offset + ((nbyte + 1) & ~1);
}

int usbdevfs_assign_packet_memory_rx(int ep_id, int offset, int nbyte)
{
	offset = (offset + 1) & ~1;
	MMIO16(USB_ADDR_RX(ep_id)) = offset;
	if (nbyte <= 62) {
		MMIO16(USB_COUNT_RX(ep_id)) = ((nbyte + 1) / 2) << 10;
		offset += ((nbyte + 1) & ~1);
	} else {
		MMIO16(USB_COUNT_RX(ep_id)) = USB_COUNT_RX_BL_SIZE |
			((nbyte + 31) / 32 - 1) << 10;
		offset += ((nbyte + 31) & ~31);
	}
	return offset;
}

int usbdevfs_assign_packet_memory_rx0(int ep_id, int offset, int nbyte)
{
	offset = (offset + 1) & ~1;
	MMIO16(USB_ADDR_TX(ep_id)) = offset;
	if (nbyte <= 62) {
		MMIO16(USB_COUNT_TX(ep_id)) = ((nbyte + 1) / 2) << 10;
		offset += ((nbyte + 1) & ~1);
	} else {
		MMIO16(USB_COUNT_TX(ep_id)) = USB_COUNT_RX_BL_SIZE |
			((nbyte + 31) / 32 - 1) << 10;
		offset += ((nbyte + 31) & ~31);
	}
	return offset;
}

void usbdevfs_setup_endpoint(int ep_id, enum usbdevfs_ep_type type, int addr)
{
	MMIO32(USB_EPR(ep_id)) = type | (addr & 0xf);
}

int usbdevfs_get_ep_status(int ep_id, int status)
{
	return MMIO32(USB_EPR(ep_id)) & status;
}

int usbdevfs_get_rx_count(int ep_id)
{
	return MMIO16(USB_COUNT_RX(ep_id)) & 0x3ff;
}

int usbdevfs_get_rx_count0(int ep_id)
{
	return MMIO16(USB_COUNT_TX(ep_id)) & 0x3ff;
}

int usbdevfs_write(int ep_id, void *buf, int nbyte)
{
	uint16_t *d;
	uint8_t *sc;
	int i;
	uint16_t *ss;

	d = (uint16_t *)(USB_SRAM_BASE + (MMIO16(USB_ADDR_TX(ep_id)) & ~1));
	if ((int)buf & 1) {
		sc = (uint8_t *)buf;
		for (i = 0; i < nbyte / 2; i++) {
			*d++ = *(sc + 1) << 8 | *sc;
			sc += 2;
		}
		if (nbyte & 1)
			*d = *sc;
	} else {
		ss = (uint16_t *)buf;
		for (i = 0; i < nbyte / 2; i++)
			*d++ = *ss++;
		if (nbyte & 1)
			*d = *ss;
	}
	MMIO16(USB_COUNT_TX(ep_id)) = nbyte;
	return nbyte;
}

int usbdevfs_write1(int ep_id, void *buf, int nbyte)
{
	uint16_t *d;
	uint8_t *sc;
	int i;
	uint16_t *ss;

	d = (uint16_t *)(USB_SRAM_BASE + (MMIO16(USB_ADDR_RX(ep_id)) & ~1));
	if ((int)buf & 1) {
		sc = (uint8_t *)buf;
		for (i = 0; i < nbyte / 2; i++) {
			*d++ = *(sc + 1) << 8 | *sc;
			sc += 2;
		}
		if (nbyte & 1)
			*d = *sc;
	} else {
		ss = (uint16_t *)buf;
		for (i = 0; i < nbyte / 2; i++)
			*d++ = *ss++;
		if (nbyte & 1)
			*d = *ss;
	}
	MMIO16(USB_COUNT_RX(ep_id)) = nbyte;
	return nbyte;
}

int usbdevfs_read(int ep_id, void *buf, int nbyte)
{
	uint16_t *s;
	int n;
	uint8_t *dc;
	int i;
	uint16_t r;
	uint16_t *ds;


	s = (uint16_t *)(USB_SRAM_BASE + MMIO16(USB_ADDR_RX(ep_id)));
	n = MMIO16(USB_COUNT_RX(ep_id)) & 0x3ff;
	if (nbyte < n)
		n = nbyte;
	if ((int)buf & 1) {
		dc = (uint8_t *)buf;
		for (i = 0; i < n / 2; i++) {
			r = *s++;
			*dc++ = r;
			*dc++ = r >> 8;
		}
		if (n & 1)
			*dc = *s;
	} else {
		ds = (uint16_t *)buf;
		for (i = 0; i < n / 2; i++)
			*ds++ = *s++;
		if (n & 1) {
			dc = (uint8_t *)ds;
			*dc = *s;
		}
	}
	return n;
}

int usbdevfs_read0(int ep_id, void *buf, int nbyte)
{
	uint16_t *s;
	int n;
	uint8_t *dc;
	int i;
	uint16_t r;
	uint16_t *ds;


	s = (uint16_t *)(USB_SRAM_BASE + MMIO16(USB_ADDR_TX(ep_id)));
	n = MMIO16(USB_COUNT_TX(ep_id)) & 0x3ff;
	if (nbyte < n)
		n = nbyte;
	if ((int)buf & 1) {
		dc = (uint8_t *)buf;
		for (i = 0; i < n / 2; i++) {
			r = *s++;
			*dc++ = r;
			*dc++ = r >> 8;
		}
		if (n & 1)
			*dc = *s;
	} else {
		ds = (uint16_t *)buf;
		for (i = 0; i < n / 2; i++)
			*ds++ = *s++;
		if (n & 1) {
			dc = (uint8_t *)ds;
			*dc = *s;
		}
	}
	return n;
}

static void ep_bit(int ep_id, int setbit, int resetbit)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	r ^= (USB_EPR_TOGGLE & setbit);
	r &= ~((USB_EPR_TOGGLE & ~setbit & ~resetbit) |
	       (USB_EPR_C_W0 & resetbit) |
	       (USB_EPR_RW & resetbit));
	MMIO32(USB_EPR(ep_id)) = r | (USB_EPR_C_W0 & ~resetbit) |
		(USB_EPR_RW & setbit);
}

/*
 * Control transfer state
 * STALL:		RX STALL (01), TX STALL (01)
 * DATA_OUT:		RX VALID (11), TX STALL (01)
 * LAST_DATA_OUT:	RX VALID (11), TX NAK (10)
 * STATUS_OUT:		RX VALID (11), TX STALL (01), STATUS_OUT
 * DATA_IN:		RX STALL (01), TX VALID (11)
 * LAST_DATA_IN:	RX NAK (10), TX VALID (11)
 * STATUS_IN:		RX STALL (01), TX VALID (11)
 */
void usbdevfs_set_control_state(int ep_id, enum usbdevfs_control_state state)
{
	switch (state) {
	case USBDEVFS_STALL:
		ep_bit(ep_id, USB_EPR_STAT_RX0 | USB_EPR_STAT_TX0,
		       USB_EPR_STAT_RX1 | USB_EPR_STAT_TX1 | USB_EPR_EP_KIND |
		       USB_EPR_CTR_RX | USB_EPR_CTR_TX);
		break;
	case USBDEVFS_DATA_OUT:
		ep_bit(ep_id, USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0 |
		       USB_EPR_STAT_TX0,
		       USB_EPR_STAT_TX1 | USB_EPR_EP_KIND| USB_EPR_CTR_RX);
		break;
	case USBDEVFS_LAST_DATA_OUT:
		ep_bit(ep_id, USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0 |
		       USB_EPR_STAT_TX1,
		       USB_EPR_STAT_TX0 | USB_EPR_EP_KIND | USB_EPR_CTR_RX);
		break;
	case USBDEVFS_STATUS_OUT:
		ep_bit(ep_id, USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0 |
		       USB_EPR_STAT_TX0 | USB_EPR_EP_KIND,
		       USB_EPR_STAT_TX1 | USB_EPR_CTR_TX);
		break;
	case USBDEVFS_DATA_IN:
		ep_bit(ep_id, USB_EPR_STAT_RX0 |
		       USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0,
		       USB_EPR_STAT_RX1 | USB_EPR_EP_KIND |
		       USB_EPR_CTR_RX | USB_EPR_CTR_TX);
		break;
	case USBDEVFS_LAST_DATA_IN:
		ep_bit(ep_id, USB_EPR_STAT_RX1 |
		       USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0,
		       USB_EPR_STAT_RX0 | USB_EPR_EP_KIND |
		       USB_EPR_CTR_RX | USB_EPR_CTR_TX);
		break;
	case USBDEVFS_STATUS_IN:
		ep_bit(ep_id, USB_EPR_STAT_RX0 |
		       USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0,
		       USB_EPR_STAT_RX1 | USB_EPR_EP_KIND |
		       USB_EPR_CTR_RX);
		break;
	default:
		break;
	}
}

/*
 * Enable Tx
 * Isochronous:	TX DISABLED (00) -> TX VALID (11)
 * Other:	TX DISABLED (00) -> TX NAK (10)
 *		TX STALL (01)    -> TX NAK (10)
 */
void usbdevfs_enable_ep_tx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_EP_TYPE1 | USB_EPR_EP_TYPE0)) ==
	    USB_EPR_EP_TYPE_ISO) {
		if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
		    USB_EPR_STAT_TX_DISABLED)
			MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_TX1 |
				USB_EPR_STAT_TX0;
	} else {
		switch (r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) {
		case USB_EPR_STAT_TX_DISABLED:
			MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_TX1;
			break;
		case USB_EPR_STAT_TX_STALL:
			MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_TX1 |
				USB_EPR_STAT_TX0;
			break;
		default:
			break;
		}
	}
}

/*
 * Start Tx
 * TX NAK (10) -> TX VALID (11)
 */
void usbdevfs_start_ep_tx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
	    USB_EPR_STAT_TX_NAK) {
		r &= ~(USB_EPR_CTR_TX | USB_EPR_DTOG_TX | USB_EPR_DTOG_RX |
		       USB_EPR_STAT_TX1);
		MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_TX0;
	}
}

/*
 * Enable Rx
 * RX DISABLED (00) -> RX VALID (11)
 * RX STALL (01)    -> RX VALID (11)
 * RX NAK (10)      -> RX VALID (11)
 */
void usbdevfs_enable_ep_rx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	switch (r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) {
	case USB_EPR_STAT_RX_DISABLED:
		MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_RX1 |
			USB_EPR_STAT_RX0;
		break;
	case USB_EPR_STAT_RX_STALL:
		r &= ~USB_EPR_STAT_RX0;
		MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_RX1;
		break;
	case USB_EPR_STAT_RX_NAK:
		r &= ~(USB_EPR_CTR_RX | USB_EPR_DTOG_RX | USB_EPR_DTOG_TX |
		       USB_EPR_STAT_RX1);
		MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_RX0;
		break;
	default:
		break;
	}
}

/*
 * Disable Tx
 * TX STALL (01)			-> TX DISABLED (00)
 * TX NAK (10)				-> TX DISABLED (00)
 * TX VALID (11)( -> TX NAK (10))	-> TX DISABLED (00)
 */
void usbdevfs_disable_ep_tx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
	    USB_EPR_STAT_TX_DISABLED)
		return;
	r &= ~USB_EPR_CTR_TX;
	MMIO32(USB_EPR(ep_id)) = r;
	if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
	    USB_EPR_STAT_TX_STALL ||
	    (r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
	    USB_EPR_STAT_TX_NAK)
		return;
	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) !=
	    USB_EPR_STAT_TX_DISABLED) {
		r &= ~USB_EPR_CTR_TX;
		MMIO32(USB_EPR(ep_id)) = r;
	}
}

/*
 * Disable Rx
 * RX STALL (01)			-> RX DISABLED (00)
 * RX NAK (10)				-> RX DISABLED (00)
 * RX VALID (11)( -> RX NAK (10))	-> RX DISABLED (00)
 */
void usbdevfs_disable_ep_rx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) ==
	    USB_EPR_STAT_RX_DISABLED)
		return;
	r &= ~USB_EPR_CTR_RX;
	MMIO32(USB_EPR(ep_id)) = r;
	if ((r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) ==
	    USB_EPR_STAT_RX_STALL ||
	    (r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) ==
	    USB_EPR_STAT_RX_NAK)
		return;
	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) !=
	    USB_EPR_STAT_RX_DISABLED) {
		r &= ~USB_EPR_CTR_RX;
		MMIO32(USB_EPR(ep_id)) = r;
	}
}

/*
 * Halt Tx
 * TX NAK (10)				-> TX STALL (01)
 * TX VALID (11)( -> TX NAK (10))	-> TX STALL (01)
 */
void usbdevfs_halt_ep_tx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
	    USB_EPR_STAT_TX_NAK) {
		MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_TX0;
		return;
	}
	if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) ==
	    USB_EPR_STAT_TX_VALID) {
		r &= ~USB_EPR_STAT_TX0;
		MMIO32(USB_EPR(ep_id)) = r;
		r = MMIO32(USB_EPR(ep_id));
		if ((r & (USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)) !=
		    USB_EPR_STAT_TX_STALL) {
			r &= ~USB_EPR_CTR_TX;
			MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_TX0;
		}
	}
}

/*
 * Halt Rx
 * RX NAK (10)				-> RX STALL (01)
 * RX VALID (11)( -> RX NAK (10))	-> RX STALL (01)
 */
void usbdevfs_halt_ep_rx(int ep_id)
{
	int r;

	r = MMIO32(USB_EPR(ep_id));
	if ((r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) ==
	    USB_EPR_STAT_RX_NAK) {
		MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_RX0;
		return;
	}
	if ((r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) ==
	    USB_EPR_STAT_RX_VALID) {
		r &= ~USB_EPR_STAT_RX0;
		MMIO32(USB_EPR(ep_id)) = r;
		r = MMIO32(USB_EPR(ep_id));
		if ((r & (USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0)) !=
		    USB_EPR_STAT_RX_STALL) {
			r &= ~USB_EPR_CTR_RX;
			MMIO32(USB_EPR(ep_id)) = r | USB_EPR_STAT_RX0;
		}
	}
}

void usbdevfs_clear_ep_interrupt(int ep_id)
{
	MMIO32(USB_EPR(ep_id)) &= ~(USB_EPR_CTR_RX | USB_EPR_CTR_TX |
				    USB_EPR_DTOG_RX | USB_EPR_DTOG_TX |
				    USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0 |
				    USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0);
}

void usbdevfs_reset_ep_data_toggle(int ep_id)
{
	MMIO32(USB_EPR(ep_id)) &= ~(USB_EPR_STAT_RX1 | USB_EPR_STAT_RX0 |
				    USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0);
}
