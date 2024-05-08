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
#include "flash.h"
#include "timer.h"

/* Output port constants */
#define GATE1 GPIO_ODR_3
#define GATE2 GPIO_ODR_15
#define GATE3 GPIO_ODR_13
#define DINRS GPIO_ODR_1
#define DINCK GPIO_ODR_0
#define DINFL GPIO_ODR_2
#define OUTMASK (DINCK|DINRS|DINFL|GATE1|GATE2|GATE3)
const uint32_t out_pins[6U] = {
	DINCK, DINRS, DINFL, GATE1, GATE2, GATE3
};

uint32_t trig_start[6U];

/* Device config */
#define CONFIG_LENGTH	8U

// Return a bit mask for outputs matching provided condition flags
static uint32_t output_mask(uint32_t condition)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->flags & condition) {
			mask |= out_pins[i];
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	return mask;
}

// Disable any output configured as trigger or continue that has expired
static void output_expire(uint32_t nt)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t elap;
	uint32_t oset = GPIOC->ODR & OUTMASK;
	if (oset) {
		uint32_t mask = 0;
		do {
			if (oset & out_pins[i]) {
				// out is currently set
				out = &config.output[i];
				if (out->flags &
				    (SETTING_TRIG | SETTING_CONTINUE)) {
					elap = nt - trig_start[i];
					if (elap > config.triglen) {
						mask |= out_pins[i];
					}
				}
			}
			out = &config.output[i];
			++i;
		} while (i < SETTINGS_NROUTS);
		GPIOC->BRR = mask;
	}
}

// Update outputs in response to an ALL-OFF condition
static void output_alloff(void)
{
	GPIOC->BRR = output_mask(SETTING_NOTE);
}

// Turn on any outputs matching a note-on message
static void output_noteon(uint32_t note)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->flags & SETTING_NOTE) {
			if (out->note == note) {
				mask |= out_pins[i];
				trig_start[i] = Uptime;
			}
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	GPIOC->BSRR = mask;
	if (mask) {
		display_din_blink();
	}
}

// Turn off any outputs matching a note-off message
static void output_noteoff(uint32_t note)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->flags & SETTING_NOTE) {
			if (out->note == note) {
				mask |= out_pins[i];
			}
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	GPIOC->BRR = mask;
	if (IS_ENABLED(NOTE_OFF_BLINK)) {
		if (mask) {
			display_din_blink();
		}
	}
}

// Set any continue outputs as if they were matched with NOTE|TRIG
static void output_continue(void)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->flags & SETTING_CONTINUE) {
			mask |= out_pins[i];
			trig_start[i] = Uptime;
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	GPIOC->BSRR = mask;
}

// Set the lower 7 bits of divisor and/or offset
static void set_divlo(struct output_config *out, uint32_t value)
{
	if (out->flags & SETTING_CTRLDIV) {
		out->divisor = (out->divisor & (0x7f << 7)) | value;
	}
	if (out->flags & SETTING_CTRLOFT) {
		out->offset = (out->offset & (0x7f << 7)) | value;
	}
}

// Set the upper 7 bits of divisor and/or offset
static void set_divhi(struct output_config *out, uint32_t value)
{
	value <<= 7;
	if (out->flags & SETTING_CTRLDIV) {
		out->divisor = (out->divisor & 0x7f) | value;
	}
	if (out->flags & SETTING_CTRLOFT) {
		out->offset = (out->offset & 0x7f) | value;
	}
}

// Set output divisor and/or offset
static void set_divoft(struct output_config *out, uint32_t number,
		       uint32_t value)
{
	if (out->note > 31 && out->note < 64) {
		// Value is 14 bits in two parts
		if (out->note == number) {
			set_divlo(out, value);
		} else {
			set_divhi(out, value);
		}
	} else {
		if (out->flags & SETTING_CTRLDIV) {
			out->divisor = value;
		}
		if (out->flags & SETTING_CTRLOFT) {
			out->offset = value;
		}
	}
}

// Update outputs with configured with controller flag
static void output_controller(uint32_t number, uint32_t value)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->flags & (SETTING_CTRLDIV | SETTING_CTRLOFT)) {
			if (out->note == number
			    || (out->note > 31 && out->note < 64
				&& (out->note - 32U) == number)) {
				set_divoft(out, number, value);
			}
		}
		if (out->flags & SETTING_CTRL) {
			if (out->note == number) {
				if (value < 64) {
					// Switch Off
					mask |= out_pins[i] << 16;
				} else {
					// Switch On
					mask |= out_pins[i];
					trig_start[i] = Uptime;
				}
			}
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	GPIOC->BSRR = mask;
}

// Handle a MIDI mode message
static void mode_msg(uint32_t number)
{
	switch (number) {
	case MIDI_MODE_ALLOFF:
		output_alloff();
		break;
	case MIDI_MODE_OMNIOFF:
		config.mode = SETTING_OMNIOFF;
		output_alloff();
		break;
	case MIDI_MODE_OMNION:
		config.mode = SETTING_OMNION;
		output_alloff();
		break;
	default:
		break;
	}
}

// Handle a midi controller message
static void ctrl_msg(uint32_t number, uint32_t value)
{
	if (number > 121 && number < 128) {
		mode_msg(number);
	} else {
		output_controller(number, value);
	}
}

// Handle a channel message
static void channel_msg(struct midi_event *msg)
{
	uint32_t channel = msg->evt.raw.midi0 & MIDI_CHANNEL_MASK;
	if (channel == config.channel || config.mode == SETTING_OMNION) {
		uint32_t status = msg->evt.raw.midi0 & MIDI_STATUS_MASK;
		switch (status) {
		case MIDI_STATUS_NOTEON:
			output_noteon(msg->evt.raw.midi1);
			break;
		case MIDI_STATUS_NOTEOFF:
			output_noteoff(msg->evt.raw.midi1);
			break;
		case MIDI_STATUS_CONTROL:
			ctrl_msg(msg->evt.raw.midi1, msg->evt.raw.midi2);
			break;
		default:
			break;
		}
	}
}

// Update outputs in response to a start condition
static void output_start(void)
{
	// Ensure all clocked outputs are lowered before setting run/stop
	GPIOC->BRR = output_mask(SETTING_CLOCK | SETTING_RUNSTOP);
	GPIOC->BSRR = output_mask(SETTING_RUNSTOP);
	// Temp: set the uptime on output 1
	trig_start[1U] = Uptime;
	display_din_on();
}

// Update outputs in response to a stop message
static void output_stop(void)
{
	GPIOC->BRR = output_mask(SETTING_RUNSTOP);
	display_din_off();
}

static void rt_msg(struct midi_event *msg)
{
	switch (msg->evt.raw.midi0) {
	case MIDI_RT_CLOCK:
		timer_clock(msg);
		break;
	case MIDI_RT_START:
		timer_preroll();
		output_start();
		break;
	case MIDI_RT_CONTINUE:
		// Continue is not properly handled yet
		output_continue();
		break;
	case MIDI_RT_STOP:
		output_stop();
		timer.on = 0;
		break;
	case MIDI_RT_RESET:
		output_alloff();
		output_stop();
		midi_sysex_done(msg);
		break;
	default:
		break;
	}
}

// Handle a preset request
static void config_preset(uint32_t preset)
{
	if (preset < PRESETS_LEN) {
		settings_preset(preset);
		// Temp
		TIM2->ARR = config.delay;
	}
}

// Handle an output config update
static void config_output(uint8_t * cfg)
{
	if (cfg[1] < SETTINGS_NROUTS) {
		struct output_config *out = &config.output[cfg[1]];
		out->flags = cfg[2] | (cfg[3] << 7);
		out->divisor = cfg[4] | (cfg[5] << 7);
		out->offset = cfg[6] | (cfg[7] << 7);
		out->note = cfg[8];
	}
}

// Handle a general config update
static void config_general(uint8_t * cfg)
{
	config.delay = cfg[1] | (cfg[2] << 7) | (cfg[3] << 14) | (cfg[4] << 21);
	config.inertia = cfg[5];	// Inertia is specified in uptimes
	config.channel = cfg[6];
	config.mode = cfg[7];
	config.master = cfg[8];
	config.fusb = cfg[9] | (cfg[10] << 7) | (cfg[11] << 14);
	config.fmidi = cfg[12] | (cfg[13] << 7) | (cfg[14] << 14);
	config.triglen = (cfg[15] << 3);	// Convert triglen ms to uptimes
	// Temp
	TIM2->ARR = config.delay;
}

// Process a validated configuration request packet (temp)
static void config_message(uint8_t * cfg, uint32_t len)
{
	switch (cfg[0]) {
	case 0x01:
	case 0x02:
		// Ignore ACK/NACK
		break;
	case 0x03:
		if (len == 2) {
			config_preset(cfg[1]);
		}
		break;
	case 0x04:
		if (len == 16) {
			config_general(cfg);
		}
		break;
	case 0x05:
		if (len == 9) {
			config_output(cfg);
		}
		break;
	default:
		break;
	};
}

// Compute the expected CRC-7/MMC value on the received message
static uint32_t sysex_crc(struct midi_sysex_config *cfg, uint32_t len)
{
	CRC->CR |= CRC_CR_RESET;
	CRC->DR = SYSEX_INVID;
	uint32_t i = 0;
	while (i < len) {
		*(__IO uint8_t *) (__IO void *)(&CRC->DR) = cfg->data[i];
		++i;
	}
	return CRC->DR;
}

static void sysex_config(struct midi_sysex_config *cfg, uint32_t len)
{
	// Minimum message includes header, command and crc
	if (len > 5) {
		len -= 5;
		uint32_t crc = sysex_crc(cfg, len);
		if (crc == cfg->data[len]) {
			config_message(&cfg->data[0], len);
		}
	}
}

// Sysex ID is matched against build value in case ROM is damaged
static void sysex(struct midi_event *event)
{
	struct midi_sysex_config *cfg;
	uint32_t len = event->evt.raw.midi1;
	cfg = midi_sysex_buf(event);
	if (cfg != NULL) {
		if (cfg->idcfg == SYSEX_ID) {
			sysex_config(cfg, len);
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
			uint8_t cin = msg->evt.raw.header & MIDI_CIN_MASK;
			switch (cin) {
			case MIDI_CIN_EOX_3:	// special case
				sysex(msg);
				break;
			case MIDI_CIN_NOTE_ON:
			case MIDI_CIN_NOTE_OFF:
			case MIDI_CIN_CONTROL:
				channel_msg(msg);
				break;
			case MIDI_CIN_BYTE:
				rt_msg(msg);
				break;
			default:	// Ignore all others
				break;
			}
			midi_event_done();
		}
	} while (msg != NULL);
	if (lt != t) {
		output_expire(t);
		display_update(t);
	}
	lt = t;
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xaaaa;
}

void main(void)
{
	settings_init();
	timer_init();
	midi_event_init();
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xcccc;
}
