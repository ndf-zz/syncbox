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

/* Output port constants */
#define GATE1 GPIO_ODR_3
#define GATE2 GPIO_ODR_15
#define GATE3 GPIO_ODR_13
#define DINRS GPIO_ODR_1
#define DINCK GPIO_ODR_0
#define DINFL GPIO_ODR_2
static const uint32_t out_pins[6U] = {
	GPIO_ODR_0, GPIO_ODR_1, GPIO_ODR_2,
	GPIO_ODR_3, GPIO_ODR_15, GPIO_ODR_13
};

/* Device config */
#define CONFIG_LENGTH	8U

static uint32_t beat;
static uint32_t running;
static uint32_t cpulse;

// Return a bit set mask for the desired output source mask
static uint32_t output_mask(uint32_t condition)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->source & condition) {
			mask |= out_pins[i];
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	return mask;
}

// Process an ALL-OFF condition
static void all_off(void)
{
	GPIOC->BRR = output_mask(SETTING_NOTE);
}

// Process a note-on midi message
static void note_on(uint32_t note)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->source & SETTING_NOTE) {
			if (out->note == note) {
				mask |= out_pins[i];
			}
		}
		++i;
	} while (i < SETTINGS_NROUTS);
	GPIOC->BSRR = mask;
	if (mask) {
		display_din_blink();
	}
}

static void note_off(uint32_t note)
{
	struct output_config *out;
	uint32_t i = 0;
	uint32_t mask = 0;
	do {
		out = &config.output[i];
		if (out->source & SETTING_NOTE) {
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

// Handle a MIDI mode message
static void mode_message(uint32_t number)
{
	switch (number) {
	case MIDI_MODE_ALLOFF:
		all_off();
		break;
	case MIDI_MODE_OMNIOFF:
		config.mode = SETTING_OMNIOFF;
		all_off();
		break;
	case MIDI_MODE_OMNION:
		config.mode = SETTING_OMNION;
		all_off();
		break;
	default:
		break;
	}
}

static void set_divisor(struct output_config *out, uint32_t value)
{
	if (out->source & SETTING_CTRLDIV)
		out->divisor = value;
}

static void set_offset(struct output_config *out, uint32_t value)
{
	if (out->source & SETTING_CTRLOFT)
		out->offset = value;
}

static void set_divoft(struct output_config *out, uint32_t value)
{
	set_divisor(out, value);
	set_offset(out, value);
}

static void set_divlo(struct output_config *out, uint32_t value)
{
	if (out->source & SETTING_CTRLDIV) {
		out->divisor = (out->divisor & (0x7f << 7)) | value;
	}
	if (out->source & SETTING_CTRLOFT) {
		out->offset = (out->offset & (0x7f << 7)) | value;
	}
}

static void set_divhi(struct output_config *out, uint32_t value)
{
	value <<= 7;
	if (out->source & SETTING_CTRLDIV) {
		out->divisor = (out->divisor & 0x7f) | value;
	}
	if (out->source & SETTING_CTRLOFT) {
		out->offset = (out->offset & 0x7f) | value;
	}
}

// Update output divisor or offset - number has already been matched to output
static void update_divoft(struct output_config *out, uint32_t number,
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
		set_divoft(out, value);
	}
}

// Receive a divisor / offset update
static void divoft(uint32_t number, uint32_t value)
{
	struct output_config *out;
	uint32_t i = 0;
	do {
		out = &config.output[i];
		if (out->source & (SETTING_CTRLDIV | SETTING_CTRLOFT)) {
			if (out->note == number
			    || (out->note > 31 && out->note < 64
				&& (out->note - 32U) == number)) {
				update_divoft(out, number, value);
			}
		}
		++i;
	} while (i < SETTINGS_NROUTS);
}

// Handle a controller message
static void controller(uint32_t number, uint32_t value)
{
	if (number > 121 && number < 128) {
		mode_message(number);
	} else {
		divoft(number, value);
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
			note_on(msg->evt.raw.midi1);
			break;
		case MIDI_STATUS_NOTEOFF:
			note_off(msg->evt.raw.midi1);
			break;
		case MIDI_STATUS_CONTROL:
			controller(msg->evt.raw.midi1, msg->evt.raw.midi2);
			break;
		default:
			break;
		}
	}
}

static void start(void)
{
	// Ensure all clocked outputs are lowered before setting run/stop
	GPIOC->BRR = output_mask(SETTING_CLOCK | SETTING_RUNSTOP);
	GPIOC->BSRR = output_mask(SETTING_RUNSTOP);
	running = 1U;

	// flag start on display
	display_din_on();
}

static void stop(void)
{
	// No change is made to timer state or clock outputs
	GPIOC->BRR = output_mask(SETTING_RUNSTOP);
	running = 0;

	// flag stop on display
	display_din_off();
}

// De-assert outputs configured as continue
static void discontinue(void)
{
	GPIOC->BRR = output_mask(SETTING_CONTINUE);
}

static void docontinue(void)
{
	// Assert any outputs configured as continue and set an expiry flag
	GPIOC->BSRR = output_mask(SETTING_CONTINUE);
	cpulse = 2U;
}

static void tick(void)
{
	uint32_t cv = GPIOC->IDR & DINCK;
	GPIOC->BSRR = (~cv & DINCK) | (cv << 16);
	if ((beat % 24) == 0) {
		display_midi_blink();
	}
	if (cpulse) {
		--cpulse;
		if (cpulse == 0) {
			discontinue();
		}
	}
	if (running)
		beat++;
}

static void rt_msg(struct midi_event *msg)
{
	switch (msg->evt.raw.midi0) {
	case MIDI_RT_CLOCK:
		tick();
		break;
	case MIDI_RT_START:
		// reset timer state [tbc]
		beat = 0;
		start();
		break;
	case MIDI_RT_CONTINUE:
		docontinue();
		if (!running) {
			// load cached timer phase (tbc)
			start();
		}
		break;
	case MIDI_RT_STOP:
		stop();
		break;
	case MIDI_RT_RESET:
		all_off();
		stop();
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
	}
}

// Handle an output config update
static void config_output(uint8_t *cfg)
{
	if (cfg[1] < SETTINGS_NROUTS) {
		struct output_config *out = &config.output[cfg[1]];
		out->source = cfg[2];
		out->divisor = cfg[3] | (cfg[4]<<7);
		out->offset = cfg[5] | (cfg[6]<<7);
		out->note = cfg[7];
	}
}

// Handle a general config update
static void config_general(uint8_t *cfg)
{
	config.delay = cfg[1] | (cfg[2]<<7) | (cfg[3]<<14) | (cfg[4]<<21);
	config.inertia = cfg[5];
	config.channel = cfg[6];
	config.mode = cfg[7];
	config.master = cfg[8];
	config.fusb = cfg[9] | (cfg[10]<<7) | (cfg[11]<<14);
	config.fmidi = cfg[12] | (cfg[13]<<7) | (cfg[14]<<14);
	config.reserved = cfg[15];
}

// Process a validated configuration request packet
static void config_message(uint8_t *cfg, uint32_t len)
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
		if (len == 8) {
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
		*(__IO uint8_t *)(__IO void *)(&CRC->DR) = cfg->data[i];
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

static void sysex(struct midi_event *event)
{
	struct midi_sysex_config *cfg;
	uint32_t len = event->evt.raw.midi1;
	cfg = midi_sysex_buf(event);
	if (cfg != NULL) {
		if (cfg->idcfg == OPTION->sysid) {
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
			TRACEVAL(5U, msg->evt.val);
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
		display_update(t);
	}
	lt = t;
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xaaaa;
}

void main(void)
{
	settings_init();
	//timer_init();
	midi_event_init();
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xcccc;

}
