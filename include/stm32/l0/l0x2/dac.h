/*
 * DAC
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
 * 15  Digital-to-analog converter (DAC)
 *
 *
 * DS10182: STM32L052x6 STM32L052x8
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 64 KB Flash memory,
 * 8 KB SRAM, 2 KB EEPROM, USB, ADC, DAC
 * (22-Mar-2016 Rev 6)
 *
 * Table 66. DAC characteristics
 *
 *
 * DS10183: STM32L062K8 STM32L062T8
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, 64 KB Flash,
 * 8 KB SRAM, 2 KB EEPROM, USB, ADC, DAC, AES
 * (09-Jub-2016 Rev 7)
 *
 * Table 58. DAC characteristics
 *
 *
 * DS10689: STM32L072x8 STM32L072xB STM32L072xZ
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 192KB Flash,
 * 20KB SRAM, 6KB EEPROM, USB, ADC, DACs
 * (22-Mar-2016 Rev 3)
 *
 * Table 67. DAC characteristics
 *
 *
 * DS10688: STM32L082KB STM32L082KZ STM32L082CZ
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 192KB Flash,
 * 20KB SRAM, 6KB EEPROM, USB, ADC, DACs, AES
 * (03-May-2016 Rev 4)
 *
 * Table 64. DAC characteristics
 */

/* --- DAC registers ------------------------------------------------------- */
/*
 * Offset	Register
 * 0x00		DAC_CR		DAC control register
 * 0x04		DAC_SWTRIGR	DAC software trigger register
 * 0x08		DAC_DHR12R1	DAC channel1 12-bit right-aligned
 *				data holding register
 * 0x0c		DAC_DHR12L1	DAC channel1 12-bit left-aligned
 *				data holding register
 * 0x10		DAC_DHR8R1	DAC channel1 8-bit right-aligned
 *				data holding register
 * 0x14		DAC_DHR12R2	DAC channel2 12-bit right-aligned
 *				data holding register
 * 0x18		DAC_DHR12L2	DAC channel2 12-bit left-aligned
 *				data holding register
 * 0x1c		DAC_DHR8R2	DAC channel2 8-bit right-aligned
 *				data holding register
 * 0x20		DAC_DHR12RD	Dual DAC 12-bit right-aligned
 *				data holding register
 * 0x24		DAC_DHR12LD	Dual DAC 12-bit left-aligned
 *				data holding register
 * 0x28		DAC_DHR8RD	Dual DAC 8-bit right-aligned
 *				data holding register
 * 0x2c		DAC_DOR1	DAC channel1 data output register
 * 0x30		DAC_DOR2	DAC channel2 data output register
 * 0x34		DAC_SR		DAC status register
 */

#define DAC_CR				(DAC_BASE + 0x00)
#define DAC_SWTRIGR			(DAC_BASE + 0x04)
#define DAC_DHR12R1			(DAC_BASE + 0x08)
#define DAC_DHR12L1			(DAC_BASE + 0x0c)
#define DAC_DHR8R1			(DAC_BASE + 0x10)
#define DAC_DHR12R2			(DAC_BASE + 0x14)
#define DAC_DHR12L2			(DAC_BASE + 0x18)
#define DAC_DHR8R2			(DAC_BASE + 0x1c)
#define DAC_DHR12RD			(DAC_BASE + 0x20)
#define DAC_DHR12LD			(DAC_BASE + 0x24)
#define DAC_DHR8RD			(DAC_BASE + 0x28)
#define DAC_DOR1			(DAC_BASE + 0x2c)
#define DAC_DOR2			(DAC_BASE + 0x30)
#define DAC_SR				(DAC_BASE + 0x34)

/* --- DAC_CR values ------------------------------------------------------- */

#define DAC_CR_DMAUDRIE2		(1 << 29)
#define DAC_CR_DMAEN2			(1 << 28)
#define DAC_CR_MAMP23			(1 << 27)
#define DAC_CR_MAMP22			(1 << 26)
#define DAC_CR_MAMP21			(1 << 25)
#define DAC_CR_MAMP20			(1 << 24)
#define DAC_CR_WAVE21			(1 << 23)
#define DAC_CR_WAVE20			(1 << 22)
#define DAC_CR_TSEL22			(1 << 21)
#define DAC_CR_TSEL21			(1 << 20)
#define DAC_CR_TSEL20			(1 << 19)
#define DAC_CR_TEN2			(1 << 18)
#define DAC_CR_BOFF2			(1 << 17)
#define DAC_CR_EN2			(1 << 16)
#define DAC_CR_DMAUDRIE1		(1 << 13)
#define DAC_CR_DMAEN1			(1 << 12)
#define DAC_CR_MAMP13			(1 << 11)
#define DAC_CR_MAMP12			(1 << 10)
#define DAC_CR_MAMP11			(1 << 9)
#define DAC_CR_MAMP10			(1 << 8)
#define DAC_CR_WAVE11			(1 << 7)
#define DAC_CR_WAVE10			(1 << 6)
#define DAC_CR_TSEL12			(1 << 5)
#define DAC_CR_TSEL11			(1 << 4)
#define DAC_CR_TSEL10			(1 << 3)
#define DAC_CR_TEN1			(1 << 2)
#define DAC_CR_BOFF1			(1 << 1)
#define DAC_CR_EN1			(1 << 0)

/* DAC channel2 mask/amplitude selector */
#define DAC_CR_MAMP2_UNMASK_0		(0 << 24)
#define DAC_CR_MAMP2_UNMASK_1_0		(1 << 24)
#define DAC_CR_MAMP2_UNMASK_2_0		(2 << 24)
#define DAC_CR_MAMP2_UNMASK_3_0		(3 << 24)
#define DAC_CR_MAMP2_UNMASK_4_0		(4 << 24)
#define DAC_CR_MAMP2_UNMASK_5_0		(5 << 24)
#define DAC_CR_MAMP2_UNMASK_6_0		(6 << 24)
#define DAC_CR_MAMP2_UNMASK_7_0		(7 << 24)
#define DAC_CR_MAMP2_UNMASK_8_0		(8 << 24)
#define DAC_CR_MAMP2_UNMASK_9_0		(9 << 24)
#define DAC_CR_MAMP2_UNMASK_10_0	(10 << 24)
#define DAC_CR_MAMP2_UNMASK_11_0	(11 << 24)

#define DAC_CR_MAMP2_AMP_1		(0 << 24)
#define DAC_CR_MAMP2_AMP_3		(1 << 24)
#define DAC_CR_MAMP2_AMP_7		(2 << 24)
#define DAC_CR_MAMP2_AMP_15		(3 << 24)
#define DAC_CR_MAMP2_AMP_31		(4 << 24)
#define DAC_CR_MAMP2_AMP_63		(5 << 24)
#define DAC_CR_MAMP2_AMP_127		(6 << 24)
#define DAC_CR_MAMP2_AMP_255		(7 << 24)
#define DAC_CR_MAMP2_AMP_511		(8 << 24)
#define DAC_CR_MAMP2_AMP_1023		(9 << 24)
#define DAC_CR_MAMP2_AMP_2047		(10 << 24)
#define DAC_CR_MAMP2_AMP_4095		(11 << 24)

/* DAC channel2 noise/triangle wave generation enable */
#define DAC_CR_WAVE2_DISABLE		(0 << 22)
#define DAC_CR_WAVE2_NOISE		(1 << 22)
#define DAC_CR_WAVE2_TRIANGLE		(2 << 22)

/* DAC channel2 trigger selection */
#define DAC_CR_TSEL2_TIM6_TRGO		(0 << 19)
#define DAC_CR_TSEL2_TIM3_TRGO		(1 << 19)
#define DAC_CR_TSEL2_TIM3_CH3		(2 << 19)
#define DAC_CR_TSEL2_TIM21_TRGO		(3 << 19)
#define DAC_CR_TSEL2_TIM2_TRGO		(4 << 19)
#define DAC_CR_TSEL2_TIM7_TRGO		(5 << 19)
#define DAC_CR_TSEL2_EXTI9		(6 << 19)
#define DAC_CR_TSEL2_SOFTWARE		(7 << 19)

/* DAC channel1 mask/amplitude selector */
#define DAC_CR_MAMP1_UNMASK_0		(0 << 8)
#define DAC_CR_MAMP1_UNMASK_1_0		(1 << 8)
#define DAC_CR_MAMP1_UNMASK_2_0		(2 << 8)
#define DAC_CR_MAMP1_UNMASK_3_0		(3 << 8)
#define DAC_CR_MAMP1_UNMASK_4_0		(4 << 8)
#define DAC_CR_MAMP1_UNMASK_5_0		(5 << 8)
#define DAC_CR_MAMP1_UNMASK_6_0		(6 << 8)
#define DAC_CR_MAMP1_UNMASK_7_0		(7 << 8)
#define DAC_CR_MAMP1_UNMASK_8_0		(8 << 8)
#define DAC_CR_MAMP1_UNMASK_9_0		(9 << 8)
#define DAC_CR_MAMP1_UNMASK_10_0	(10 << 8)
#define DAC_CR_MAMP1_UNMASK_11_0	(11 << 8)

#define DAC_CR_MAMP1_AMP_1		(0 << 8)
#define DAC_CR_MAMP1_AMP_3		(1 << 8)
#define DAC_CR_MAMP1_AMP_7		(2 << 8)
#define DAC_CR_MAMP1_AMP_15		(3 << 8)
#define DAC_CR_MAMP1_AMP_31		(4 << 8)
#define DAC_CR_MAMP1_AMP_63		(5 << 8)
#define DAC_CR_MAMP1_AMP_127		(6 << 8)
#define DAC_CR_MAMP1_AMP_255		(7 << 8)
#define DAC_CR_MAMP1_AMP_511		(8 << 8)
#define DAC_CR_MAMP1_AMP_1023		(9 << 8)
#define DAC_CR_MAMP1_AMP_2047		(10 << 8)
#define DAC_CR_MAMP1_AMP_4095		(11 << 8)

/* DAC channel1 noise/triangle wave generation enable */
#define DAC_CR_WAVE1_DISABLE		(0 << 6)
#define DAC_CR_WAVE1_NOISE		(1 << 6)
#define DAC_CR_WAVE1_TRIANGLE		(2 << 6)

/* DAC channel1 trigger selection */
#define DAC_CR_TSEL1_TIM6_TRGO		(0 << 3)
#define DAC_CR_TSEL1_TIM3_TRGO		(1 << 3)
#define DAC_CR_TSEL1_TIM3_CH3		(2 << 3)
#define DAC_CR_TSEL1_TIM21_TRGO		(3 << 3)
#define DAC_CR_TSEL1_TIM2_TRGO		(4 << 3)
#define DAC_CR_TSEL1_TIM7_TRGO		(5 << 3)
#define DAC_CR_TSEL1_EXTI9		(6 << 3)
#define DAC_CR_TSEL1_SOFTWARE		(7 << 3)

/* --- DAC_SWTRIGR values -------------------------------------------------- */

#define DAC_SWTRIGR_SWTRIG2		(1 << 1)
#define DAC_SWTRIGR_SWTRIG1		(1 << 0)

/* --- DAC_DHR12R1 values -------------------------------------------------- */

/* DAC_DHR12R1[11:0]: DACC1DHR[11:0]: DAC channel1 12-bit right-aligned data */

/* --- DAC_DHR12L1 values -------------------------------------------------- */

/* DAC_DHR12L1[15:4]: DACC1DHR[11:0]: DAC channel1 12-bit left-aligned data */

/* --- DAC_DHR8R1 values --------------------------------------------------- */

/* DAC_DHR8R1[7:0]: DACC1DHR[7:0]: DAC channel1 8-bit right-aligned data */

/* --- DAC_DHR12R2 values -------------------------------------------------- */

/* DAC_DHR12R2[11:0]: DACC2DHR[11:0]: DAC channel2 12-bit right-aligned data */

/* --- DAC_DHR12L2 values -------------------------------------------------- */

/* DAC_DHR12L2[15:4]: DACC2DHR[11:0]: DAC channel2 12-bit left-aligned data */

/* --- DAC_DHR8R2 values --------------------------------------------------- */

/* DAC_DHR8R2[7:0]: DACC2DHR[7:0]: DAC channel2 8-bit right-aligned data */

/* --- DAC_DHR12RD values -------------------------------------------------- */

/* DAC_DHR12RD[27:16]: DACC2DHR[11:0]: DAC channel2 12-bit right-aligned data */
/* DAC_DHR12RD[11:0]: DACC1DHR[11:0]: DAC channel1 12-bit right-aligned data */

/* --- DAC_DHR12LD values -------------------------------------------------- */

/* DAC_DHR12LD[31:20]: DACC2DHR[11:0]: DAC channel1 12-bit left-aligned data */
/* DAC_DHR12LD[15:4]: DACC1DHR[11:0]: DAC channel1 12-bit left-aligned data */

/* --- DAC_DHR8RD values --------------------------------------------------- */

/* DAC_DHR8RD[15:8]: DACC2DHR[7:0]: DAC channel2 8-bit right-aligned data */
/* DAC_DHR8RD[7:0]: DACC1DHR[7:0]: DAC channel1 8-bit right-aligned data */

/* --- DAC_DOR1 values ----------------------------------------------------- */

/* DAC_DOR1[11:0]: DACC1DOR[11:0]: DAC channel1 data output */

/* --- DAC_DOR2 values ----------------------------------------------------- */

/* DAC_DOR2[11:0]: DACC2DOR[11:0]: DAC channel2 data output */

/* --- DAC_SR values ------------------------------------------------------- */

#define DAC_SR_DMAUDR2			(1 << 29)
#define DAC_SR_DMAUDR1			(1 << 13)

/* --- Wakeup time --------------------------------------------------------- */

#define DAC_T_WAKEUP			15	/* 15 us */

/* --- Function prototypes ------------------------------------------------- */

/* Noise generation */
/* Unmask bits[b:0] of LFSR */
#define DAC_CH1_NOISE(b)		((b) << 8 | 1 << 6)
#define DAC_CH2_NOISE(b)		((b) << 24 | 1 << 22)

/* Triangle wave generation */
/* Triangle amplitude equal to (2**(b + 1) - 1) */
#define DAC_CH1_TRIANGLE(b)		((b) << 8 | 2 << 6)
#define DAC_CH2_TRIANGLE(b)		((b) << 24 | 2 << 22)

/* Mode */
enum {
	/* Trigger */
	DAC_CH1_TRIGGER_NONE = (0 << 2),
	DAC_CH1_TIM6_TRGO = (1 << 2),
	DAC_CH1_TIM3_TRGO = (3 << 2),
	DAC_CH1_TIM3_CH3 = (5 << 2),
	DAC_CH1_TIM21_TRGO = (7 << 2),
	DAC_CH1_TIM2_TRGO = (9 << 2),
	DAC_CH1_TIM7_TRGO = (11 << 2),
	DAC_CH1_EXTI9 = (13 << 2),
	DAC_CH1_SOFTWARE = (15 << 2),

	DAC_CH2_TRIGGER_NONE = (0 << 18),
	DAC_CH2_TIM6_TRGO = (1 << 18),
	DAC_CH2_TIM3_TRGO = (3 << 18),
	DAC_CH2_TIM3_CH3 = (5 << 18),
	DAC_CH2_TIM21_TRGO = (7 << 18),
	DAC_CH2_TIM2_TRGO = (9 << 18),
	DAC_CH2_TIM7_TRGO = (11 << 18),
	DAC_CH2_EXTI9 = (13 << 18),
	DAC_CH2_SOFTWARE = (15 << 18),

	/* Noise generation */
	/* Unmask bits[x:0] of LFSR */
	DAC_CH1_NOISE_0 = (0 << 8 | 1 << 6),
	DAC_CH1_NOISE_1 = (1 << 8 | 1 << 6),
	DAC_CH1_NOISE_2 = (2 << 8 | 1 << 6),
	DAC_CH1_NOISE_3 = (3 << 8 | 1 << 6),
	DAC_CH1_NOISE_4 = (4 << 8 | 1 << 6),
	DAC_CH1_NOISE_5 = (5 << 8 | 1 << 6),
	DAC_CH1_NOISE_6 = (6 << 8 | 1 << 6),
	DAC_CH1_NOISE_7 = (7 << 8 | 1 << 6),
	DAC_CH1_NOISE_8 = (8 << 8 | 1 << 6),
	DAC_CH1_NOISE_9 = (9 << 8 | 1 << 6),
	DAC_CH1_NOISE_10 = (10 << 8 | 1 << 6),
	DAC_CH1_NOISE_11 = (11 << 8 | 1 << 6),

	DAC_CH2_NOISE_0 = (0 << 24 | 1 << 22),
	DAC_CH2_NOISE_1 = (1 << 24 | 1 << 22),
	DAC_CH2_NOISE_2 = (2 << 24 | 1 << 22),
	DAC_CH2_NOISE_3 = (3 << 24 | 1 << 22),
	DAC_CH2_NOISE_4 = (4 << 24 | 1 << 22),
	DAC_CH2_NOISE_5 = (5 << 24 | 1 << 22),
	DAC_CH2_NOISE_6 = (6 << 24 | 1 << 22),
	DAC_CH2_NOISE_7 = (7 << 24 | 1 << 22),
	DAC_CH2_NOISE_8 = (8 << 24 | 1 << 22),
	DAC_CH2_NOISE_9 = (9 << 24 | 1 << 22),
	DAC_CH2_NOISE_10 = (10 << 24 | 1 << 22),
	DAC_CH2_NOISE_11 = (11 << 24 | 1 << 22),

	/* Triangle wave generation */
	/* Triangle amplitude equal to x */
	DAC_CH1_TRIANGLE_1 = (0 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_3 = (1 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_7 = (2 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_15 = (3 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_31 = (4 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_63 = (5 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_127 = (6 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_255 = (7 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_511 = (8 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_1023 = (9 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_2047 = (10 << 8 | 2 << 6),
	DAC_CH1_TRIANGLE_4095 = (11 << 8 | 2 << 6),

	DAC_CH2_TRIANGLE_1 = (0 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_3 = (1 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_7 = (2 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_15 = (3 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_31 = (4 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_63 = (5 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_127 = (6 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_255 = (7 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_511 = (8 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_1023 = (9 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_2047 = (10 << 24 | 2 << 22),
	DAC_CH2_TRIANGLE_4095 = (11 << 24 | 2 << 22),

	/* Others */
	DAC_CH1_ENABLE = (1 << 0),
	DAC_CH2_ENABLE = (1 << 16),
	DAC_CH1_BUFFER_OFF = (1 << 1),
	DAC_CH2_BUFFER_OFF = (1 << 17),
	DAC_CH1_DMA = (1 << 12),
	DAC_CH2_DMA = (1 << 28)
};

/* Channel */
enum dac_channel {
	DAC_CH1,
	DAC_CH2,
	DAC_DUAL
};

/* Data format */
enum dac_format {
	DAC_8R,
	DAC_12L,
	DAC_12R
};

/* Interrupt */
enum {
	DAC_CH1_DMA_UNDERRUN = (1 << 13),
	DAC_CH2_DMA_UNDERRUN = (1 << 29)
};

void dac_set_mode(int mode);
void dac_enable(enum dac_channel channel);
void dac_disable(enum dac_channel channel);
void dac_enable_dma(enum dac_channel channel);
void dac_disable_dma(enum dac_channel channel);
void dac_enable_noise_wave_generation(enum dac_channel channel);
void dac_enable_triangle_wave_generation(enum dac_channel channel);
void dac_disable_wave_generation(enum dac_channel channel);
void dac_enable_trigger(enum dac_channel channel);
void dac_disable_trigger(enum dac_channel channel);
void dac_set_data(enum dac_format format, enum dac_channel channel, int data);
void dac_software_trigger(enum dac_channel channel);
void dac_enable_interrupt(int interrupt);
void dac_disable_interrupt(int interrupt);
int dac_get_interrupt_mask(int interrupt);
int dac_get_interrupt_status(int interrupt);
void dac_clear_interrupt(int interrupt);
