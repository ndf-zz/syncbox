// SPDX-License-Identifier: MIT

/*
 * ROM Options - included in binary image
 * 
 * available at runtime address 0x08004800 via OPTION pointer eg:
 *
 * #include "flash.h"
 *
 * something = OPTION->sysid;
 *
 */
#include "settings.h"

static __attribute__((used))
struct option_struct rom = {
	.sysid = SYSEX_ID,
	.version = SYSTEMVERSION,
	.preset = {
		   // Preset 0: Omni on, Roland sync, clock triggers
		   {
		    .delay = 125000U,	// ~120bpm
		    .inertia = 40U,	// ~5.0ms inertia
		    .channel = 0,	// MIDI channel 1
		    .mode = SETTING_OMNION,	// Omni on
		    .master = SETTING_AUTO,	// Clock Master: Auto
		    .fusb = SETTING_DEFFILT,	// Note/Control/RT
		    .fmidi = SETTING_DEFFILT,	// Note/Control/RT
		    .triglen = 20U,	// 20ms Trigger Length
		    .output = {
			       // CK Ouput
			       {
				.flags = SETTING_CLOCK,
				.divisor = SETTING_24PPQ,
				.offset = 0,
				.note = 0,
				},
			       // RN Output
			       {
				.flags = SETTING_RUNSTOP,
				.divisor = 0,
				.offset = 0,
				.note = 0,
				},
			       // FL Output
			       {
				.flags = SETTING_CONTINUE | SETTING_TRIG,
				.divisor = 0,
				.offset = 0,
				.note = 0,
				},
			       // G1 Output
			       {
				.flags =
				SETTING_CLOCK | SETTING_TRIG | SETTING_RUNMASK,
				.divisor = SETTING_16TH,
				.offset = 0,
				.note = 0,
				},
			       // G2 Output
			       {
				.flags =
				SETTING_CLOCK | SETTING_TRIG | SETTING_RUNMASK,
				.divisor = SETTING_BEAT,
				.offset = 0,
				.note = 0,
				},
			       // G3 Output
			       {
				.flags =
				SETTING_CLOCK | SETTING_TRIG | SETTING_RUNMASK,
				.divisor = SETTING_BAR,
				.offset = 0,
				.note = 0,
				},
			       },
		    },
		   // Preset 1: Omni off, Roland sync, clock triggers
		   {
		    .delay = 125000U,	// ~120bpm
		    .inertia = 40U,	// ~5.0ms inertia
		    .channel = 0,	// MIDI channel 1
		    .mode = SETTING_OMNIOFF,	// Omni off
		    .master = SETTING_AUTO,	// Clock Master: Auto
		    .fusb = SETTING_DEFFILT,	// Note/Control/RT
		    .fmidi = SETTING_DEFFILT,	// Note/Control/RT
		    .triglen = 20U,	// 20ms Trigger Length
		    .output = {
			       // CK Ouput
			       {
				.flags = SETTING_CLOCK,
				.divisor = SETTING_24PPQ,
				.offset = 0,
				.note = 0,
				},
			       // RN Output
			       {
				.flags = SETTING_RUNSTOP,
				.divisor = 0,
				.offset = 0,
				.note = 0,
				},
			       // FL Output
			       {
				.flags = SETTING_CONTINUE | SETTING_TRIG,
				.divisor = 0,
				.offset = 0,
				.note = 0,
				},
			       // G1 Output
			       {
				.flags =
				SETTING_CLOCK | SETTING_TRIG | SETTING_RUNMASK,
				.divisor = SETTING_16TH,
				.offset = 0,
				.note = 0,
				},
			       // G2 Output
			       {
				.flags =
				SETTING_CLOCK | SETTING_TRIG | SETTING_RUNMASK,
				.divisor = SETTING_BEAT,
				.offset = 0,
				.note = 0,
				},
			       // G3 Output
			       {
				.flags =
				SETTING_CLOCK | SETTING_TRIG | SETTING_RUNMASK,
				.divisor = SETTING_BAR,
				.offset = 0,
				.note = 0,
				},
			       },
		    },
		    },
	.usb = {

		// Device Descriptor
		.device = {
			   .bLength = USB_DEVLEN,
			   .bDescriptorType = USB_DEVICE,
			   .bcdUSB = 0x0200,
			   .bMaxPacketSize0 = USB_ENDPOINT_SIZE,
			   .idVendor = USB_VENDOR,
			   .idProduct = USB_PRODUCT,
			   .bcdDevice = USB_DEVRELEASE,
			   .iManufacturer = USB_DESCR_MANUF,
			   .iProduct = USB_DESCR_PROD,
			   .iSerialNumber = USB_DESCR_SERIAL,
			   .bNumConfigurations = 1U,
			   },

		// Configuration Descriptor
		.configuration = {
				  .cfg_bLength = 9U,
				  .cfg_bDescriptorType = USB_CONFIGURATION,
				  .cfg_wTotalLength = USB_CFGLEN,
				  .cfg_bNumInterfaces = 2U,
				  .cfg_bConfigurationValue = 1U,
				  .cfg_bmAttributes =
				  USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPWR,
				  .cfg_MaxPower = USB_CFG_100MA,

				  .aci_bLength = 9U,
				  .aci_bDescriptorType = USB_INTERFACE,
				  .aci_bInterfaceClass = USB_AUDIO,
				  .aci_bInterfaceSubclass = USB_AUDIO_CONTROL,

				  .csci_bLength = 9U,
				  .csci_bDescriptorType = USB_CS_INTERFACE,
				  .csci_bDescriptorSubType = USB_HEADER,
				  .csci_bcdADC = 0x0100,
				  .csci_wTotalLength = 9U,	// Header only
				  .csci_bInCollection = 1U,
				  .csci_baInterfaceNr1 = 1U,

				  .smsi_bLength = 9U,
				  .smsi_bDescriptorType = USB_INTERFACE,
				  .smsi_bInterfaceNumber = 1U,
				  .smsi_bNumEndpoints = 2U,
				  .smsi_bInterfaceClass = USB_AUDIO,
				  .smsi_bInterfaceSubclass = USB_MIDISTREAMING,
				  .smsi_iInterface = USB_DESCR_IF,

				  .cmsi_bLength = 7U,
				  .cmsi_bDescriptorType = USB_CS_INTERFACE,
				  .cmsi_bDescriptorSubtype = USB_HEADER,
				  .cmsi_BcdMSC = 0x0100,
				  .cmsi_wTotalLength = USB_CSLEN,

				  .j1_bLength = 6U,
				  .j1_bDescriptorType = USB_CS_INTERFACE,
				  .j1_bDescriptorSubtype = USB_MIDI_IN_JACK,
				  .j1_bJackType = USB_EMBEDDED,
				  .j1_bJackID = 1U,
				  .j1_iJack = USB_DESCR_SYNC,

				  .j2_bLength = 9U,
				  .j2_bDescriptorType = USB_CS_INTERFACE,
				  .j2_bDescriptorSubtype = USB_MIDI_OUT_JACK,
				  .j2_bJackType = USB_EMBEDDED,
				  .j2_bJackID = 2U,
				  .j2_bNrInputPins = 1U,
				  .j2_baSourceID1 = 5U,
				  .j2_BaSourcePin1 = 1U,
				  .j2_iJack = USB_DESCR_SYNC,

				  .j3_bLength = 9U,
				  .j3_bDescriptorType = USB_CS_INTERFACE,
				  .j3_bDescriptorSubtype = USB_MIDI_OUT_JACK,
				  .j3_bJackType = USB_EMBEDDED,
				  .j3_bJackID = 3U,
				  .j3_bNrInputPins = 1U,
				  .j3_baSourceID1 = 4U,
				  .j3_BaSourcePin1 = 1U,
				  .j3_iJack = USB_DESCR_MIDI,

				  .j4_bLength = 6U,
				  .j4_bDescriptorType = USB_CS_INTERFACE,
				  .j4_bDescriptorSubtype = USB_MIDI_IN_JACK,
				  .j4_bJackType = USB_EXTERNAL,
				  .j4_bJackID = 4U,
				  .j4_iJack = USB_DESCR_MIDI,

				  .e5_bLength = 14U,
				  .e5_bDescriptorType = USB_CS_INTERFACE,
				  .e5_bDescriptorSubtype = USB_ELEMENT,
				  .e5_bElementID = 5U,
				  .e5_bNrInputPins = 1U,
				  .e5_baSourceID1 = 1U,
				  .e5_BaSourcePin1 = 1U,
				  .e5_bNrOutputPins = 1U,
				  .e5_bInTerminalLink = 1U,
				  .e5_bOutTerminalLink = 1U,
				  .e5_bElCapsSize = 2U,
				  .e5_bmElementCaps =
				  USB_ELEMENT_CLOCK | USB_ELEMENT_CUSTOM,
				  .e5_iElement = 6U,

				  .epout_bLength = 9U,
				  .epout_bDescriptorType = USB_ENDPOINT,
				  .epout_bEndpointAddress = USB_EP_OUT,
				  .epout_bmAttributes = USB_ENDPOINT_BULK,
				  .epout_wMaxPacketSize = USB_ENDPOINT_SIZE,

				  .msout_bLength = 5U,
				  .msout_bDescriptorType = USB_CS_ENDPOINT,
				  .msout_bDescriptorSubType = USB_MS_GENERAL,
				  .msout_bNumEmbMIDIJack = 1U,
				  .msout_baAssocJackID1 = 1U,

				  .epin_bLength = 9U,
				  .epin_bDescriptorType = USB_ENDPOINT,
				  .epin_bEndpointAddress = USB_EP_IN,
				  .epin_bmAttributes = USB_ENDPOINT_BULK,
				  .epin_wMaxPacketSize = USB_ENDPOINT_SIZE,

				  .msin_bLength = 6U,
				  .msin_bDescriptorType = USB_CS_ENDPOINT,
				  .msin_bDescriptorSubType = USB_MS_GENERAL,
				  .msin_bNumEmbMIDIJack = 2U,
				  .msin_baAssocJackID1 = 2U,
				  .msin_baAssocJackID2 = 3U,
				  },

		// Description Strings
		.string = {
			   // Language
			   {
			    .wString = { 0x0409},
			    .length = 1U,
			    },
			   // Manufacturer
			   {
			    .wString = { 0x2b21, 0x20, 0x36, 0x2d, 0x76},
			    .length = 5U,
			    },
			   // Product
			   {
			    .wString =
			    { 0x53, 0x79, 0x6e, 0x63, 0x62, 0x6f, 0x78},
			    .length = 7U,
			    },
			   // Serial
			   {
			    .wString =
			    { 0x50, 0x2d, 0xffff, 0xffff, 0xffff, 0xffff,
			     0xffff, 0xffff, 0xffff, 0xffff, 0x2d, 0x31},
			    .length = 12U,
			    },
			   // Usb Cable
			   {
			    .wString = { 0x53, 0x79, 0x6e, 0x63},
			    .length = 4U,
			    },
			   // MIDI Cable
			   {
			    .wString =
			    { 0x4d, 0x49, 0x44, 0x49, 0x20, 0x49, 0x6e},
			    .length = 7U,
			    },
			   // Element
			   {
			    .wString =
			    { 0x4d, 0x49, 0x44, 0x49, 0x20, 0x2192, 0x20, 0x53,
			     0x79, 0x6e, 0x63},
			    .length = 11U,
			    },
			   // Interface
			   {
			    .wString =
			    { 0x55, 0x53, 0x42, 0x2d, 0x4d, 0x49, 0x44, 0x49},
			    .length = 8U,
			    },
			   },
		 },
};
