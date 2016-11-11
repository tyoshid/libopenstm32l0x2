# Makefile for constant

# Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

PROG		= constant
PERIPHERAL	= memorymap.o flash.o crc.o fw.o pwr.o rcc.o crs.o \
		  gpio.o syscfg.o dma.o vector.o exti.o adc.o dac.o \
		  comp.o tsc.o aes.o rng.o tim.o lptim.o iwdg.o wwdg.o rtc.o \
		  i2c.o usart.o lpuart.o spi.o usbdevfs.o dbg.o misc.o
COREPERIPHERAL	= ppb.o nvic.o stk.o scb.o mpu.o
OBJS		= $(PERIPHERAL) $(COREPERIPHERAL) symtab.o constant.o
LIBDIR		?= ../../../../../../..

CC		?= gcc
CFLAGS		+= -O2 -Wall -Wno-pointer-to-int-cast \
		   -I$(LIBDIR)/include -I$(LIBDIR)/include/stm32/l0 \
		   -I$(LIBDIR)/include/stm32/l0/l0x2

.PHONY: all clean

all: $(PROG)

%.o: %.c
	echo "  $<"
	$(CC) $(CFLAGS) -c $<

$(PROG): $(OBJS)
	echo "  $@"
	$(CC) -o $(PROG) $(OBJS)

clean:
	rm -f $(PROG) $(OBJS) $(PERIPHERAL:.o=.c) $(COREPERIPHERAL:.o=.c) \
	      symtab.c

constant.o: symtab.h constant.c

symtab.o: symtab.h symtab.c

$(PERIPHERAL): $(LIBDIR)/include/stm32/l0/l0x2/memorymap.h

$(COREPERIPHERAL): $(LIBDIR)/include/stm32/l0/ppb.h

$(PERIPHERAL:.o=.c): %.c: $(LIBDIR)/include/stm32/l0/l0x2/%.h
	awk -f constant.awk -v dev="$(basename $@)" -v map="memorymap" $< > $@

$(COREPERIPHERAL:.o=.c): %.c: $(LIBDIR)/include/stm32/l0/%.h
	awk -f constant.awk -v dev="$(basename $@)" -v map="ppb" $< > $@

symtab.c: $(addprefix $(LIBDIR)/include/stm32/l0/l0x2/,$(PERIPHERAL:.o=.h)) \
	  $(addprefix $(LIBDIR)/include/stm32/l0/,$(COREPERIPHERAL:.o=.h))
	awk -f enum.awk $^ > $@
