/* SPDX-License-Identifier: MIT */

/*
 * Application code link script for CCMRAM / ~8KiB stack
 */

/* Program entry point */
ENTRY(Reset_Handler)

/* Memory Layout */
MEMORY
{
	CCMRAM (rx) :	ORIGIN = 0x10000000, LENGTH = 16K
	RAM (!rx) :	ORIGIN = 0x20000000, LENGTH = 56K
}

/* Sections */
SECTIONS
{
	/* Code */
	.text :
	{
		KEEP (*(.vector*))
		*(.text*)
		. = ALIGN(4);
		_etext = .;
	} >CCMRAM

	/* Data */
	_sidata = LOADADDR(.data);
	.data :
	{
		_sdata = .;
		*(.rodata*)
		*(.data*)
		. = ALIGN(4);
		_edata = .;
	} >RAM AT> CCMRAM

	/* data initialized to 0 at the start of the program */
	.bss (NOLOAD) :
	{
		_sbss = .;
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} >RAM
}
