/*
 * ethernet_camera.h
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

enum tcp_state {
	TCP_CLOSED,
	TCP_LISTEN,
	TCP_ESTABLISHED,
	TCP_FIN_WAIT,
	TCP_CLOSE_WAIT,
	TCP_LAST_ACK
};

#define SOCKET_BUFFER_SIZE	16 /* 16KB */

void delay_ms(int ms);
int rom_read_eui48(char *buf);
int camera_fbuf_ctrl_stop(void);
int camera_fbuf_ctrl_resume(void);
int camera_get_fbuf_len(void);
int camera_read_fbuf(int addr, int nbyte, char *buf);
int camera_init(void);
int w5200_getc(int offset);
void w5200_read_rxmem(char *buf, int nbyte, int offset);
void w5200_free_rxmem(int size);
void w5200_write_txmem(char *data, int nbyte);
void w5200_isr(void);
void w5200_enable(void);
void w5200_start(void);
int http_proc(void);
