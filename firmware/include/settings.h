// SPDX-License-Identifier: MIT

/*
 * Runtime settings
 */
#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdint.h>
#include "midi_event.h"

#define SETTINGS_BITS		5U
#define SETTINGS_KEYLEN		(1U<<SETTINGS_BITS)
#define SETTINGS_KEYMASK	(SETTINGS_KEYLEN-1U)
#define PRESETS_LEN		16U

// Setting value constants
#define SETTING_48PPQ		1U	// refclock / 1
#define SETTING_24PPQ		2U	// refclock / 2
#define SETTING_32ND		4U	// refclock / 6
#define SETTING_16TH		12U	// refclock / 12 -> ~ 16th note
#define SETTING_8TH		24U	// refclock / 24 -> ~ 8th note (PO)
#define SETTING_BEAT		48U	// refclock / 48 -> on the beat
#define SETTING_AUTO		0xf0	// auto-select clock master
#define SETTING_CLOCK		1U	// use clock/div for output
#define SETTING_RS		2U	// Use run/stop for output
#define SETTING_NOTE		4U	// Use note on/off for output
#define SETTING_CTRL		8U	// Use clock/ctrl for output

// Setting keys
enum setting_key {
	SETTING_DELAY,		// 96ppq period in HCLKs
	SETTING_INERTIA,	// PLL slew rate (0=disable, 127=sloth)
	SETTING_CHANNEL,	// MIDI Channel for voice/ctrl messages
	SETTING_OMNI,		// Omni mode enabled
	SETTING_MASTER,		// Master clock selection
	SETTING_FUSB,		// USB Cable event filter
	SETTING_FMIDI,		// MIDI Cable event filter
	SETTING_CKSOURCE,	// CK output trigger
	SETTING_CKVAL,		// CK option value
	SETTING_RNSOURCE,	// RN output trigger
	SETTING_RNVAL,		// RN option value
	SETTING_FLSOURCE,	// FL output trigger
	SETTING_FLVAL,		// FL option value
	SETTING_G1SOURCE,	// G1 output trigger
	SETTING_G1VAL,		// G1 option value
	SETTING_G2SOURCE,	// G2 output trigger
	SETTING_G2VAL,		// G2 option value
	SETTING_G3SOURCE,	// G3 output trigger
	SETTING_G3VAL,		// G3 option value
};

// Flash ROM data structure
struct option_struct {
	uint32_t preset[PRESETS_LEN][SETTINGS_KEYLEN];
	uint32_t usbdesc;
	uint32_t usbcfg;
	uint32_t version;
};

// Global settings
extern uint32_t config[SETTINGS_KEYLEN];

// Load preset from ROM
void settings_preset(uint32_t preset);

// Update settings from sysex config 
void settings_sysex(struct midi_sysex_config *cfg);

// Update a setting value and schedule save to flash
uint32_t settings_set(enum setting_key key, uint32_t value);

// Request backup of all settings to flash
void settings_save(void);

// Prepare settings interface and read from flash
void settings_init(void);

#endif // SETTINGS_H
