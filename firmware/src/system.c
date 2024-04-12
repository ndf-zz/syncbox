// SPDX-License-Identifier: MIT

/*
 * System Initialisation
 *
 * Includes default, reset, systick and fault handlers.
 */
#include "stm32f3xx.h"
#include "flash.h"

/* Exported globals */
uint32_t Version = SYSTEMVERSION;
uint32_t SystemID;
volatile uint32_t Uptime;

/* Values provided by the linker */
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

/* Function declarations */
void main(void);

/* Initialise hardware system */
void Reset_Handler(void)
{
	/* Relocate vector table at start of CCMRAM */
	SCB->VTOR = CCMDATARAM_BASE;
	barrier();

	/* Copy .data segment into RAM */
	uint32_t *dsrc = &_sidata;
	uint32_t *ddst = &_sdata;
	uint32_t *dend = &_edata;
	while(ddst < dend)
		*ddst++ = *dsrc++;

	/* Zero .bss */
	uint32_t *bdst = &_sbss;
	uint32_t *bend = &_ebss;
	while(bdst < bend)
		*bdst++ = 0UL;
	__DSB();

	/* Prepare system clock */
	if (IS_ENABLED(USE_HSE)) {
		/* Enable external clock */
		RCC->CR &= ~RCC_CR_HSEON;
		wait_for_bit_clr(RCC->CR, RCC_CR_HSERDY);
		RCC->CR |= RCC_CR_CSSON|RCC_CR_HSEBYP;
		RCC->CR |= RCC_CR_HSEON;
		wait_for_bit_set(RCC->CR, RCC_CR_HSERDY);
	
		/* Re-configure PLL for use with HSEx2 */
		RCC->CR &=~ RCC_CR_PLLON;
		wait_for_bit_clr(RCC->CR, RCC_CR_PLLRDY);
		RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV;
		RCC->CR |= RCC_CR_PLLON;
		wait_for_bit_set(RCC->CR, RCC_CR_PLLRDY);
		RCC->CFGR |= RCC_CFGR_SW_PLL;
		wait_for_bit_set(RCC->CFGR, RCC_CFGR_SWS_PLL);
	}

	/* Prepare MPU */

	/* Disable regions 0 & 1 */
	MPU->RBAR = MPU_RBAR_VALID_Msk | 0U;
	barrier();
	MPU->RASR = 0UL;
	barrier();
	MPU->RBAR = MPU_RBAR_VALID_Msk | 1U;
	barrier();
	MPU->RASR = 0UL;
	barrier();

	/* Region 2: CCMRAM r/o */
	MPU->RBAR = CCMDATARAM_BASE | MPU_RBAR_VALID_Msk | 2U;
	barrier();
	MPU->RASR = RASR_AP_RO | RASR_TSCB_SRAM | RASR_SZ_16K | RASR_ENABLE;
	barrier();

	/* Region 3: SRAM r/w+NX */
	MPU->RBAR = SRAM_BASE | MPU_RBAR_VALID_Msk | 3U;
	barrier();
	MPU->RASR = RASR_AP_RWNX | RASR_TSCB_SRAM | RASR_SZ_64K | RASR_ENABLE;
	barrier();

	/* Region 4: Flash mem r/w+NX */
	MPU->RBAR = FLASH_BASE | MPU_RBAR_VALID_Msk | 4U;
	barrier();
	MPU->RASR = RASR_AP_RWNX | RASR_TSCB_FLASH | RASR_SZ_512K | RASR_ENABLE;
	barrier();

	/* Region 5: Loader, program text & ro data r/o+NX */
	MPU->RBAR = FLASH_BASE | MPU_RBAR_VALID_Msk | 5U;
	barrier();
	MPU->RASR = RASR_AP_RONX | RASR_TSCB_FLASH | RASR_SZ_32K | RASR_ENABLE;
	barrier();

	/* Region 6: Flash ram alias no access */
	MPU->RBAR = FLASH_ALIAS | MPU_RBAR_VALID_Msk | 6U;
	barrier();
	MPU->RASR = RASR_AP_NONE | RASR_TSCB_FLASH | RASR_SZ_512K | RASR_ENABLE;
	barrier();

	/* Region 7: Stack guard no-access */
	MPU->RBAR = STACK_BOTTOM | MPU_RBAR_VALID_Msk | 7U;
	barrier();
	MPU->RASR = RASR_AP_NONE | RASR_TSCB_SRAM | RASR_SZ_32B | RASR_ENABLE;
	barrier();

	/* Enable MPU & MemManage fault handler */
	ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);
	barrier();

	/* Enable peripheral clocks */
	RCC->AHBENR |= RCC_AHBENR_CRCEN |
		RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;
	RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
	barrier();

	/* Compute a hardware-unique identifier */
	CRC->DR = UID->XY;
	CRC->DR = UID->LOTWAF;
	CRC->DR = UID->LOT;
	SystemID = CRC->DR;
	CRC->INIT = SystemID;

	/* I/O Port configuration */

	/* LED & SWD, switch rest to analog */
        GPIOA->MODER = 0xebfffff5;
	/* Configure gate outputs and uart, switch rest to analog */
	GPIOC->AFR[1] = 0x00050000UL;
	GPIOC->MODER = 0x76ffff55;
	/* TBC: PD2 MIDI UART */
	GPIOD->AFR[0] = 0x00000500UL;
	GPIOD->MODER = 0x00000020UL;
	barrier();

	/* ensure option bytes are correctly set */
	flash_set_options();
	barrier();

	/* Update interrupt priority grouping field */
	__NVIC_SetPriorityGrouping(0x5U);	/* 0bxx.yy 4/4 split */

	/* Configure the SysTick timer at 1ms */
	Uptime = 0UL;
	SysTick->LOAD = SYSTEMTICKLEN - 1UL;
	NVIC_SetPriority(SysTick_IRQn, 0x7U);	/* grp=1 sub=3 */
	SysTick->VAL = 0UL;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
			SysTick_CTRL_TICKINT_Msk |
			 SysTick_CTRL_ENABLE_Msk;
	__DSB();

	main();

	while(1);
}

/* Start IWDG if configured */
void watchdog_enable(void)
{
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xccccU;
}

/* Reset watchdog timer */
void watchdog_kick(void)
{
	if (IS_ENABLED(USE_IWDG))
		IWDG->KR = 0xaaaaU;
}

/* Wait for next interupt - in place */
inline void wait_for_interrupt(void)
{
	__DSB();
	__WFI();
}

/* Busy wait on delay CPU clocks using SysTick timer */
/* NONO NON NO NON NO - this is all wrong !@@ */
/* void spin_wait(uint32_t delay)
{
	if (delay < 10U)
		return;
	uint32_t target = SysTick->VAL - delay;
	while (delay > SYSTEMTICKLEN)
		delay += SYSTEMTICKLEN;
        uint32_t oft;
        do {
                oft = SysTick->VAL - tgt;
        } while (oft <= delay);
} */

/* ARM SysTick Handler */
void SysTick_Handler(void)
{
	Uptime++;
}

/* Default handler */
void Default_Handler(void)
{
	BREAKPOINT(254);
	while (1);
}

/* Handlers for System-critical faults */
void NMI_Handler(void)
{
	BREAKPOINT(0);
	while (1);
}

void HardFault_Handler(void)
{
	BREAKPOINT(1);
	while (1);
}

void MemManage_Handler(void)
{
	BREAKPOINT(2);
	while (1);
}

void BusFault_Handler(void)
{
	BREAKPOINT(3);
	while (1);
}

void UsageFault_Handler(void)
{
	BREAKPOINT(4);
	while (1);
}
