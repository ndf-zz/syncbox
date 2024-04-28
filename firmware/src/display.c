// SPDX-License-Identifier: MIT

/*
 * Simple 2 LED display
 */
#include "stm32f3xx.h"
#include "display.h"

#define DISPLAY_GPIO	GPIOA
#define DISPLAY_DIN	GPIO_ODR_0
#define DISPLAY_MIDI	GPIO_ODR_1
#define DISPLAY_ONTIME	25UL
#define DISPLAY_OFFTIME	75UL
#define DISPLAY_PERIOD	100UL

// Status flags
#define DISPLAY_DIN_SET (1UL<<0)
#define DISPLAY_MIDI_SET (1UL<<1)
#define DISPLAY_DIN_ON (1UL<<2)
#define DISPLAY_MIDI_ON (1UL<<3)
#define DISPLAY_DIN_BLINK (1UL<<4)
#define DISPLAY_MIDI_BLINK (1UL<<5)
#define DISPLAY_DIN_START (1UL<<6)
#define DISPLAY_MIDI_START (1UL<<7)

static struct display_stat {
	uint32_t flags;
	uint32_t midi_start;
	uint32_t din_start;
} display;

// Latch LEDs on or off
void display_midi_on(void)
{
	display.flags |= DISPLAY_MIDI_ON;
}

void display_midi_off(void)
{
	display.flags &= ~DISPLAY_MIDI_ON;
}

void display_din_on(void)
{
	display.flags |= DISPLAY_DIN_ON;
}

void display_din_off(void)
{
	display.flags &= ~DISPLAY_DIN_ON;
}

// Trigger blink if not already blinking
void display_midi_blink(void)
{
	if (!(display.flags & DISPLAY_MIDI_BLINK)) {
		display.midi_start = Uptime;
		display.flags |= DISPLAY_MIDI_BLINK | DISPLAY_MIDI_START;
	}
}

void display_din_blink(void)
{
	if (!(display.flags & DISPLAY_DIN_BLINK)) {
		display.din_start = Uptime;
		display.flags |= DISPLAY_DIN_BLINK | DISPLAY_DIN_START;
	}
}

// Update display state and toggle GPIO as required
void display_update(uint32_t clock)
{
	uint32_t ontime;
	uint32_t tmp = 0;

	uint32_t set = display.flags & (DISPLAY_DIN_SET | DISPLAY_MIDI_SET);
	uint32_t on = (display.flags & (DISPLAY_DIN_ON | DISPLAY_MIDI_ON)) >> 2;
	if (set != on) {
		if (display.flags & DISPLAY_DIN_ON) {
			tmp |= DISPLAY_DIN;
			display.flags |= DISPLAY_DIN_SET;
		} else {
			tmp |= DISPLAY_DIN << 16;
			display.flags &= ~DISPLAY_DIN_SET;
		}
		if (display.flags & DISPLAY_MIDI_ON) {
			tmp |= DISPLAY_MIDI;
			display.flags |= DISPLAY_MIDI_SET;
		} else {
			tmp |= DISPLAY_MIDI << 16;
			display.flags &= ~DISPLAY_MIDI_SET;
		}
		DISPLAY_GPIO->BSRR = tmp;
	}

	tmp = 0;
	if (display.flags & DISPLAY_MIDI_START) {
		if (display.flags & DISPLAY_MIDI_SET) {
			tmp |= DISPLAY_MIDI << 16;
		} else {
			tmp |= DISPLAY_MIDI;
		}
		display.flags &= ~DISPLAY_MIDI_START;
	} else if (display.flags & DISPLAY_MIDI_BLINK) {
		ontime = clock - display.midi_start;
		if (ontime > DISPLAY_PERIOD) {
			display.flags &= ~DISPLAY_MIDI_BLINK;
		} else if (display.flags & DISPLAY_MIDI_SET) {
			if (ontime > DISPLAY_OFFTIME) {
				tmp |= DISPLAY_MIDI;
			}
		} else if (ontime > DISPLAY_ONTIME) {
			tmp |= DISPLAY_MIDI << 16;
		}
	}
	if (display.flags & DISPLAY_DIN_START) {
		if (display.flags & DISPLAY_DIN_SET) {
			tmp |= DISPLAY_DIN << 16;
		} else {
			tmp |= DISPLAY_DIN;
		}
		display.flags &= ~DISPLAY_DIN_START;
	} else if (display.flags & DISPLAY_DIN_BLINK) {
		ontime = clock - display.din_start;
		if (ontime > DISPLAY_PERIOD) {
			display.flags &= ~DISPLAY_DIN_BLINK;
		} else if (display.flags & DISPLAY_DIN_SET) {
			if (ontime > DISPLAY_OFFTIME) {
				tmp |= DISPLAY_DIN;
			}
		} else if (ontime > DISPLAY_ONTIME) {
			tmp |= DISPLAY_DIN << 16;
		}
	}
	if (tmp)
		DISPLAY_GPIO->BSRR = tmp;
}
