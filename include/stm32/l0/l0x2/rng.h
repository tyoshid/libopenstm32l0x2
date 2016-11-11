/*
 * RNG
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
 * 19  Random number generator (RNG)
 */

/* --- RNG registers ------------------------------------------------------- */
/*
 * Offset	Register
 * 0x00		RNG_CR		RNG control register
 * 0x04		RNG_SR		RNG status register
 * 0x08		RNG_DR		RNG data register
 */

#define RNG_CR				(RNG_BASE + 0x00)
#define RNG_SR				(RNG_BASE + 0x04)
#define RNG_DR				(RNG_BASE + 0x08)

/* --- RNG_CR values ------------------------------------------------------- */

#define RNG_CR_IE			(1 << 3)
#define RNG_CR_RNGEN			(1 << 2)

/* --- RNG_SR values ------------------------------------------------------- */

#define RNG_SR_SEIS			(1 << 6)
#define RNG_SR_CEIS			(1 << 5)
#define RNG_SR_SECS			(1 << 2)
#define RNG_SR_CECS			(1 << 1)
#define RNG_SR_DRDY			(1 << 0)

/* --- RNG_DR values ------------------------------------------------------- */

/* RNG_DR[31:0]: RNDATA[31:0]: Random data */

/* --- Function prototypes ------------------------------------------------- */

/* Interrupt and status */
enum {
	RNG_DRDY = (1 << 0),
	RNG_CECS = (1 << 1),
	RNG_SECS = (1 << 2),

	RNG_CEIS = (1 << 5),
	RNG_SEIS = (1 << 6)
};

void rng_enable(void);
void rng_disable(void);
int rng_get_data(void);
void rng_enable_interrupt(int interrupt);
void rng_disable_interrupt(int interrupt);
int rng_get_interrupt_mask(int interrupt);
int rng_get_interrupt_status(int interrupt);
void rng_clear_interrupt(int interrupt);
