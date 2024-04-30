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
#define SETTINGS_NROUTS		6U	// CK, RN, FL, G1, G2, G3
#define SETTINGS_OUTCK		0U
#define SETTINGS_OUTRN		1U
#define SETTINGS_OUTFL		2U
#define SETTINGS_OUTG1		3U
#define SETTINGS_OUTG2		4U
#define SETTINGS_OUTG3		5U
#define SETTING_48PPQ		1U	// refclock / 1
#define SETTING_24PPQ		2U	// refclock / 2
#define SETTING_32ND		6U	// refclock / 6
#define SETTING_16TH		12U	// refclock / 12 -> ~ 16th note
#define SETTING_8TH		24U	// refclock / 24 -> ~ 8th note (PO)
#define SETTING_BEAT		48U	// refclock / 48 -> on the beat
#define SETTING_BAR		192U	// refclock / 192 -> on the bar
#define SETTING_AUTO		0xf0	// auto-select clock master
#define SETTING_CLOCK		(1U<<0)	// use clock/div for output
#define SETTING_RUNSTOP		(1U<<1)	// Use run/stop for output
#define SETTING_CONTINUE	(1U<<2)	// Use continue for output
#define SETTING_NOTE		(1U<<3)	// Use note on/off for output
#define SETTING_TRIG		(1U<<4)	// Limit pulse length to 96th period
#define SETTING_CTRLDIV		(1U<<5)	// Use controller for divisor
#define SETTING_CTRLOFT		(1U<<6)	// Use controller for phase offset
#define SETTING_OMNION		1U	// Omni enabled, Poly On
#define SETTING_OMNIOFF		3U	// Omni disabled, Poly On
#define SETTING_INVALID		0x100	// Non-matching note/ctrl
#define SETTING_DEFFILT		((1U<<MIDI_CIN_NOTE_OFF)| \
				(1U<<MIDI_CIN_NOTE_ON)| \
				(1U<<MIDI_CIN_CONTROL)| \
				(1U<<MIDI_CIN_BYTE))

// Journal setting keys
enum setting_key {
	SETTING_DELAY,		// 96ppq period in HCLKs
	SETTING_INERTIA,	// PLL slew rate (0=disable, 127=sloth)
	SETTING_CHANNEL,	// MIDI Channel for voice/ctrl messages
	SETTING_MODE,		// MIDI Mode (Omni on/off)
	SETTING_MASTER,		// Master clock selection
	SETTING_FUSB,		// USB Cable event filter
	SETTING_FMIDI,		// MIDI Cable event filter
	SETTING_RSVD,		// Reserved
	SETTING_CKSOURCE,	// CK source
	SETTING_CKDIV,		// CK divisor
	SETTING_CKOFT,		// CK offset
	SETTING_CKNOTE,		// CK note
	SETTING_RNSOURCE,	// RN source
	SETTING_RNDIV,		// RN divisor
	SETTING_RNOFT,		// RN offset
	SETTING_RNNOTE,		// RN note
	SETTING_FLSOURCE,	// FL source
	SETTING_FLDIV,		// FL divisor
	SETTING_FLOFT,		// FL offset
	SETTING_FLNOTE,		// FL note
	SETTING_G1SOURCE,	// G1 source
	SETTING_G1DIV,		// G1 divisor
	SETTING_G1OFT,		// G1 offset
	SETTING_G1NOTE,		// G1 note
	SETTING_G2SOURCE,	// G2 source
	SETTING_G2DIV,		// G2 divisor
	SETTING_G2OFT,		// G2 offset
	SETTING_G2NOTE,		// G2 note
	SETTING_G3SOURCE,	// G3 source
	SETTING_G3DIV,		// G3 divisor
	SETTING_G3OFT,		// G3 offset
	SETTING_G3NOTE,		// G3 note
};

// Output Configuration
struct output_config {
	uint32_t source;
	uint32_t divisor;
	uint32_t offset;
	uint32_t note;
};

// General system config
struct general_config {
	uint32_t delay;
	uint32_t inertia;
	uint32_t channel;
	uint32_t mode;
	uint32_t master;
	uint32_t fusb;
	uint32_t fmidi;
	uint32_t reserved;
	struct output_config output[SETTINGS_NROUTS];
};

// Flash ROM data structure
struct option_struct {
	struct general_config preset[PRESETS_LEN];
	uint32_t usbdesc;
	uint32_t usbcfg;
	uint32_t version;
};

// Global settings
extern struct general_config config;

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
