/*
 * USB
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

/*
 * STMicroelectronics(www.st.com)
 *
 * RM0376: Reference manual
 * Ultra-low-power STM32L0x2 advanced ARM-based 32-bit MCUs
 * (19-Feb-2016 Rev 3)
 *
 * 31  Universal serial bus full-speed device interface (USB)
 *
 *
 * DS10182: STM32L052x6 STM32L052x8
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 64 KB Flash memory,
 * 8 KB SRAM, 2 KB EEPROM, USB, ADC, DAC
 * (22-Mar-2016 Rev 6)
 *
 * Table 78. USB startup time
 *
 *
 * DS10183: STM32L062K8 STM32L062T8
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, 64 KB Flash,
 * 8 KB SRAM, 2 KB EEPROM, USB, ADC, DAC, AES
 * (09-Jun-2016 Rev 7)
 *
 * Table 69. USB startup time
 *
 *
 * DS10689: STM32L072x8 STM32L072xB STM32L072xZ
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 192KB Flash,
 * 20KB SRAM, 6KB EEPROM, USB, ADC, DACs
 * (22-Mar-2016 Rev 3)
 *
 * Table 79. USB startup time
 *
 *
 * DS10688: STM32L082KB STM32L082KZ STM32L082CZ
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 192KB Flash,
 * 20KB SRAM, 6KB EEPROM, USB, ADC, DACs, AES
 * (03-May-2016 Rev 4)
 *
 * Table 75. USB startup time
 */

/* --- USB registers ------------------------------------------------------- */
/*
 * Offset	Register
 * 0x00		USB_EP0R	USB endpoint 0 register
 * 0x04		USB_EP1R	USB endpoint 1 register
 * 0x08		USB_EP2R	USB endpoint 2 register
 * 0x0c		USB_EP3R	USB endpoint 3 register
 * 0x10		USB_EP4R	USB endpoint 4 register
 * 0x14		USB_EP5R	USB endpoint 5 register
 * 0x18		USB_EP6R	USB endpoint 6 register
 * 0x1c		USB_EP7R	USB endpoint 7 register
 * 0x40		USB_CNTR	USB control register
 * 0x44		USB_ISTR	USB interrupt status register
 * 0x48		USB_FNR		USB frame number register
 * 0x4c		USB_DADDR	USB device address
 * 0x50		USB_BTABLE	Buffer table address
 * 0x54		USB_LPMCSR	LPM control and status register
 * 0x58		USB_BCDR	Battery charging detector
 */

#define USB_EPR(n)			(USB_FS_BASE + (n) * 4)
#define USB_EP0R			USB_EPR(0)
#define USB_EP1R			USB_EPR(1)
#define USB_EP2R			USB_EPR(2)
#define USB_EP3R			USB_EPR(3)
#define USB_EP4R			USB_EPR(4)
#define USB_EP5R			USB_EPR(5)
#define USB_EP6R			USB_EPR(6)
#define USB_EP7R			USB_EPR(7)

#define USB_CNTR			(USB_FS_BASE + 0x40)
#define USB_ISTR			(USB_FS_BASE + 0x44)
#define USB_FNR				(USB_FS_BASE + 0x48)
#define USB_DADDR			(USB_FS_BASE + 0x4c)
#define USB_BTABLE			(USB_FS_BASE + 0x50)
#define USB_LPMCSR			(USB_FS_BASE + 0x54)
#define USB_BCDR			(USB_FS_BASE + 0x58)

/* --- Buffer descriptor table --------------------------------------------- */

#define USB_ADDR_TX(n)			(USB_SRAM_BASE + MMIO32(USB_BTABLE) + \
					 (n) * 8 + 0)
#define USB_ADDR0_TX			USB_ADDR_TX(0)
#define USB_ADDR1_TX			USB_ADDR_TX(1)
#define USB_ADDR2_TX			USB_ADDR_TX(2)
#define USB_ADDR3_TX			USB_ADDR_TX(3)
#define USB_ADDR4_TX			USB_ADDR_TX(4)
#define USB_ADDR5_TX			USB_ADDR_TX(5)
#define USB_ADDR6_TX			USB_ADDR_TX(6)
#define USB_ADDR7_TX			USB_ADDR_TX(7)

#define USB_COUNT_TX(n)			(USB_SRAM_BASE + MMIO32(USB_BTABLE) + \
					 (n) * 8 + 2)
#define USB_COUNT0_TX			USB_COUNT_TX(0)
#define USB_COUNT1_TX			USB_COUNT_TX(1)
#define USB_COUNT2_TX			USB_COUNT_TX(2)
#define USB_COUNT3_TX			USB_COUNT_TX(3)
#define USB_COUNT4_TX			USB_COUNT_TX(4)
#define USB_COUNT5_TX			USB_COUNT_TX(5)
#define USB_COUNT6_TX			USB_COUNT_TX(6)
#define USB_COUNT7_TX			USB_COUNT_TX(7)

#define USB_ADDR_RX(n)			(USB_SRAM_BASE + MMIO32(USB_BTABLE) + \
					 (n) * 8 + 4)
#define USB_ADDR0_RX			USB_ADDR_RX(0)
#define USB_ADDR1_RX			USB_ADDR_RX(1)
#define USB_ADDR2_RX			USB_ADDR_RX(2)
#define USB_ADDR3_RX			USB_ADDR_RX(3)
#define USB_ADDR4_RX			USB_ADDR_RX(4)
#define USB_ADDR5_RX			USB_ADDR_RX(5)
#define USB_ADDR6_RX			USB_ADDR_RX(6)
#define USB_ADDR7_RX			USB_ADDR_RX(7)

#define USB_COUNT_RX(n)			(USB_SRAM_BASE + MMIO32(USB_BTABLE) + \
					 (n) * 8 + 6)
#define USB_COUNT0_RX			USB_COUNT_RX(0)
#define USB_COUNT1_RX			USB_COUNT_RX(1)
#define USB_COUNT2_RX			USB_COUNT_RX(2)
#define USB_COUNT3_RX			USB_COUNT_RX(3)
#define USB_COUNT4_RX			USB_COUNT_RX(4)
#define USB_COUNT5_RX			USB_COUNT_RX(5)
#define USB_COUNT6_RX			USB_COUNT_RX(6)
#define USB_COUNT7_RX			USB_COUNT_RX(7)

/* --- USB_CNTR values ----------------------------------------------------- */

#define USB_CNTR_CTRM			(1 << 15)
#define USB_CNTR_PMAOVRM		(1 << 14)
#define USB_CNTR_ERRM			(1 << 13)
#define USB_CNTR_WKUPM			(1 << 12)
#define USB_CNTR_SUSPM			(1 << 11)
#define USB_CNTR_RESETM			(1 << 10)
#define USB_CNTR_SOFM			(1 << 9)
#define USB_CNTR_ESOFM			(1 << 8)
#define USB_CNTR_L1REQM			(1 << 7)
#define USB_CNTR_L1RESUME		(1 << 5)
#define USB_CNTR_RESUME			(1 << 4)
#define USB_CNTR_FSUSP			(1 << 3)
#define USB_CNTR_LP_MODE		(1 << 2)
#define USB_CNTR_PDWN			(1 << 1)
#define USB_CNTR_FRES			(1 << 0)

/* --- USB_ISTR values ----------------------------------------------------- */

#define USB_ISTR_CTR			(1 << 15)
#define USB_ISTR_PMAOVR			(1 << 14)
#define USB_ISTR_ERR			(1 << 13)
#define USB_ISTR_WKUP			(1 << 12)
#define USB_ISTR_SUSP			(1 << 11)
#define USB_ISTR_RESET			(1 << 10)
#define USB_ISTR_SOF			(1 << 9)
#define USB_ISTR_ESOF			(1 << 8)
#define USB_ISTR_L1REQ			(1 << 7)
#define USB_ISTR_DIR			(1 << 4)
#define USB_ISTR_EP_ID3			(1 << 3)
#define USB_ISTR_EP_ID2			(1 << 2)
#define USB_ISTR_EP_ID1			(1 << 1)
#define USB_ISTR_EP_ID0			(1 << 0)

/* --- USB_FNR values ------------------------------------------------------ */

#define USB_FNR_RXDP			(1 << 15)
#define USB_FNR_RXDM			(1 << 14)
#define USB_FNR_LCK			(1 << 13)
#define USB_FNR_LSOF1			(1 << 12)
#define USB_FNR_LSOF0			(1 << 11)
#define USB_FNR_FN10			(1 << 10)
#define USB_FNR_FN9			(1 << 9)
#define USB_FNR_FN8			(1 << 8)
#define USB_FNR_FN7			(1 << 7)
#define USB_FNR_FN6			(1 << 6)
#define USB_FNR_FN5			(1 << 5)
#define USB_FNR_FN4			(1 << 4)
#define USB_FNR_FN3			(1 << 3)
#define USB_FNR_FN2			(1 << 2)
#define USB_FNR_FN1			(1 << 1)
#define USB_FNR_FN0			(1 << 0)

/* --- USB_DADDR values ---------------------------------------------------- */

#define USB_DADDR_EF			(1 << 7)
#define USB_DADDR_ADD6			(1 << 6)
#define USB_DADDR_ADD5			(1 << 5)
#define USB_DADDR_ADD4			(1 << 4)
#define USB_DADDR_ADD3			(1 << 3)
#define USB_DADDR_ADD2			(1 << 2)
#define USB_DADDR_ADD1			(1 << 1)
#define USB_DADDR_ADD0			(1 << 0)

/* --- USB_BTABLE values --------------------------------------------------- */

/* USB_BTABLE[15:3]: BTABLE[15:3]: Buffer table */

/* --- USB_LPMCSR values --------------------------------------------------- */

#define USB_LPMCSR_BESL3		(1 << 7)
#define USB_LPMCSR_BESL2		(1 << 6)
#define USB_LPMCSR_BESL1		(1 << 5)
#define USB_LPMCSR_BESL0		(1 << 4)
#define USB_LPMCSR_REMWAKE		(1 << 3)
#define USB_LPMCSR_LPMACK		(1 << 1)
#define USB_LPMCSR_LPMEN		(1 << 0)

/* --- USB_BCDR values ----------------------------------------------------- */

#define USB_BCDR_DPPU			(1 << 15)
#define USB_BCDR_PS2DET			(1 << 7)
#define USB_BCDR_SDET			(1 << 6)
#define USB_BCDR_PDET			(1 << 5)
#define USB_BCDR_DCDET			(1 << 4)
#define USB_BCDR_SDEN			(1 << 3)
#define USB_BCDR_PDEN			(1 << 2)
#define USB_BCDR_DCDEN			(1 << 1)
#define USB_BCDR_BCDEN			(1 << 0)

/* --- USB_EPnR values ----------------------------------------------------- */

#define USB_EPR_CTR_RX			(1 << 15)
#define USB_EPR_DTOG_RX			(1 << 14)
#define USB_EPR_STAT_RX1		(1 << 13)
#define USB_EPR_STAT_RX0		(1 << 12)
#define USB_EPR_SETUP			(1 << 11)
#define USB_EPR_EP_TYPE1		(1 << 10)
#define USB_EPR_EP_TYPE0		(1 << 9)
#define USB_EPR_EP_KIND			(1 << 8)
#define USB_EPR_CTR_TX			(1 << 7)
#define USB_EPR_DTOG_TX			(1 << 6)
#define USB_EPR_STAT_TX1		(1 << 5)
#define USB_EPR_STAT_TX0		(1 << 4)
#define USB_EPR_EA3	       		(1 << 3)
#define USB_EPR_EA2	       		(1 << 2)
#define USB_EPR_EA1	       		(1 << 1)
#define USB_EPR_EA0	       		(1 << 0)

/* Reception status */
#define USB_EPR_STAT_RX_DISABLED	(0 << 12)
#define USB_EPR_STAT_RX_STALL		(1 << 12)
#define USB_EPR_STAT_RX_NAK		(2 << 12)
#define USB_EPR_STAT_RX_VALID		(3 << 12)

/* Endpoint type */
#define USB_EPR_EP_TYPE_BULK		(0 << 9)
#define USB_EPR_EP_TYPE_CONTROL		(1 << 9)
#define USB_EPR_EP_TYPE_ISO		(2 << 9)
#define USB_EPR_EP_TYPE_INTERRUPT	(3 << 9)

/* Transmission status */
#define USB_EPR_STAT_TX_DISABLED	(0 << 4)
#define USB_EPR_STAT_TX_STALL		(1 << 4)
#define USB_EPR_STAT_TX_NAK		(2 << 4)
#define USB_EPR_STAT_TX_VALID		(3 << 4)

#define USB_EPR_TOGGLE			(USB_EPR_DTOG_RX | USB_EPR_STAT_RX1 | \
					 USB_EPR_STAT_RX0 | USB_EPR_DTOG_TX | \
					 USB_EPR_STAT_TX1 | USB_EPR_STAT_TX0)

#define USB_EPR_C_W0			(USB_EPR_CTR_RX | USB_EPR_CTR_TX)
#define USB_EPR_RW			(USB_EPR_EP_TYPE1 | \
					 USB_EPR_EP_TYPE0 | USB_EPR_EP_KIND | \
					 USB_EPR_EA3 | USB_EPR_EA2 | \
					 USB_EPR_EA1 | USB_EPR_EA0)

/* --- USB_ADDRn_TX values ------------------------------------------------- */

/* USB_ADDRn_TX[15:1]: ADDRn_TX[15:1]: Transmission buffer address */

/* --- USB_COUNTn_TX values ------------------------------------------------ */

/* USB_COUNTn_TX[9:0]: COUNTn_TX[9:0]: Transmission byte count */

/* --- USB_ADDRn_RX values ------------------------------------------------- */

/* USB_ADDRn_RX[15:1]: ADDRn_RX[15:1]: Reception buffer address */

/* --- USB_COUNTn_RX values ------------------------------------------------ */

#define USB_COUNT_RX_BL_SIZE		(1 << 15)
#define USB_COUNT_RX_NUM_BLOCK4		(1 << 14)
#define USB_COUNT_RX_NUM_BLOCK3		(1 << 13)
#define USB_COUNT_RX_NUM_BLOCK2		(1 << 12)
#define USB_COUNT_RX_NUM_BLOCK1		(1 << 11)
#define USB_COUNT_RX_NUM_BLOCK0		(1 << 10)

/* USB_COUNTn_RX[9:0]: COUNTn_RX[9:0]: Reception byte count */

/* --- Miscellaneous definitions ------------------------------------------- */

/* USB transceiver startup time */
#define USBDEVFS_T_STARTUP		1	/* 1 us */

/* Endpoint */
#define USBDEVFS_EP_MAX			8

/* Buffer descriptor table */
#define USBDEVFS_BUFFER_TABLE_SIZE	(2 * 4 * USBDEVFS_EP_MAX)

/* --- Function prototypes ------------------------------------------------- */

/* Alias */
#define usbdevfs_assign_packet_memory_tx0 usbdevfs_assign_packet_memory_tx
#define usbdevfs_assign_packet_memory_rx1 usbdevfs_assign_packet_memory_rx
#define usbdevfs_get_rx_count1 usbdevfs_get_rx_count
#define usbdevfs_write0 usbdevfs_write
#define usbdevfs_read1 usbdevfs_read

/* Interrupt and status */
enum {
	USBDEVFS_L1REQ = (1 << 7),
	USBDEVFS_ESOF = (1 << 8),
	USBDEVFS_SOF = (1 << 9),
	USBDEVFS_RESET = (1 << 10),
	USBDEVFS_SUSP = (1 << 11),
	USBDEVFS_WKUP = (1 << 12),
	USBDEVFS_ERR = (1 << 13),
	USBDEVFS_PMAOVR = (1 << 14),
	USBDEVFS_CTR = (1 << 15),

	USBDEVFS_EP_ID = (15 << 0),
	USBDEVFS_DIR = (1 << 4)
};

/* Endpoint type */
enum usbdevfs_ep_type {
	USBDEVFS_BULK = 0,
	USBDEVFS_BULK_DBL_BUF = (1 << 8),
	USBDEVFS_CONTROL = (2 << 8),
	USBDEVFS_ISOCHRONOUS = (4 << 8),
	USBDEVFS_INTERRUPT = (6 << 8)
};

/* Endpoint status */
enum {
	USBDEVFS_TX_DATA1 = (1 << 6),
	USBDEVFS_TX = (1 << 7),
	USBDEVFS_SETUP = (1 << 11),
	USBDEVFS_RX_DATA1 = (1 << 14),
	USBDEVFS_RX = (1 << 15)
};

/* Control transfer state */
enum usbdevfs_control_state {
	USBDEVFS_STALL,
	USBDEVFS_DATA_IN,
	USBDEVFS_DATA_OUT,
	USBDEVFS_LAST_DATA_IN,
	USBDEVFS_LAST_DATA_OUT,
	USBDEVFS_STATUS_IN,
	USBDEVFS_STATUS_OUT
};

/* Battery charging detector (BCD) mode */
enum usbdevfs_bcd_mode {
	USBDEVFS_BCD_OFF = 0,
	USBDEVFS_DCD = 3,
	USBDEVFS_PD = 5,
	USBDEVFS_SD = 9
};

/* BCD status */
enum {
	USBDEVFS_DCDET = (1 << 4),
	USBDEVFS_PDET = (1 << 5),
	USBDEVFS_SDET = (1 << 6),
	USBDEVFS_PS2DET = (1 << 7)
};

void usbdevfs_enable_usb_reset(void);
void usbdevfs_disable_usb_reset(void);
void usbdevfs_enable_power_down(void);
void usbdevfs_disable_power_down(void);
void usbdevfs_enable_low_power_mode(void);
void usbdevfs_disable_low_power_mode(void);
void usbdevfs_enable_suspend_mode(void);
void usbdevfs_disable_suspend_mode(void);
void usbdevfs_enable_resume(void);
void usbdevfs_disable_resume(void);
void usbdevfs_request_lpm_l1_resume(void);
void usbdevfs_enable_interrupt(int interrupt);
void usbdevfs_disable_interrupt(int interrupt);
int usbdevfs_get_interrupt_mask(int interrupt);
int usbdevfs_get_interrupt_status(int interrupt);
void usbdevfs_clear_interrupt(int interrupt);
int usbdevfs_get_frame_number(void);
void usbdevfs_set_device_address(int addr);
void usbdevfs_set_buffer_table_address(int offset);
void usbdevfs_enable_lpm(void);
void usbdevfs_disable_lpm(void);
void usbdevfs_enable_lpm_ack(void);
void usbdevfs_disable_lpm_ack(void);
int usbdevfs_get_lpm_status(void);
void usbdevfs_enable_dp_pull_up(void);
void usbdevfs_disable_dp_pull_up(void);
void usbdevfs_set_bcd_mode(enum usbdevfs_bcd_mode mode);
int usbdevfs_get_bcd_status(int status);
int usbdevfs_assign_packet_memory_tx(int ep_id, int offset, int nbyte);
int usbdevfs_assign_packet_memory_tx1(int ep_id, int offset, int nbyte);
int usbdevfs_assign_packet_memory_rx(int ep_id, int offset, int nbyte);
int usbdevfs_assign_packet_memory_rx0(int ep_id, int offset, int nbyte);
void usbdevfs_setup_endpoint(int ep_id, enum usbdevfs_ep_type type, int addr);
int usbdevfs_get_ep_status(int ep_id, int status);
int usbdevfs_get_rx_count(int ep_id);
int usbdevfs_get_rx_count0(int ep_id);
int usbdevfs_write(int ep_id, void *buf, int nbyte);
int usbdevfs_write1(int ep_id, void *buf, int nbyte);
int usbdevfs_read(int ep_id, void *buf, int nbyte);
int usbdevfs_read0(int ep_id, void *buf, int nbyte);
void usbdevfs_set_control_state(int ep_id, enum usbdevfs_control_state state);
void usbdevfs_enable_ep_tx(int ep_id);
void usbdevfs_start_ep_tx(int ep_id);
void usbdevfs_enable_ep_rx(int ep_id);
void usbdevfs_disable_ep_tx(int ep_id);
void usbdevfs_disable_ep_rx(int ep_id);
void usbdevfs_halt_ep_tx(int ep_id);
void usbdevfs_halt_ep_rx(int ep_id);
void usbdevfs_clear_ep_interrupt(int ep_id);
void usbdevfs_reset_ep_data_toggle(int ep_id);
