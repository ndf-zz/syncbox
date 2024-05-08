// SPDX-License-Identifier: MIT

/*
 * USB Device interface
 *
 * Based on libusb_stm32 by Dmitry Filimonchuk
 *
 * References:
 *
 *  - Universal Serial Bus Device Class Definition for MIDI Devices 1.0
 *  - https://github.com/dmitrystu/libusb_stm32
 */
#ifndef USB_H
#define USB_H
#include "stm32f303xe.h"

// USB Constants
#define USB_EP_OUT		0x1
#define USB_EP_IN		0x81
#define USB_VENDOR		0x1209
#define USB_PRODUCT		0x5362
#define USB_DEVRELEASE		0x0101
#define USB_DEVICE		0x1
#define USB_CONFIGURATION	0x2
#define USB_INTERFACE		0x4
#define USB_HEADER		0x1
#define USB_ENDPOINT		0x5
#define USB_AUDIO		0x1
#define USB_MIDISTREAMING	0x3
#define USB_AUDIO_CONTROL	0x1
#define USB_CS_INTERFACE	0x24
#define USB_CS_ENDPOINT		0x25
#define USB_ENDPOINT_SIZE	0x40
#define USB_ENDPOINT_BULK	0x2
#define USB_MAXSTRLEN		14U
#define USB_NRDESCR		8U
#define USB_CFG_ATTR_RESERVED	0x80
#define USB_CFG_ATTR_SELFPWR	0x40
#define USB_CFG_100MA		0x32
#define USB_MS_GENERAL		0x1
#define USB_MIDI_IN_JACK	0x2
#define USB_MIDI_OUT_JACK	0x3
#define USB_ELEMENT		0x4
#define USB_EMBEDDED		0x1
#define USB_EXTERNAL		0x2
#define USB_ELEMENT_CLOCK	0x2
#define USB_ELEMENT_CUSTOM	0x1

// Information string indices
enum usb_descr {
	USB_DESCR_LANG,
	USB_DESCR_MANUF,
	USB_DESCR_PROD,
	USB_DESCR_SERIAL,
	USB_DESCR_SYNC,
	USB_DESCR_MIDI,
	USB_DESCR_SB,
	USB_DESCR_IF,
};

// B.1 Device Descriptor
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
} __attribute__((packed));
#define USB_DEVLEN (sizeof(struct usb_device_descriptor))
#define USB_DEVPAD (ALIGN16(USB_DEVLEN) - USB_DEVLEN)

// MIDI Streaming device configuration structure
struct usb_device_configuration {
	// B.2 Configuration Descriptor
	uint8_t cfg_bLength;
	uint8_t cfg_bDescriptorType;
	uint16_t cfg_wTotalLength;
	uint8_t cfg_bNumInterfaces;
	uint8_t cfg_bConfigurationValue;
	uint8_t cfg_iConfiguration;
	uint8_t cfg_bmAttributes;
	uint8_t cfg_MaxPower;

	// B.3.1 Standard AC Interface Descriptor
	uint8_t aci_bLength;
	uint8_t aci_bDescriptorType;
	uint8_t aci_bInterfaceNumber;
	uint8_t aci_bAlternateSetting;
	uint8_t aci_bNumEndpoints;
	uint8_t aci_bInterfaceClass;
	uint8_t aci_bInterfaceSubclass;
	uint8_t aci_bInterfaceProtocol;
	uint8_t aci_iInterface;

	// B.3.2 Class-specific AC Interface Descriptor
	uint8_t csci_bLength;
	uint8_t csci_bDescriptorType;
	uint8_t csci_bDescriptorSubType;
	uint16_t csci_bcdADC;
	uint16_t csci_wTotalLength;
	uint8_t csci_bInCollection;
	uint8_t csci_baInterfaceNr1;

	// 6.1.1 Standard MS Interface Descriptor
	uint8_t smsi_bLength;
	uint8_t smsi_bDescriptorType;
	uint8_t smsi_bInterfaceNumber;
	uint8_t smsi_bAlternateSetting;
	uint8_t smsi_bNumEndpoints;
	uint8_t smsi_bInterfaceClass;
	uint8_t smsi_bInterfaceSubclass;
	uint8_t smsi_bInterfaceProtocol;
	uint8_t smsi_iInterface;

	// 6.1.2.1 Class-Specific MS Interface Header Descriptor
	uint8_t cmsi_bLength;
	uint8_t cmsi_bDescriptorType;
	uint8_t cmsi_bDescriptorSubtype;
	uint16_t cmsi_BcdMSC;
	uint16_t cmsi_wTotalLength;

	// 6.1.2.2 MIDI IN Jack Descriptor : Host -> Syncbox (Embedded)
	uint8_t j1_bLength;
	uint8_t j1_bDescriptorType;
	uint8_t j1_bDescriptorSubtype;
	uint8_t j1_bJackType;
	uint8_t j1_bJackID;
	uint8_t j1_iJack;

	// 6.1.2.3 MIDI OUT Jack Descriptor : Syncbox -> Host (Embedded)
	uint8_t j2_bLength;
	uint8_t j2_bDescriptorType;
	uint8_t j2_bDescriptorSubtype;
	uint8_t j2_bJackType;
	uint8_t j2_bJackID;
	uint8_t j2_bNrInputPins;
	uint8_t j2_baSourceID1;
	uint8_t j2_BaSourcePin1;
	uint8_t j2_iJack;

	// 6.1.2.3 MIDI OUT Jack Descriptor : MIDI -> Host (Embedded)
	uint8_t j3_bLength;
	uint8_t j3_bDescriptorType;
	uint8_t j3_bDescriptorSubtype;
	uint8_t j3_bJackType;
	uint8_t j3_bJackID;
	uint8_t j3_bNrInputPins;
	uint8_t j3_baSourceID1;
	uint8_t j3_BaSourcePin1;
	uint8_t j3_iJack;

	// 6.1.2.2 MIDI IN Jack Descriptor : MIDI -> Syncbox (External)
	uint8_t j4_bLength;
	uint8_t j4_bDescriptorType;
	uint8_t j4_bDescriptorSubtype;
	uint8_t j4_bJackType;
	uint8_t j4_bJackID;
	uint8_t j4_iJack;

	// 6.1.2.4 Element Descriptor : Syncbox
	uint8_t e5_bLength;
	uint8_t e5_bDescriptorType;
	uint8_t e5_bDescriptorSubtype;
	uint8_t e5_bElementID;
	uint8_t e5_bNrInputPins;
	uint8_t e5_baSourceID1;
	uint8_t e5_BaSourcePin1;
	uint8_t e5_bNrOutputPins;
	uint8_t e5_bInTerminalLink;
	uint8_t e5_bOutTerminalLink;
	uint8_t e5_bElCapsSize;
	uint16_t e5_bmElementCaps;
	uint8_t e5_iElement;

	// 6.2.1 Standard MS Bulk Data Endpoint Descriptor : Output
	uint8_t epout_bLength;
	uint8_t epout_bDescriptorType;
	uint8_t epout_bEndpointAddress;
	uint8_t epout_bmAttributes;
	uint16_t epout_wMaxPacketSize;
	uint8_t epout_bInterval;
	uint8_t epout_bRefresh;
	uint8_t epout_bSynchAddress;

	// 6.2.2 Class-Specific MS Bulk Data Endpoint Descriptor : Output
	uint8_t msout_bLength;
	uint8_t msout_bDescriptorType;
	uint8_t msout_bDescriptorSubType;
	uint8_t msout_bNumEmbMIDIJack;
	uint8_t msout_baAssocJackID1;

	// 6.2.1 Standard MS Bulk Data Endpoint Descriptor : Input
	uint8_t epin_bLength;
	uint8_t epin_bDescriptorType;
	uint8_t epin_bEndpointAddress;
	uint8_t epin_bmAttributes;
	uint16_t epin_wMaxPacketSize;
	uint8_t epin_bInterval;
	uint8_t epin_bRefresh;
	uint8_t epin_bSynchAddress;

	// 6.2.2 Class-Specific MS Bulk Data Endpoint Descriptor : Input
	uint8_t msin_bLength;
	uint8_t msin_bDescriptorType;
	uint8_t msin_bDescriptorSubType;
	uint8_t msin_bNumEmbMIDIJack;
	uint8_t msin_baAssocJackID1;
	uint8_t msin_baAssocJackID2;
} __attribute__((packed));
#define USB_CFGLEN (sizeof(struct usb_device_configuration))
#define USB_CFGPAD (ALIGN16(USB_CFGLEN) - USB_CFGLEN)

// Total length of class-specific descriptors
#define USB_CSLEN 80U

// ROM container struct for USB UTF-16LE strings
struct usb_string {
	uint16_t wString[USB_MAXSTRLEN];
	const uint32_t length;
} __attribute__((packed));

// ROM USB structure
struct usb_config {
	struct usb_string string[USB_NRDESCR];
	struct usb_device_descriptor device;
	uint8_t devpad[USB_DEVPAD];
	struct usb_device_configuration configuration;
	uint8_t cfgpad[USB_CFGPAD];
};

#endif // USB_H
