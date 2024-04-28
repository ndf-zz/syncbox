// SPDX-License-Identifier: MIT

/*
 * MIDI UART Interface
 *
 * Receive bytes from UART interface and pass
 * on to midi event interface.
 *
 */
#include "stm32f3xx.h"
#include "midi_event.h"
#include "midi_uart.h"

#define MIDI_BAUD              31250U

// Receive byte from serial port
void UART5_IRQHandler(void)
{
	uint32_t tmp = UART5->RDR;
	if (UART5->ISR & (USART_ISR_FE | USART_ISR_NE)) {
		// Assume byte was status
		midi_reset(MIDI_CABLE_UART);
		UART5->ICR |= (USART_ICR_FECF | USART_ICR_NCF);
	} else {
		midi_receive(MIDI_CABLE_UART, tmp);
	}
	if (UART5->ISR & USART_ISR_ORE) {
		// Assume lost byte would have been status
		midi_reset(MIDI_CABLE_UART);
		UART5->ICR |= USART_ICR_ORECF;
	}
}

// Initialise hardware and enable receive interrupt
void midi_uart_init(void)
{
	UART5->CR1 |= USART_CR1_RXNEIE;
	UART5->BRR = (SYSTEMCORECLOCK / MIDI_BAUD);
	UART5->CR1 |= USART_CR1_UE | USART_CR1_RE;
	NVIC_SetPriority(UART5_IRQn, PRIGROUP2 | PRISUB1);
	NVIC_EnableIRQ(UART5_IRQn);
}
