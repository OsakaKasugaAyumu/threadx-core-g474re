/* BUILD-LOCAL linker configuration for the original Cortex-M4 GNU CRT.
   Based on ports/cortex_m4/gnu/example_build/sample_threadx.ld in ST ThreadX 6.2.0
   (SHA-256 ce9709912140bdfc1b8d9255b4f012e7a6745597654c8dbba8cf70448ef458a1).
   No ST source or startup object was modified. Not an STM32 board memory map. */
MEMORY
{
  RAM (wx) : ORIGIN = 0x20000000, LENGTH = 0x00800000
  FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00400000
}
#include "autoconf.h"
__STACKSIZE__ = 1024;
__STACKSIZE_PROCESS__ = 0;
__HEAPSIZE__ = 128;

SECTIONS
{
  .vectors : 
  {
    KEEP(*(.vectors .vectors.*))
  } > FLASH

	.text :
	{
		__text_start__ = .;
		*(.text*)

		KEEP(*(.init))
		KEEP(*(.fini))
		__text_end__ = .;

		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
    __ctors_start__ = ALIGN(4);
		*(SORT(.ctors.*))
		*(.ctors)
    __ctors_end__ = ALIGN(4);

		/* .dtors */
 		*crtbegin.o(.dtors)
 		*crtbegin?.o(.dtors)
 		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
    __dtors_start__ = ALIGN(4);
 		*(SORT(.dtors.*))
 		*(.dtors)
    __dtors_end__ = ALIGN(4);

		__rodata_start__ = .;
		*(.rodata*)
		__rodata_end__ = .;

		KEEP(*(.eh_frame*))
	} > FLASH

  /* These four ranges reside inside the ST .text output section. Compute
     their load addresses from that section, preserving the FLASH XIP layout. */
  __text_load_start__   = LOADADDR(.text) + (__text_start__   - ADDR(.text));
  __ctors_load_start__  = LOADADDR(.text) + (__ctors_start__  - ADDR(.text));
  __dtors_load_start__  = LOADADDR(.text) + (__dtors_start__  - ADDR(.text));
  __rodata_load_start__ = LOADADDR(.text) + (__rodata_start__ - ADDR(.text));

	.ARM.extab : 
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASH

	__exidx_start = .;
	.ARM.exidx :
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
	} > FLASH
	__exidx_end = .;

	__data_load_start__ = ALIGN (4);

	.data : AT (__data_load_start__) 
	{
		__data_start__ = .;

		*(vtable)
		*(.data*)

		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		KEEP(*(.jcr*))
		. = ALIGN(4);
		/* All data end */
		
    __data_end__ = .;

	} > RAM

  /* The original CRT also copies .fast. Put its optional initializers after
     the .data load image in FLASH and its run range after .data in RAM.
     With no .fast inputs, the run range is genuinely empty. */
  .fast ALIGN(4) : AT (ALIGN(LOADADDR(.data) + SIZEOF(.data), 4))
  {
    __fast_start__ = .;
    *(.fast .fast.*)
    __fast_end__ = .;
  } > RAM
  __fast_load_start__ = LOADADDR(.fast);
  ASSERT(LOADADDR(.fast) + SIZEOF(.fast) <= ORIGIN(FLASH) + LENGTH(FLASH),
         "CRT load image exceeds FLASH")

	.bss :
	{
		. = ALIGN(4);
		__bss_start__ = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end__ = .;

	} > RAM
	
	.heap (COPY):
	{
    __heap_start__ = ALIGN(4);
    *(.heap)
    . = ALIGN(. + __HEAPSIZE__, 4);
    __heap_end__ = ALIGN(4);
	} > RAM

  .stack ALIGN(4) (NOLOAD) :
  {
    __stack_start__ = ALIGN(4);
    *(.stack)
    . = ALIGN(. + __STACKSIZE__, 4);
    __stack_end__ = ALIGN(4);
  } > RAM

  __RAM_segment_used_end__ = .;
}
