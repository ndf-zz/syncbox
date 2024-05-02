// SPDX-License-Identifier: MIT

/*
 * ROM Options - saved into binary image
 * at address 0x08004800.
 *
 */
#include "settings.h"

static __attribute__((used))
struct option_struct option = {
	.preset = {
		   // Preset 0: Omni on, Roland sync, fill+gates note on/off
		   {
		    .delay = 125000U,	// ~120bpm
		    .inertia = 0,	// Disable inertia
		    .channel = 0,	// MIDI channel 1
		    .mode = SETTING_OMNION,	// Omni on
		    .master = SETTING_AUTO,	// Clock Master: Auto
		    .fusb = SETTING_DEFFILT,	// Note/Control/RT
		    .fmidi = SETTING_DEFFILT,	// Note/Control/RT
		    .triglen = 8U,	// 8ms Trigger Length
		    .output = {
			       // CK Ouput
			       {
				.source = SETTING_CLOCK,
				.divisor = SETTING_24PPQ,
				.offset = 0,
				.note = SETTING_INVALID,
				},
			       // RN Output
			       {
				.source = SETTING_RUNSTOP,
				.divisor = 0,
				.offset = 0,
				.note = SETTING_INVALID,
				},
			       // FL Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 60U,	// C3
				},
			       // G1 Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 62U,	// D3
				},
			       // G2 Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 64U,	// E3
				},
			       // G3 Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 65U,	// F3
				},
			       },
		    },
		   // Preset 1: Omni off, internal clock, channel 1, notes only
		   {
		    .delay = 125000U,	// ~120bpm
		    .inertia = 0,	// Disable inertia
		    .channel = 0,	// MIDI channel 1
		    .mode = SETTING_OMNIOFF,	// Omni off
		    .master = 0x0f,	// Clock Master: Disabled
		    .fusb = SETTING_DEFFILT,	// Note, Control, RT
		    .fmidi = SETTING_DEFFILT,	// Note, Control, RT
		    .triglen = 0,	// Minimum trigger length
		    .output = {
			       // CK Ouput
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 58U,
				},
			       // RN Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 59U,
				},
			       // FL Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 60U,	// C3
				},
			       // G1 Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 62U,	// D3
				},
			       // G2 Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 64U,	// E3
				},
			       // G3 Output
			       {
				.source = SETTING_NOTE,
				.divisor = 0,
				.offset = 0,
				.note = 65U,	// E3
				},
			       },
		    },
		   // Terminal Preset - Outputs disabled
		   {
		    .delay = 125000U,
		    },
		    },
	.sysid = SYSEX_ID,
	.usbdesc = 0,
	.usbcfg = 0,
	.version = SYSTEMVERSION,
};
