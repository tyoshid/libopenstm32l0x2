# Makefile for ethernet_camera

# Copyright 2015 Toshiaki Yoshida <yoshida@mpc.net>

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

NAME		= ethernet_camera
OBJS		= favicon.o camera.o rom.o w5200.o http.o ethernet_camera.o
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
CFLAGS		+= -MMD -Os \
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

%.o: %.c
	echo "  $<"
	$(CC) $(CFLAGS) -o $@ -c $<

favicon.c: favicon.ico
	echo "  $<"
	echo -n "/* This file was generated automatically " > $@
	echo -e "- don't edit it. */\n" >> $@
	echo "const char favicon_ico_response[] = {" >> $@
	echo -e "HTTP/1.1 200 OK\r" | \
	hexdump -v -e '"\t" 8/1 "0x%02x, " "  /* "' -e '8/1 "%_p" " */\n"' | \
	sed 's/0x  ,/     /g' >> $@
	echo -e "Content-Type: image/vnd.microsoft.icon\r" | \
	hexdump -v -e '"\t" 8/1 "0x%02x, " "  /* "' -e '8/1 "%_p" " */\n"' | \
	sed 's/0x  ,/     /g' >> $@
	fsize=`cat $< | wc -c`; \
	echo -e "Content-Length: $${fsize}\r\n\r" | \
	hexdump -v -e '"\t" 8/1 "0x%02x, " "  /* "' -e '8/1 "%_p" " */\n"' | \
	sed 's/0x  ,/     /g' >> $@
	hexdump -v -e '"\t" 8/1 "0x%02x, " "\n"' $< | \
	sed 's/0x  ,/     /g' >> $@
	echo -e "};\n" >> $@
	echo -n "const int favicon_ico_response_size = " >> $@
	echo -e	"sizeof(favicon_ico_response);\n" >> $@

clean:
	rm -f $(OUTFILES) $(NAME).elf $(OBJS) $(OBJS:.o=.d) $(OBJS:.o=.su) \
	$(NAME).map favicon.c

flash: $(NAME).bin
	sudo $(USARTUTIL) -D $<
#	sudo $(USARTUTIL) -D $< 2>&1 | grep --color=never wrote

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
