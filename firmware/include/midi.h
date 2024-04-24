/*
 * Shared MIDI definitions
 */
#ifndef MIDI_H
#define MIDI_H

#define MIDI_SENSE_TIMEOUT      400U

#define MIDI_RT_MASK            0xf8
#define MIDI_STATUS_MASK        0xf0
#define MIDI_DATA_MASK		0x7f
#define MIDI_CHANNEL_MASK	0x0f
#define MIDI_STATUS_FLAG	0x80

#define MIDI_STATUS_NULL        0x00

// Channel Voice
#define MIDI_STATUS_NOTEOFF     0x80
#define MIDI_STATUS_NOTEON      0x90
#define MIDI_STATUS_POLYPRESS   0xa0
#define MIDI_STATUS_CONTROL     0xb0
#define MIDI_STATUS_MODE	0xb0
#define MIDI_STATUS_PROGRAM     0xc0
#define MIDI_STATUS_CHANPRESS   0xd0
#define MIDI_STATUS_BENDER      0xe0

// System Exclusive
#define MIDI_STATUS_SYSTEM	0xf0

// System Common
#define MIDI_STATUS_MTCQF	0xf1
#define MIDI_STATUS_SPP		0xf2
#define MIDI_STATUS_SONGSEL	0xf3
#define MIDI_STATUS_UNDEF4	0xf4
#define MIDI_STATUS_UNDEF5	0xf5
#define MIDI_STATUS_TUNEREQ	0xf6
#define MIDI_STATUS_EOX         0xf7

// System Realtime
#define MIDI_RT_CLOCK           0xf8
#define MIDI_RT_UNDEF9		0xf9
#define MIDI_RT_START           0xfa
#define MIDI_RT_CONTINUE        0xfb
#define MIDI_RT_STOP            0xfc
#define MIDI_RT_UNDEFd		0xfd
#define MIDI_RT_SENSE           0xfe
#define MIDI_RT_RESET		0xff

// Channel Mode Messages
#define MIDI_MODE_SOUNDOFF	0x78
#define MIDI_MODE_RESET		0x79
#define MIDI_MODE_LOCAL		0x7a
#define MIDI_MODE_ALLOFF	0x7b
#define MIDI_MODE_OMNIOFF	0x7c
#define MIDI_MODE_OMNION	0x7d
#define MIDI_MODE_MONO		0x7e
#define MIDI_MODE_POLY		0x7f

#endif // MIDI_H
