/* SPDX-License-Identifier: MIT */

/*
 * Linker Script for flash memory loader
 */

/* Program entry point */
ENTRY(Reset_Handler)

/* Memory Layout */
MEMORY
{
	FLASH (rx) :	ORIGIN = 0x08000000, LENGTH = 2K
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
	} >FLASH
}
