# Makefile for libopenstm32l0x2

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

LIBS		= lib/stm32/l0/l0x2
TOOLS		= tools/stm32/usart-util

EXAMPLES	= examples/stm32/l0/l0x2/stm32l052k8t/miniblink \
		  examples/stm32/l0/l0x2/stm32l052k8t/fancyblink \
		  examples/stm32/l0/l0x2/stm32l052k8t/button \
		  examples/stm32/l0/l0x2/stm32l052k8t/exti \
		  examples/stm32/l0/l0x2/stm32l052k8t/tim \
		  examples/stm32/l0/l0x2/stm32l052k8t/tim_delay \
		  examples/stm32/l0/l0x2/stm32l052k8t/tim_pwm \
		  examples/stm32/l0/l0x2/stm32l052k8t/tim_encoder \
		  examples/stm32/l0/l0x2/stm32l052k8t/ultrasonic_sensor \
		  examples/stm32/l0/l0x2/stm32l052k8t/lptim \
		  examples/stm32/l0/l0x2/stm32l052k8t/lptim_delay \
		  examples/stm32/l0/l0x2/stm32l052k8t/lptim_encoder \
		  examples/stm32/l0/l0x2/stm32l052k8t/usart \
		  examples/stm32/l0/l0x2/stm32l052k8t/usart_irq \
		  examples/stm32/l0/l0x2/stm32l052k8t/usart_printf \
		  examples/stm32/l0/l0x2/stm32l052k8t/usart_getchar \
		  examples/stm32/l0/l0x2/stm32l052k8t/rtc_lcd \
		  examples/stm32/l0/l0x2/stm32l052k8t/crc \
		  examples/stm32/l0/l0x2/stm32l052k8t/1wire_temp \
		  examples/stm32/l0/l0x2/stm32l052k8t/potentiometer \
		  examples/stm32/l0/l0x2/stm32l052k8t/comp \
		  examples/stm32/l0/l0x2/stm32l052k8t/dac \
		  examples/stm32/l0/l0x2/stm32l052k8t/i2c_rom \
		  examples/stm32/l0/l0x2/stm32l052k8t/i2c_proximity \
		  examples/stm32/l0/l0x2/stm32l052k8t/spi_rom \
		  examples/stm32/l0/l0x2/stm32l052k8t/spi_barometer \
		  examples/stm32/l0/l0x2/stm32l052k8t/spi_sd \
		  examples/stm32/l0/l0x2/stm32l052k8t/spi_dac \
		  examples/stm32/l0/l0x2/stm32l052k8t/usb_custom \
		  examples/stm32/l0/l0x2/stm32l052k8t/usb_cdcacm \
		  examples/stm32/l0/l0x2/stm32l052k8t/usb_speaker \
		  examples/stm32/l0/l0x2/stm32l052k8t/usb_radio \
		  examples/stm32/l0/l0x2/stm32l052k8t/touch \
		  examples/stm32/l0/l0x2/stm32l052k8t/iwdg \
		  examples/stm32/l0/l0x2/stm32l052k8t/wwdg \
		  examples/stm32/l0/l0x2/stm32l052k8t/fw \
		  examples/stm32/l0/l0x2/stm32l052k8t/kitchen_timer \
		  examples/stm32/l0/l0x2/stm32l052k8t/ethernet_camera \
		  examples/stm32/l0/l0x2/stm32l052k8t/oled_game \
		  examples/stm32/l0/l0x2/stm32l052k8t/monitor

ifeq ($(MAKECMDGOALS), clean_example)
EXAMPLEGOAL	= clean
endif

.PHONY: all clean $(LIBS) $(TOOLS)

all clean: $(LIBS) $(TOOLS)

$(LIBS) $(TOOLS):
	@echo "-- $@ --"
	@$(MAKE) -C $@ -s $(MAKECMDGOALS)

.PHONY: example clean_example $(EXAMPLES)

example clean_example: $(EXAMPLES)

$(EXAMPLES):
	@echo "-- $@ --"
	@$(MAKE) -C $@ -s $(EXAMPLEGOAL)
