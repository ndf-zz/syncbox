// SPDX-License-Identifier: MIT

/*
 * Vector table for STM32F303xE
 */

#include "stm32f3xx.h"

/* Vector Table for Cortex-M4 */
static __attribute__((used, section(".vector")))
void (*vtable[])(void) = {
	MSP_INIT,
	Reset_Handler,
	NMI_Handler,
	HardFault_Handler,
	MemManage_Handler,
	BusFault_Handler,
	UsageFault_Handler,
	0UL,
	0UL,
	0UL,
	0UL,
	PendSV_Handler,		/* SVC_Handler */
	Default_Handler,	/* DebugMon_Handler */
	0UL,
	PendSV_Handler,		/* PendSV_Handler */
	SysTick_Handler,
	Default_Handler,	/* WWDG_IRQHandler */
	Default_Handler,	/* PVD_IRQHandler */
	Default_Handler,	/* TAMP_STAMP_IRQHandler */
	Default_Handler,	/* RTC_WKUP_IRQHandler */
	Default_Handler,	/* FLASH_IRQHandler */
	Default_Handler,	/* RCC_IRQHandler */
	Default_Handler,	/* EXTI0_IRQHandler */
	Default_Handler,	/* EXTI1_IRQHandler */
	Default_Handler,	/* EXTI2_TSC_IRQHandler */
	Default_Handler,	/* EXTI3_IRQHandler */
	Default_Handler,	/* EXTI4_IRQHandler */
	Default_Handler,	/* DMA1_Channel1_IRQHandler */
	Default_Handler,	/* DMA1_Channel2_IRQHandler */
	Default_Handler,	/* DMA1_Channel3_IRQHandler */
	Default_Handler,	/* DMA1_Channel4_IRQHandler */
	Default_Handler,	/* DMA1_Channel5_IRQHandler */
	Default_Handler,	/* DMA1_Channel6_IRQHandler */
	Default_Handler,	/* DMA1_Channel7_IRQHandler */
	Default_Handler,	/* ADC1_2_IRQHandler */
	Default_Handler,	/* USB_HP_CAN_TX_IRQHandler */
	Default_Handler,	/* USB_LP_CAN_RX0_IRQHandler */
	Default_Handler,	/* CAN_RX1_IRQHandler */
	Default_Handler,	/* CAN_SCE_IRQHandler */
	Default_Handler,	/* EXTI9_5_IRQHandler */
	Default_Handler,	/* TIM1_BRK_TIM15_IRQHandler */
	Default_Handler,	/* TIM1_UP_TIM16_IRQHandler */
	Default_Handler,	/* TIM1_TRG_COM_TIM17_IRQHandler */
	Default_Handler,	/* TIM1_CC_IRQHandler */
	Default_Handler,	/* TIM2_IRQHandler */
	Default_Handler,	/* TIM3_IRQHandler */
	Default_Handler,	/* TIM4_IRQHandler */
	Default_Handler,	/* I2C1_EV_IRQHandler */
	Default_Handler,	/* I2C1_ER_IRQHandler */
	Default_Handler,	/* I2C2_EV_IRQHandler */
	Default_Handler,	/* I2C2_ER_IRQHandler */
	Default_Handler,	/* SPI1_IRQHandler */
	Default_Handler,	/* SPI2_IRQHandler */
	Default_Handler,	/* USART1_IRQHandler */
	Default_Handler,	/* USART2_IRQHandler */
	Default_Handler,	/* USART3_IRQHandler */
	Default_Handler,	/* EXTI15_10_IRQHandler */
	Default_Handler,	/* RTC_Alarm_IRQHandler */
	Default_Handler,	/* USBWakeUp_IRQHandler */
	Default_Handler,	/* TIM8_BRK_IRQHandler */
	Default_Handler,	/* TIM8_UP_IRQHandler */
	Default_Handler,	/* TIM8_TRG_COM_IRQHandler */
	Default_Handler,	/* TIM8_CC_IRQHandler */
	Default_Handler,	/* ADC3_IRQHandler */
	Default_Handler,	/* FMC_IRQHandler */
	0UL,
	0UL,
	Default_Handler,	/* SPI3_IRQHandler */
	Default_Handler,	/* UART4_IRQHandler */
	UART5_IRQHandler,	/* UART5_IRQHandler */
	Default_Handler,	/* TIM6_DAC_IRQHandler */
	Default_Handler,	/* TIM7_IRQHandler */
	Default_Handler,	/* DMA2_Channel1_IRQHandler */
	Default_Handler,	/* DMA2_Channel2_IRQHandler */
	Default_Handler,	/* DMA2_Channel3_IRQHandler */
	Default_Handler,	/* DMA2_Channel4_IRQHandler */
	Default_Handler,	/* DMA2_Channel5_IRQHandler */
	Default_Handler,	/* ADC4_IRQHandler */
	0UL,
	0UL,
	Default_Handler,	/* COMP1_2_3_IRQHandler */
	Default_Handler,	/* COMP4_5_6_IRQHandler */
	Default_Handler,	/* COMP7_IRQHandler */
	0UL,
	0UL,
	0UL,
	0UL,
	0UL,
	Default_Handler,	/* I2C3_EV_IRQHandler */
	Default_Handler,	/* I2C3_ER_IRQHandler */
	Default_Handler,	/* USB_HP_IRQHandler */
	Default_Handler,	/* USB_LP_IRQHandler */
	Default_Handler,	/* USBWakeUp_RMP_IRQHandler */
	Default_Handler,	/* TIM20_BRK_IRQHandler */
	Default_Handler,	/* TIM20_UP_IRQHandler */
	Default_Handler,	/* TIM20_TRG_COM_IRQHandler */
	Default_Handler,	/* TIM20_CC_IRQHandler */
	Default_Handler,	/* FPU_IRQHandler */
	0UL,
	0UL,
	Default_Handler,	/* SPI4_IRQHandler */
};
