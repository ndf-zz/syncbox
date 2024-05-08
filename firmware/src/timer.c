// SPDX-License-Identifier: MIT

/*
 * Reference clock timer
 *
 * This clock runs at 96ppq and manages triggering of all
 * outputs configured with a clock source.
 *
 */
#include "stm32f303xe.h"
#include "timer.h"
#include "settings.h"
#include "display.h"

// Import io pins from main [temp]
extern const uint32_t out_pins[6U];
extern uint32_t trig_start[6U];

// Global timer status
struct timer_state timer;

// Cached period calc
static uint32_t delinv;

// Set the next output register based on phase and config
static void update_nextout(void)
{
	struct output_config *out;
	uint32_t phase;
	uint32_t period;
	uint32_t setmark;
	uint32_t clrmark;
	uint32_t i = 0;
	timer.nextout = 0;
	do {
		out = &config.output[i];
		if (out->flags & SETTING_CLOCK) {
			period = out->divisor << 1;
			setmark = out->offset % period;
			clrmark = (out->divisor + out->offset) % period;
			phase = timer.phase % period;

			if (phase == setmark) {
				// Set output
				timer.nextout |= out_pins[i];
				if (out->flags & SETTING_TRIG) {
					// This has not yet happened - fudge
					trig_start[i] =
					    Uptime +
					    config.delay / SYSTEMTICKLEN;
				}
				// TODO: this requires attention
				if ((out->flags & SETTING_RUNMASK) && !timer.on) {
					timer.nextout &= ~out_pins[i];
				}
			} else if (phase == clrmark) {
				// outputs clear even if trig set
				timer.nextout |= (out_pins[i] << 16);
			}
		}
		++i;
	} while (i < SETTINGS_NROUTS);
}

// Timer update handler
void timer_update(void)
{
	// Update
	GPIOC->BSRR = timer.nextout;
	if (timer.phase % 96U == 0) {
		display_midi_blink();
	}
	// Prepare
	timer.phase++;
	update_nextout();

	// Clear interrupt flag
	TIM2->SR &= ~(TIM_SR_UIF);
}

// halt timer in preparation for a start message
void timer_preroll(void)
{
	TIM2->CR1 &= ~(TIM_CR1_CEN);
	timer.running = 0;
	timer.phase = 0;
	update_nextout();
}

// Generate a reset event and roll timer
static void timer_roll(void)
{
	timer.on = 1;
	if (!timer.running) {
		TIM2->CNT = 0;
		TIM2->CR1 |= TIM_CR1_CEN;
		TIM2->EGR = TIM_EGR_UG;
		timer.running = 1U;
	}
}

// Handle arrival of a midi timing message
void timer_clock(struct midi_event *event)
{
	static uint32_t lco;
	static uint32_t bc;
	uint32_t co = event->clock;

	if (timer.running) {
		if (bc > 1) {
			// temp: track rate and ignore phase
			uint32_t dt = SYSTEMTICKLEN * (co - lco);
			uint64_t dc = ((uint64_t) config.delay) << 2;
			uint32_t nv =
			    (uint32_t) ((31 * dc + dt + (1U << 6)) >> 7);
			TRACEVAL(2, nv);
			config.delay = nv;

			delinv = config.delay / SYSTEMTICKLEN;
			TIM2->ARR = config.delay;
			//GPIOC->BSRR = out_pins[1];
			//trig_start[1] = Uptime;
		}
	} else {
		timer_roll();
		bc = 0;
	}

	++bc;
	lco = co;
}

// Prepare timer interface
void timer_init(void)
{
	NVIC_SetPriority(TIM2_IRQn, PRIGROUP0 | PRISUB1);
	NVIC_EnableIRQ(TIM2_IRQn);

	// Reset timer peripheral 
	RCC->APB1RSTR |= (RCC_APB1RSTR_TIM2RST);
	RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM2RST);

	// Enale interrupt on event, shadow ARR register
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CR1 = TIM_CR1_ARPE;

	// Set initial delay and enable timer
	TIM2->ARR = config.delay;
	timer_roll();
}
