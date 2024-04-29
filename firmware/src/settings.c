// SPDX-License-Identifier: MIT

/*
 * Settings interface
 *
 * Provides a flash journal backed array of 1-24 bit
 * configuration values and access to presets stored
 * in system ROM
 *
 */
#include "settings.h"
#include "flash.h"

// Global config
uint32_t config[SETTINGS_KEYLEN];

// Load preset from ROM
void settings_preset(uint32_t preset)
{
	uint32_t oft = SETTINGS_KEYLEN * preset;
	uint32_t *src = &OPTION->preset[0][oft];
	uint32_t *dst = &config[0];
	uint32_t cnt = 0;
	while (cnt < SETTINGS_KEYLEN) {
		*dst++ = *src++;
		cnt++;
	}
}

// Update settings from sysex config 
void settings_sysex(struct midi_sysex_config *cfg)
{
	if (cfg->idcfg == 0x7d) {
		BREAKPOINT(33);
	}
}

// Update a setting value and schedule save to flash
uint32_t settings_set(enum setting_key key, uint32_t value)
{
	return key & value;
}

// Request backup of all settings to flash
void settings_save(void)
{
}

// Prepare settings interface and read from flash
void settings_init(void)
{
	settings_preset(0);
}
