// SPDX-License-Identifier: MIT

/*
 * Low-level flash memory interface
 */
#ifndef FLASH_H
#define FLASH_H
#include "stm32f3xx.h"

#define FLASH_PAGESZ	2048UL
#define FLASH_WORDCOUNT	(FLASH_PAGESZ / sizeof(uint32_t))
#define FLASH_CODEPAGES	10UL
#define FLASH_DATAPAGES 246UL

struct flash_page {
	uint32_t word[FLASH_WORDCOUNT];
};
struct flash_memory {
	struct flash_page code[FLASH_CODEPAGES];	/* Program code */
	struct flash_page data[FLASH_DATAPAGES];	/* Sparse data */
};
#define FLASHMEM ((struct flash_memory *) FLASH_BASE)

/* Unlock flash memory for writing */
void flash_unlock(void);

/* Lock the flash to stop writing */
void flash_lock(void);

/* Program a single half-word to flash addr */
int32_t flash_program(uint32_t addr, uint16_t val);

/* Program a word at flash addr */
int32_t flash_word(uint32_t addr, uint32_t val);

/* Zero a single half-word at flash addr */
int32_t flash_zero(uint32_t addr);

/* Erase a page of flash memory containing addr */
int32_t flash_erase(uint32_t addr);

/* Write a block of len words from src to addr, updating CRC */
int32_t flash_block(uint32_t addr, uint32_t *src, uint32_t len);

/* Check and optionally reset option bytes */
void flash_set_options(void);

#endif /* FLASH_H */
