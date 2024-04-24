// SPDX-License-Identifier: MIT

/*
 * MIDI UART Device Interface
 */
#ifndef MIDI_UART_H
#define MIDI_UART_H
#include <stdint.h>

// Global sysex packet buffer
extern uint8_t midi_uart_sysbuf[];

// Initialise hardware and enable receive interrupt
void midi_uart_init(void);

// Return non-zero if device active sense has expired
uint32_t midi_uart_sense(void);

#endif // MIDI_UART_H
