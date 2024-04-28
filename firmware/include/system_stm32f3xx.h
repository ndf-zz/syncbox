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

/* From kconfig.h (linux kernel), for conditional compilation */
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

/* Bit band access macros */
#define BB_PERIPH(reg) ((uint32_t *)(PERIPH_BB_BASE + ((uint32_t)&(reg) - PERIPH_BASE) * 32U))
#define BB_SRAM(reg) ((uint32_t *)(SRAM_BB_BASE + ((uint32_t)&(reg) - SRAM_BASE) * 32U))

/* convenience busy wait macros */
#define wait_for_bit_set(reg, bit) do { } while (((reg)&bit) != bit)
#define wait_for_bit_clr(reg, bit) do { } while (((reg)&bit) == bit)

/* convenience debug enabled macros */
#define DEBUG_ENABLED (CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
#define BREAKPOINT(cond) do { if (DEBUG_ENABLED) __BKPT( (cond) ); } while(0)
#define TRACEVAL(port, value) do { \
			ITM->PORT[(port)].u32 = (value); \
	} while(0)
#define PENDSV() do { SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; } while(0)

/* service call */
#define __SVC() __ASM volatile ("SVC #0")

/* optimisation barrier - for ordering co-dependent register access  */
#define barrier() __asm__ __volatile__("": : :"memory")

/* Main stack defines for 56k RAM / ~8KiB stack (see loader script) */
#define STACK_TOP	0x20010000UL
#define STACK_BOTTOM	0x2000e000UL
#define MSP_INIT ( (void(*)(void))STACK_TOP )

/* NVIC Constants */
#define PRIGROUP_4_4		0x5
#define PRIGROUP0		(0U<<2)
#define PRIGROUP1		(1U<<2)
#define PRIGROUP2		(2U<<2)
#define PRIGROUP3		(3U<<2)
#define PRISUB0			0U
#define PRISUB1			1U
#define PRISUB2			2U
#define PRISUB3			3U

/* MPU Constants */
#define FLASH_ALIAS		0x0UL
#define FLASH_USERBASE		0x08005000UL
#define RASR_ENABLE		0x1UL
#define RASR_AP_NONE		(0x0UL << 24U)
#define RASR_AP_RO		(0x06UL << 24U)
#define RASR_AP_RONX		(0x16UL << 24U)
#define RASR_AP_RWNX		(0x13UL << 24U)
#define RASR_TSCB_FLASH		(0x2UL << 16U)
#define RASR_TSCB_SRAM		(0x6UL << 16U)
#define RASR_SZ_512K		(18U << 1U)
#define RASR_SZ_64K		(15U << 1U)
#define RASR_SZ_32K		(14U << 1U)
#define RASR_SZ_16K		(13U << 1U)
#define RASR_SZ_32B		(4U << 1U)

/* watchdog functions */
void watchdog_enable(void);
void watchdog_kick(void);

/* special-case wait for interrupt - with barriers */
void wait_for_interrupt(void);

/* busy wait for roughly delay systicks less than SYSTEMTICKLEN */
void spin_wait(uint32_t delay);

/* Unique hardware identifier, computed in system_init() */
extern uint32_t SystemID;

/* Software version */
extern uint32_t Version;

/* Main function prototype - requires -ffreestanding */
void main(void);

/* Unique device ID register */
typedef struct {
	__I uint32_t XY;	/* X,Y coordinates on wafer */
	__I uint32_t LOTWAF;	/* Lot high and wafer number */
	__I uint32_t LOT;	/* Lot low */
} UID_TypeDef;
#define UID                 ((UID_TypeDef *) UID_BASE)

/* System heartbeat clock - updated by SystemTick interrupt */
extern volatile uint32_t Uptime;

/* Handler function declarations */
void Default_Handler(void);
void Reset_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void WWDG_IRQHandler(void);
void PVD_IRQHandler(void);
void TAMP_STAMP_IRQHandler(void);
void RTC_WKUP_IRQHandler(void);
void FLASH_IRQHandler(void);
void RCC_IRQHandler(void);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_TSC_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
void ADC1_2_IRQHandler(void);
void USB_HP_CAN_TX_IRQHandler(void);
void USB_LP_CAN_RX0_IRQHandler(void);
void CAN_RX1_IRQHandler(void);
void CAN_SCE_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void TIM1_BRK_TIM15_IRQHandler(void);
void TIM1_UP_TIM16_IRQHandler(void);
void TIM1_TRG_COM_TIM17_IRQHandler(void);
void TIM1_CC_IRQHandler(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);
void I2C1_EV_IRQHandler(void);
void I2C1_ER_IRQHandler(void);
void I2C2_EV_IRQHandler(void);
void I2C2_ER_IRQHandler(void);
void SPI1_IRQHandler(void);
void SPI2_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void RTC_Alarm_IRQHandler(void);
void USBWakeUp_IRQHandler(void);
void TIM8_BRK_IRQHandler(void);
void TIM8_UP_IRQHandler(void);
void TIM8_TRG_COM_IRQHandler(void);
void TIM8_CC_IRQHandler(void);
void ADC3_IRQHandler(void);
void FMC_IRQHandler(void);
void SPI3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
void TIM6_DAC_IRQHandler(void);
void TIM7_IRQHandler(void);
void DMA2_Channel1_IRQHandler(void);
void DMA2_Channel2_IRQHandler(void);
void DMA2_Channel3_IRQHandler(void);
void DMA2_Channel4_IRQHandler(void);
void DMA2_Channel5_IRQHandler(void);
void ADC4_IRQHandler(void);
void COMP1_2_3_IRQHandler(void);
void COMP4_5_6_IRQHandler(void);
void COMP7_IRQHandler(void);
void I2C3_EV_IRQHandler(void);
void I2C3_ER_IRQHandler(void);
void USB_HP_IRQHandler(void);
void USB_LP_IRQHandler(void);
void USBWakeUp_RMP_IRQHandler(void);
void TIM20_BRK_IRQHandler(void);
void TIM20_UP_IRQHandler(void);
void TIM20_TRG_COM_IRQHandler(void);
void TIM20_CC_IRQHandler(void);
void FPU_IRQHandler(void);
void SPI4_IRQHandler(void);

#endif /* SYSTEM_H */
