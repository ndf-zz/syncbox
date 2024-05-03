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
struct general_config config;

// Load preset from ROM
void settings_preset(uint32_t preset)
{
	uint32_t *src = (uint32_t *)&OPTION->preset[preset];
	uint32_t *dst = (uint32_t *)&config;
	uint32_t cnt = 0;
	uint32_t len = sizeof(struct general_config)>>2;
	while (cnt < len) {
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
	settings_preset(2);
}
