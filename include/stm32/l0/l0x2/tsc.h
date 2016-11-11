/*
 * TSC
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
 * 17  Touch sensing controller (TSC)
 */

/* --- TSC registers ------------------------------------------------------- */
/*
 * Offset	Register
 * 0x00		TSC_CR		TSC control register
 * 0x04		TSC_IER		TSC interrupt enable register
 * 0x08		TSC_ICR		TSC interrupt clear register
 * 0x0c		TSC_ISR		TSC interrupt status register
 * 0x10		TSC_IOHCR	TSC I/O hysteresis control register
 * 0x18		TSC_IOASCR	TSC I/O analog switch control register
 * 0x20		TSC_IOSCR	TSC I/O sampling control register
 * 0x28		TSC_IOCCR	TSC I/O channel control register
 * 0x30		TSC_IOGCSR	TSC I/O group control status register
 * 0x34		TSC_IOG1CR	TSC I/O group 1 counter register
 * 0x38		TSC_IOG2CR	TSC I/O group 2 counter register
 * 0x3c		TSC_IOG3CR	TSC I/O group 3 counter register
 * 0x40		TSC_IOG4CR	TSC I/O group 4 counter register
 * 0x44		TSC_IOG5CR	TSC I/O group 5 counter register
 * 0x48		TSC_IOG6CR	TSC I/O group 6 counter register
 * 0x4c		TSC_IOG7CR	TSC I/O group 7 counter register
 * 0x50		TSC_IOG8CR	TSC I/O group 8 counter register
 */

#define TSC_CR				(TSC_BASE + 0x00)
#define TSC_IER				(TSC_BASE + 0x04)
#define TSC_ICR				(TSC_BASE + 0x08)
#define TSC_ISR				(TSC_BASE + 0x0c)
#define TSC_IOHCR			(TSC_BASE + 0x10)
#define TSC_IOASCR			(TSC_BASE + 0x18)
#define TSC_IOSCR			(TSC_BASE + 0x20)
#define TSC_IOCCR			(TSC_BASE + 0x28)
#define TSC_IOGCSR			(TSC_BASE + 0x30)

#define TSC_IOGCR(x)			(TSC_BASE + 0x30 + (x) * 4)
#define TSC_IOG1CR			TSC_IOGCR(1)
#define TSC_IOG2CR			TSC_IOGCR(2)
#define TSC_IOG3CR			TSC_IOGCR(3)
#define TSC_IOG4CR			TSC_IOGCR(4)
#define TSC_IOG5CR			TSC_IOGCR(5)
#define TSC_IOG6CR			TSC_IOGCR(6)
#define TSC_IOG7CR			TSC_IOGCR(7)
#define TSC_IOG8CR			TSC_IOGCR(8)

/* --- TSC_CR values ------------------------------------------------------- */

#define TSC_CR_CTPH3			(1 << 31)
#define TSC_CR_CTPH2			(1 << 30)
#define TSC_CR_CTPH1			(1 << 29)
#define TSC_CR_CTPH0			(1 << 28)
#define TSC_CR_CTPL3			(1 << 27)
#define TSC_CR_CTPL2			(1 << 26)
#define TSC_CR_CTPL1			(1 << 25)
#define TSC_CR_CTPL0			(1 << 24)
#define TSC_CR_SSD6			(1 << 23)
#define TSC_CR_SSD5			(1 << 22)
#define TSC_CR_SSD4			(1 << 21)
#define TSC_CR_SSD3			(1 << 20)
#define TSC_CR_SSD2			(1 << 19)
#define TSC_CR_SSD1			(1 << 18)
#define TSC_CR_SSD0			(1 << 17)
#define TSC_CR_SSE			(1 << 16)
#define TSC_CR_SSPSC			(1 << 15)
#define TSC_CR_PGPSC2			(1 << 14)
#define TSC_CR_PGPSC1			(1 << 13)
#define TSC_CR_PGPSC0			(1 << 12)
#define TSC_CR_MCV2			(1 << 7)
#define TSC_CR_MCV1			(1 << 6)
#define TSC_CR_MCV0			(1 << 5)
#define TSC_CR_IODEF			(1 << 4)
#define TSC_CR_SYNCPOL			(1 << 3)
#define TSC_CR_AM			(1 << 2)
#define TSC_CR_START			(1 << 1)
#define TSC_CR_TSCE			(1 << 0)

/* Pulse generator prescaler */
#define TSC_CR_PGPSC_F_HCLK		(0 << 12)
#define TSC_CR_PGPSC_F_HCLK_2		(1 << 12)
#define TSC_CR_PGPSC_F_HCLK_4		(2 << 12)
#define TSC_CR_PGPSC_F_HCLK_8		(3 << 12)
#define TSC_CR_PGPSC_F_HCLK_16		(4 << 12)
#define TSC_CR_PGPSC_F_HCLK_32		(5 << 12)
#define TSC_CR_PGPSC_F_HCLK_64		(6 << 12)
#define TSC_CR_PGPSC_F_HCLK_128		(7 << 12)

/* Max count value */
#define TSC_CR_MCV_255			(0 << 5)
#define TSC_CR_MCV_511			(1 << 5)
#define TSC_CR_MCV_1023			(2 << 5)
#define TSC_CR_MCV_2047			(3 << 5)
#define TSC_CR_MCV_4095			(4 << 5)
#define TSC_CR_MCV_8191			(5 << 5)
#define TSC_CR_MCV_16383		(6 << 5)

/* --- TSC_IER values ------------------------------------------------------ */

#define TSC_IER_MCEIE			(1 << 1)
#define TSC_IER_EOAIE			(1 << 0)

/* --- TSC_ICR values ------------------------------------------------------ */

#define TSC_ICR_MCEIC			(1 << 1)
#define TSC_ICR_EOAIC			(1 << 0)

/* --- TSC_ISR values ------------------------------------------------------ */

#define TSC_ISR_MCEF			(1 << 1)
#define TSC_ISR_EOAF			(1 << 0)

/* --- TSC_IOHCR values ---------------------------------------------------- */

#define TSC_IOHCR_G8_IO4		(1 << 31)
#define TSC_IOHCR_G8_IO3		(1 << 30)
#define TSC_IOHCR_G8_IO2		(1 << 29)
#define TSC_IOHCR_G8_IO1		(1 << 28)
#define TSC_IOHCR_G7_IO4		(1 << 27)
#define TSC_IOHCR_G7_IO3		(1 << 26)
#define TSC_IOHCR_G7_IO2		(1 << 25)
#define TSC_IOHCR_G7_IO1		(1 << 24)
#define TSC_IOHCR_G6_IO4		(1 << 23)
#define TSC_IOHCR_G6_IO3		(1 << 22)
#define TSC_IOHCR_G6_IO2		(1 << 21)
#define TSC_IOHCR_G6_IO1		(1 << 20)
#define TSC_IOHCR_G5_IO4		(1 << 19)
#define TSC_IOHCR_G5_IO3		(1 << 18)
#define TSC_IOHCR_G5_IO2		(1 << 17)
#define TSC_IOHCR_G5_IO1		(1 << 16)
#define TSC_IOHCR_G4_IO4		(1 << 15)
#define TSC_IOHCR_G4_IO3		(1 << 14)
#define TSC_IOHCR_G4_IO2		(1 << 13)
#define TSC_IOHCR_G4_IO1		(1 << 12)
#define TSC_IOHCR_G3_IO4		(1 << 11)
#define TSC_IOHCR_G3_IO3		(1 << 10)
#define TSC_IOHCR_G3_IO2		(1 << 9)
#define TSC_IOHCR_G3_IO1		(1 << 8)
#define TSC_IOHCR_G2_IO4		(1 << 7)
#define TSC_IOHCR_G2_IO3		(1 << 6)
#define TSC_IOHCR_G2_IO2		(1 << 5)
#define TSC_IOHCR_G2_IO1		(1 << 4)
#define TSC_IOHCR_G1_IO4		(1 << 3)
#define TSC_IOHCR_G1_IO3		(1 << 2)
#define TSC_IOHCR_G1_IO2		(1 << 1)
#define TSC_IOHCR_G1_IO1		(1 << 0)

/* --- TSC_IOASCR values ---------------------------------------------------- */

#define TSC_IOASCR_G8_IO4		(1 << 31)
#define TSC_IOASCR_G8_IO3		(1 << 30)
#define TSC_IOASCR_G8_IO2		(1 << 29)
#define TSC_IOASCR_G8_IO1		(1 << 28)
#define TSC_IOASCR_G7_IO4		(1 << 27)
#define TSC_IOASCR_G7_IO3		(1 << 26)
#define TSC_IOASCR_G7_IO2		(1 << 25)
#define TSC_IOASCR_G7_IO1		(1 << 24)
#define TSC_IOASCR_G6_IO4		(1 << 23)
#define TSC_IOASCR_G6_IO3		(1 << 22)
#define TSC_IOASCR_G6_IO2		(1 << 21)
#define TSC_IOASCR_G6_IO1		(1 << 20)
#define TSC_IOASCR_G5_IO4		(1 << 19)
#define TSC_IOASCR_G5_IO3		(1 << 18)
#define TSC_IOASCR_G5_IO2		(1 << 17)
#define TSC_IOASCR_G5_IO1		(1 << 16)
#define TSC_IOASCR_G4_IO4		(1 << 15)
#define TSC_IOASCR_G4_IO3		(1 << 14)
#define TSC_IOASCR_G4_IO2		(1 << 13)
#define TSC_IOASCR_G4_IO1		(1 << 12)
#define TSC_IOASCR_G3_IO4		(1 << 11)
#define TSC_IOASCR_G3_IO3		(1 << 10)
#define TSC_IOASCR_G3_IO2		(1 << 9)
#define TSC_IOASCR_G3_IO1		(1 << 8)
#define TSC_IOASCR_G2_IO4		(1 << 7)
#define TSC_IOASCR_G2_IO3		(1 << 6)
#define TSC_IOASCR_G2_IO2		(1 << 5)
#define TSC_IOASCR_G2_IO1		(1 << 4)
#define TSC_IOASCR_G1_IO4		(1 << 3)
#define TSC_IOASCR_G1_IO3		(1 << 2)
#define TSC_IOASCR_G1_IO2		(1 << 1)
#define TSC_IOASCR_G1_IO1		(1 << 0)

/* --- TSC_IOSCR values ---------------------------------------------------- */

#define TSC_IOSCR_G8_IO4		(1 << 31)
#define TSC_IOSCR_G8_IO3		(1 << 30)
#define TSC_IOSCR_G8_IO2		(1 << 29)
#define TSC_IOSCR_G8_IO1		(1 << 28)
#define TSC_IOSCR_G7_IO4		(1 << 27)
#define TSC_IOSCR_G7_IO3		(1 << 26)
#define TSC_IOSCR_G7_IO2		(1 << 25)
#define TSC_IOSCR_G7_IO1		(1 << 24)
#define TSC_IOSCR_G6_IO4		(1 << 23)
#define TSC_IOSCR_G6_IO3		(1 << 22)
#define TSC_IOSCR_G6_IO2		(1 << 21)
#define TSC_IOSCR_G6_IO1		(1 << 20)
#define TSC_IOSCR_G5_IO4		(1 << 19)
#define TSC_IOSCR_G5_IO3		(1 << 18)
#define TSC_IOSCR_G5_IO2		(1 << 17)
#define TSC_IOSCR_G5_IO1		(1 << 16)
#define TSC_IOSCR_G4_IO4		(1 << 15)
#define TSC_IOSCR_G4_IO3		(1 << 14)
#define TSC_IOSCR_G4_IO2		(1 << 13)
#define TSC_IOSCR_G4_IO1		(1 << 12)
#define TSC_IOSCR_G3_IO4		(1 << 11)
#define TSC_IOSCR_G3_IO3		(1 << 10)
#define TSC_IOSCR_G3_IO2		(1 << 9)
#define TSC_IOSCR_G3_IO1		(1 << 8)
#define TSC_IOSCR_G2_IO4		(1 << 7)
#define TSC_IOSCR_G2_IO3		(1 << 6)
#define TSC_IOSCR_G2_IO2		(1 << 5)
#define TSC_IOSCR_G2_IO1		(1 << 4)
#define TSC_IOSCR_G1_IO4		(1 << 3)
#define TSC_IOSCR_G1_IO3		(1 << 2)
#define TSC_IOSCR_G1_IO2		(1 << 1)
#define TSC_IOSCR_G1_IO1		(1 << 0)

/* --- TSC_IOCCR values ---------------------------------------------------- */

#define TSC_IOCCR_G8_IO4		(1 << 31)
#define TSC_IOCCR_G8_IO3		(1 << 30)
#define TSC_IOCCR_G8_IO2		(1 << 29)
#define TSC_IOCCR_G8_IO1		(1 << 28)
#define TSC_IOCCR_G7_IO4		(1 << 27)
#define TSC_IOCCR_G7_IO3		(1 << 26)
#define TSC_IOCCR_G7_IO2		(1 << 25)
#define TSC_IOCCR_G7_IO1		(1 << 24)
#define TSC_IOCCR_G6_IO4		(1 << 23)
#define TSC_IOCCR_G6_IO3		(1 << 22)
#define TSC_IOCCR_G6_IO2		(1 << 21)
#define TSC_IOCCR_G6_IO1		(1 << 20)
#define TSC_IOCCR_G5_IO4		(1 << 19)
#define TSC_IOCCR_G5_IO3		(1 << 18)
#define TSC_IOCCR_G5_IO2		(1 << 17)
#define TSC_IOCCR_G5_IO1		(1 << 16)
#define TSC_IOCCR_G4_IO4		(1 << 15)
#define TSC_IOCCR_G4_IO3		(1 << 14)
#define TSC_IOCCR_G4_IO2		(1 << 13)
#define TSC_IOCCR_G4_IO1		(1 << 12)
#define TSC_IOCCR_G3_IO4		(1 << 11)
#define TSC_IOCCR_G3_IO3		(1 << 10)
#define TSC_IOCCR_G3_IO2		(1 << 9)
#define TSC_IOCCR_G3_IO1		(1 << 8)
#define TSC_IOCCR_G2_IO4		(1 << 7)
#define TSC_IOCCR_G2_IO3		(1 << 6)
#define TSC_IOCCR_G2_IO2		(1 << 5)
#define TSC_IOCCR_G2_IO1		(1 << 4)
#define TSC_IOCCR_G1_IO4		(1 << 3)
#define TSC_IOCCR_G1_IO3		(1 << 2)
#define TSC_IOCCR_G1_IO2		(1 << 1)
#define TSC_IOCCR_G1_IO1		(1 << 0)

/* --- TSC_IOGCSR values --------------------------------------------------- */

#define TSC_IOGCSR_G8S			(1 << 23)
#define TSC_IOGCSR_G7S			(1 << 22)
#define TSC_IOGCSR_G6S			(1 << 21)
#define TSC_IOGCSR_G5S			(1 << 20)
#define TSC_IOGCSR_G4S			(1 << 19)
#define TSC_IOGCSR_G3S			(1 << 18)
#define TSC_IOGCSR_G2S			(1 << 17)
#define TSC_IOGCSR_G1S			(1 << 16)
#define TSC_IOGCSR_G8E			(1 << 7)
#define TSC_IOGCSR_G7E			(1 << 6)
#define TSC_IOGCSR_G6E			(1 << 5)
#define TSC_IOGCSR_G5E			(1 << 4)
#define TSC_IOGCSR_G4E			(1 << 3)
#define TSC_IOGCSR_G3E			(1 << 2)
#define TSC_IOGCSR_G2E			(1 << 1)
#define TSC_IOGCSR_G1E			(1 << 0)

/* --- TSC_IOGxCR values --------------------------------------------------- */

/* TSC_IOGxCR[13:0]: CNT[13:0]: Counter value */

/* --- Function prototypes ------------------------------------------------- */

/* Group */
enum tsc_group {
	TSC_G1,
	TSC_G2,
	TSC_G3,
	TSC_G4,
	TSC_G5,
	TSC_G6,
	TSC_G7,
	TSC_G8,
	TSC_ALL
};

/* I/O */
enum {
	TSC_IO1 = (1 << 0),
	TSC_IO2 = (1 << 1),
	TSC_IO3 = (1 << 2),
	TSC_IO4 = (1 << 3),

	/* G1 */
	TSC_PA0 = (1 << 0),
	TSC_PA1 = (1 << 1),
	TSC_PA2 = (1 << 2),
	TSC_PA3 = (1 << 3),
	/* G2 */
	TSC_PA4 = (1 << 0),
	TSC_PA5 = (1 << 1),
	TSC_PA6 = (1 << 2),
	TSC_PA7 = (1 << 3),
	/* G3 */
	TSC_PC5 = (1 << 0),
	TSC_PB0 = (1 << 1),
	TSC_PB1 = (1 << 2),
	TSC_PB2 = (1 << 3),
	/* G4 */
	TSC_PA9 = (1 << 0),
	TSC_PA10 = (1 << 1),
	TSC_PA11 = (1 << 2),
	TSC_PA12 = (1 << 3),
	/* G5 */
	TSC_PB3 = (1 << 0),
	TSC_PB4 = (1 << 1),
	TSC_PB6 = (1 << 2),
	TSC_PB7 = (1 << 3),
	/* G6 */
	TSC_PB11 = (1 << 0),
	TSC_PB12 = (1 << 1),
	TSC_PB13 = (1 << 2),
	TSC_PB14 = (1 << 3),
	/* G7 */
	TSC_PC0 = (1 << 0),
	TSC_PC1 = (1 << 1),
	TSC_PC2 = (1 << 2),
	TSC_PC3 = (1 << 3),
	/* G8 */
	TSC_PC6 = (1 << 0),
	TSC_PC7 = (1 << 1),
	TSC_PC8 = (1 << 2),
	TSC_PC9 = (1 << 3)
};

/* TSC_Gx_IOy */
enum {
	TSC_G1_IO1 = (1 << 0),
	TSC_G1_IO2 = (1 << 1),
	TSC_G1_IO3 = (1 << 2),
	TSC_G1_IO4 = (1 << 3),
	
	TSC_G2_IO1 = (1 << 4),
	TSC_G2_IO2 = (1 << 5),
	TSC_G2_IO3 = (1 << 6),
	TSC_G2_IO4 = (1 << 7),
	
	TSC_G3_IO1 = (1 << 8),
	TSC_G3_IO2 = (1 << 9),
	TSC_G3_IO3 = (1 << 10),
	TSC_G3_IO4 = (1 << 11),
	
	TSC_G4_IO1 = (1 << 12),
	TSC_G4_IO2 = (1 << 13),
	TSC_G4_IO3 = (1 << 14),
	TSC_G4_IO4 = (1 << 15),
	
	TSC_G5_IO1 = (1 << 16),
	TSC_G5_IO2 = (1 << 17),
	TSC_G5_IO3 = (1 << 18),
	TSC_G5_IO4 = (1 << 19),
	
	TSC_G6_IO1 = (1 << 20),
	TSC_G6_IO2 = (1 << 21),
	TSC_G6_IO3 = (1 << 22),
	TSC_G6_IO4 = (1 << 23),
	
	TSC_G7_IO1 = (1 << 24),
	TSC_G7_IO2 = (1 << 25),
	TSC_G7_IO3 = (1 << 26),
	TSC_G7_IO4 = (1 << 27),
	
	TSC_G8_IO1 = (1 << 28),
	TSC_G8_IO2 = (1 << 29),
	TSC_G8_IO3 = (1 << 30),
	TSC_G8_IO4 = (1 << 31)
};

/* Option */
enum {
	TSC_ENABLE = (1 << 0),
	TSC_START = (1 << 1),
	TSC_SYNC = (1 << 2),
	TSC_SYNC_RISING = (3 << 2),
	TSC_SYNC_FALLING = (1 << 2),
	TSC_IO_INPUT = (1 << 4),
	TSC_IO_OUTPUT = 0,
	TSC_SS_ENABLE = (1 << 16)
};

/* Interrupt */
enum {
	TSC_EOA = (1 << 0),	/* End of acquisition */
	TSC_MCE = (1 << 1)	/* Max count error */
};

void tsc_enable_channel(enum tsc_group group, int channel, int sampling);
void tsc_disable_channel(enum tsc_group group, int channel, int sampling);
void tsc_set_charge_transfer_pulse(int prescaler, int high, int low);
void tsc_set_max_count_value(int count);
void tsc_enable_spread_spectrum(int prescaler, int deviation);
void tsc_disable_spread_spectrum(void);
void tsc_enable_sync(bool rising);
void tsc_disable_sync(void);
void tsc_enable_io_default_input(void);
void tsc_disable_io_default_input(void);
void tsc_enable(void);
void tsc_disable(void);
void tsc_start(void);
void tsc_init(int prescaler, int high, int low, int max_count,
	      int sspsc, int ssd, int option);
void tsc_enable_interrupt(int interrupt);
void tsc_disable_interrupt(int interrupt);
int tsc_get_interrupt_mask(int interrupt);
int tsc_get_interrupt_status(int interrupt);
void tsc_clear_interrupt(int interrupt);
int tsc_get_status(enum tsc_group group);
int tsc_get_counter(enum tsc_group group);
void tsc_enable_hysteresis(int g_io);
void tsc_disable_hysteresis(int g_io);
void tsc_close_analog_switch(int g_io);
void tsc_open_analog_switch(int g_io);
