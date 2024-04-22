// SPDX-License-Identifier: MIT

/*
 * MIDI Event Interface
 */
#ifndef _MIDI_EVENT_H
#define _MIDI_EVENT_H
#include <stdint.h>

// missing defs
#define MIDI_ERROR_SENSE (1<<0)
#define MIDI_ERROR_BUFFER (1<<1)
#define MIDI_ERROR_FRAME (1<<2)
#define MIDI_ERROR_OVERRUN (1<<3)

/* Raw USB-MIDI style packet */
struct midi_event_raw {
	uint8_t	header;
	uint8_t midi0;
	uint8_t midi1;
	uint8_t midi2;
};

/* MIDI Event union */
union midi_event {
	uint32_t val;
	struct midi_event_raw raw;
};

/* MIDI Event Code Indexes */
#define MIDI_CIN_MASK		0xfU
#define MIDI_CIN_RESERVED_0	0x0U	/* Miscellaneous function codes */
#define MIDI_CIN_RESERVED_1	0x1U	/* Cable events */
#define MIDI_CIN_COMMON_2	0x2U	/* 2 byte system common */
#define MIDI_CIN_COMMON_3	0x3U	/* 3 byte system common */
#define MIDI_CIN_SYSEX		0x4U	/* Sysex start or contiue */
#define MIDI_CIN_SYS_1		0x5U	/* Single byte common or EOX */
#define MIDI_CIN_EOX_2		0x6U	/* Sysex ends with 2 bytes */
#define MIDI_CIN_EOX_3		0x7U	/* Sysex ends with 3 bytes */
#define MIDI_CIN_NOTE_OFF	0x8U	/* Note off */
#define MIDI_CIN_NOTE_ON	0x9U	/* Note on */
#define MIDI_CIN_POLY		0xaU	/* Poly key press */
#define MIDI_CIN_CONTROL	0xbU	/* Controller */
#define MIDI_CIN_PROG		0xcU	/* Program change */
#define MIDI_CIN_PRESS		0xdU	/* Channel pressure */
#define MIDI_CIN_BEND		0xeU	/* Pitchbend */
#define MIDI_CIN_BYTE		0xfU	/* Single byte / realtime */

/* MIDI Event Cable mask */
#define MIDI_CN_MASK		0xf0U	/* Cable number mask */

/* MIDI Cable numbers */
#define MIDI_CABLE_UART		0x10U	/* Hardware MIDI port */
#define MIDI_CABLE_USB		0x20U	/* USB-MIDI */

/* Handle arrival of a new MIDI event */
void midi_event_process(uint32_t e);

// temp
uint32_t midi_poll(void);

#endif /* _MIDI_EVENT_H */
