/*
 * Miscellaneous registers (factory option bytes)
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
 * 33  Device electronic signature
 *
 *
 * DS10182: STM32L052x6 STM32L052x8
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 64 KB Flash memory,
 * 8 KB SRAM, 2 KB EEPROM, USB, ADC, DAC
 * (11-Sep-2017 Rev 8)
 *
 * Table 7. Temperature sensor calibration values
 * Table 8. Internal voltage reference measured values
 *
 *
 * DS10183: STM32L062K8 STM32L062T8
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, 64 KB Flash,
 * 8 KB SRAM, 2 KB EEPROM, USB, ADC, DAC, AES
 * (11-Sep-2017 Rev 9)
 *
 * Table 6. Temperature sensor calibration values
 * Table 7. Internal voltage reference measured values
 *
 *
 * DS10689: STM32L072x8 STM32L072xB STM32L072xZ
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 192KB Flash,
 * 20KB SRAM, 6KB EEPROM, USB, ADC, DACs
 * (13-Sep-2017 Rev 4)
 *
 * Table 7. Temperature sensor calibration values
 * Table 8. Internal voltage reference measured values
 *
 *
 * DS10688: STM32L082KB STM32L082KZ STM32L082CZ
 * Ultra-low-power 32-bit MCU ARM-based Cortex-M0+, up to 192KB Flash,
 * 20KB SRAM, 6KB EEPROM, USB, ADC, DACs, AES
 * (14-Sep-2017 Rev 5)
 *
 * Table 6. Temperature sensor calibration values
 * Table 7. Internal voltage reference measured values
 */

/* --- miscellaneous registers --------------------------------------------- */
/*
 * Offset	Register
 * 0x50		MISC_U_ID0		31:0 unique ID bits
 * 0x54		MISC_U_ID1		63:32 unique ID bits
 * 0x64		MISC_U_ID2		95:64 unique ID bits
 * 0x78		MISC_VREFINT_CAL	VREFINT measured value
 * 0x7a		MISC_TS_CAL1		TS calibration value (30 degrees)
 * 0x7c		MISC_F_SIZE		Flash size register
 * 0x7e		MISC_TS_CAL2		TS calibration value (130 degrees)
 */

#define MISC_U_ID0			(OPTION_BYTE_BASE + 0x50)
#define MISC_U_ID1			(OPTION_BYTE_BASE + 0x54)
#define MISC_U_ID2			(OPTION_BYTE_BASE + 0x64)
#define MISC_VREFINT_CAL		(OPTION_BYTE_BASE + 0x78)
#define MISC_TS_CAL1			(OPTION_BYTE_BASE + 0x7a)
#define MISC_F_SIZE			(OPTION_BYTE_BASE + 0x7c)
#define MISC_TS_CAL2			(OPTION_BYTE_BASE + 0x7e)

/* --- Function prototypes ------------------------------------------------- */

int misc_get_flash_size(void);
void misc_get_unique_id(int *u_id);
int misc_get_vrefint_cal(void);
int misc_get_ts_cal1(void);
int misc_get_ts_cal2(void);
