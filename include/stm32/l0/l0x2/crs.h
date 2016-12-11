/*
 * CRS
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
 * (14-Nov-2016 Rev 4)
 *
 * 8  Clock recovery system (CRS)
 */

/* --- CRS registers ------------------------------------------------------- */
/*
 * Offset	Register
 * 0x00		CRS_CR		CRS control register
 * 0x04		CRS_CFGR	CRS configuration register
 * 0x08		CRS_ISR		CRS interrupt and status register
 * 0x0c		CRS_ICR		CRS interrupt flag clear register
 */

#define CRS_CR				(CRS_BASE + 0x00)
#define CRS_CFGR			(CRS_BASE + 0x04)
#define CRS_ISR				(CRS_BASE + 0x08)
#define CRS_ICR				(CRS_BASE + 0x0c)

/* --- CRS_CR values ------------------------------------------------------- */

#define CRS_CR_TRIM5			(1 << 13)
#define CRS_CR_TRIM4			(1 << 12)
#define CRS_CR_TRIM3			(1 << 11)
#define CRS_CR_TRIM2			(1 << 10)
#define CRS_CR_TRIM1			(1 << 9)
#define CRS_CR_TRIM0			(1 << 8)
#define CRS_CR_SWSYNC			(1 << 7)
#define CRS_CR_AUTOTRIMEN		(1 << 6)
#define CRS_CR_CEN			(1 << 5)
#define CRS_CR_ESYNCIE			(1 << 3)
#define CRS_CR_ERRIE			(1 << 2)
#define CRS_CR_SYNCWARNIE		(1 << 1)
#define CRS_CR_SYNCOKIE			(1 << 0)

/* --- CRS_CFGR values ----------------------------------------------------- */

#define CRS_CFGR_SYNCPOL		(1 << 31)
#define CRS_CFGR_SYNCSRC1		(1 << 29)
#define CRS_CFGR_SYNCSRC0		(1 << 28)
#define CRS_CFGR_SYNCDIV2		(1 << 26)
#define CRS_CFGR_SYNCDIV1		(1 << 25)
#define CRS_CFGR_SYNCDIV0		(1 << 24)
/* CRS_CFGR[23:16]: FELIM[7:0]: Frequency error limit */
/* CRS_CFGR[15:0]: RELOAD[15:0]: Counter reload value */

/* SYNC signal source selection */
#define CRS_CFGR_SYNCSRC_GPIO		(0 << 28)
#define CRS_CFGR_SYNCSRC_LSE		(1 << 28)
#define CRS_CFGR_SYNCSRC_USB_SOF	(2 << 28)

/* SYNC divider */
#define CRS_CFGR_SYNCDIV_1		(0 << 24)
#define CRS_CFGR_SYNCDIV_2		(1 << 24)
#define CRS_CFGR_SYNCDIV_4		(2 << 24)
#define CRS_CFGR_SYNCDIV_8		(3 << 24)
#define CRS_CFGR_SYNCDIV_16		(4 << 24)
#define CRS_CFGR_SYNCDIV_32		(5 << 24)
#define CRS_CFGR_SYNCDIV_64		(6 << 24)
#define CRS_CFGR_SYNCDIV_128		(7 << 24)

/* --- CRS_ISR values ------------------------------------------------------ */

/* CRS_ISR[31:16]: FECAP[15:0]: Frequency error capture */
#define CRS_ISR_FEDIR			(1 << 15)
#define CRS_ISR_TRIMOVF			(1 << 10)
#define CRS_ISR_SYNCMISS		(1 << 9)
#define CRS_ISR_SYNCERR			(1 << 8)
#define CRS_ISR_ESYNCF			(1 << 3)
#define CRS_ISR_ERRF			(1 << 2)
#define CRS_ISR_SYNCWARNF		(1 << 1)
#define CRS_ISR_SYNCOKF			(1 << 0)

/* --- CRS_ICR values ------------------------------------------------------ */

#define CRS_ICR_ESYNCC			(1 << 3)
#define CRS_ICR_ERRC			(1 << 2)
#define CRS_ICR_SYNCWARNC		(1 << 1)
#define CRS_ICR_SYNCOKC			(1 << 0)

/* --- Function prototypes ------------------------------------------------- */

/* SYNC signal source */
enum crs_src {
	CRS_GPIO = (0 << 28),
	CRS_LSE = (1 << 28),
	CRS_USB_SOF = (2 << 28),

	CRS_GPIO_FALLING = (8 << 28),
	CRS_LSE_FALLING = (9 << 28),
	CRS_USB_SOF_FALLING = (10 << 28)
};

/* interrupt and status */
enum {
	CRS_SYNCOK = (1 << 0),
	CRS_SYNCWARN = (1 << 1),
	CRS_ERR = (1 << 2),
	CRS_ESYNC = (1 << 3),

	CRS_SYNCERR = (1 << 8),
	CRS_SYNCMISS = (1 << 9),
	CRS_TRIMOVF = (1 << 10),
	CRS_FEDIR = (1 << 15),
	CRS_FECAP = (0xffff << 16)
};

void crs_config(int reload, int felim, int div, enum crs_src src);
void crs_enable(void);
void crs_disable(void);
void crs_enable_autotrim(void);
void crs_disable_autotrim(void);
void crs_software_sync(void);
void crs_set_trim(int trim);
void crs_enable_interrupt(int interrupt);
void crs_disable_interrupt(int interrupt);
int crs_get_interrupt_mask(int interrupt);
int crs_get_interrupt_status(int interrupt);
void crs_clear_interrupt(int interrupt);
