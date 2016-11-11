# Makefile for cdcmng

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

PROG	= cdcmng
OBJS	= cdcmng.o

CC	?= gcc
CFLAGS	+= -MMD -O2 -Wall
LDFLAGS	+= -lusb-1.0

.PHONY: all clean

all: $(PROG)

%.o: %.c
	echo "  $<"
	$(CC) $(CFLAGS) -c $<

$(PROG): $(OBJS)
	echo "  $@"
	$(CC) -o $(PROG) $(OBJS) $(LDFLAGS)

clean:
	rm -f $(PROG) $(OBJS) $(OBJS:.o=.d)

ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
