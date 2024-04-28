// SPDX-License-Identifier: MIT

/*
 * MIDI UART Device Interface
 */
#ifndef MIDI_UART_H
#define MIDI_UART_H
#include <stdint.h>

// Initialise hardware and enable receive interrupt
void midi_uart_init(void);

#endif // MIDI_UART_H
