// SPDX-License-Identifier: MIT

/*
 * MIDI USB Device Interface
 */
#ifndef MIDI_USB_H
#define MIDI_USB_H
#include <stdint.h>

// Initialise hardware and enable interrupt
void midi_usb_init(void);

// Return non-zero if device active sense has expired
uint32_t midi_usb_sense(void);

#endif /* MIDI_USB_H */
