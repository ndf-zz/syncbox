// SPDX-License-Identifier: MIT

/*
 * Main program
 *
 * preliminary "naive" hardware test
 */
#include "stm32f3xx.h"
#include "midi.h"
#include "midi_event.h"
#include "midi_uart.h"

/* Output port constants */
#define GATE1 GPIO_ODR_3
#define GATE2 GPIO_ODR_15
#define GATE3 GPIO_ODR_13
#define DINRS GPIO_ODR_1
#define DINCK GPIO_ODR_0
#define DINFL GPIO_ODR_2
#define DACT GPIO_ODR_0
#define MACT GPIO_ODR_1

void note_on(uint32_t note)
{
	GPIOC->BSRR = GATE1;
	switch (note) {
	case 61:
		GPIOC->BSRR = GATE2;
		break;
	case 62:
		GPIOC->BSRR = GATE3;
		break;
	case 63:
		GPIOC->BSRR = DINFL;
		break;
	default:
		break;
	}
}

void note_off(uint32_t note)
{
	GPIOC->BRR = GATE1;
	switch (note) {
	case 61:
		GPIOC->BRR = GATE2;
		break;
	case 62:
		GPIOC->BRR = GATE3;
		break;
	case 63:
		GPIOC->BRR = DINFL;
		break;
	default:
		break;
	}
}

void start(void)
{
	GPIOC->BSRR = DINRS | (DINCK << 16U);
	GPIOA->BSRR = DACT;
}

void stop(void)
{
	GPIOC->BRR = DINRS;
	GPIOA->BRR = DACT;
}

void tick(void)
{
	uint32_t cv = GPIOC->IDR & DINCK;
	GPIOC->BSRR = (~cv & DINCK) | (cv << 16);
}

void cont(void)
{
	/* set the continue output */
	/* GPIOC->BSRR = DINFL; */
	start();
}

void rt_msg(uint32_t msg)
{
	switch(msg) {
	case MIDI_RT_CLOCK:
		tick();
		break;
	case MIDI_RT_START:
		start();
		break;
	case MIDI_RT_CONTINUE:
		cont();
		break;
	case MIDI_RT_STOP:
		stop();
		break;
	default:
		break;
	}
}

void ch_msg(uint32_t msg)
{
	uint32_t chan = msg>>24U;
	uint32_t note = (msg & 0xff00U) >> 8U;
	switch (chan & MIDI_STATUS_MASK) {
	case MIDI_STATUS_NOTEON:
		note_on(note);
		break;
	case MIDI_STATUS_NOTEOFF:
		note_off(note);
		break;
	default:
		break;
	}
}

void main(void)
{
	midi_uart_init();
	uint32_t lt = 0U;
	uint32_t msg;
	uint32_t tmp;
	volatile uint32_t pri = __NVIC_GetPriorityGrouping();
	GPIOC->ODR = pri;
	do {
		wait_for_interrupt();
		msg = midi_poll();
		tmp = msg>>24U;
		if (tmp) {
			if ((tmp & MIDI_RT_CLOCK) == MIDI_RT_CLOCK) {
				rt_msg(tmp);
			} else {
				ch_msg(msg);
			}
		}
		if (lt != Uptime) {
			lt = Uptime;
		}
	} while (1);
}
