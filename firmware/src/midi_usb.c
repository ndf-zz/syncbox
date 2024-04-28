// SPDX-License-Identifier: MIT

/*
 * USB-MIDI Class Device Interface
 */
#include "stm32f303xe.h"
#include "midi_usb.h"

// Initialise hardware and enable interrupts
void midi_usb_init(void)
{
        // Force USB re-enumeration
        GPIOA->BSRR = GPIO_BSRR_BR_12;
        uint32_t nm = GPIOA->MODER & ~GPIO_MODER_MODER12_Msk;
        GPIOA->MODER = nm | (0x1 << GPIO_MODER_MODER12_Pos);
        delay_ms(6);
        GPIOA->MODER = nm | (0x3 << GPIO_MODER_MODER12_Pos);
}
