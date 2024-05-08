// SPDX-License-Identifier: MIT

/*
 * USB-MIDI Class Device Interface
 */
#include "stm32f303xe.h"
#include "flash.h"
#include "settings.h"
#include "usb.h"
#include "midi_usb.h"

// Initialise hardware and enable interrupts
void midi_usb_init(void)
{
	// Check for a sane-ish USB config
	if (OPTION->usb.device.bLength == USB_DEVLEN
	    && OPTION->usb.device.bDescriptorType == USB_DEVICE) {
		// Force USB re-enumeration
		GPIOA->BSRR = GPIO_BSRR_BR_12;
		uint32_t nm = GPIOA->MODER & ~GPIO_MODER_MODER12_Msk;
		GPIOA->MODER = nm | (0x1 << GPIO_MODER_MODER12_Pos);
		delay_ms(6);
		GPIOA->MODER = nm | (0x3 << GPIO_MODER_MODER12_Pos);
	}
}
