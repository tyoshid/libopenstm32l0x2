/*
 * w5200.c
 *
 * Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of ethernet_camera.
 *
 * ethernet_camera is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ethernet_camera is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ethernet_camera.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdbool.h>

#include <gpio.h>
#include <spi.h>
#include <dma.h>
#include <memorymap.h>
#include <nvic.h>
#include <vector.h>

#include "ethernet_camera.h"
#include "w5200.h"

/*
 * WIZnet WIZ820io
 *
 * nRESET - PB0, PWDN - PB1
 */

volatile int rcvlen;
volatile int sndlen;
volatile int tx_rd;
volatile enum tcp_state tcp_state;

static int w5200_read8(int addr)
{
	static char sndbuf[5] = {
		0,		/* Address high */
		0,		/* Address low */
		OP_READ,	/* OP Code + length high */
		1,		/* Length low */
		0		/* Data */
	};
	char r;

	sndbuf[0] = addr >> 8;
	sndbuf[1] = addr;
	dma_setup_channel(DMA_CH2, (int)&r, SPI1_DR, 5, DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)sndbuf, SPI1_DR, 5,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
	return r;
}

static void w5200_write8(int addr, int data)
{
	static char sndbuf[5] = {
		0,		/* Address high */
		0,		/* Address low */
		OP_WRITE,	/* OP Code + length high */
		1,		/* Length low */
		0		/* Data */
	};
	char r;

	sndbuf[0] = addr >> 8;
	sndbuf[1] = addr;
	sndbuf[4] = data;
	dma_setup_channel(DMA_CH2, (int)&r, SPI1_DR, 5, DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)sndbuf, SPI1_DR, 5,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
}

static int w5200_read16(int addr)
{
	static char sndbuf[6] = {
		0,		/* Address high */
		0,		/* Address low */
		OP_READ,	/* OP Code + length high */
		2,		/* Length low */
		0,		/* Data high */
		0		/* Data low */
	};
	unsigned char rcvbuf[6];

	sndbuf[0] = addr >> 8;
	sndbuf[1] = addr;
	dma_setup_channel(DMA_CH2, (int)rcvbuf, SPI1_DR, 6,
			  DMA_M_INC | DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)sndbuf, SPI1_DR, 6,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
	return rcvbuf[4] << 8 | rcvbuf[5];
}

static void w5200_write16(int addr, int data)
{
	static char sndbuf[6] = {
		0,		/* Address high */
		0,		/* Address low */
		OP_WRITE,	/* OP Code + length high */
		2,		/* Length low */
		0,		/* Data high */
		0		/* Data low */
	};
	char r;

	sndbuf[0] = addr >> 8;
	sndbuf[1] = addr;
	sndbuf[4] = data >> 8;
	sndbuf[5] = data;
	dma_setup_channel(DMA_CH2, (int)&r, SPI1_DR, 6, DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)sndbuf, SPI1_DR, 6,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
}

static void w5200_read(int addr, char *data, int nbyte)
{
	char sndbuf[4];
	char r;
	char s = 0;
	
	sndbuf[0] = addr >> 8;
	sndbuf[1] = addr;
	sndbuf[2] = OP_READ | ((nbyte >> 8) & 0x7f);
	sndbuf[3] = nbyte;
	dma_setup_channel(DMA_CH2, (int)&r, SPI1_DR, 4, DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)sndbuf, SPI1_DR, 4,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);

	dma_setup_channel(DMA_CH2, (int)data, SPI1_DR, nbyte,
			  DMA_M_INC | DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)&s, SPI1_DR, nbyte,
			  DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
}

static void w5200_write(int addr, char *data, int nbyte)
{
	char sndbuf[4];
	char r;
	
	sndbuf[0] = addr >> 8;
	sndbuf[1] = addr;
	sndbuf[2] = OP_WRITE | ((nbyte >> 8) & 0x7f);
	sndbuf[3] = nbyte;
	dma_setup_channel(DMA_CH2, (int)&r, SPI1_DR, 4 + nbyte, DMA_ENABLE);
	dma_setup_channel(DMA_CH3, (int)sndbuf, SPI1_DR, 4,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH3, DMA_COMPLETE))
		;
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);

	dma_setup_channel(DMA_CH3, (int)data, SPI1_DR, nbyte,
			  DMA_M_INC | DMA_M_TO_P | DMA_ENABLE);
	while (!dma_get_interrupt_status(DMA_CH2, DMA_COMPLETE))
		;
	dma_disable(DMA_CH2);
	dma_disable(DMA_CH3);
	dma_clear_interrupt(DMA_CH2, DMA_GLOBAL);
	dma_clear_interrupt(DMA_CH3, DMA_GLOBAL);
}

int w5200_getc(int offset)
{
	int p;

	p = (w5200_read16(S0_RX_RD0) + offset) &
		(SOCKET_BUFFER_SIZE * 1024 - 1);
	return w5200_read8(RX_MEMORY + p);
}

void w5200_read_rxmem(char *buf, int nbyte, int offset)
{
	int p;
	int n;

	p = (w5200_read16(S0_RX_RD0) + offset) &
		(SOCKET_BUFFER_SIZE * 1024 - 1);
	if (p + nbyte > SOCKET_BUFFER_SIZE * 1024) {
		n = SOCKET_BUFFER_SIZE * 1024 - p;
		w5200_read(RX_MEMORY + p, buf, n);
		w5200_read(RX_MEMORY, buf + n, nbyte - n);
	} else {
		w5200_read(RX_MEMORY + p, buf, nbyte);
	}
}

void w5200_free_rxmem(int size)
{
	int p;

	p = w5200_read16(S0_RX_RD0) + size;
	w5200_write16(S0_RX_RD0, p);
	w5200_write8(S0_CR, SN_CR_RECV);
}

void w5200_write_txmem(char *data, int nbyte)
{
	int w;
	int p;
	int n;

	w = w5200_read16(S0_TX_WR0);
	p = w & (SOCKET_BUFFER_SIZE * 1024 - 1);
	if (p + nbyte > SOCKET_BUFFER_SIZE * 1024) {
		n = SOCKET_BUFFER_SIZE * 1024 - p;
		w5200_write(TX_MEMORY + p, data, n);
		w5200_write(TX_MEMORY, data + n, nbyte - n);
	} else {
		w5200_write(TX_MEMORY + p, data, nbyte);
	}
	tx_rd = w5200_read16(S0_TX_RD0);
	w += nbyte;
	w5200_write16(S0_TX_WR0, w);
	w5200_write8(S0_CR, SN_CR_SEND);
}

void w5200_isr(void)
{
	int s;
	int r;

	s = w5200_read8(S0_IR);
	if (s & SN_IR_CON)
		tcp_state = TCP_ESTABLISHED;

	if (s & SN_IR_SEND_OK) {
		if (sndlen) {
			r = w5200_read16(S0_TX_RD0);
			sndlen -= (r - tx_rd) & 0xffff;
			if (sndlen) {
				tx_rd = r;
				w5200_write8(S0_CR, SN_CR_SEND);
			}
		}
	}

	if (s & SN_IR_RECV)
		rcvlen = w5200_read16(S0_RX_RSR0);

	if (s & SN_IR_DISCON) {
		r = w5200_read8(S0_SR);
		if (r == SN_SR_SOCK_CLOSE_WAIT)
			tcp_state = TCP_CLOSE_WAIT;
		else if (r == SN_SR_SOCK_CLOSED)
			tcp_state = TCP_CLOSED;
	}

	if (s & SN_IR_TIMEOUT) {
		r = w5200_read8(S0_SR);
		if (r == SN_SR_SOCK_CLOSED)
			tcp_state = TCP_CLOSED;
	}
	
	w5200_write8(S0_IR, s);
}

static void w5200_tcp_server(int port)
{
#ifdef DEBUG
	char dest[14];
	int i;
#endif
	int n;
	extern bool connection_close;

	w5200_write8(S0_MR, SN_MR_P_TCP);
	w5200_write16(S0_PORT0, port);
	w5200_write8(S0_CR, SN_CR_OPEN);
	while (w5200_read8(S0_SR) != SN_SR_SOCK_INIT)
		;
	tcp_state = TCP_LISTEN;
	w5200_write8(S0_CR, SN_CR_LISTEN);
	gpio_clear(GPIO_PA0);
	while (tcp_state == TCP_LISTEN)
		;

#ifdef DEBUG
	if (tcp_state == TCP_ESTABLISHED) {
		puts("ESTABLISHED");
		nvic_disable_irq(NVIC_EXTI2_3);
		w5200_read(S0_DHAR0, dest, 14);
		nvic_enable_irq(NVIC_EXTI2_3);
		printf("inet ");
		for (i = 0; i < 4; i++)
			printf("%u%c", (unsigned char)dest[6 + i],
			       i == 3 ? ' ' : '.');
		printf(" port %u\n", (unsigned char)dest[10] << 8 |
		       (unsigned char)dest[11]);
		printf("ether ");
		for (i = 0; i < 6; i++)
			printf("%02x%c", (unsigned char)dest[i],
			       i == 5 ? '\n' : ':');
		printf("mss %u\n", (unsigned char)dest[12] << 8 |
		       (unsigned char)dest[13]);
	}
#endif
	
	while (tcp_state != TCP_CLOSED) {
		if (tcp_state == TCP_ESTABLISHED) {
			if (rcvlen) {
#ifdef DEBUG
				printf("receive %d bytes\n", rcvlen);
#endif
				n = http_proc();
#ifdef DEBUG
				if (n > 0)
					printf("process %d bytes\n", n);
#endif
				if (n < 0 || connection_close) {
					nvic_disable_irq(NVIC_EXTI2_3);
					w5200_write8(S0_CR, SN_CR_DISCON);
					tcp_state = TCP_FIN_WAIT;
					nvic_enable_irq(NVIC_EXTI2_3);
#ifdef DEBUG
					puts("FIN_WAIT");
#endif
				}
			}
		}

		if (tcp_state == TCP_CLOSE_WAIT) {
			nvic_disable_irq(NVIC_EXTI2_3);
			w5200_write8(S0_CR, SN_CR_DISCON);
			tcp_state = TCP_LAST_ACK;
			nvic_enable_irq(NVIC_EXTI2_3);
#ifdef DEBUG
			puts("CLOSE_WAIT");
#endif
		}
	}
#ifdef DEBUG	
	puts("CLOSED");
#endif
}

void w5200_enable(void)
{
	delay_ms(TRC);
	gpio_set(GPIO_PB0);
	delay_ms(TPL);
}

void w5200_start(void)
{
	extern char netinfo[];
	
	w5200_write8(IMR, IMR_S0_IMR);
	w5200_write(GAR0, netinfo, 18);
	w5200_write8(S0_RXMEM_SIZE, SOCKET_BUFFER_SIZE);
	w5200_write8(S0_TXMEM_SIZE, SOCKET_BUFFER_SIZE);
	w5200_write8(S0_IMR, SN_IMR_SEND_OK | SN_IMR_TIMEOUT | SN_IMR_RECV |
		     SN_IMR_DISCON | SN_IMR_CON);

	while (1)
		w5200_tcp_server(80);
}
