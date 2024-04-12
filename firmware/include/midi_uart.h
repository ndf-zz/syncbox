// SPDX-License-Identifier: MIT

/*
 * MIDI UART Receiver Interface
 */
#ifndef _MIDI_UART_H
#define _MIDI_UART_H

/* Check for expiry of active sense */
void midi_uart_sense(void);

/* Prepare hardware and start data reception */
void midi_uart_init(void);

#endif /* _MIDI_UART_H */
