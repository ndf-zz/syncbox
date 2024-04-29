// SPDX-License-Identifier: MIT

/*
 * ROM Options - saved into binary image
 * at address 0x08004800.
 *
 */
#include "settings.h"

static __attribute__((used)) struct option_struct option = {
	.preset = {
	// Preset 0: Roland sync, fill + gates from note on/off
	{
	 125000U,		// 120bpm
	 0,			// Disable intertia
	 0,			// MIDI channel 1
	 1U,			// Omni on
	 SETTING_AUTO,		// Clock Master: Auto
	 0x7f,			// Accept all messages from USB
	 0x7f,			// Accept all messages from MIDI
	 SETTING_CLOCK,		// CK source = clock
	 SETTING_24PPQ,		// 24ppq
	 SETTING_RS,		// RN source = run/stop
	 0,			// N/A
	 SETTING_NOTE,		// FL source = note on/off
	 60U,			// Key C4
	 SETTING_NOTE,		// G1 source = note on/off
	 62U,			// Key D4
	 SETTING_NOTE,		// G2 source = note on/off
	 64U,			// Key E4
	 SETTING_NOTE,		// G3 source = note on/off
	 65U,			// Key F4
	  },
	{ 0xffffffff,
          0xdeadbeef,
          0xcafecafe,
	  0x11223344,
	  0x55667788, },		//
	},
	.usbdesc = 0,
	.usbcfg = 0,
	.version = SYSTEMVERSION,
};
