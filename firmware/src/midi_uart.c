// SPDX-License-Identifier: MIT

/*
 * MIDI UART Interface
 *
 * Filter incoming bytes and queue valid messages
 * as usb-midi event packets via midi_event interface
 * flagged with UART cable number (1).
 *
 * Sysex packets up to a length of 46 bytes are buffered
 * to midi_uart_sysbuf. Valid sysex messages are indicated
 * to the event interface with a three byte sysex event packet
 * containing the length of data received.
 *
 * References:
 *
 *  - MIDI 1.0 Detailed Specification 4.2
 *  - Universal Serial Bus Device Class Definition for MIDI Devices 1.0
 */
#include "stm32f3xx.h"
#include "midi.h"
#include "midi_event.h"
#include "midi_uart.h"
#include "display.h"

#define MIDI_BAUD              31250U

uint8_t midi_uart_sysbuf[MIDI_MAX_SYSEX];

static struct uart_receiver {
	volatile uint32_t time;	// system time of last received byte
	uint32_t status;	// current running status
	uint32_t cin;		// current Code Index (CIN)
	uint32_t data1;		// previous data byte
	uint32_t bytes;		// number of data bytes expected  (0,1,2)
	uint32_t count;		// count of data bytes collected
} rcv;

// Reset receive status
static void rcv_reset(void)
{
	rcv.cin = MIDI_CIN_RESERVED_0;
	rcv.status = MIDI_STATUS_NULL;
	rcv.bytes = 0U;
	rcv.count = 0U;
}

// Receive system exclusive data byte
static void rcv_sys(uint32_t db)
{
	if (rcv.count >= MIDI_MAX_SYSEX) {
		// Too many bytes for this device, ignore whole packet
		rcv_reset();
		return;
	}
	midi_uart_sysbuf[rcv.count] = (uint8_t) db;
	rcv.count++;
}

// Receive a data byte
static void rcv_data(uint32_t db)
{
	rcv.count++;
	if (rcv.count == rcv.bytes) {
		uint32_t sb = rcv.status;
		union midi_event_pkt e;
		e.raw.header = MIDI_CABLE_UART;
		uint32_t cin = rcv.cin;
		if ((sb & MIDI_STATUS_MASK) == MIDI_STATUS_NOTEON) {
			if (db == 0U) {	// Convert to note off
				cin = MIDI_CIN_NOTE_OFF;
				sb &= ~(0x10U);
			}
		}
		e.raw.header |= (uint8_t) cin;
		e.raw.midi0 = (uint8_t) sb;
		if (rcv.bytes == 2U) {
			e.raw.midi1 = (uint8_t) rcv.data1;
			e.raw.midi2 = (uint8_t) db;
		} else {
			e.raw.midi1 = (uint8_t) db;
			e.raw.midi2 = 0U;
		}
		midi_event_append(e.val, rcv.time);
		if (rcv.cin == MIDI_CIN_COMMON_3
		    || rcv.cin == MIDI_CIN_COMMON_2) {
			rcv_reset();
		} else {
			rcv.count = 0U;
		}
	} else {
		rcv.data1 = db;
	}
}

// Enqueue system exclusive message packet length
static void sys_msg(void)
{
	if (rcv.count) {
		union midi_event_pkt e = {
			.raw = {
				.header = MIDI_CABLE_UART | MIDI_CIN_EOX_3,
				.midi0 = MIDI_STATUS_SYSTEM,
				.midi1 = (uint8_t) rcv.count,
				.midi2 = MIDI_STATUS_EOX,
				 }
		};
		midi_event_append(e.val, rcv.time);
	}
}

// Enqueue a single byte midi message
static void single_byte_msg(uint32_t cin, uint32_t midi0)
{
	union midi_event_pkt e = {
		.raw = {
			.header = (uint8_t) cin | MIDI_CABLE_UART,
			.midi0 = (uint8_t) midi0,
			.midi1 = 0U,
			.midi2 = 0U,
			 }
	};
	midi_event_append(e.val, rcv.time);
}

// Ignore a status byte
static void ignore_status(void)
{
}

// Prepare for a 2 byte message
static void status_2_byte(uint32_t cin, uint32_t status)
{
	rcv.cin = cin;
	rcv.status = status;
	rcv.bytes = 1U;
	rcv.count = 0U;
}

// Prepare for a 3 byte message
static void status_3_byte(uint32_t cin, uint32_t status)
{
	rcv.cin = cin;
	rcv.status = status;
	rcv.bytes = 2U;
	rcv.count = 0U;
}

// Receive a status byte
static void rcv_status(uint32_t sb)
{
	display_midi_on();
	switch (sb) {
	case MIDI_RT_CLOCK:
	case MIDI_RT_START:
	case MIDI_RT_CONTINUE:
	case MIDI_RT_STOP:
		single_byte_msg(MIDI_CIN_BYTE, sb);
		break;
	case MIDI_RT_RESET:
		single_byte_msg(MIDI_CIN_BYTE, sb);
		rcv_reset();
		break;
	case MIDI_RT_SENSE:
	case MIDI_RT_UNDEF9:
	case MIDI_RT_UNDEFd:
		/* MIDI 1.0 Detailed Specification 4.2, A-1:
		 * "undefined Real Time status bytes (F9H, FDH)
		 *  [...] should always be ignored, and the running
		 *  status buffer should remain unaffected"
		 */
		ignore_status();
		break;
	case MIDI_STATUS_SYSTEM:
		// Special case: Sysex msg len in EOX3 packet
		status_3_byte(MIDI_CIN_EOX_3, MIDI_STATUS_SYSTEM);
		break;
	case MIDI_STATUS_SPP:
		status_3_byte(MIDI_CIN_COMMON_3, MIDI_STATUS_SPP);
		break;
	case MIDI_STATUS_MTCQF:
	case MIDI_STATUS_SONGSEL:
		status_2_byte(MIDI_CIN_COMMON_2, sb);
		break;
	case MIDI_STATUS_UNDEF4:
	case MIDI_STATUS_UNDEF5:
		/* MIDI 1.0 Detailed Specification 4.2 A-1:
		 * "undefined System Common status bytes (F4H and F5H)
		 * [...] should be ignored and the running status buffer
		 * should be cleared"
		 */
		ignore_status();
		rcv_reset();
		break;
	case MIDI_STATUS_TUNEREQ:
		single_byte_msg(MIDI_CIN_SYS_1, sb);
		rcv_reset();
		break;
	case MIDI_STATUS_EOX:
		sys_msg();
		rcv_reset();
		break;
	default:
		// Channel Messages
		switch (sb & MIDI_STATUS_MASK) {
		case MIDI_STATUS_NOTEOFF:
		case MIDI_STATUS_NOTEON:
		case MIDI_STATUS_POLYPRESS:
		case MIDI_STATUS_CONTROL:
		case MIDI_STATUS_BENDER:
			status_3_byte(sb >> 4, sb);
			break;
		case MIDI_STATUS_PROGRAM:
		case MIDI_STATUS_CHANPRESS:
			status_2_byte(sb >> 4, sb);
			break;
		default:
			break;
		}
	}
}

// Receive byte from serial port
void UART5_IRQHandler(void)
{
	uint32_t tmp = UART5->RDR;
	if (UART5->ISR & (USART_ISR_FE | USART_ISR_NE)) {
		// Assume byte was status
		rcv_reset();
		UART5->ICR |= (USART_ICR_FECF | USART_ICR_NCF);
	} else {
		rcv.time = Uptime;
		if (tmp & MIDI_STATUS_FLAG) {
			rcv_status(tmp);
		} else if (rcv.status == MIDI_STATUS_SYSTEM) {
			rcv_sys(tmp);
		} else if (rcv.bytes) {
			rcv_data(tmp);
		}
	}
	if (UART5->ISR & USART_ISR_ORE) {
		// Assume byte would have been status
		rcv_reset();
		UART5->ICR |= USART_ICR_ORECF;
	}
}

// Initialise hardware and enable receive interrupt
void midi_uart_init(void)
{
	UART5->CR1 |= USART_CR1_RXNEIE;
	UART5->BRR = (SYSTEMCORECLOCK / MIDI_BAUD);
	UART5->CR1 |= USART_CR1_UE | USART_CR1_RE;
	NVIC_SetPriority(UART5_IRQn, PRIGROUP2|PRISUB1);
	NVIC_EnableIRQ(UART5_IRQn);
}

// Return non-zero if device active sense has expired
uint32_t midi_uart_sense(void)
{
	return (Uptime - rcv.time + 4U) > MIDI_SENSE_TIMEOUT;
}
