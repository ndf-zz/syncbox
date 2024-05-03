// SPDX-License-Identifier: MIT

/*
 * Reference clock timer
 *
 * This clock runs at 96ppq and manages triggering of all
 * outputs configured with a clock source.
 *
 */
#ifndef TIMER_H
#define TIMER_H
#include "midi_event.h"

#define TIMER	TIM2

// timer status structure
struct timer_state {
	uint32_t phase;
	uint32_t nextout;
	uint32_t running;
};

extern struct timer_state timer;

// Halt timer in preparation for a start message
void timer_preroll(void);

// Handle arrival of a midi timing message
void timer_clock(struct midi_event *event);

// Prepare timer interface
void timer_init(void);

#endif // TIMER_H
