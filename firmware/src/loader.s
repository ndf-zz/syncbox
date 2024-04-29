// SPDX-License-Identifier: MIT

/*
 * Loader (STM32F303xE)
 *
 * Transfer the program code block from flash to ccmram
 * then execute reset handler.
 */
	.syntax unified
	.cpu cortex-m4
	.fpu softvfp
	.thumb

/* Default handler */
	.global Default_Handler
	.section .text.Default_Handler,"ax",%progbits
	.type Default_Handler, %function
Default_Handler:
	b Default_Handler
	.size Default_Handler, .-Default_Handler

/* Vector Table for Cortex-M4 */
	.global vtable
	.section .vector,"a",%progbits
	.type vtable, %object
vtable:
	.word 0x20010000
	.word Reset_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word Default_Handler
	.word Default_Handler
	.word 0
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word 0
	.word 0
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word 0
	.word 0
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word 0
	.word 0
	.word 0
	.word 0
	.word 0
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word Default_Handler
	.word 0
	.word 0
	.word Default_Handler
	.size vtable, .-vtable

/* System init reset handler */
	.global Reset_Handler
	.section .text.Reset_Handler,"ax",%progbits
	.type Reset_Handler, %function
Reset_Handler:
	mov r0, #0
	mov r1, #0x08000800
	mov r2, #0x10000000
	mov r3, #0x4000
CopyCode:
	ldr r4, [r1, r0]
	str r4, [r2, r0]
	add r0, r0, #4
	cmp r0, r3
	bcc CopyCode

/* Execute reset handler in target program code */
	ldr r0, [r2], #4
	ldr r1, [r2]
	msr msp, r0
	bx r1

