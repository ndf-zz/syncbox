// SPDX-License-Identifier: MIT

/*
 * MIDI Event Queue
 *
 * Filter incoming bytes and queue valid messages
 * as usb-midi event packets via midi_event interface
 * flagged with relevant cable number.
 *
 * Sysex packets up to a length of 46 bytes are buffered
 * in the receiver. Valid sysex messages are indicated
 * to the event interface with a three byte sysex event
 * packet containing the length of data received.
 *
 * References:
 *
 *  - MIDI 1.0 Detailed Specification 4.2
 *  - Universal Serial Bus Device Class Definition for MIDI Devices 1.0
 */
#include "midi.h"
#include "midi_event.h"
#include "midi_uart.h"
#include "midi_usb.h"
#include "display.h"
#include "stm32f303xe.h"
#include "settings.h"

#define RCVBUFBITS		4U
#define RCVBUFLEN		(1U << RCVBUFBITS)
#define	RCVBUFMASK		(RCVBUFLEN - 1U)
#define SYSBUFLEN		64U
#define MIDI_OVERRUN		24U

static struct midi_event_buf {
	volatile uint32_t wi;
	volatile uint32_t ri;
	struct midi_event rcv[RCVBUFLEN];
} event_buf;

struct midi_receiver {
	uint32_t time;		// system time of last received byte
	uint32_t status;	// current running status
	uint32_t cin;		// current Code Index (CIN)
	uint32_t data1;		// previous data byte
	uint32_t bytes;		// number of data bytes expected  (0,1,2)
	uint32_t count;		// count of data bytes collected
	uint32_t sense;		// recent signal flag
	uint32_t sysid;		// id of sysex packet for curent buf
	uint8_t sysbuf[MIDI_MAX_SYSEX];	// sysex packet buffer
} rcv[2];

/* Copy raw midi packet into receive buffer
 *
 * Drop packet in case of overrun (tbc)
 *
 */
void midi_event_append(uint32_t event, uint32_t clock)
{
	uint32_t look = (event_buf.wi + 1U) & RCVBUFMASK;
	if (look != event_buf.ri) {
		struct midi_event *dst = &event_buf.rcv[look];
		dst->evt.val = event;
		dst->clock = clock;
		barrier();
		event_buf.wi = look;
	} else {
		BREAKPOINT(MIDI_OVERRUN);
	}
	PENDSV();
}

// Reset receive status
void midi_reset(const uint32_t cableno)
{
	rcv[cableno].cin = MIDI_CIN_RESERVED_0;
	rcv[cableno].status = MIDI_STATUS_NULL;
	rcv[cableno].bytes = 0U;
	rcv[cableno].count = 0U;
}

// Receive system exclusive data byte
static void rcv_sys(const uint32_t cableno, uint32_t db)
{
	if (rcv[cableno].count >= MIDI_MAX_SYSEX) {
		// Too many bytes for this device, ignore whole packet
		midi_reset(cableno);
		return;
	}
	rcv[cableno].sysbuf[rcv[cableno].count] = (uint8_t) db;
	rcv[cableno].count++;
}

// Receive a data byte
static void rcv_data(const uint32_t cableno, uint32_t db)
{
	rcv[cableno].count++;
	if (rcv[cableno].count == rcv[cableno].bytes) {
		uint32_t sb = rcv[cableno].status;
		union midi_event_pkt e;
		e.raw.header = (uint8_t) cableno << 4;
		uint32_t cin = rcv[cableno].cin;
		if ((sb & MIDI_STATUS_MASK) == MIDI_STATUS_NOTEON) {
			if (db == 0U) {	// Convert to note off
				cin = MIDI_CIN_NOTE_OFF;
				sb &= ~(0x10U);
			}
		}
		e.raw.header |= (uint8_t) cin;
		e.raw.midi0 = (uint8_t) sb;
		if (rcv[cableno].bytes == 2U) {
			e.raw.midi1 = (uint8_t) rcv[cableno].data1;
			e.raw.midi2 = (uint8_t) db;
		} else {
			e.raw.midi1 = (uint8_t) db;
			e.raw.midi2 = 0U;
		}
		midi_event_append(e.val, rcv[cableno].time);
		if (rcv[cableno].cin == MIDI_CIN_COMMON_3
		    || rcv[cableno].cin == MIDI_CIN_COMMON_2) {
			midi_reset(cableno);
		} else {
			rcv[cableno].count = 0U;
		}
	} else {
		rcv[cableno].data1 = db;
	}
}

// Enqueue system exclusive message packet length
static void sys_msg(const uint32_t cableno)
{
	if (rcv[cableno].count) {
		union midi_event_pkt e = {
			.raw = {
				.header =
				(uint8_t) (cableno << 4) | MIDI_CIN_EOX_3,
				.midi0 = MIDI_STATUS_SYSTEM,
				.midi1 = (uint8_t) rcv[cableno].count,
				.midi2 = MIDI_STATUS_EOX,
				 }
		};
		midi_event_append(e.val, rcv[cableno].sysid);
	}
}

// Enqueue a single byte midi message
static void single_byte_msg(const uint32_t cableno, uint32_t cin,
			    uint32_t midi0)
{
	union midi_event_pkt e = {
		.raw = {
			.header = (uint8_t) (cin | cableno << 4),
			.midi0 = (uint8_t) midi0,
			.midi1 = 0U,
			.midi2 = 0U,
			 }
	};
	midi_event_append(e.val, rcv[cableno].time);
}

// Prepare for a 2 byte message
static void status_2_byte(const uint32_t cableno, uint32_t cin, uint32_t status)
{
	rcv[cableno].cin = cin;
	rcv[cableno].status = status;
	rcv[cableno].bytes = 1U;
	rcv[cableno].count = 0U;
}

// Prepare for a 3 byte message
static void status_3_byte(const uint32_t cableno, uint32_t cin, uint32_t status)
{
	rcv[cableno].cin = cin;
	rcv[cableno].status = status;
	rcv[cableno].bytes = 2U;
	rcv[cableno].count = 0U;
}

// Receive a status byte
static void rcv_status(const uint32_t cableno, uint32_t sb)
{
	display_midi_on();
	rcv[cableno].sense = 1U;
	switch (sb) {
	case MIDI_RT_CLOCK:
	case MIDI_RT_START:
	case MIDI_RT_CONTINUE:
	case MIDI_RT_STOP:
		single_byte_msg(cableno, MIDI_CIN_BYTE, sb);
		break;
	case MIDI_RT_RESET:
		single_byte_msg(cableno, MIDI_CIN_BYTE, sb);
		midi_reset(cableno);
		break;
	case MIDI_RT_SENSE:
	case MIDI_RT_UNDEF9:
	case MIDI_RT_UNDEFd:
		/* MIDI 1.0 Detailed Specification 4.2, A-1:
		 * "undefined Real Time status bytes (F9H, FDH)
		 *  [...] should always be ignored, and the running
		 *  status buffer should remain unaffected"
		 */
		break;
	case MIDI_STATUS_SYSTEM:
		// Special case: Sysex msg len in EOX3 packet
		if (rcv[cableno].sysid == 0) {
			rcv[cableno].sysid = Uptime | 0x80000000;
			status_3_byte(cableno, MIDI_CIN_EOX_3,
				      MIDI_STATUS_SYSTEM);
		} else {
			// sysbuf still contains unread data, ignore packet
			midi_reset(cableno);
		}
		break;
	case MIDI_STATUS_SPP:
		status_3_byte(cableno, MIDI_CIN_COMMON_3, MIDI_STATUS_SPP);
		break;
	case MIDI_STATUS_MTCQF:
	case MIDI_STATUS_SONGSEL:
		status_2_byte(cableno, MIDI_CIN_COMMON_2, sb);
		break;
	case MIDI_STATUS_UNDEF4:
	case MIDI_STATUS_UNDEF5:
		/* MIDI 1.0 Detailed Specification 4.2 A-1:
		 * "undefined System Common status bytes (F4H and F5H)
		 * [...] should be ignored and the running status buffer
		 * should be cleared"
		 */
		midi_reset(cableno);
		break;
	case MIDI_STATUS_TUNEREQ:
		single_byte_msg(cableno, MIDI_CIN_SYS_1, sb);
		midi_reset(cableno);
		break;
	case MIDI_STATUS_EOX:
		sys_msg(cableno);
		midi_reset(cableno);
		break;
	default:
		// Channel Messages
		switch (sb & MIDI_STATUS_MASK) {
		case MIDI_STATUS_NOTEOFF:
		case MIDI_STATUS_NOTEON:
		case MIDI_STATUS_POLYPRESS:
		case MIDI_STATUS_CONTROL:
		case MIDI_STATUS_BENDER:
			status_3_byte(cableno, sb >> 4, sb);
			break;
		case MIDI_STATUS_PROGRAM:
		case MIDI_STATUS_CHANPRESS:
			status_2_byte(cableno, sb >> 4, sb);
			break;
		default:
			break;
		}
	}
}

// Test for recent reception of data on nominated cable
static uint32_t rcv_signal(const uint32_t cableno)
{
	if (rcv[cableno].sense) {
		if ((Uptime - rcv[cableno].time + 4U) > MIDI_SENSE_TIMEOUT) {
			// Timeout - may impact very slow sysex
			midi_reset(cableno);
			rcv[cableno].sense = 0;
			rcv[cableno].sysid = 0;
			return 1U;
		}
	}
	return 0;
}

// Update sense status on both cables
static uint32_t rcv_sense(void)
{
	return rcv_signal(MIDI_CABLE_UART) || rcv_signal(MIDI_CABLE_USB);
}

// Receive a single byte from the nominated cable
void midi_receive(const uint32_t cableno, uint32_t val)
{
	rcv[cableno].time = Uptime;
	if (val & MIDI_STATUS_FLAG) {
		rcv_status(cableno, val);
	} else if (rcv[cableno].status == MIDI_STATUS_SYSTEM) {
		rcv_sys(cableno, val);
	} else if (rcv[cableno].bytes) {
		rcv_data(cableno, val);
	}
}

// Mark sysex buffer event as read
void midi_sysex_done(struct midi_event *event)
{
	uint32_t cableno = (event->evt.raw.header & MIDI_CABLE_MASK) >> 4;
	rcv[cableno].sysid = 0;
}

/* Filter event according to config cable filter
 *
 * Events not matching cable's filter will be converted
 * to MIDI_CIN_RESERVED_0
 *
 * Sysex packets are always received
 */
static void filter_event(struct midi_event *event)
{
	uint32_t cableno = (event->evt.raw.header & MIDI_CABLE_MASK) >> 4;
	uint32_t mask = 1U << (event->evt.raw.header & MIDI_CIN_MASK);
	if (cableno == MIDI_CABLE_UART) {
		mask &= config.fmidi | (1U << MIDI_CIN_EOX_3);
	} else {
		mask &= config.fusb | (1U << MIDI_CIN_EOX_3);
	}
	if (!mask)
		event->evt.raw.header = MIDI_CABLE_MASK | MIDI_CIN_RESERVED_0;
}

/* Extract next event from buffer
 *
 * Returns pointer to event or NULL if no event available
 * Call midi_event_done() to release the event once processed
 */
struct midi_event *midi_event_poll(void)
{
	struct midi_event *event = NULL;
	if (event_buf.ri != event_buf.wi) {
		uint32_t look = (event_buf.ri + 1) & RCVBUFMASK;
		event = &event_buf.rcv[look];
		filter_event(event);
	} else {
		if (rcv_sense()) {
			display_midi_off();
		}
	}
	return event;
}

// Flag the last received event as done
void midi_event_done(void)
{
	event_buf.ri = (event_buf.ri + 1) & RCVBUFMASK;
}

// Return pointer to sysex packet buffer if event valid
struct midi_sysex_config *midi_sysex_buf(struct midi_event *event)
{
	uint32_t cableno = (event->evt.raw.header & MIDI_CABLE_MASK) >> 4;
	return (struct midi_sysex_config *)&rcv[cableno].sysbuf[0];
}

/* Setup USB and MIDI devices */
void midi_event_init(void)
{
	event_buf.ri = 0U;
	event_buf.wi = 0U;
	midi_uart_init();
	midi_usb_init();
}
