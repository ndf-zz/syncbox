// SPDX-License-Identifier: MIT

/*
 * Vector table for STM32F303xE
 */

#include "stm32f3xx.h"
#include <stddef.h>

// Vector Table for Cortex-M4
static __attribute__((used, section(".vector")))
void (*vtable[])(void) = {
	MSP_INIT,
	system_init,		// Reset_Handler
	nmi_handler,		// NMI_Handler
	fault_handler,		// HardFault_Handler
	fault_handler,		// MemManage_Handler
	fault_handler,		// BusFault_Handler
	fault_handler,		// UsageFault_Handler
	NULL,
	NULL,
	NULL,
	NULL,
	system_update,		// SVC_Handler
	undefined_handler,	// DebugMon_Handler
	NULL,
	system_update,		// PendSV_Handler
	ms_timer,		// SysTick_Handler
	undefined_handler,	// WWDG_IRQHandler
	undefined_handler,	// PVD_IRQHandler
	undefined_handler,	// TAMP_STAMP_IRQHandler
	undefined_handler,	// RTC_WKUP_IRQHandler
	undefined_handler,	// FLASH_IRQHandler
	undefined_handler,	// RCC_IRQHandler
	undefined_handler,	// EXTI0_IRQHandler
	undefined_handler,	// EXTI1_IRQHandler
	undefined_handler,	// EXTI2_TSC_IRQHandler
	undefined_handler,	// EXTI3_IRQHandler
	undefined_handler,	// EXTI4_IRQHandler
	undefined_handler,	// DMA1_Channel1_IRQHandler
	undefined_handler,	// DMA1_Channel2_IRQHandler
	undefined_handler,	// DMA1_Channel3_IRQHandler
	undefined_handler,	// DMA1_Channel4_IRQHandler
	undefined_handler,	// DMA1_Channel5_IRQHandler
	undefined_handler,	// DMA1_Channel6_IRQHandler
	undefined_handler,	// DMA1_Channel7_IRQHandler
	undefined_handler,	// ADC1_2_IRQHandler
	undefined_handler,	// USB_HP_CAN_TX_IRQHandler
	undefined_handler,	// USB_LP_CAN_RX0_IRQHandler
	undefined_handler,	// CAN_RX1_IRQHandler
	undefined_handler,	// CAN_SCE_IRQHandler
	undefined_handler,	// EXTI9_5_IRQHandler
	undefined_handler,	// TIM1_BRK_TIM15_IRQHandler
	undefined_handler,	// TIM1_UP_TIM16_IRQHandler
	undefined_handler,	// TIM1_TRG_COM_TIM17_IRQHandler
	undefined_handler,	// TIM1_CC_IRQHandler
	undefined_handler,	// TIM2_IRQHandler
	undefined_handler,	// TIM3_IRQHandler
	undefined_handler,	// TIM4_IRQHandler
	undefined_handler,	// I2C1_EV_IRQHandler
	undefined_handler,	// I2C1_ER_IRQHandler
	undefined_handler,	// I2C2_EV_IRQHandler
	undefined_handler,	// I2C2_ER_IRQHandler
	undefined_handler,	// SPI1_IRQHandler
	undefined_handler,	// SPI2_IRQHandler
	undefined_handler,	// USART1_IRQHandler
	undefined_handler,	// USART2_IRQHandler
	undefined_handler,	// USART3_IRQHandler
	undefined_handler,	// EXTI15_10_IRQHandler
	undefined_handler,	// RTC_Alarm_IRQHandler
	undefined_handler,	// USBWakeUp_IRQHandler
	undefined_handler,	// TIM8_BRK_IRQHandler
	undefined_handler,	// TIM8_UP_IRQHandler
	undefined_handler,	// TIM8_TRG_COM_IRQHandler
	undefined_handler,	// TIM8_CC_IRQHandler
	undefined_handler,	// ADC3_IRQHandler
	undefined_handler,	// FMC_IRQHandler
	NULL,
	NULL,
	undefined_handler,	// SPI3_IRQHandler
	undefined_handler,	// UART4_IRQHandler
	midi_uart_receive,	// UART5_IRQHandler
	undefined_handler,	// TIM6_DAC_IRQHandler
	undefined_handler,	// TIM7_IRQHandler
	undefined_handler,	// DMA2_Channel1_IRQHandler
	undefined_handler,	// DMA2_Channel2_IRQHandler
	undefined_handler,	// DMA2_Channel3_IRQHandler
	undefined_handler,	// DMA2_Channel4_IRQHandler
	undefined_handler,	// DMA2_Channel5_IRQHandler
	undefined_handler,	// ADC4_IRQHandler
	NULL,
	NULL,
	undefined_handler,	// COMP1_2_3_IRQHandler
	undefined_handler,	// COMP4_5_6_IRQHandler
	undefined_handler,	// COMP7_IRQHandler
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	undefined_handler,	// I2C3_EV_IRQHandler
	undefined_handler,	// I2C3_ER_IRQHandler
	undefined_handler,	// USB_HP_IRQHandler
	undefined_handler,	// USB_LP_IRQHandler
	undefined_handler,	// USBWakeUp_RMP_IRQHandler
	undefined_handler,	// TIM20_BRK_IRQHandler
	undefined_handler,	// TIM20_UP_IRQHandler
	undefined_handler,	// TIM20_TRG_COM_IRQHandler
	undefined_handler,	// TIM20_CC_IRQHandler
	undefined_handler,	// FPU_IRQHandler
	NULL,
	NULL,
	undefined_handler,	// SPI4_IRQHandler
};
