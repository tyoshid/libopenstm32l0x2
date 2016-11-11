/*
 * rtc_lcd - An alarm clock.
 *
 * rtc_lcd.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of rtc_lcd.
 *
 * rtc_lcd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rtc_lcd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rtc_lcd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <unistd.h>
#include <stdbool.h>

#include <rcc.h>
#include <pwr.h>
#include <flash.h>
#include <gpio.h>
#include <nvic.h>
#include <vector.h>
#include <usart.h>
#include <exti.h>
#include <spi.h>
#include <tim.h>
#include <rtc.h>
#include <memorymap.h>

#include <syscall.h>

#include "rtc_lcd.h"

#define FCK		32000000 /* USART clock frequency */
#define TIMX_CLK_APB2	32000000 /* TIM21 and TIM22 clock frequency */
#define TIMX_CLK_APB1	32000000 /* TIM2 clock frequency */
#define TXBUFSIZE	32	 /* Tx buffer size (power of 2) */
#define RXBUFSIZE	32	 /* Rx buffer size (power of 2) */

/* EEPROM address */
int * const rom_contrast = (int *)(EEPROM_BASE + 0);
int * const rom_bright = (int *)(EEPROM_BASE + 4);
int * const rom_calib = (int *)(EEPROM_BASE + 8);
/* Tx buffer */
static volatile int tx_buf[TXBUFSIZE];
static volatile int tx_len;
/* Rx buffer */
static volatile int rx_buf[RXBUFSIZE];
static volatile int rx_len;
/* Alarm */
volatile bool alarm_a_flag;
volatile bool alarm_b_flag;

/*
 * Clock
 * PLL: 16 MHz * 4 / 2 = 32 MHz
 * SYSCLK	32 MHz
 * HCLK		32 MHz (AHB)
 * PCLK1	32 MHz (APB1)
 * PCLK2	32 MHz (APB2)
 * LSE -> RTC
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

	pwr_disable_backup_write_protection();
	rcc_enable_osc(RCC_LSE);
}	

/*
 * GPIO
 * PA0: output		LCD E
 * PA1: output		LCD R/W
 * PA2: output		LCD RS
 * PA3: output		LCD DB4
 * PA4: output		LCD DB5
 * PA5: output		LCD DB6
 * PA6: output		LCD DB7
 * PA9: USART1_TX
 * PA10: USART1_RX
 * PA15: TIM2_OC1	LCD Backlight
 * PB0: output		LED1
 * PB1: output		LED2
 * PB3: SPI1_SCK	MCP4131 SCK
 * PB4: SPI1_MISO	MCP4131 SDO
 * PB5: SPI1_MOSI	MCP4131 SDI
 * PB6: output		MCP4131 /CS
 */
static void gpio_setup(void)
{
	rcc_enable_clock(RCC_GPIOA);
	gpio_write_port(GPIOA, 0x7e);
	gpio_config(GPIO_OUTPUT, GPIO_35MHZ, GPIO_PA(0, 1, 2, 3, 4, 5, 6));
	gpio_config(GPIO_AF4, 0, GPIO_PA(9, 10));
	gpio_config(GPIO_AF5, 0, GPIO_PA15);

	rcc_enable_clock(RCC_GPIOB);
	gpio_set(GPIO_PB(0, 1));
	gpio_config(GPIO_OUTPUT, 0, GPIO_PB(0, 1));
	gpio_config(GPIO_AF0, 0, GPIO_PB(3, 4, 5));
	gpio_set(GPIO_PB6);
	gpio_config(GPIO_OUTPUT, 0, GPIO_PB6);
}

/* USART1: 115200 8N1, interrupt */
static void usart_setup(void)
{
	rcc_enable_clock(RCC_USART1);
	nvic_enable_irq(NVIC_USART1);
	usart_enable_interrupt(USART1, USART_RXNE);
	usart_init(USART1, FCK, 115200, 8, USART_STOP_1, USART_PARITY_NONE,
		   USART_FLOW_NONE, USART_TX_RX);
}

/* EXTI20: RTC wakeup timer event, EXTI17: RTC Alarm event */
static void exti_setup(void)
{
	nvic_enable_irq(NVIC_RTC);
	exti_set_trigger(EXTI_RTC_WAKEUP_TIMER, EXTI_RISING);
	exti_set_trigger(EXTI_RTC_ALARM, EXTI_RISING);
	exti_enable_interrupt(EXTI_RTC_WAKEUP_TIMER | EXTI_RTC_ALARM);
}

/* SPI1: SCK: 32 MHz / 128 = 250 kHz (MCP4131 read max.) */
static void spi_setup(void)
{
	rcc_enable_clock(RCC_SPI1);
	spi_init_master(SPI1, 128, 8, SPI_NSS_OUTPUT | SPI_ENABLE);
}

/*
 * TIM
 * TIM21: CK_CNT = 2 MHz, one-pulse mode
 * TIM22: CK_CNT = 2 kHz, one-pulse mode
 * TIM2: CK_CNT = 12.8 kHz, count = 256
 *  OC1: PWM mode 1 (TIMx_CNT < TIMx_CCR1: active), (compare = 0)
 */
static void tim_setup(void)
{
	rcc_enable_clock(RCC_TIM21);
	tim_enable_one_pulse_mode(TIM21);
	tim_disable_update_interrupt_on_any(TIM21);
	tim_load_prescaler(TIM21, TIMX_CLK_APB2 / 2000000);

	rcc_enable_clock(RCC_TIM22);
	tim_enable_one_pulse_mode(TIM22);
	tim_disable_update_interrupt_on_any(TIM22);
	tim_load_prescaler(TIM22, TIMX_CLK_APB2 / 2000);

	rcc_enable_clock(RCC_TIM2);
	tim_setup_counter(TIM2, TIMX_CLK_APB1 / 12800, 256);
	tim_set_capture_compare_mode(TIM2_CC1, TIM_OC_PRELOAD | TIM_OC_PWM1 |
				     TIM_CC_ENABLE);
	tim_enable_counter(TIM2);
}

/* RTC: wakeup: 1 Hz */
static void rtc_setup(void)
{
	rcc_set_rtc_clock(RCC_LSE, 0);
	rcc_enable_clock(RCC_RTC);
	rtc_unlock();
	rtc_init(PREDIV_A, PREDIV_S, 0x00012101, 0x00000000, false, false);
	if (*rom_calib < 0 && *rom_calib >= -511)
		rtc_set_smooth_digital_calibration(0, -*rom_calib, RTC_32S);
	else if (*rom_calib > 0 && *rom_calib <= 512)
		rtc_set_smooth_digital_calibration(1, 512 - *rom_calib,
						   RTC_32S);
	rtc_enable_interrupt(RTC_ALARM_A | RTC_ALARM_B | RTC_WAKEUP);
	rtc_enable_wakeup_timer(RTC_CK_SPRE, 1 - 1);
	rtc_lock();
}

void usart1_isr(void)
{
	int m;
	int s;
	static int rx_wp;
	static int tx_rp;

	m = usart_get_interrupt_mask(USART1, USART_RXNE | USART_TXE);
	s = usart_get_interrupt_status(USART1, USART_RXNE | USART_TXE);
	if (m & s & USART_RXNE) {
		if (rx_len < RXBUFSIZE) {
			rx_buf[rx_wp++] = usart_recv(USART1);
			rx_wp &= RXBUFSIZE - 1;
			rx_len++;
		} else {
			usart_flush_receive_data(USART1);
		}
	}
	if (m & s & USART_TXE) {
		if (tx_len) {
			usart_send(USART1, tx_buf[tx_rp++]);
			tx_rp &= TXBUFSIZE - 1;
			tx_len--;
	        } else {
			usart_disable_interrupt(USART1, USART_TXE);
		}
	}
}

static void tx_write_blocking(int data)
{
	static int tx_wp;

	while (tx_len == TXBUFSIZE)
		;
	usart_disable_interrupt(USART1, USART_TXE);
	tx_buf[tx_wp++] = data;
	tx_wp &= TXBUFSIZE - 1;
	tx_len++;
	usart_enable_interrupt(USART1, USART_TXE);
}

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n')
				tx_write_blocking('\r');
			tx_write_blocking(ptr[i]);
		}
		return i;
	}

	errno = EIO;
	return -1;
}

int _read(int file, char *ptr, int len)
{
	int i;
	static int rx_rp;

	if (file == STDIN_FILENO) {
		for (i = 0; i < len; i++) {
			while (rx_len == 0)
				;
			usart_disable_interrupt(USART1, USART_RXNE);
			ptr[i] = rx_buf[rx_rp++];
			rx_rp &= RXBUFSIZE - 1;
			rx_len--;
			usart_enable_interrupt(USART1, USART_RXNE);
			if (ptr[i] == '\r')
				ptr[i] = '\n';
		}
		return i;
	}

	errno = EIO;
	return -1;
}

/* Delay: 1 - 32768 us */
void delay_us(int us)
{
	tim_set_autoreload(TIM21, us << 1);
	tim_enable_counter(TIM21);
	while (!tim_get_interrupt_status(TIM21, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM21, TIM_UPDATE);
}

/* Delay: 1 - 32768 ms */
void delay_ms(int ms)
{
	tim_set_autoreload(TIM22, ms << 1);
	tim_enable_counter(TIM22);
	while (!tim_get_interrupt_status(TIM22, TIM_UPDATE))
		;
	tim_clear_interrupt(TIM22, TIM_UPDATE);
}

void rtc_isr(void)
{
	int date;
	int time;
	int ss;

	if (exti_get_interrupt_mask(EXTI_RTC_WAKEUP_TIMER) &&
	    exti_get_interrupt_status(EXTI_RTC_WAKEUP_TIMER)) {
		if (rtc_get_interrupt_mask(RTC_WAKEUP) &&
		    rtc_get_interrupt_status(RTC_WAKEUP)) {
			rtc_get_calendar(&date, &time, &ss);
			lcd_date(date);
			lcd_time(time);
			rtc_clear_interrupt(RTC_WAKEUP);
		}
		exti_clear_interrupt(EXTI_RTC_WAKEUP_TIMER);
	}
	if (exti_get_interrupt_mask(EXTI_RTC_ALARM) &&
	    exti_get_interrupt_status(EXTI_RTC_ALARM)) {
		if (rtc_get_interrupt_mask(RTC_ALARM_A) &&
		    rtc_get_interrupt_status(RTC_ALARM_A)) {
			if (!alarm_a_flag) {
				alarm_a_flag = true;
				gpio_clear(GPIO_PB0);
			}
			rtc_clear_interrupt(RTC_ALARM_A);
		}
		if (rtc_get_interrupt_mask(RTC_ALARM_B) &&
		    rtc_get_interrupt_status(RTC_ALARM_B)) {
			if (!alarm_b_flag) {
				alarm_b_flag = true;
				gpio_clear(GPIO_PB1);
			}
			rtc_clear_interrupt(RTC_ALARM_B);
		}
		exti_clear_interrupt(EXTI_RTC_ALARM);
	}
}

void eeprom_write(int *addr, int data)
{
	flash_unlock(FLASH_LOCK_PECR);
	*addr = data;
	while (!flash_get_status(FLASH_EOP))
		;
	flash_clear_status(FLASH_EOP);
	flash_lock(FLASH_LOCK_PECR);
}

int main(void)
{
	clock_setup();
	gpio_setup();
	usart_setup();
	exti_setup();
	spi_setup();
	tim_setup();
	if (*rom_contrast >= 0 && *rom_contrast <= 128)
		mcp_write_wiper(*rom_contrast);
	if (*rom_bright >= 0 && *rom_bright <= 255)
		tim_set_capture_compare_value(TIM2_CC1, *rom_bright);
	lcd_init();
	rtc_setup();
	
	command();

	return 0;
}
