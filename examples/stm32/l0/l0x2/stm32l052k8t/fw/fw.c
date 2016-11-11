/*
 * fw - Test firewall.
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

#include <stdbool.h>
#include <stdint.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <tim.h>
#include <syscfg.h>
#include <fw.h>
#include <scb.h>
#include <memorymap.h>
#include <ppb.h>
#include <mmio.h>

#define TIMX_CLK_APB1		32000000 /* TIM6 clock frequency */

/* Unprotected code segment */
int main(void)			__attribute__ ((section (".unprotect_text")));
/* Code segment */
void protect_main(void)		__attribute__ ((section (".protect_start")));
/* Non-volatile data segment */
volatile int * const rom_protect = (volatile int *)EEPROM_BASE;
/* Volatile data segment */
volatile int ram_protect	__attribute__ ((section (".protect_data")));

extern char end;		/* The end of BSS segment */
volatile char * const testnum = &end;	/* Test number */
volatile char * const flag = &end + 1;

/*
 * Clock
 * PLL: 16 MHz * 4 / 2 = 32 MHz
 * SYSCLK	32 MHz
 * HCLK		32 MHz (AHB)
 * PCLK1	32 MHz (APB1)
 * PCLK2	32 MHz (APB2)
 */
static void clock_setup(void)
{
	rcc_enable_clock(RCC_PWR);
	pwr_set_vos(PWR_1_8V);

	rcc_enable_osc(RCC_HSI16);
	rcc_setup_pll(RCC_HSI16, 4, 2);
	rcc_enable_osc(RCC_PLL);
	flash_config_access(1, FLASH_PREFETCH | FLASH_PREREAD);
	rcc_set_sysclk_source(RCC_PLL);
	rcc_disable_osc(RCC_MSI);
}

/*
 * GPIO
 * PA0: output	LED1
 * PA1: output	LED2
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PA(0, 1));
	gpio_set(GPIO_PA(0, 1));
}

/* TIM6: CK_CNT = 2 kHz */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM6);
	tim_enable_one_pulse_mode(TIM6);
	tim_disable_update_interrupt_on_any(TIM6);
	tim_load_prescaler(TIM6, TIMX_CLK_APB1 / 2000);
}

static void fw_setup(void)
{
	extern char _cssa[];
	extern char _text_end[];
	extern char _vdssa[];
	extern char _vdssa_end[];
	
	rcc_enable_clock(RCC_SYSCFG);
	rcc_enable_clock(RCC_FW);
	fw_init((int)_cssa, (int)_text_end - (int)_cssa + 255,
		EEPROM_BASE, 256,
		(int)_vdssa, (int)_vdssa_end - (int)_vdssa + 63, 0);
}

/* Delay: 1 - 32768 ms */
static void delay_ms(int ms)
{
	tim_set_autoreload(TIM6, ms << 1);
	tim_enable_counter(TIM6);
	while (!tim_get_interrupt_status(TIM6, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM6, TIM_UPDATE);
}

static void blink_led(int n)
{
	int i;

	delay_ms(250);
	for (i = 0; i < n; i++) {
		gpio_clear(GPIO_PA0);
		delay_ms(250);
		gpio_set(GPIO_PA0);
		delay_ms(250);
	}
	delay_ms(250);
}

int main(void)
{
	extern char callgate[];
	int s;
	void (*f)(void);
	
	clock_setup();
	gpio_setup();
	tim_setup();
	fw_setup();

	s = rcc_get_reset_flag(RCC_POR_PDR | RCC_FIREWALL);
	rcc_clear_reset_flag();
	if (s & RCC_POR_PDR) {
		*testnum = 1;
	} else if (s & RCC_FIREWALL) {
		if ((*testnum == 3 || *testnum == 7 || *testnum == 9)
		    && *flag == 0) {
			delay_ms(1000);
			(*testnum)++;
		} else if (*testnum == 5 && *flag == 1) {
			(*testnum)++;
		} else {
			/* unexpected firewall reset */
			while (1) {
				gpio_toggle(GPIO_PA1);
				delay_ms(*flag ? 1000 : 500);
			}
		}
	} else {
		/* Software reset */
		if (*flag == 0) {
			while (1) {
				gpio_toggle(GPIO_PA1);
				delay_ms(2000);
			}
		}
		(*testnum)++;
	}
	if (*testnum > 10)
		*testnum = 1;
	*flag = 0;
	switch (*testnum) {
	case 1:			/* FW: Disable, call gate: No */
		blink_led(1);
		protect_main();
		break;
	case 2:			/* FW: Disable, call gate: Yes */
		blink_led(2);
		f = (void (*)(void))((int)callgate | 1);
		(*f)();
		break;
	case 3:			/* FW: Enable, call gate: No */
		blink_led(3);
		syscfg_enable_firewall();
		protect_main();	/* Firewall reset ! */
		break;
	case 4:			/* FW: Enable, call gate: Yes */
		blink_led(4);
		f = (void (*)(void))((int)callgate | 1);
		syscfg_enable_firewall();
		(*f)();
		break;
	case 5:			/* FPA: 0 */
		blink_led(5);
		f = (void (*)(void))((int)callgate | 1);
		syscfg_enable_firewall();
		(*f)();
		/* Firewall reset ! */
		break;
	case 6:			/* FPA: 1 */
		blink_led(6);
		fw_enable_fpa();
		f = (void (*)(void))((int)callgate | 1);
		syscfg_enable_firewall();
		(*f)();
		MMIO32(SCB_AIRCR) = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;
		break;
	case 7:		       /* Non-volatile data segment: Closed */
		blink_led(7);
		syscfg_enable_firewall();
		*rom_protect;	/* Firewall reset ! */
		break;
	case 8:		       /* Non-volatile data segment: Opened */
		blink_led(8);
		f = (void (*)(void))((int)callgate | 1);
		syscfg_enable_firewall();
		(*f)();
		break;
	case 9:			/* Volatile data segment: Closed */
		blink_led(9);
		syscfg_enable_firewall();
		ram_protect;	/* Firewall reset ! */
		break;
	case 10:		/* Volatile data segment: Opened */
		blink_led(10);
		f = (void (*)(void))((int)callgate | 1);
		syscfg_enable_firewall();
		(*f)();
		break;
	default:
		break;
	}

	gpio_clear(GPIO_PA(0, 1));
	while (1)
		;

	return 0;
}

void protect_main(void)
{
	delay_ms(1000);
	*flag = 1;
	if (*testnum == 8)
		*rom_protect;
	if (*testnum == 10)
		ram_protect;
	if (*testnum != 5 && *testnum != 6)
		scb_reset_system();
	return;
}
