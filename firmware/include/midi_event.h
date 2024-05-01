// SPDX-License-Identifier: MIT

/*
 * MIDI Event and Receiver Interface
 */
#ifndef MIDI_EVENT_H
#define MIDI_EVENT_H
#include <stdint.h>
#include <stddef.h>

// Raw USB-MIDI style packet
union midi_event_pkt {
	struct midi_event_raw {
		uint8_t header;
		uint8_t midi0;
		uint8_t midi1;
		uint8_t midi2;
	} raw;
	uint32_t val;
};

// Timestamped midi event packet
struct midi_event {
	union midi_event_pkt evt;
	uint32_t clock;
};

// Sysex config packet
struct midi_sysex_config {
	uint32_t idcfg;
	uint8_t data[];
};
#define MIDI_MAX_SYSEX 46U

// Special case: No pending event
#define MIDI_EVENT_NULL		NULL

// Provide a system id CRC match value
#ifndef SYSEX_ID
#define SYSEX_ID 0x1100007d
#endif
#define SYSEX_INVID ( \
	((SYSEX_ID&0xff)<<24U) | \
	(((SYSEX_ID>>8U)&0xff)<<16U) | \
	(((SYSEX_ID>>16U)&0xff)<<8U) | \
	((SYSEX_ID>>24U)&0xff))

// MIDI Event Code Indexes
#define MIDI_CIN_MASK		0xf
#define MIDI_CIN_RESERVED_0	0x0	// Miscellaneous function codes
#define MIDI_CIN_RESERVED_1	0x1	// Cable events
#define MIDI_CIN_COMMON_2	0x2	// 2 byte system common
#define MIDI_CIN_COMMON_3	0x3	// 3 byte system common
#define MIDI_CIN_SYSEX		0x4	// Sysex start or contiue
#define MIDI_CIN_SYS_1		0x5	// Single byte common or EOX
#define MIDI_CIN_EOX_2		0x6	// Sysex ends with 2 bytes
#define MIDI_CIN_EOX_3		0x7	// Sysex ends with 3 bytes
#define MIDI_CIN_NOTE_OFF	0x8	// Note off
#define MIDI_CIN_NOTE_ON	0x9	// Note on
#define MIDI_CIN_POLY		0xa	// Poly key press
#define MIDI_CIN_CONTROL	0xb	// Controller
#define MIDI_CIN_PROG		0xc	// Program change
#define MIDI_CIN_PRESS		0xd	// Channel pressure
#define MIDI_CIN_BEND		0xe	// Pitchbend
#define MIDI_CIN_BYTE		0xf	// Single byte / realtime

// MIDI Event Cable mask
#define MIDI_CABLE_MASK		0xf0	// Cable number mask

// MIDI Cable numbers 
#define MIDI_CABLE_UART		0x1	// MIDI UART
#define MIDI_CABLE_USB		0x0	// USB-MIDI

// Enqueue a filtered raw midi event packet
void midi_event_append(uint32_t event, uint32_t clock);

// Check for queued midi event and probe device sense
struct midi_event *midi_event_poll(void);

// Flag the last received event as done
void midi_event_done(void);

// Reset receive status on the nominated cable
void midi_reset(const uint32_t cable);

// Receive a single byte on the nominated cable
void midi_receive(const uint32_t cable, uint32_t val);

// Return pointer to sysex config buffer for the provided event handle
struct midi_sysex_config *midi_sysex_buf(struct midi_event *event);

// Mark the sysex event as done
void midi_sysex_done(struct midi_event *event);

// Prepare midi device interfaces
void midi_event_init(void);

#endif // MIDI_EVENT_H
