// SPDX-License-Identifier: MIT

/*
 * System Initialisation
 *
 * Includes default, reset, systick and fault handlers.
 */
#include "stm32f3xx.h"
#include "flash.h"

// Exported globals
uint32_t SystemID;
volatile uint32_t Uptime;

// Values provided by linker
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

// Initialise HSE and PLL
static void setup_clock(void)
{
	// Enable external clock
	RCC->CR &= ~RCC_CR_HSEON;
	wait_for_bit_clr(RCC->CR, RCC_CR_HSERDY);
	RCC->CR |= RCC_CR_CSSON | RCC_CR_HSEBYP;
	RCC->CR |= RCC_CR_HSEON;
	wait_for_bit_set(RCC->CR, RCC_CR_HSERDY);

	// Configure PLL to generate 48MHz for USB
	RCC->CR &= ~RCC_CR_PLLON;
	wait_for_bit_clr(RCC->CR, RCC_CR_PLLRDY);
	RCC->CFGR |= RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_USBPRE_DIV1;
	RCC->CR |= RCC_CR_PLLON;
	wait_for_bit_set(RCC->CR, RCC_CR_PLLRDY);

	// Switch system clock to HSE
	RCC->CFGR |= RCC_CFGR_SW_HSE;
	wait_for_bit_set(RCC->CFGR, RCC_CFGR_SWS_HSE);
}

// Prepare MPU
static void setup_mpu(void)
{
	// Disable regions 0 & 1
	MPU->RBAR = MPU_RBAR_VALID_Msk | 0;
	barrier();
	MPU->RASR = 0;
	barrier();
	MPU->RBAR = MPU_RBAR_VALID_Msk | 1U;
	barrier();
	MPU->RASR = 0;
	barrier();

	// Region 2: CCMRAM r/o
	MPU->RBAR = CCMDATARAM_BASE | MPU_RBAR_VALID_Msk | 2U;
	barrier();
	MPU->RASR = RASR_AP_RO | RASR_TSCB_SRAM | RASR_SZ_16K | RASR_ENABLE;
	barrier();

	// Region 3: SRAM r/w+NX
	MPU->RBAR = SRAM_BASE | MPU_RBAR_VALID_Msk | 3U;
	barrier();
	MPU->RASR = RASR_AP_RWNX | RASR_TSCB_SRAM | RASR_SZ_64K | RASR_ENABLE;
	barrier();

	// Region 4: Flash mem r/w+NX
	MPU->RBAR = FLASH_BASE | MPU_RBAR_VALID_Msk | 4U;
	barrier();
	MPU->RASR = RASR_AP_RWNX | RASR_TSCB_FLASH | RASR_SZ_512K | RASR_ENABLE;
	barrier();

	// Region 5: Loader, program text & reset data r/o+NX
	MPU->RBAR = FLASH_BASE | MPU_RBAR_VALID_Msk | 5U;
	barrier();
	MPU->RASR = RASR_AP_RONX | RASR_TSCB_FLASH | RASR_SZ_32K | RASR_ENABLE;
	barrier();

	// Region 6: Flash ram alias no access
	MPU->RBAR = FLASH_ALIAS | MPU_RBAR_VALID_Msk | 6U;
	barrier();
	MPU->RASR = RASR_AP_NONE | RASR_TSCB_FLASH | RASR_SZ_512K | RASR_ENABLE;
	barrier();

	// Region 7: Stack guard no-access
	MPU->RBAR = STACK_BOTTOM | MPU_RBAR_VALID_Msk | 7U;
	barrier();
	MPU->RASR = RASR_AP_NONE | RASR_TSCB_SRAM | RASR_SZ_32B | RASR_ENABLE;
	barrier();

	// Enable MPU & MemManage fault handler
	ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk);

}

// Prepare .data and .bss
static void mem_init(void)
{
	// Copy .data segment into RAM
	uint32_t *dsrc = &_sidata;
	uint32_t *ddst = &_sdata;
	uint32_t *dend = &_edata;
	while (ddst < dend)
		*ddst++ = *dsrc++;

	// Zero .bss
	uint32_t *bdst = &_sbss;
	uint32_t *bend = &_ebss;
	while (bdst < bend)
		*bdst++ = 0;
	__DSB();
}

// Enable peripheral clocks
static void enable_peripherals(void)
{
	RCC->AHBENR |= RCC_AHBENR_CRCEN | RCC_AHBENR_GPIOAEN |
	    RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN |
	    RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOFEN;
	RCC->APB1ENR |= RCC_APB1ENR_UART5EN | RCC_APB1ENR_TIM2EN;
	barrier();
}

// Compute a hardware-unique identifier
static void generate_sysid(void)
{
	CRC->DR = UID->XY;
	CRC->DR = UID->LOTWAF;
	CRC->DR = UID->LOT;
	SystemID = CRC->DR;
}

// Reset handler - prepare system
void system_init(void)
{
	// Relocate vector table at start of CCMRAM
	SCB->VTOR = CCMDATARAM_BASE;

	mem_init();
	setup_clock();
	enable_peripherals();
	generate_sysid();

	// ensure stysem id and option bytes are correctly set
	flash_set_options();

	// I/O Port configuration
	GPIOA->MODER = 0xebfffff5;	// DA, MA, SWD, SENSE
	GPIOB->MODER = 0xffffffbf;	// SWO
	GPIOC->MODER = 0x76ffff55;	// CK, RS, FL, G1, G2, G3
	GPIOD->AFR[0] = 0x00000500;	// PD2 -> Uart5 RX
	GPIOD->MODER = 0xffffffef;	// MIDI
	GPIOF->MODER = 0xffffffff;	// Unused

	setup_mpu();

	// Update interrupt priority grouping field
	NVIC_SetPriorityGrouping(PRIGROUP_4_4);

	// Set PendSV and SVCall to lowest priority
	NVIC_SetPriority(PendSV_IRQn, PRIGROUP3 | PRISUB2);
	NVIC_SetPriority(SVCall_IRQn, PRIGROUP3 | PRISUB2);
	NVIC_SetPriority(SysTick_IRQn, PRIGROUP1 | PRISUB1);

	// Configure the SysTick timer at 1ms, AHB/1
	Uptime = 0;
	SysTick->LOAD = SYSTEMTICKLEN - 1U;
	SysTick->VAL = 0;
	SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
	    SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
	__DSB();

	if (IS_ENABLED(LOCK_GPIO)) {
		// Lock GPIO configurations
		GPIOA->LCKR = 0x0001ffff;
		GPIOB->LCKR = 0x0001ffff;
		GPIOC->LCKR = 0x0001ffff;
		GPIOD->LCKR = 0x0001ffff;
		GPIOF->LCKR = 0x0001ffff;
		GPIOA->LCKR = 0x0000ffff;
		GPIOB->LCKR = 0x0000ffff;
		GPIOC->LCKR = 0x0000ffff;
		GPIOD->LCKR = 0x0000ffff;
		GPIOF->LCKR = 0x0000ffff;
		GPIOA->LCKR = 0x0001ffff;
		GPIOB->LCKR = 0x0001ffff;
		GPIOC->LCKR = 0x0001ffff;
		GPIOD->LCKR = 0x0001ffff;
		GPIOF->LCKR = 0x0001ffff;
	}
	// Disable unused peripherals
	RCC->AHBENR &= ~(RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIODEN |
			 RCC_AHBENR_GPIOFEN);

	main();

	// If main returns, assume PendSV style updates
	if (IS_ENABLED(ENABLE_SLEEP)) {
		SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
	}
	__SVC(0);
	SPIN();
}

// Busy wait roughly delay uptimes (~1/8ms)
void delay_uptime(uint32_t delay)
{
	uint32_t st = Uptime;
	while (Uptime - st < delay) ;
}

// Busy wait roughly delay millisconds
void delay_ms(uint32_t delay)
{
	delay_uptime(delay << 3);
}

/* ARM SysTick Handler */
void ms_timer(void)
{
	Uptime++;
	// Flag PENDSV every millisecond except the zero'th
	if (!(Uptime & 0x7)) {
		PENDSV();
	}
}

void undefined_handler(void)
{
	BREAKPOINT(BKPT_UNDEF);
	SPIN();
}

void nmi_handler(void)
{
	if (SYSCFG->CFGR2 & SYSCFG_CFGR2_SRAM_PE) {
		// Check for SRAM Parity error
		BREAKPOINT(BKPT_PARITY);
		SYSCFG->CFGR2 |= SYSCFG_CFGR2_SRAM_PE;
	} else if (RCC->CIR & RCC_CIR_CSSF) {
		// Check for CSS error
		BREAKPOINT(BKPT_CSS);
		RCC->CIR |= RCC_CIR_CSSC;
	} else {
		BREAKPOINT(BKPT_NMI);
	}
	SPIN();
}

void fault_handler(void)
{
	BREAKPOINT(BKPT_FAULT);
	SPIN();
}
