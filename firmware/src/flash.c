// SPDX-License-Identifier: MIT

/*
 * Low-level blocking flash memory interface
 */
#include "stm32f3xx.h"
#include "flash.h"

/* Record error and clear flags */
void flash_regerr(uint32_t flags)
{
	if (flags) {
		/* -- */
		BREAKPOINT(9);
		FLASH->SR = flags;
	}
}

/* Wait for a flash operation to complete and barrier optimisation */
void flash_wait(void)
{
	wait_for_bit_clr(FLASH->SR, FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP)
		FLASH->SR = FLASH_SR_EOP;
	flash_regerr(FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGERR));
	barrier();
}

/* Unlock flash memory */
void flash_unlock(void)
{
	if ((FLASH->CR & FLASH_CR_LOCK) != 0UL) {
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
	}
	wait_for_bit_clr(FLASH->CR, FLASH_CR_LOCK);
	barrier();
}

/* Unlock flash option bytes */
void flash_ob_unlock(void)
{
	if ((FLASH->CR & FLASH_CR_OPTWRE) == 0UL) {
		FLASH->OPTKEYR = FLASH_OPTKEY1;
		FLASH->OPTKEYR = FLASH_OPTKEY2;
	}
	wait_for_bit_set(FLASH->CR, FLASH_CR_OPTWRE);
	barrier();
}

/* Lock flash memory */
void flash_lock(void)
{
	FLASH->CR |= FLASH_CR_LOCK;
}

/* Lock flash options */
void flash_ob_lock(void)
{
	FLASH->CR &= (~FLASH_CR_OPTWRE);
}

/* Program a single half-word to flash addr and wait for completion */
int32_t flash_program(uint32_t addr, uint16_t val)
{
	flash_unlock();
	flash_wait();
	FLASH->CR = FLASH_CR_PG;
	barrier();
	*((volatile uint16_t *)addr) = val;
	__DSB();		/* or read back value */
	flash_wait();
	return *((volatile uint16_t *)addr) != val;
}

/* Program a word at flash addr */
int32_t flash_word(uint32_t addr, uint32_t val)
{
	return flash_program(addr, val & 0xffffU) |
	    flash_program(addr + 2, (uint16_t) (val >> 16UL));
}

/* Zero a single half-word at flash addr */
int32_t flash_zero(uint32_t addr)
{
	return flash_program(addr, 0U);
}

/* Erase the flash page containing addr */
int32_t flash_erase(uint32_t addr)
{
	flash_unlock();
	flash_wait();
	FLASH->CR = FLASH_CR_PER;
	barrier();
	FLASH->AR = addr;
	barrier();
	FLASH->CR |= FLASH_CR_STRT;
	flash_wait();
	return *((volatile uint16_t *)addr) != 0xffffU;
}

/* Write a block of len words from src to addr */
int32_t flash_block(uint32_t addr, uint32_t * src, uint32_t len)
{
	do {
		CRC->DR = *src;
		if (flash_word(addr, *src++))
			break;
		addr += 4;
		len--;
	} while (len);
	return len == 0U;
}

/* Force reload of option byte */
void flash_ob_reload(void)
{
	__DSB();
	SET_BIT(FLASH->CR, FLASH_CR_OBL_LAUNCH);
	flash_wait();
}

/* Check and optionally reset option bytes */
void flash_set_options(void)
{
	if (FLASH->OBR != 0x4446bf00UL) {
		flash_unlock();
		flash_ob_unlock();
		SET_BIT(FLASH->CR, FLASH_CR_OPTER);
		SET_BIT(FLASH->CR, FLASH_CR_STRT);
		__DSB();
		flash_wait();
		CLEAR_BIT(FLASH->CR, FLASH_CR_OPTER);
		SET_BIT(FLASH->CR, FLASH_CR_OPTPG);
		/* Read protection: Level 0 */
		OB->RDP = 0xaaU;
		__DSB();
		flash_wait();
		/* Enable SRAM hardware parity check, SW watchdog, nBOOT1 */
		OB->USER = 0xbfU;
		__DSB();
		flash_wait();
		OB->Data0 = 0x46U;
		__DSB();
		flash_wait();
		OB->Data1 = 0x44U;
		__DSB();
		flash_wait();
		/* No write protect on flash memory */
		OB->WRP0 = 0xffU;
		__DSB();
		flash_wait();
		OB->WRP1 = 0xffU;
		__DSB();
		flash_wait();
		OB->WRP2 = 0xffU;
		__DSB();
		flash_wait();
		OB->WRP3 = 0xffU;
		__DSB();
		flash_wait();
		CLEAR_BIT(FLASH->CR, FLASH_CR_OPTPG);
		flash_ob_lock();
		flash_lock();
		flash_ob_reload();
	}
}
