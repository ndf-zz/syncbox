// SPDX-License-Identifier: MIT

/*
 * Hardware MIDI receiver interface
 *
 * Serial data is read from PD2 through UART5 @ 31250baud
 * valid messages are dispatched through the midi_event interface
 */
#include "stm32f3xx.h"
#include "midi.h"
#include "midi_event.h"

#define MIDI_BAUD		31250U
#define SENSE_TIMEOUT		400U


#define RCVBUFBITS		4U
#define RCVBUFLEN		(1UL << RCVBUFBITS)
#define	RCVBUFMASK		(RCVBUFLEN - 1UL)

struct midi_receiver {
	volatile uint32_t sense;	/* active sense flag */
	volatile uint32_t time;	/* system time of last received */
	uint32_t status;	/* current running status or null */
	uint32_t data;		/* register for storing data bytes */
	uint32_t bytes;		/* count of data bytes expected */
	uint32_t count;		/* count of data bytes collected */
	volatile uint32_t wi;	/* write index */
	volatile uint32_t ri;	/* read index */
	uint32_t buf[RCVBUFLEN];	/* circular receive buffer */
	uint32_t error;		/* internal error state */
};

static struct midi_receiver rcv;

/* Extract next event from buffer */
uint32_t midi_poll(void)
{
	uint32_t ret = MIDI_STATUS_NULL;
	uint32_t nxt = (rcv.ri + 1) & RCVBUFMASK;
	if (nxt != rcv.wi) {
		ret = rcv.buf[nxt];
		rcv.ri = nxt;
		if ((ret & 0xf00000ffUL) == (MIDI_STATUS_NOTEON << 24UL)) {
			ret &= (~0x10000000UL);	/* convert to note off */
		}
	} else if (rcv.sense) {
		/* Note: Uptime and rcv.time may change, pad with 4 */
		if ((Uptime - rcv.time + 4) > SENSE_TIMEOUT) {
			rcv.sense = 0UL;
			ret = MIDI_STATUS_UNDEF5;
			rcv.error |= MIDI_ERROR_SENSE;
        		GPIOA->BRR = GPIO_ODR_1;
		}
	}
	return ret;
}

/* Receive status byte */
static inline void rcv_status(uint32_t sb)
{
        GPIOA->BSRR = GPIO_ODR_1;
	if ((sb & MIDI_RT_MASK) == MIDI_RT_MASK) {
		switch (sb) {
		case MIDI_RT_CLOCK:
		case MIDI_RT_START:
		case MIDI_RT_CONTINUE:
		case MIDI_RT_STOP:
			/* insert rt event */
			if (rcv.wi != rcv.ri) {
				rcv.buf[rcv.wi] = (sb << 24U);
				rcv.wi = (rcv.wi + 1U) & RCVBUFMASK;
			} else {
				rcv.error |= MIDI_ERROR_BUFFER;
			}
			break;
		case MIDI_RT_SENSE:
			rcv.sense = 1UL;
			break;
		default:
			/* ignore undefined RT messages */
			break;
		}
		/* save cpu clock and spool this as a sep message */
		if (sb == MIDI_RT_SENSE) {
			rcv.sense = 1UL;
		}
	} else {
		switch (sb & MIDI_STATUS_MASK) {
		case MIDI_STATUS_NOTEOFF:
		case MIDI_STATUS_NOTEON:
		case MIDI_STATUS_BENDER:
			rcv.bytes = 2UL;
			break;
		case MIDI_STATUS_PROGRAM:
			rcv.bytes = 1UL;
			break;
			/* Ignored message types */
		case MIDI_STATUS_POLYPRESS:
		case MIDI_STATUS_CONTROL:
		case MIDI_STATUS_CHANPRESS:
		case MIDI_STATUS_SYSTEM:
		default:
			sb = MIDI_STATUS_NULL;
			rcv.bytes = 0UL;
			break;
		}
		rcv.count = 0UL;
		rcv.status = sb;
	}
}

/* Receive next data byte and spool re-assembled events to buffer */
static inline void rcv_data(uint32_t db)
{
	uint32_t tmp;
        GPIOA->BRR = GPIO_ODR_1;
	rcv.data = (rcv.data << 8U) | db;
	rcv.count++;
	if (rcv.count == rcv.bytes) {
		tmp = (rcv.status << 24U) | (rcv.data & 0x0000ffffUL);
		if (rcv.wi != rcv.ri) {
			rcv.buf[rcv.wi] = tmp;
			rcv.wi = (rcv.wi + 1U) & RCVBUFMASK;
		} else {
			rcv.error |= MIDI_ERROR_BUFFER;
		}
		rcv.data = 0U;
		rcv.count = 0U;
	}
}

/* Reset receive state */
static inline void rcv_reset(void)
{
	rcv.status = MIDI_STATUS_NULL;
	rcv.bytes = 0U;
	rcv.count = 0U;
}

/* Receive byte from serial port */
void UART5_IRQHandler(void)
{
	barrier();
	uint32_t tmp = UART5->RDR;
	if (UART5->ISR & (USART_ISR_FE | USART_ISR_NE)) {
		rcv.error |= MIDI_ERROR_FRAME;
		rcv_reset();
		UART5->ICR |= (USART_ICR_FECF | USART_ICR_NCF);
	} else {
		rcv.time = Uptime;
		if (tmp & 0x80UL) {
			rcv_status(tmp);
		} else if (rcv.bytes) {
			rcv_data(tmp);
		}
	}
	if (UART5->ISR & USART_ISR_ORE) {
		rcv.error |= MIDI_ERROR_OVERRUN;
		rcv_reset();
		UART5->ICR |= USART_ICR_ORECF;
	}
	barrier();
}

/* Initialise serial port and MIDI receiver */
void midi_uart_init(void)
{
	rcv.ri = RCVBUFMASK;
	UART5->CR1 |= USART_CR1_RXNEIE;
	UART5->BRR = (SYSTEMCORECLOCK / MIDI_BAUD);
	UART5->CR1 |= USART_CR1_UE | USART_CR1_RE;
	NVIC_SetPriority(UART5_IRQn, 3);
	/* NVIC_SetPriority(UART5_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); */
	NVIC_EnableIRQ(UART5_IRQn);
}
