# Makefile for monitor

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

NAME		= monitor
PERIPHERAL	= flash.o crc.o fw.o pwr.o rcc.o crs.o gpio.o syscfg.o dma.o \
		  exti.o adc.o dac.o comp.o tsc.o aes.o rng.o tim.o \
		  lptim.o iwdg.o wwdg.o rtc.o i2c.o usart.o lpuart.o spi.o \
		  usbdevfs.o dbg.o misc.o
COREPERIPHERAL	= nvic.o stk.o scb.o mpu.o
NORMALOBJS	= function.o indirection.o command.o monitor.o
OBJS		= $(PERIPHERAL) $(COREPERIPHERAL) functab.o $(NORMALOBJS)
OUTFILES	= $(NAME).bin $(NAME).list
LDSCRIPT	= ldscripts/stm32l052x8.x
LDSPECS		= --specs=nano.specs --specs=nosys.specs
LIBS		= -lstm32l0x2
LIBDIR		?= ../../../../../..

CC		= arm-none-eabi-gcc
LD		= arm-none-eabi-gcc
OBJCOPY		= arm-none-eabi-objcopy
OBJDUMP		= arm-none-eabi-objdump
SIZE		= arm-none-eabi-size

ARCHFLAGS	= -mthumb -mcpu=cortex-m0plus
CFLAGS		+= -Os \
		   -Wall -Wextra -Wredundant-decls -Wstrict-prototypes \
		   -Wundef \
		   -I$(LIBDIR)/include -I$(LIBDIR)/include/stm32/l0 \
		   -I$(LIBDIR)/include/stm32/l0/l0x2 \
		   -fno-common -fsigned-char -fstack-usage $(ARCHFLAGS)
LDFLAGS		+= -L$(LIBDIR)/lib/stm32/l0/l0x2 $(LIBS) \
		   -T $(LDSCRIPT) -nostartfiles $(LDSPECS) \
	           -Wl,--gc-sections -Wl,-Map=$(NAME).map -Wl,--cref \
		   $(ARCHFLAGS)

USARTUTIL	?= $(LIBDIR)/tools/stm32/usart-util/usart-util
ifneq ($(strip $(USARTUTIL_DEVICE)),)
USARTUTIL	+= -d $(USARTUTIL_DEVICE)
endif

.PHONY: all clean flash

all: $(OUTFILES)

%.bin: %.elf
	echo "  $@"
	$(OBJCOPY) -O binary $< $@

%.hex: %.elf
	echo "  $@"
	$(OBJCOPY) -O ihex $< $@

%.srec: %.elf
	echo "  $@"
	$(OBJCOPY) -O srec $< $@

%.list: %.elf
	echo "  $@"
	$(OBJDUMP) -d $< > $@
	$(OBJDUMP) -t $< >> $@

$(NAME).elf: $(OBJS)
	echo "  $@"
	$(LD) -o $@ $(OBJS) $(LDFLAGS)
	$(SIZE) $(NAME).elf

$(NORMALOBJS): %.o: %.c
	echo "  $<"
	$(CC) -MMD $(CFLAGS) -o $@ -c $<

$(PERIPHERAL) $(COREPERIPHERAL): %.o: %.c
	echo "  $<"
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(OUTFILES) $(NAME).elf $(OBJS) $(NORMALOBJS:.o=.d) \
	$(OBJS:.o=.su) $(NAME).map $(PERIPHERAL:.o=.c) $(COREPERIPHERAL:.o=.c) \
	functab.c

flash: $(NAME).bin
	sudo $(USARTUTIL) -D $<
#	sudo $(USARTUTIL) -D $< 2>&1 | grep --color=never wrote

$(PERIPHERAL) $(COREPERIPHERAL): monitor.h

$(PERIPHERAL): $(LIBDIR)/include/stm32/l0/l0x2/memorymap.h \
	       $(LIBDIR)/include/mmio.h

$(COREPERIPHERAL): $(LIBDIR)/include/stm32/l0/ppb.h \
		   $(LIBDIR)/include/mmio.h

$(PERIPHERAL:.o=.c): %.c: $(LIBDIR)/include/stm32/l0/l0x2/%.h
	awk -f function.awk -v dev="$(basename $@)" $< > $@

$(COREPERIPHERAL:.o=.c): %.c: $(LIBDIR)/include/stm32/l0/%.h
	awk -f function.awk -v dev="$(basename $@)" $< > $@

functab.o: functab.c monitor.h

functab.c:
	echo -n "/* This file was generated automatically " > $@
	echo -e "- don't edit it. */\n" >> $@
	echo -e "#include \"monitor.h\"\n" >> $@
	for i in $(basename $(PERIPHERAL)); do \
	  echo "int $${i}_func(int argc, char *argv[]);" >> $@; \
	done
	for i in $(basename $(COREPERIPHERAL)); do \
	  echo "int $${i}_func(int argc, char *argv[]);" >> $@; \
	done
	echo -e "\nconst struct functab ftab[] = {" >> $@
	for i in $(basename $(PERIPHERAL)); do \
	  echo -e "\t{\"$${i}_\", $${i}_func}," >> $@; \
	done
	for i in $(basename $(COREPERIPHERAL)); do \
	  echo -e "\t{\"$${i}_\", $${i}_func}," >> $@; \
	done
	echo -e "\t{0, 0}" >> $@
	echo "};" >> $@


ifneq ($(MAKECMDGOALS),clean)
-include $(NORMALOBJS:.o=.d)
endif
