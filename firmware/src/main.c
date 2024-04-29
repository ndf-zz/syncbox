// SPDX-License-Identifier: MIT

/*
 * Main program
 *
 * preliminary "naive" hardware test
 */
#include "stm32f3xx.h"
#include "midi.h"
#include "midi_event.h"
#include "display.h"
#include "settings.h"

/* Output port constants */
#define GATE1 GPIO_ODR_3
#define GATE2 GPIO_ODR_15
#define GATE3 GPIO_ODR_13
#define DINRS GPIO_ODR_1
#define DINCK GPIO_ODR_0
#define DINFL GPIO_ODR_2

/* Device config */
#define SYSEX_ID	0x7d	// temp
#define CONFIG_LENGTH	8U

static uint32_t beat;

void note_on(uint8_t note)
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

void note_off(uint8_t note)
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

void all_off(void)
{
	GPIOC->BRR = GATE1 | GATE2 | GATE3;
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

void rt_msg(uint8_t msg)
{
	switch (msg) {
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
	case MIDI_RT_RESET:
		all_off();
		stop();
		break;
	default:
		break;
	}
}

void sysex(struct midi_event *event)
{
	struct midi_sysex_config *cfg;
	uint32_t len = event->evt.raw.midi1;
	if (len == CONFIG_LENGTH) {
		cfg = midi_sysex_buf(event);
		if (cfg != NULL) {
			if ((cfg->idcfg & SYSEX_IDMASK) == SYSEX_ID) {
				settings_sysex(cfg);
				BREAKPOINT(0x21);
			}
		}
	}
	midi_sysex_done(event);
}

// handle any pending updates
void system_update(void)
{
	struct midi_event *msg;
	static uint32_t lt = 0;
	uint32_t t = Uptime;
	do {
		msg = midi_event_poll();
		if (msg != NULL) {
			TRACEVAL(5U, msg->evt.val);
			uint8_t cin = msg->evt.raw.header & MIDI_CIN_MASK;
			switch (cin) {
			case MIDI_CIN_EOX_3:	// special case
				sysex(msg);
				break;
			case MIDI_CIN_NOTE_ON:
				note_on(msg->evt.raw.midi1);
				break;
			case MIDI_CIN_NOTE_OFF:
				note_off(msg->evt.raw.midi1);
				break;
			case MIDI_CIN_CONTROL:
				if (msg->evt.raw.midi1 == MIDI_MODE_ALLOFF) {
					all_off();
				}
				break;
			case MIDI_CIN_BYTE:
				rt_msg(msg->evt.raw.midi0);
				break;
			case MIDI_CIN_RESERVED_0:
				// likely uninitialised event
				BREAKPOINT(23U);
				break;
			default:	// Ignore all others
				break;
			}
			midi_event_done();
		}
	} while (msg != NULL);
	if (lt != t) {
		display_update(t);
	}
	lt = t;
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xaaaa;
}

void main(void)
{
	settings_init();
	midi_event_init();
        if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xcccc;

}
