/*
 * descriptor.c
 *
 * Copyright 2016 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This file is part of usb_radio.
 *
 * usb_radio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * usb_radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with usb_radio.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>

#include <usb/standard.h>
#include <usb/langids.h>
#include <usb/audio10.h>

#include "usb_radio.h"

/* SAMPLING_CHANNELS = 2 ! */

/* struct usb_audio_ac_if_header_descriptor_1 */
USB_AUDIO_AC_IF_HEADER_DESCRIPTOR(1);
/* struct usb_audio_feature_unit_descriptor_2_2 */
USB_AUDIO_FEATURE_UNIT_DESCRIPTOR(2, 2);
/* struct usb_audio_type_i_format_type_descriptor_1 */
USB_AUDIO_TYPE_I_FORMAT_TYPE_DESCRIPTOR(1);
/* Class-Specific AC Interface Descriptor */
struct ac_interface_descriptor {
	struct usb_audio_ac_if_header_descriptor_1 header;
	struct usb_audio_input_terminal_descriptor it;
	struct usb_audio_feature_unit_descriptor_2_2 fu;
	struct usb_audio_output_terminal_descriptor ot;
} __attribute__ ((packed));
/* Configuration */
struct config_descriptor {
	struct usb_config_descriptor config;
	struct usb_interface_descriptor ac_if;
	struct ac_interface_descriptor cs_ac_if;
	struct usb_interface_descriptor as_if0;
	struct usb_interface_descriptor as_if1;
	struct usb_audio_as_interface_descriptor cs_as_if;
	struct usb_audio_type_i_format_type_descriptor_1 format;
	struct usb_audio_endpoint_descriptor as_endp;
	struct usb_audio_as_endpoint_descriptor cs_as_endp;
} __attribute__ ((packed));
/* struct usb_string_descriptor_1 */
USB_STRING_DESCRIPTOR(1);
/* struct usb_string_descriptor_17 */
USB_STRING_DESCRIPTOR(17);
/* struct usb_string_descriptor_22 */
USB_STRING_DESCRIPTOR(22);

/* Device Descriptor */
alignas(2) static const struct usb_device_descriptor dev_desc = {
	.bLength = sizeof(struct usb_device_descriptor),
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = MAXPACKETSIZE0,
	.idVendor = 0x2975,
	.idProduct = 0x0005,
	.bcdDevice = 0x0010,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 0,
	.bNumConfigurations = 1
};

alignas(2) static const struct config_descriptor config_desc = {
	/* Configuration Descriptor */
	.config = {
		.bLength = sizeof(struct usb_config_descriptor),
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = sizeof(struct config_descriptor),
		.bNumInterfaces = MAXINTERFACE,
		.bConfigurationValue = CONFIGURATION_VALUE,
		.iConfiguration = 0,
		.bmAttributes = USB_CONFIG_ATTR_D7 |
		USB_CONFIG_ATTR_SELF_POWERED,
		.bMaxPower = 0
	},
	/* Standard AC Interface Descriptor */
	.ac_if = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = INTERFACE_AC,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = USB_CLASS_AUDIO,
		.bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOCONTROL,
		.bInterfaceProtocol = USB_AUDIO_PROTO_UNDEFINED,
		.iInterface = 0
	},
	/* Class-Specific AC Interface Descriptor */
	.cs_ac_if = {
		/* Class-Specific AC Interfcae Header Descriptor */
		.header = {
			.bLength =
			sizeof(struct usb_audio_ac_if_header_descriptor_1),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_AUDIO_HEADER,
			.bcdADC = 0x0100,
			.wTotalLength =
			sizeof(struct ac_interface_descriptor),
			.bInCollection = 1,
			.baInterfaceNr = {INTERFACE_AS}
		},
		/* Input Terminal Descriptor */
		.it = {
			.bLength =
			sizeof(struct usb_audio_input_terminal_descriptor),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_AUDIO_INPUT_TERMINAL,
			.bTerminalID = INPUT_TERMINAL_ID,
			.wTerminalType = USB_AUDIO_RADIO_RECEIVER,
			.bAssocTerminal = 0,
			.bNrChannels = SAMPLING_CHANNELS,
			.wChannelConfig = USB_AUDIO_LEFT_FRONT |
			USB_AUDIO_RIGHT_FRONT,
			.iChannelNames = 0,
			.iTerminal = 0,
		},
		/* Feature Unit Descriptor */
		.fu = {
			.bLength =
			sizeof(struct usb_audio_feature_unit_descriptor_2_2),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_AUDIO_FEATURE_UNIT,
			.bUnitID = FEATURE_UNIT_ID,
			.bSourceID = INPUT_TERMINAL_ID,
			.bControlSize = 2,
			.bmaControls = {{0, 0},
					{0, 0},
					{0, 0}},
			.iFeature = 0
		},
		/* Output Terminal Descriptor */
		.ot = {
			.bLength =
			sizeof(struct usb_audio_output_terminal_descriptor),
			.bDescriptorType = USB_DT_CS_INTERFACE,
			.bDescriptorSubtype = USB_AUDIO_OUTPUT_TERMINAL,
			.bTerminalID = OUTPUT_TERMINAL_ID,
			.wTerminalType = USB_AUDIO_USB_STREAMING,
			.bAssocTerminal = 0,
			.bSourceID = FEATURE_UNIT_ID,
			.iTerminal = 0
		}
	},
	/* Standard AS Interface Descriptor */
	.as_if0 = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = INTERFACE_AS,
		.bAlternateSetting = 0,
		.bNumEndpoints = 0,
		.bInterfaceClass = USB_CLASS_AUDIO,
		.bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
		.bInterfaceProtocol = USB_AUDIO_PROTO_UNDEFINED,
		.iInterface = 0
	},
	.as_if1 = {
		.bLength = sizeof(struct usb_interface_descriptor),
		.bDescriptorType = USB_DT_INTERFACE,
		.bInterfaceNumber = INTERFACE_AS,
		.bAlternateSetting = 1,
		.bNumEndpoints = 1,
		.bInterfaceClass = USB_CLASS_AUDIO,
		.bInterfaceSubClass = USB_AUDIO_SUBCLASS_AUDIOSTREAMING,
		.bInterfaceProtocol = USB_AUDIO_PROTO_UNDEFINED,
		.iInterface = 0
	},
	/* Class-Specific AS Interface Descriptor */
	.cs_as_if = {
		.bLength = sizeof(struct usb_audio_as_interface_descriptor),
		.bDescriptorType = USB_DT_CS_INTERFACE,
		.bDescriptorSubtype = USB_AUDIO_AS_GENERAL,
		.bTerminalLink = OUTPUT_TERMINAL_ID,
		.bDelay = 0,
		.wFormatTag = USB_AUDIO_PCM
	},
	/* Class-Specific AS Format Type Descriptor */
	.format = {
		.bLength =
		sizeof(struct usb_audio_type_i_format_type_descriptor_1),
		.bDescriptorType = USB_DT_CS_INTERFACE,
		.bDescriptorSubtype = USB_AUDIO_FORMAT_TYPE,
		.bFormatType = USB_AUDIO_FORMAT_TYPE_I,
		.bNrChannels = SAMPLING_CHANNELS,
		.bSubFrameSize = SAMPLING_BYTES,
		.bBitResolution = SAMPLING_BITS,
		.bSamFreqType = 1,
		.tSamFreq = {{(SAMPLING_FREQ & 0xff),
			      ((SAMPLING_FREQ >> 8) & 0xff),
			      ((SAMPLING_FREQ >> 16) & 0xff)}}
	},
	/* Standard AS Isochronous Audio Data Endpoint Descriptor */
	.as_endp = {
		.bLength = sizeof(struct usb_audio_endpoint_descriptor),
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = AS_ENUM,
		.bmAttributes = (USB_ENDPOINT_SYNC_SYNC |
				 USB_ENDPOINT_TRANS_ISOCHRONOUS),
		.wMaxPacketSize = AS_SIZE_MAX,
		.bInterval = AS_INTERVAL,
		.bRefresh = 0,
		.bSynchAddress = 0
	},
	/* Class-Specific AS Isochronous Audio Data Endpoint Descriptor */
	.cs_as_endp = {
		.bLength = sizeof(struct usb_audio_as_endpoint_descriptor),
		.bDescriptorType = USB_DT_CS_ENDPOINT,
		.bDescriptorSubtype = USB_AUDIO_EP_GENERAL,
		.bmAttributes = 0,
		.bLockDelayUnits = 0,
		.wLockDelay = 0
	}
};

/* Strings */
alignas(2) static const struct usb_string_descriptor_1 string_desc0 = {
	.bLength = 4,
	.bDescriptorType = USB_DT_STRING,
	.wData = {LANGID_ENGLISH_US}
};
alignas(2) static const struct usb_string_descriptor_17 string_desc1 = {
	.bLength = 36,
	.bDescriptorType = USB_DT_STRING,
	.wData = u"MPC Research Ltd."
};
alignas(2) static const struct usb_string_descriptor_22 string_desc2 = {
	.bLength = 46,
	.bDescriptorType = USB_DT_STRING,
	.wData = u"STM32L052K8T USB Radio"
};

int get_descriptor(int type, int index, char **data)
{
	int len;

	switch (type) {
	case USB_DT_DEVICE:
		len = sizeof(dev_desc);
		*data = (char *)&dev_desc;
		break;
	case USB_DT_CONFIGURATION:
		len = sizeof(config_desc);
		*data = (char *)&config_desc;
		break;
	case USB_DT_STRING:
		switch (index) {
		case 0:
			len = sizeof(string_desc0);
			*data = (char *)&string_desc0;
			break;
		case 1:
			len = sizeof(string_desc1);
			*data = (char *)&string_desc1;
			break;
		case 2:
			len = sizeof(string_desc2);
			*data = (char *)&string_desc2;
			break;
		default:
			len = 0;
			break;
		}
		break;
	default:
		len = 0;
		break;
	}

	return len;
}
