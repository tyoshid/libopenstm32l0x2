/*
 * indirection.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of monitor.
 *
 * monitor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * monitor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with monitor.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "monitor.h"

enum ind_op {
	IND_ASSIGN,
	IND_OR,
	IND_AND
};

static void ind_size(char *p, int *size)
{
	if (strcmp(p, "char*") == 0)
		*size = 1;
	else if (strcmp(p, "short*") == 0)
		*size = 2;
	else if (strcmp(p, "int*") == 0)
		*size = 4;
	else
		*size = 0;
}

static int ind_addr(char *p, int *addr)
{
	char *endp;

	*addr = strtoul(p, &endp, 0);
	if (*endp != '\0') {
		printf("Invalid address: %s\n", p);
		return -1;
	}
	return 0;
}
	
static void ind_print(int size, int addr)
{
	if (size == 2)
		addr &= ~1;
	else if (size != 1)
		addr &= ~3;
	printf("0x%08x: ", addr);
	if (size == 1)
		printf("0x%02x\n", *(unsigned char *)addr);
	else if (size == 2)
		printf("0x%04x\n", *(unsigned short *)addr);
	else
		printf("0x%08x\n", *(unsigned int *)addr);
}

static int ind_operator(char *p, enum ind_op *op)
{
	if (strcmp(p, "=") == 0) {
		*op = IND_ASSIGN;
	} else if (strcmp(p, "|=") == 0) {
		*op = IND_OR;
	} else if (strcmp(p, "&=~") == 0) {
		*op = IND_AND;
	} else {
		printf("Unknown operator: %s\n", p);
		return -1;
	}
	return 0;
}

static int ind_data(char *p, int *data)
{
	char *endp;

	*data = strtoul(p, &endp, 0);
	if (*endp != '\0') {
		printf("Invalid data: %s\n", p);
		return -1;
	}
	return 0;
}

static void ind_assign_simple(int size, int addr, int data)
{
	if (size == 1) {
		*(char *)addr = data;
		printf("0x%08x: <- 0x%02x\n", addr, data & 0xff);
	} else if (size == 2) {
		*(short *)addr = data;
		printf("0x%08x: <- 0x%04x\n", addr, data & 0xffff);
	} else {
		*(int *)addr = data;
		printf("0x%08x: <- 0x%08x\n", addr, data);
	}
}

static void ind_assign_or(int size, int addr, int data)
{
	if (size == 1) {
		*(char *)addr |= data;
		printf("0x%08x: OR 0x%02x\n", addr, data & 0xff);
	} else if (size == 2) {
		*(short *)addr |= data;
		printf("0x%08x: OR 0x%04x\n", addr, data & 0xffff);
	} else {
		*(int *)addr |= data;
		printf("0x%08x: OR 0x%08x\n", addr, data);
	}
}

static void ind_assign_and(int size, int addr, int data)
{
	if (size == 1) {
		*(char *)addr &= ~data;
		printf("0x%08x: AND 0x%02x\n", addr, ~data & 0xff);
	} else if (size == 2) {
		*(short *)addr &= ~data;
		printf("0x%08x: AND 0x%04x\n", addr, ~data & 0xffff);
	} else {
		*(int *)addr &= ~data;
		printf("0x%08x: AND 0x%08x\n", addr, ~data);
	}
}

static void ind_assign(int size, int addr, enum ind_op op, int data)
{
	if (size == 2)
		addr &= ~1;
	else if (size != 1)
		addr &= ~3;
	if (op == IND_ASSIGN)
		ind_assign_simple(size, addr, data);
	else if (op == IND_OR)
		ind_assign_or(size, addr, data);
	else if (op == IND_AND)
		ind_assign_and(size, addr, data);
}
		
int indirection(void)
{
	char *p;
	int size;
	int addr;
	enum ind_op op;
	int data;
	
	p = strtok(NULL, DELIMITER);
	if (p == NULL) {
		printf("???: *\n");
		return -1;
	}
	ind_size(p, &size);
	if (size) {
		p = strtok(NULL, DELIMITER);
		if (p == NULL) {
			printf("No address\n");
			return -1;
		}
	} else {
		size = 4;
	}
	if (ind_addr(p, &addr))
		return -1;
	p = strtok(NULL, DELIMITER);
	if (p == NULL) {
		ind_print(size, addr);
		return 0;
	}
	if (ind_operator(p, &op))
		return -1;
	p = strtok(NULL, DELIMITER);
	if (p == NULL) {
		printf("No data\n");
		return -1;
	}
	if (ind_data(p, &data))
		return -1;
	p = strtok(NULL, DELIMITER);
	if (p == NULL) {
		ind_assign(size, addr, op, data);
	} else {
		printf("???: %s\n", p);
		return -1;
	}
	return 0;
}
