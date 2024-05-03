// SPDX-License-Identifier: MIT

/*
 * System and compiler specific defines
 */
#ifndef SYSTEM_H
#define SYSTEM_H

#define SYSTEMCORECLOCK 24000000UL
#define SYSTEMTICKLEN	(SYSTEMCORECLOCK/1000UL)

#ifndef SYSTEMVERSION
#define SYSTEMVERSION 0xdbffffffUL
#endif

// From kconfig.h (linux kernel)
#define __ARG_PLACEHOLDER_1 0,
#define __take_second_arg(__ignored, val, ...) val
#define __and(x, y)                     ___and(x, y)
#define ___and(x, y)                    ____and(__ARG_PLACEHOLDER_##x, y)
#define ____and(arg1_or_junk, y)        __take_second_arg(arg1_or_junk y, 0, 0)
#define __or(x, y)                      ___or(x, y)
#define ___or(x, y)                     ____or(__ARG_PLACEHOLDER_##x, y)
#define ____or(arg1_or_junk, y)         __take_second_arg(arg1_or_junk 1, y, 0)
#define __is_defined(x)			___is_defined(x)
#define ___is_defined(val)		____is_defined(__ARG_PLACEHOLDER_##val)
#define ____is_defined(arg1_or_junk)	__take_second_arg(arg1_or_junk 1, 0, 0)
#define IS_ENABLED(option) __is_defined(option)
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// convenience macros
#define wait_for_bit_set(reg, bit) do { } while (((reg)&bit) != bit)
#define wait_for_bit_clr(reg, bit) do { } while (((reg)&bit) == bit)
#define DEBUG_ENABLED (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
#define BREAKPOINT(cond) do { if (DEBUG_ENABLED) __BKPT( (cond) ); } while(0)
#define TRACEVAL(port, value) do { ITM->PORT[(port)].u32 = (value); } while(0)
#define PENDSV() do { SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; } while(0)
#define SPIN() do { } while (1U)

// service call
#define __SVC(code) __ASM volatile ("svc "#code)

// optimisation barrier - for ordering co-dependent register access
#define barrier() __asm__ __volatile__("": : :"memory")

// Main stack defines for 56k RAM / ~8KiB stack (see loader script)
#define STACK_TOP	0x20010000UL
#define STACK_BOTTOM	0x2000e000UL
#define MSP_INIT ( (void(*)(void))STACK_TOP )

// CRC Constants
#define CRC7_POLY		0x09
#define CRC7_INIT		0x0

// NVIC Constants
#define PRIGROUP_4_4		0x5
#define PRIGROUP0		(0U<<2)
#define PRIGROUP1		(1U<<2)
#define PRIGROUP2		(2U<<2)
#define PRIGROUP3		(3U<<2)
#define PRISUB0			0U
#define PRISUB1			1U
#define PRISUB2			2U
#define PRISUB3			3U

// MPU Constants
#define FLASH_ALIAS		0x0
#define RASR_ENABLE		0x1
#define RASR_AP_NONE		(0x0 << 24)
#define RASR_AP_RO		(0x06 << 24)
#define RASR_AP_RONX		(0x16 << 24)
#define RASR_AP_RWNX		(0x13 << 24)
#define RASR_TSCB_FLASH		(0x2 << 16)
#define RASR_TSCB_SRAM		(0x6 << 16)
#define RASR_SZ_512K		(18U << 1)
#define RASR_SZ_64K		(15U << 1)
#define RASR_SZ_32K		(14U << 1)
#define RASR_SZ_16K		(13U << 1)
#define RASR_SZ_32B		(4U << 1)

// Breakpoint labels
#define BKPT_FAULT		0xfe
#define BKPT_UNDEF		0xfd
#define BKPT_PARITY		0xfc
#define BKPT_CSS		0xfb
#define BKPT_NMI		0xfa

// Unique device ID register
typedef struct {
	__I uint32_t XY;	/* X,Y coordinates on wafer */
	__I uint32_t LOTWAF;	/* Lot high and wafer number */
	__I uint32_t LOT;	/* Lot low */
} UID_TypeDef;
#define UID                 ((UID_TypeDef *) UID_BASE)

// Unique hardware identifier, computed in system_init()
extern uint32_t SystemID;

// System heartbeat clock
extern volatile uint32_t Uptime;

// Busy wait for roughly delay ms
void delay_ms(uint32_t delay);

// Main function prototype - requires -ffreestanding */
void main(void);

// Fault/Interrupt handler prototypes
void system_init(void);
void system_update(void);
void ms_timer(void);
void nmi_handler(void);
void fault_handler(void);
void undefined_handler(void);
void midi_uart_receive(void);
void timer_update(void);

#endif /* SYSTEM_H */
