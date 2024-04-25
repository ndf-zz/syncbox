// SPDX-License-Identifier: MIT

/*
 * MIDI Event Queue
 *
 * Receives filtered MIDI events in USB-MIDI packets from
 * UART and USB handles active sense via realtime messages
 */
#include "midi.h"
#include "midi_event.h"
#include "midi_uart.h"
#include "midi_usb.h"
#include "display.h"
#include "stm32f303xe.h"

#define RCVBUFBITS		4U
#define RCVBUFLEN		(1U << RCVBUFBITS)
#define	RCVBUFMASK		(RCVBUFLEN - 1U)
#define SYSBUFLEN		64U
#define MIDI_OVERRUN		24U

static struct midi_event_buf {
	volatile uint32_t wi;
	volatile uint32_t ri;
	uint32_t sys_cnt;
	struct midi_event rcv[RCVBUFLEN];
	uint8_t sys[SYSBUFLEN];
} event_buf;

/* Copy filtered raw midi packet into receive buffer
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
}

/* Extract next event from buffer
 *
 * Returns returns pointer to event or NULL if no event available
 * Call midi_event_done() to release the event once processed
 */
struct midi_event *midi_event_poll(void)
{
	struct midi_event *event = NULL;
	if (event_buf.ri != event_buf.wi) {
		uint32_t look = (event_buf.ri + 1) & RCVBUFMASK;
		event = &event_buf.rcv[look];
	} else {
		if ((midi_usb_sense() | midi_uart_sense())) {
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

// Return pointer to sysex buffer if event valid
struct midi_sysex_config *midi_sysex_buf(struct midi_event *event)
{
	struct midi_sysex_config *cfg = NULL;
	uint8_t cable = event->evt.raw.header&MIDI_CABLE_MASK;
	if (cable == MIDI_CABLE_UART) {
		cfg = (struct midi_sysex_config *)(&midi_uart_sysbuf[0]);
	} else if (cable == MIDI_CABLE_USB) {
	}
	return cfg;
}

/* Setup USB and MIDI devices */
void midi_event_init(void)
{
	event_buf.ri = 0U;
	event_buf.wi = 0U;
	midi_uart_init();
	midi_usb_init();
}
