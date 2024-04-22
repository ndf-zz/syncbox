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
#include "display.h"

/* Output port constants */
#define GATE1 GPIO_ODR_3
#define GATE2 GPIO_ODR_15
#define GATE3 GPIO_ODR_13
#define DINRS GPIO_ODR_1
#define DINCK GPIO_ODR_0
#define DINFL GPIO_ODR_2

static uint32_t beat;

void note_on(uint32_t note)
{
	display_din_blink();
	switch (note) {
	case 0x3c:
		GPIOC->BSRR = GATE1;
		break;
	case 0x3d:
		GPIOC->BSRR = GATE2;
		break;
	case 0x3e:
		GPIOC->BSRR = GATE3;
		break;
	default:
		break;
	}
}

void note_off(uint32_t note)
{
	switch (note) {
	case 0x3c:
		GPIOC->BRR = GATE1;
		break;
	case 0x3d:
		GPIOC->BRR = GATE2;
		break;
	case 0x3e:
		GPIOC->BRR = GATE3;
		break;
	default:
		break;
	}
}

void start(void)
{
	GPIOC->BSRR = DINRS | (DINCK << 16U);
	display_din_on();
	beat = 0;
}

void stop(void)
{
	GPIOC->BRR = DINRS;
	display_din_off();
}

void tick(void)
{
	uint32_t cv = GPIOC->IDR & DINCK;
	GPIOC->BSRR = (~cv & DINCK) | (cv << 16);
	if ((beat % 24) == 0) {
		display_midi_blink();
	}
	beat++;
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
        /* Force USB re-enumeration if connected */
        GPIOA->BSRR = GPIO_BSRR_BR_12;
        uint32_t nm = GPIOA->MODER & ~GPIO_MODER_MODER12_Msk;
        GPIOA->MODER = nm | (0x1 << GPIO_MODER_MODER12_Pos);
	uint32_t nt = Uptime;
	while(Uptime - nt < 6);
	GPIOA->MODER = nm | (0x3 << GPIO_MODER_MODER12_Pos);

	midi_uart_init();
	uint32_t lt = 0U;
	uint32_t msg;
	uint32_t tmp;
	do {
		wait_for_interrupt();
		uint32_t t = Uptime;
		msg = midi_poll();
		tmp = msg>>24U;
		if (tmp) {
			if ((tmp & MIDI_RT_CLOCK) == MIDI_RT_CLOCK) {
				rt_msg(tmp);
			} else {
				ch_msg(msg);
			}
		}
		if (lt != t) {
			display_update(t);
		}
		lt = t;
	} while (1);
}
