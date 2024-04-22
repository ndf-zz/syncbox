// SPDX-License-Identifier: MIT

/*
 * Simple 2 LED display
 */
#ifndef DISPLAY_H
#define DISPLAY_H
#include <stdint.h>

// Update the display state - call once per millisecond with current uptime
void display_update(uint32_t clock);

// Turn on/off the indicators
void display_midi_on(void);
void display_midi_off(void);
void display_din_on(void);
void display_din_off(void);

// Trigger blink on an indicator
void display_midi_blink(void);
void display_din_blink(void);

#endif /* DISPLAY_H */
