/*
 * Universal Serial Bus
 * standard.h
 *
 * Copyright 2014 Toshiaki Yoshida <yoshida@mpc.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Universal Serial Bus Specification Revision 2.0
 * April 27, 2000
 *
 * Chapter 9
 * USB Device Framework
 */

/* 9.1.1 Visible Device States */
/* Figure 9-1. Device State Diagram */
enum usb_device_state {
	USB_STATE_ATTACHED,
	USB_STATE_POWERED,
	USB_STATE_DEFAULT,
	USB_STATE_ADDRESS,
	USB_STATE_CONFIGURED,
	USB_STATE_SUSPENDED
};

/* 9.3 USB Device Requests */
/* Table 9-2. Format of Setup Data */
struct usb_setup_data {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__ ((packed));

/* bmRequestType */
/* D7: Data transfer direction */
#define USB_DIR_OUT				(0 << 7) /* Host-to-device */
#define USB_DIR_IN				(1 << 7) /* Device-to-host */
/* D6...5: Type */
#define USB_TYPE_MASK				(3 << 5)
#define USB_TYPE_STANDARD			(0 << 5)
#define USB_TYPE_CLASS				(1 << 5)
#define USB_TYPE_VENDOR				(2 << 5)
/* D4...0: Recipient */
#define USB_RECIP_MASK				(31 << 0)
#define USB_RECIP_DEVICE			(0 << 0)
#define USB_RECIP_INTERFACE			(1 << 0)
#define USB_RECIP_ENDPOINT			(2 << 0)
#define USB_RECIP_OTHER				(3 << 0)

/* 9.4 Standard Device Requests */
/* bRequest */
/* Table 9-4. Standard Request Codes */
#define USB_REQ_GET_STATUS			0
#define USB_REQ_CLEAR_FEATURE			1
#define USB_REQ_SET_FEATURE			3
#define USB_REQ_SET_ADDRESS			5
#define USB_REQ_GET_DESCRIPTOR			6
#define USB_REQ_SET_DESCRIPTOR			7
#define USB_REQ_GET_CONFIGURATION		8
#define USB_REQ_SET_CONFIGURATION		9
#define USB_REQ_GET_INTERFACE			10
#define USB_REQ_SET_INTERFACE			11
#define USB_REQ_SYNCH_FRAME			12

/* wValue */
/* Table 9-5. Discriptor types */
#define USB_DT_DEVICE				1
#define USB_DT_CONFIGURATION			2
#define USB_DT_STRING				3
#define USB_DT_INTERFACE			4
#define USB_DT_ENDPOINT				5
#define USB_DT_DEVICE_QUALIFIER			6
#define USB_DT_OTHER_SPEED_CONFIGURATION	7
#define USB_DT_INTERFACE_POWER			8
/* USB ECN: Interface Association Descriptor */
#define USB_DT_OTG				9
#define USB_DT_DEBUG				10
#define USB_DT_INTERFACE_ASSOCIATION		11

/* Table 9-6. Standard Feature Selectors */
#define USB_FEAT_ENDPOINT_HALT			0
#define USB_FEAT_DEVICE_REMOTE_WAKEUP		1
#define USB_FEAT_TEST_MODE			2

/* wIndex */
/* Table 9-7. Test Mode Selectors */
#define USB_TEST_J				0x01
#define USB_TEST_K				0x02
#define USB_TEST_SE0_NAK			0x03
#define USB_TEST_PACKET				0x04
#define USB_TEST_FORCE_ENABLE			0x05

/* Data */
/* Figure 9-4. Information Returned by a GetStatus() Request to a Device */
#define USB_DEV_STATUS_SELF_POWERED		(1 << 0)
#define USB_DEV_STATUS_REMOTE_WAKEUP		(1 << 1)

/* Figure 9-6. Information Returned by a GetStatus() Request to an Endpoint */
#define USB_DEV_STATUS_HALT			(1 << 0)

/* 9.5 Descriptors */
/* 9.6 Standard USB Descriptor Definitions */
/* 9.6.1 Device */
/* Table 9-8. Standard Device Descriptor */
struct usb_device_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __attribute__ ((packed));

/* bDeviceClass */
#define USB_CLASS_VENDOR			0xff

/* bDeviceProtocol */
#define USB_PROTO_VENDOR			0xff

/* bMaxPacketSize0 */
/* only 8, 16, 32, or 64 are valid */
#define USB_MAX_MAXPACKETSIZE0			64

/* 9.6.2 Device_Qualifier */
/* Table 9-9. Device_Qualifier Descriptor */
struct usb_device_qualifier_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint8_t bNumConfigurations;
	uint8_t bReserved;
} __attribute__ ((packed));

/* 9.6.3 Configuration */
/* Table 9-10. Standard Configuration Descriptor */
struct usb_config_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__ ((packed));

/* bmAttributes */
#define USB_CONFIG_ATTR_D7			(1 << 7)
#define USB_CONFIG_ATTR_SELF_POWERED		(1 << 6)
#define USB_CONFIG_ATTR_REMOTE_WAKEUP		(1 << 5)

/* 9.6.4 Othe_Speed_Configuration */
/*
 * The structure of the other_speed_configuration is identical to
 * a configuration decriptor.
 */

/* 9.6.5 Interface */
/* Table 9-12. Standard Interface Descriptor */
struct usb_interface_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__ ((packed));

/* 9.6.6 Endpoint */
/* Table 9-13. Standard Endpoint Descriptor */
struct usb_endpoint_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__ ((packed));

/* bmAttributes */
/* Bits 1..0: Transfer Type */
#define USB_ENDPOINT_TRANS_MASK			(3 << 0)
#define USB_ENDPOINT_TRANS_CONTROL		(0 << 0)
#define USB_ENDPOINT_TRANS_ISOCHRONOUS		(1 << 0)
#define USB_ENDPOINT_TRANS_BULK			(2 << 0)
#define USB_ENDPOINT_TRANS_INTERRUPT		(3 << 0)
/* Bits 3..2: Synchronization Type */
#define USB_ENDPOINT_SYNC_MASK			(3 << 2)
#define USB_ENDPOINT_SYNC_NOSYNC		(0 << 2)
#define USB_ENDPOINT_SYNC_ASYNC			(1 << 2)
#define USB_ENDPOINT_SYNC_ADAPTIVE		(2 << 2)
#define USB_ENDPOINT_SYNC_SYNC			(3 << 2)
/* Bits 5..4: Usage Type */
#define USB_ENDPOINT_USAGE_MASK			(3 << 4)
#define USB_ENDPOINT_USAGE_DATA			(0 << 4)
#define USB_ENDPOINT_USAGE_FEEDBACK		(1 << 4)
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK	(3 << 4)

/* 9.6.7 String */
/*
 * Table 9-15. String Descriptor Zero, Specifying Languages Supported by
 * the Device
 */
/* Table 9-16. UNICODE String Descriptor */
#define USB_STRING_DESCRIPTOR(n)		\
	struct usb_string_descriptor_##n {	\
		uint8_t bLength;		\
		uint8_t bDescriptorType;	\
		uint16_t wData[n];		\
	} __attribute__ ((packed))

/* USB ECN: Interface Association Descriptor */
/* Table 9-Z. Standard Interface Association Descriptor */
struct usb_iface_assoc_descriptor {
	uint8_t bLength;
	uint8_t bDeecriptorType;
	uint8_t bFirstInterface;
	uint8_t bInterfaceCount;
	uint8_t bFunctionClass;
	uint8_t bFunctionSubClass;
	uint8_t bFunctionProtocol;
	uint8_t iFunction;
} __attribute__ ((packed));
