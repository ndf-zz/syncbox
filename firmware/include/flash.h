// SPDX-License-Identifier: MIT

/*
 * Low-level flash memory interface
 *
 * See Readme.md and flash_plan.svg for layout
 */
#ifndef FLASH_H
#define FLASH_H
#include "stm32f3xx.h"
#include "settings.h"

#define FLASH_PAGESZ		2048U
#define FLASH_WORDCOUNT		(FLASH_PAGESZ / sizeof(uint32_t))
#define FLASH_LOADPAGES 	1U
#define FLASH_CODEPAGES		8U
#define FLASH_PRESETPAGES 	7U
#define FLASH_JNLPAGES 		246UL

struct flash_page {
	uint32_t word[FLASH_WORDCOUNT];
};
struct flash_memory {
	struct flash_page loader;			// Loader
	struct flash_page application[FLASH_CODEPAGES];	// Application code
	struct flash_page options[FLASH_PRESETPAGES];	// ROM Options
	struct flash_page journal[FLASH_JNLPAGES];	// Settings
};
#define FLASHMEM ((struct flash_memory *) FLASH_BASE)
#define OPTION ((struct option_struct *)(&FLASHMEM->options[0]))

// Unlock flash memory for writing
void flash_unlock(void);

// Lock the flash to stop writing
void flash_lock(void);

// Program a single half-word to flash addr
int32_t flash_program(uint32_t addr, uint16_t val);

// Program a word at flash addr
int32_t flash_word(uint32_t addr, uint32_t val);

// Zero a single half-word at flash addr
int32_t flash_zero(uint32_t addr);

// Erase a page of flash memory containing addr
int32_t flash_erase(uint32_t addr);

// Write a block of len words from src to addr, updating CRC
int32_t flash_block(uint32_t addr, uint32_t * src, uint32_t len);

// Check and optionally reset option bytes
void flash_set_options(void);

#endif // FLASH_H
