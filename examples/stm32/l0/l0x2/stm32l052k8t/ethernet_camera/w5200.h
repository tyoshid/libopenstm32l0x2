/*
 * w5200.h
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

/* WIZnet iEthernet W5200 */

/* Common registers */
#define MR			0x0000	/* Mode */
#define GAR0			0x0001	/* Gateway Address */
#define GAR1			0x0002
#define GAR2			0x0003
#define GAR3			0x0004
#define SUBR0			0x0005	/* Subnet mask */
#define SUBR1			0x0006
#define SUBR2			0x0007
#define SUBR3			0x0008
#define SHAR0			0x0009	/* Source Hardware Address */
#define SHAR1			0x000a
#define SHAR2			0x000b
#define SHAR3			0x000c
#define SHAR4			0x000d
#define SHAR5			0x000e
#define SIPR0			0x000f	/* Source IP Address */
#define SIPR1			0x0010
#define SIPR2			0x0011
#define SIPR3			0x0012
#define IR			0x0015	/* Interrupt */
#define IMR			0x0016	/* Interrupt Mask */
#define RTR0			0x0017	/* Retry Time */
#define RTR1			0x0018
#define RCR			0x0019	/* Retry Count */
#define PATR0			0x001c	/* Authentication Type in PPPoE */
#define PATR1			0x001d
#define PPPALFO			0x001e	/* Authentication Algorithm in PPPoE */
#define VERSIONR		0x001f	/* Chip version */
#define PTIMER			0x0028	/* PPP LCP RequestTimer */
#define PMAGIC			0x0029	/* PPP LCP Magic number */
#define INTLEVEL0		0x0030	/* Interrupt Low Level Timer */
#define INTLEVEL1		0x0031
#define IR2			0x0034	/* Socket Interrupt */
#define PSTATUS			0x0035	/* PHY Status */
#define IMR2			0x0036	/* Socket Interrupt Mask */
/* Socket registers */
/* Socket n Mode */
#define S_MR(n)			(0x4000 + (n) * 0x100)
#define S0_MR			S_MR(0)
#define S1_MR			S_MR(1)
#define S2_MR			S_MR(2)
#define S3_MR			S_MR(3)
#define S4_MR			S_MR(4)
#define S5_MR			S_MR(5)
#define S6_MR			S_MR(6)
#define S7_MR			S_MR(7)
/* Socket n Command */
#define S_CR(n)			(0x4001 + (n) * 0x100)
#define S0_CR			S_CR(0)
#define S1_CR			S_CR(1)
#define S2_CR			S_CR(2)
#define S3_CR			S_CR(3)
#define S4_CR			S_CR(4)
#define S5_CR			S_CR(5)
#define S6_CR			S_CR(6)
#define S7_CR			S_CR(7)
/* Socket n Interrupt */
#define S_IR(n)			(0x4002 + (n) * 0x100)
#define S0_IR			S_IR(0)
#define S1_IR			S_IR(1)
#define S2_IR			S_IR(2)
#define S3_IR			S_IR(3)
#define S4_IR			S_IR(4)
#define S5_IR			S_IR(5)
#define S6_IR			S_IR(6)
#define S7_IR			S_IR(7)
/* Socket n Status */
#define S_SR(n)			(0x4003 + (n) * 0x100)
#define S0_SR			S_SR(0)
#define S1_SR			S_SR(1)
#define S2_SR			S_SR(2)
#define S3_SR			S_SR(3)
#define S4_SR			S_SR(4)
#define S5_SR			S_SR(5)
#define S6_SR			S_SR(6)
#define S7_SR			S_SR(7)
/* Socket n Source Port */
#define S_PORT0(n)		(0x4004 + (n) * 0x100)
#define S0_PORT0		S_PORT0(0)
#define S1_PORT0		S_PORT0(1)
#define S2_PORT0		S_PORT0(2)
#define S3_PORT0		S_PORT0(3)
#define S4_PORT0		S_PORT0(4)
#define S5_PORT0		S_PORT0(5)
#define S6_PORT0		S_PORT0(6)
#define S7_PORT0		S_PORT0(7)
#define S_PORT1(n)		(0x4005 + (n) * 0x100)
#define S0_PORT1		S_PORT1(0)
#define S1_PORT1		S_PORT1(1)
#define S2_PORT1		S_PORT1(2)
#define S3_PORT1		S_PORT1(3)
#define S4_PORT1		S_PORT1(4)
#define S5_PORT1		S_PORT1(5)
#define S6_PORT1		S_PORT1(6)
#define S7_PORT1		S_PORT1(7)
/* Socket n Destination Hardware Address */
#define S_DHAR0(n)		(0x4006 + (n) * 0x100)
#define S0_DHAR0		S_DHAR0(0)
#define S1_DHAR0		S_DHAR0(1)
#define S2_DHAR0		S_DHAR0(2)
#define S3_DHAR0		S_DHAR0(3)
#define S4_DHAR0		S_DHAR0(4)
#define S5_DHAR0		S_DHAR0(5)
#define S6_DHAR0		S_DHAR0(6)
#define S7_DHAR0		S_DHAR0(7)
#define S_DHAR1(n)		(0x4007 + (n) * 0x100)
#define S0_DHAR1		S_DHAR1(0)
#define S1_DHAR1		S_DHAR1(1)
#define S2_DHAR1		S_DHAR1(2)
#define S3_DHAR1		S_DHAR1(3)
#define S4_DHAR1		S_DHAR1(4)
#define S5_DHAR1		S_DHAR1(5)
#define S6_DHAR1		S_DHAR1(6)
#define S7_DHAR1		S_DHAR1(7)
#define S_DHAR2(n)		(0x4008 + (n) * 0x100)
#define S0_DHAR2		S_DHAR2(0)
#define S1_DHAR2		S_DHAR2(1)
#define S2_DHAR2		S_DHAR2(2)
#define S3_DHAR2		S_DHAR2(3)
#define S4_DHAR2		S_DHAR2(4)
#define S5_DHAR2		S_DHAR2(5)
#define S6_DHAR2		S_DHAR2(6)
#define S7_DHAR2		S_DHAR2(7)
#define S_DHAR3(n)		(0x4009 + (n) * 0x100)
#define S0_DHAR3		S_DHAR3(0)
#define S1_DHAR3		S_DHAR3(1)
#define S2_DHAR3		S_DHAR3(2)
#define S3_DHAR3		S_DHAR3(3)
#define S4_DHAR3		S_DHAR3(4)
#define S5_DHAR3		S_DHAR3(5)
#define S6_DHAR3		S_DHAR3(6)
#define S7_DHAR3		S_DHAR3(7)
#define S_DHAR4(n)		(0x400a + (n) * 0x100)
#define S0_DHAR4		S_DHAR4(0)
#define S1_DHAR4		S_DHAR4(1)
#define S2_DHAR4		S_DHAR4(2)
#define S3_DHAR4		S_DHAR4(3)
#define S4_DHAR4		S_DHAR4(4)
#define S5_DHAR4		S_DHAR4(5)
#define S6_DHAR4		S_DHAR4(6)
#define S7_DHAR4		S_DHAR4(7)
#define S_DHAR5(n)		(0x400b + (n) * 0x100)
#define S0_DHAR5		S_DHAR5(0)
#define S1_DHAR5		S_DHAR5(1)
#define S2_DHAR5		S_DHAR5(2)
#define S3_DHAR5		S_DHAR5(3)
#define S4_DHAR5		S_DHAR5(4)
#define S5_DHAR5		S_DHAR5(5)
#define S6_DHAR5		S_DHAR5(6)
#define S7_DHAR5		S_DHAR5(7)
/* Socket n Destination IP Address */
#define S_DIPR0(n)		(0x400c + (n) * 0x100)
#define S0_DIPR0		S_DIPR0(0)
#define S1_DIPR0		S_DIPR0(1)
#define S2_DIPR0		S_DIPR0(2)
#define S3_DIPR0		S_DIPR0(3)
#define S4_DIPR0		S_DIPR0(4)
#define S5_DIPR0		S_DIPR0(5)
#define S6_DIPR0		S_DIPR0(6)
#define S7_DIPR0		S_DIPR0(7)
#define S_DIPR1(n)		(0x400d + (n) * 0x100)
#define S0_DIPR1		S_DIPR1(0)
#define S1_DIPR1		S_DIPR1(1)
#define S2_DIPR1		S_DIPR1(2)
#define S3_DIPR1		S_DIPR1(3)
#define S4_DIPR1		S_DIPR1(4)
#define S5_DIPR1		S_DIPR1(5)
#define S6_DIPR1		S_DIPR1(6)
#define S7_DIPR1		S_DIPR1(7)
#define S_DIPR2(n)		(0x400e + (n) * 0x100)
#define S0_DIPR2		S_DIPR2(0)
#define S1_DIPR2		S_DIPR2(1)
#define S2_DIPR2		S_DIPR2(2)
#define S3_DIPR2		S_DIPR2(3)
#define S4_DIPR2		S_DIPR2(4)
#define S5_DIPR2		S_DIPR2(5)
#define S6_DIPR2		S_DIPR2(6)
#define S7_DIPR2		S_DIPR2(7)
#define S_DIPR3(n)		(0x400f + (n) * 0x100)
#define S0_DIPR3		S_DIPR3(0)
#define S1_DIPR3		S_DIPR3(1)
#define S2_DIPR3		S_DIPR3(2)
#define S3_DIPR3		S_DIPR3(3)
#define S4_DIPR3		S_DIPR3(4)
#define S5_DIPR3		S_DIPR3(5)
#define S6_DIPR3		S_DIPR3(6)
#define S7_DIPR3		S_DIPR3(7)
/* Socket n Destination Port */
#define S_DPORT0(n)		(0x4010 + (n) * 0x100)
#define S0_DPORT0		S_DPORT0(0)
#define S1_DPORT0		S_DPORT0(1)
#define S2_DPORT0		S_DPORT0(2)
#define S3_DPORT0		S_DPORT0(3)
#define S4_DPORT0		S_DPORT0(4)
#define S5_DPORT0		S_DPORT0(5)
#define S6_DPORT0		S_DPORT0(6)
#define S7_DPORT0		S_DPORT0(7)
#define S_DPORT1(n)		(0x4011 + (n) * 0x100)
#define S0_DPORT1		S_DPORT1(0)
#define S1_DPORT1		S_DPORT1(1)
#define S2_DPORT1		S_DPORT1(2)
#define S3_DPORT1		S_DPORT1(3)
#define S4_DPORT1		S_DPORT1(4)
#define S5_DPORT1		S_DPORT1(5)
#define S6_DPORT1		S_DPORT1(6)
#define S7_DPORT1		S_DPORT1(7)
/* Socket n Maximum Segment Size */
#define S_MSSR0(n)		(0x4012 + (n) * 0x100)
#define S0_MSSR0		S_MSSR0(0)
#define S1_MSSR0		S_MSSR0(1)
#define S2_MSSR0		S_MSSR0(2)
#define S3_MSSR0		S_MSSR0(3)
#define S4_MSSR0		S_MSSR0(4)
#define S5_MSSR0		S_MSSR0(5)
#define S6_MSSR0		S_MSSR0(6)
#define S7_MSSR0		S_MSSR0(7)
#define S_MSSR1(n)		(0x4013 + (n) * 0x100)
#define S0_MSSR1		S_MSSR1(0)
#define S1_MSSR1		S_MSSR1(1)
#define S2_MSSR1		S_MSSR1(2)
#define S3_MSSR1		S_MSSR1(3)
#define S4_MSSR1		S_MSSR1(4)
#define S5_MSSR1		S_MSSR1(5)
#define S6_MSSR1		S_MSSR1(6)
#define S7_MSSR1		S_MSSR1(7)
/* Socket n Protocol in IP Raw mode */
#define S_PROTO(n)		(0x4014 + (n) * 0x100)
#define S0_PROTO		S_PROTO(0)
#define S1_PROTO		S_PROTO(1)
#define S2_PROTO		S_PROTO(2)
#define S3_PROTO		S_PROTO(3)
#define S4_PROTO		S_PROTO(4)
#define S5_PROTO		S_PROTO(5)
#define S6_PROTO		S_PROTO(6)
#define S7_PROTO		S_PROTO(7)
/* Socket n IP TOS */
#define S_TOS(n)		(0x4015 + (n) * 0x100)
#define S0_TOS			S_TOS(0)
#define S1_TOS			S_TOS(1)
#define S2_TOS			S_TOS(2)
#define S3_TOS			S_TOS(3)
#define S4_TOS			S_TOS(4)
#define S5_TOS			S_TOS(5)
#define S6_TOS			S_TOS(6)
#define S7_TOS			S_TOS(7)
/* Socket n IP TTL */
#define S_TTL(n)		(0x4016 + (n) * 0x100)
#define S0_TTL			S_TTL(0)
#define S1_TTL			S_TTL(1)
#define S2_TTL			S_TTL(2)
#define S3_TTL			S_TTL(3)
#define S4_TTL			S_TTL(4)
#define S5_TTL			S_TTL(5)
#define S6_TTL			S_TTL(6)
#define S7_TTL			S_TTL(7)
/* Socket n Receive Memory Size */
#define S_RXMEM_SIZE(n)		(0x401e + (n) * 0x100)
#define S0_RXMEM_SIZE		S_RXMEM_SIZE(0)
#define S1_RXMEM_SIZE		S_RXMEM_SIZE(1)
#define S2_RXMEM_SIZE		S_RXMEM_SIZE(2)
#define S3_RXMEM_SIZE		S_RXMEM_SIZE(3)
#define S4_RXMEM_SIZE		S_RXMEM_SIZE(4)
#define S5_RXMEM_SIZE		S_RXMEM_SIZE(5)
#define S6_RXMEM_SIZE		S_RXMEM_SIZE(6)
#define S7_RXMEM_SIZE		S_RXMEM_SIZE(7)
/* Socket n Transmit Memory Size */
#define S_TXMEM_SIZE(n)		(0x401f + (n) * 0x100)
#define S0_TXMEM_SIZE		S_TXMEM_SIZE(0)
#define S1_TXMEM_SIZE		S_TXMEM_SIZE(1)
#define S2_TXMEM_SIZE		S_TXMEM_SIZE(2)
#define S3_TXMEM_SIZE		S_TXMEM_SIZE(3)
#define S4_TXMEM_SIZE		S_TXMEM_SIZE(4)
#define S5_TXMEM_SIZE		S_TXMEM_SIZE(5)
#define S6_TXMEM_SIZE		S_TXMEM_SIZE(6)
#define S7_TXMEM_SIZE		S_TXMEM_SIZE(7)
/* Socket n TX Free Size */
#define S_TX_FSR0(n)		(0x4020 + (n) * 0x100)
#define S0_TX_FSR0		S_TX_FSR0(0)
#define S1_TX_FSR0		S_TX_FSR0(1)
#define S2_TX_FSR0		S_TX_FSR0(2)
#define S3_TX_FSR0		S_TX_FSR0(3)
#define S4_TX_FSR0		S_TX_FSR0(4)
#define S5_TX_FSR0		S_TX_FSR0(5)
#define S6_TX_FSR0		S_TX_FSR0(6)
#define S7_TX_FSR0		S_TX_FSR0(7)
#define S_TX_FSR1(n)		(0x4021 + (n) * 0x100)
#define S0_TX_FSR1		S_TX_FSR1(0)
#define S1_TX_FSR1		S_TX_FSR1(1)
#define S2_TX_FSR1		S_TX_FSR1(2)
#define S3_TX_FSR1		S_TX_FSR1(3)
#define S4_TX_FSR1		S_TX_FSR1(4)
#define S5_TX_FSR1		S_TX_FSR1(5)
#define S6_TX_FSR1		S_TX_FSR1(6)
#define S7_TX_FSR1		S_TX_FSR1(7)
/* Socket n TX Read Pointer */
#define S_TX_RD0(n)		(0x4022 + (n) * 0x100)
#define S0_TX_RD0		S_TX_RD0(0)
#define S1_TX_RD0		S_TX_RD0(1)
#define S2_TX_RD0		S_TX_RD0(2)
#define S3_TX_RD0		S_TX_RD0(3)
#define S4_TX_RD0		S_TX_RD0(4)
#define S5_TX_RD0		S_TX_RD0(5)
#define S6_TX_RD0		S_TX_RD0(6)
#define S7_TX_RD0		S_TX_RD0(7)
#define S_TX_RD1(n)		(0x4023 + (n) * 0x100)
#define S0_TX_RD1		S_TX_RD1(0)
#define S1_TX_RD1		S_TX_RD1(1)
#define S2_TX_RD1		S_TX_RD1(2)
#define S3_TX_RD1		S_TX_RD1(3)
#define S4_TX_RD1		S_TX_RD1(4)
#define S5_TX_RD1		S_TX_RD1(5)
#define S6_TX_RD1		S_TX_RD1(6)
#define S7_TX_RD1		S_TX_RD1(7)
/* Socket n TX Write Pointer */
#define S_TX_WR0(n)		(0x4024 + (n) * 0x100)
#define S0_TX_WR0		S_TX_WR0(0)
#define S1_TX_WR0		S_TX_WR0(1)
#define S2_TX_WR0		S_TX_WR0(2)
#define S3_TX_WR0		S_TX_WR0(3)
#define S4_TX_WR0		S_TX_WR0(4)
#define S5_TX_WR0		S_TX_WR0(5)
#define S6_TX_WR0		S_TX_WR0(6)
#define S7_TX_WR0		S_TX_WR0(7)
#define S_TX_WR1(n)		(0x4025 + (n) * 0x100)
#define S0_TX_WR1		S_TX_WR1(0)
#define S1_TX_WR1		S_TX_WR1(1)
#define S2_TX_WR1		S_TX_WR1(2)
#define S3_TX_WR1		S_TX_WR1(3)
#define S4_TX_WR1		S_TX_WR1(4)
#define S5_TX_WR1		S_TX_WR1(5)
#define S6_TX_WR1		S_TX_WR1(6)
#define S7_TX_WR1		S_TX_WR1(7)
/* Socket n RX Received Size */
#define S_RX_RSR0(n)		(0x4026 + (n) * 0x100)
#define S0_RX_RSR0		S_RX_RSR0(0)
#define S1_RX_RSR0		S_RX_RSR0(1)
#define S2_RX_RSR0		S_RX_RSR0(2)
#define S3_RX_RSR0		S_RX_RSR0(3)
#define S4_RX_RSR0		S_RX_RSR0(4)
#define S5_RX_RSR0		S_RX_RSR0(5)
#define S6_RX_RSR0		S_RX_RSR0(6)
#define S7_RX_RSR0		S_RX_RSR0(7)
#define S_RX_RSR1(n)		(0x4027 + (n) * 0x100)
#define S0_RX_RSR1		S_RX_RSR1(0)
#define S1_RX_RSR1		S_RX_RSR1(1)
#define S2_RX_RSR1		S_RX_RSR1(2)
#define S3_RX_RSR1		S_RX_RSR1(3)
#define S4_RX_RSR1		S_RX_RSR1(4)
#define S5_RX_RSR1		S_RX_RSR1(5)
#define S6_RX_RSR1		S_RX_RSR1(6)
#define S7_RX_RSR1		S_RX_RSR1(7)
/* Socket n RX Read Pointer */
#define S_RX_RD0(n)		(0x4028 + (n) * 0x100)
#define S0_RX_RD0		S_RX_RD0(0)
#define S1_RX_RD0		S_RX_RD0(1)
#define S2_RX_RD0		S_RX_RD0(2)
#define S3_RX_RD0		S_RX_RD0(3)
#define S4_RX_RD0		S_RX_RD0(4)
#define S5_RX_RD0		S_RX_RD0(5)
#define S6_RX_RD0		S_RX_RD0(6)
#define S7_RX_RD0		S_RX_RD0(7)
#define S_RX_RD1(n)		(0x4029 + (n) * 0x100)
#define S0_RX_RD1		S_RX_RD1(0)
#define S1_RX_RD1		S_RX_RD1(1)
#define S2_RX_RD1		S_RX_RD1(2)
#define S3_RX_RD1		S_RX_RD1(3)
#define S4_RX_RD1		S_RX_RD1(4)
#define S5_RX_RD1		S_RX_RD1(5)
#define S6_RX_RD1		S_RX_RD1(6)
#define S7_RX_RD1		S_RX_RD1(7)
/* Socket n RX Write Pointer */
#define S_RX_WR0(n)		(0x402a + (n) * 0x100)
#define S0_RX_WR0		S_RX_WR0(0)
#define S1_RX_WR0		S_RX_WR0(1)
#define S2_RX_WR0		S_RX_WR0(2)
#define S3_RX_WR0		S_RX_WR0(3)
#define S4_RX_WR0		S_RX_WR0(4)
#define S5_RX_WR0		S_RX_WR0(5)
#define S6_RX_WR0		S_RX_WR0(6)
#define S7_RX_WR0		S_RX_WR0(7)
#define S_RX_WR1(n)		(0x402b + (n) * 0x100)
#define S0_RX_WR1		S_RX_WR1(0)
#define S1_RX_WR1		S_RX_WR1(1)
#define S2_RX_WR1		S_RX_WR1(2)
#define S3_RX_WR1		S_RX_WR1(3)
#define S4_RX_WR1		S_RX_WR1(4)
#define S5_RX_WR1		S_RX_WR1(5)
#define S6_RX_WR1		S_RX_WR1(6)
#define S7_RX_WR1		S_RX_WR1(7)
/* Socket n Interrupt Mask */
#define S_IMR(n)		(0x402c + (n) * 0x100)
#define S0_IMR			S_IMR(0)
#define S1_IMR			S_IMR(1)
#define S2_IMR			S_IMR(2)
#define S3_IMR			S_IMR(3)
#define S4_IMR			S_IMR(4)
#define S5_IMR			S_IMR(5)
#define S6_IMR			S_IMR(6)
#define S7_IMR			S_IMR(7)
/* Socket n Fragment Offset in IP header */
#define S_FRAG0(n)		(0x402d + (n) * 0x100)
#define S0_FRAG0		S_FRAG0(0)
#define S1_FRAG0		S_FRAG0(1)
#define S2_FRAG0		S_FRAG0(2)
#define S3_FRAG0		S_FRAG0(3)
#define S4_FRAG0		S_FRAG0(4)
#define S5_FRAG0		S_FRAG0(5)
#define S6_FRAG0		S_FRAG0(6)
#define S7_FRAG0		S_FRAG0(7)
#define S_FRAG1(n)		(0x402e + (n) * 0x100)
#define S0_FRAG1		S_FRAG1(0)
#define S1_FRAG1		S_FRAG1(1)
#define S2_FRAG1		S_FRAG1(2)
#define S3_FRAG1		S_FRAG1(3)
#define S4_FRAG1		S_FRAG1(4)
#define S5_FRAG1		S_FRAG1(5)
#define S6_FRAG1		S_FRAG1(6)
#define S7_FRAG1		S_FRAG1(7)

#define TX_MEMORY		0x8000
#define RX_MEMORY		0xc000

/* MR (Mode Register) */
#define MR_RST			(1 << 7)
#define MR_WOL			(1 << 5)
#define MR_PB			(1 << 4)
#define MR_PPPOE		(1 << 3)

/* IR (Interrupt Register) */
#define IR_CONFLICT		(1 << 7)
#define IR_PPPOE		(1 << 5)

/* IMR (Interrupt Mask Register) */
#define IMR_S7_IMR		(1 << 7)
#define IMR_S6_IMR		(1 << 6)
#define IMR_S5_IMR		(1 << 5)
#define IMR_S4_IMR		(1 << 4)
#define IMR_S3_IMR		(1 << 3)
#define IMR_S2_IMR		(1 << 2)
#define IMR_S1_IMR		(1 << 1)
#define IMR_S0_IMR		(1 << 0)

/* IR2 (Socket Interrupt Register) */
#define IR2_S7_INT		(1 << 7)
#define IR2_S6_INT		(1 << 6)
#define IR2_S5_INT		(1 << 5)
#define IR2_S4_INT		(1 << 4)
#define IR2_S3_INT		(1 << 3)
#define IR2_S2_INT		(1 << 2)
#define IR2_S1_INT		(1 << 1)
#define IR2_S0_INT		(1 << 0)

/* PHYSTATUS (PHY Status Register) */
#define PHYSTATUS_LINK		(1 << 5)
#define PHYSTATUS_POWERDOWN	(1 << 3)

/* IMR2 (Socket Interrupt Mask Register) */
#define IMR2_CONFLICT		(1 << 7)
#define IMR2_PPPOE		(1 << 5)

/* Sn_MR (Socket n Mode Register) */
#define SN_MR_MULTI		(1 << 7)
#define SN_MR_MF		(1 << 6)
#define SN_MR_ND_MC		(1 << 5)
#define SN_MR_P3		(1 << 3)
#define SN_MR_P2		(1 << 2)
#define SN_MR_P1		(1 << 1)
#define SN_MR_P0		(1 << 0)
/* Protocol */
#define SN_MR_P_CLOSED		(0 << 0)
#define SN_MR_P_TCP		(1 << 0)
#define SN_MR_P_UDP		(2 << 0)
#define SN_MR_P_IPRAW		(3 << 0)
#define SN_MR_P_MACRAW		(4 << 0)
#define SN_MR_P_PPPOE		(5 << 0)

/* Sn_CR (Socket n Command Register) */
#define SN_CR_OPEN		0x01
#define SN_CR_LISTEN		0x02
#define SN_CR_CONNECT		0x04
#define SN_CR_DISCON		0x08
#define SN_CR_CLOSE		0x10
#define SN_CR_SEND		0x20
#define SN_CR_SEND_MAC		0x21
#define SN_CR_SEND_KEEP		0x22
#define SN_CR_RECV		0x40

#define SN_CR_PCON		0x23
#define SN_CR_PDISCON		0x24
#define SN_CR_PCR		0x25
#define SN_CR_PCN		0x26
#define SN_CR_PCJ		0x27

/* Sn_IR (Socket n Interrupt Register) */
#define SN_IR_PRECV		(1 << 7)
#define SN_IR_PFAIL		(1 << 6)
#define SN_IR_PNEXT		(1 << 5)
#define SN_IR_SEND_OK		(1 << 4)
#define SN_IR_TIMEOUT		(1 << 3)
#define SN_IR_RECV		(1 << 2)
#define SN_IR_DISCON		(1 << 1)
#define SN_IR_CON		(1 << 0)

/* Sn_SR (Socket n Status Register) */
#define SN_SR_SOCK_CLOSED	0x00
#define SN_SR_SOCK_INIT		0x13
#define SN_SR_SOCK_LISTEN	0x14
#define SN_SR_SOCK_ESTABLISHED	0x17
#define SN_SR_SOCK_CLOSE_WAIT	0x1c
#define SN_SR_SOCK_UDP		0x22
#define SN_SR_SOCK_IPRAW	0x32
#define SN_SR_SOCK_MACRAW	0x42
#define SN_SR_SOCK_PPPOE	0x5f

#define SN_SR_SOCK_SYNSENT	0x15
#define SN_SR_SOCK_SYNRECV	0x16
#define SN_SR_SOCK_FIN_WAIT	0x18
#define SN_SR_SOCK_CLOSING	0x1a
#define SN_SR_SOCK_TIME_WAIT	0x1b
#define SN_SR_SOCK_LAST_ACK	0x1d
#define SN_SR_SOCK_ARP		0x01

/* Sn_IMR (Socket n Interrupt Mask Register) */
#define SN_IMR_PRECV		(1 << 7)
#define SN_IMR_PFAIL		(1 << 6)
#define SN_IMR_PNEXT		(1 << 5)
#define SN_IMR_SEND_OK		(1 << 4)
#define SN_IMR_TIMEOUT		(1 << 3)
#define SN_IMR_RECV		(1 << 2)
#define SN_IMR_DISCON		(1 << 1)
#define SN_IMR_CON		(1 << 0)

/* SPI Frame Format */
/* OP code */
#define OP_WRITE		0x80
#define OP_READ			0

/* Reset Timing */
#define TRC			1   /* Reset Cycle Time: 1 msec(2 usec min.) */
#define TPL			150 /* RSTn internal PLOCK: 150 msec */
