/*
 * Shared MIDI definitions
 */
#ifndef _MIDI_H
#define _MIDI_H

#define MIDI_RT_MASK            0xf8U
#define MIDI_STATUS_MASK        0xf0U
#define MIDI_DATA_MASK		0x7fU
#define MIDI_CHANNEL_MASK	0x0fU

#define MIDI_STATUS_NULL        0x00U
#define MIDI_STATUS_NOTEOFF     0x80U
#define MIDI_STATUS_NOTEON      0x90U
#define MIDI_STATUS_POLYPRESS   0xa0U
#define MIDI_STATUS_CONTROL     0xb0U
#define MIDI_STATUS_PROGRAM     0xc0U
#define MIDI_STATUS_CHANPRESS   0xd0U
#define MIDI_STATUS_BENDER      0xe0U
#define MIDI_STATUS_SYSTEM      0xf0U
#define MIDI_STATUS_MTCQF	0xf1U
#define MIDI_STATUS_SPP		0xf2U
#define MIDI_STATUS_SONGSEL	0xf3U
#define MIDI_STATUS_UNDEF4	0xf4U
#define MIDI_STATUS_UNDEF5	0xf5U
#define MIDI_STATUS_TUNEREQ	0xf6U
#define MIDI_STATUS_EOX         0xf7U
#define MIDI_RT_CLOCK           0xf8U
#define MIDI_RT_UNDEF9		0xf9U
#define MIDI_RT_START           0xfaU
#define MIDI_RT_CONTINUE        0xfbU
#define MIDI_RT_STOP            0xfcU
#define MIDI_RT_UNDEFd		0xfdU
#define MIDI_RT_SENSE           0xfeU
#define MIDI_RT_RESET		0xffU

#endif /* _MIDI_H */
