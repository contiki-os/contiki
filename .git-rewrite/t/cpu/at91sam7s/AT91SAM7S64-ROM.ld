/***********************************************************************/
/*                                                                     */
/*  Linker Script File for the AT91SAM7S64 - Code in ROM               */
/*                                                                     */
/***********************************************************************/
/*  modified for arm-elf-gcc by Martin Thomas                          */
/*  extensions: - Section for Functions in RAM,                        */
/*                Exeception-Vector remapping                          */
/*  modifications Copyright Martin Thomas 2005                         */
/*                                                                     */
/*                                                                     */
/*  Based on file that has been a part of the uVision/ARM development  */
/*  tools, Copyright KEIL ELEKTRONIK GmbH 2002-2004                    */
/***********************************************************************/


/*
//*** <<< Use Configuration Wizard in Context Menu >>> *** 
*/


/*
// <h> Memory Configuration
//   <h> Code (Read Only)
//     <o>  Start <0x0-0xFFFFFFFF>
//     <o1> Size  <0x0-0xFFFFFFFF>
//   </h>
//   <h> Data (Read/Write)
//     <o2> Start <0x0-0xFFFFFFFF>
//     <o3> Size  <0x0-0xFFFFFFFF>
//   </h>
// </h>
*/

/* Memory Definitions */

MEMORY
{
  CODE (rx) : ORIGIN = 0x00100000, LENGTH = 0x00010000
  DATA (rw) : ORIGIN = 0x00200000, LENGTH = 0x00004000
}


/* Section Definitions */

SECTIONS
{

  /* first section is .text which is used for code */

  .text :
  {
/*  *startup.o (.text)  */       /* Startup code */
		KEEP(*(.vectrom))        /* added by mthomas */
		KEEP(*(.init))
		*(.text .text.*)
		*(.gnu.linkonce.t.*)
		*(.glue_7t .glue_7)
		KEEP(*(.fini))
		*(.gcc_except_table)
  } >CODE =0

  . = ALIGN(4);
  
  
	/* .ctors .dtors are used for c++ constructors/destructors */
	/* added by mthomas, based on an Anglia-Designs example for STR7 */
	
	.ctors :
	{
		PROVIDE(__ctors_start__ = .);
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		PROVIDE(__ctors_end__ = .);
	} >CODE

	.dtors :
	{
		PROVIDE(__dtors_start__ = .); 
		KEEP(*(SORT(.dtors.*)))
		KEEP(*(.dtors))
		PROVIDE(__dtors_end__ = .);
	} >CODE

  /* .rodata section which is used for read-only data (constants) */

  .rodata :
  {
   	*(.rodata .rodata.*)
	*(.gnu.linkonce.r.*)
  } >CODE

  . = ALIGN(4);
  _etext = . ;
  PROVIDE (etext = .);
   
  .data : AT (_etext)
  {
    _data = . ;
	KEEP(*(.vectram))   /* added by mthomas */
    *(.data)
    SORT(CONSTRUCTORS)
	. = ALIGN(4);
	*(.fastrun)         /* "RAM-Functions" */ /* added by mthomas */
  } >DATA
  . = ALIGN(4);

  _edata = . ;
   PROVIDE (edata = .);

  /* .bss section which is used for uninitialized data */

  .bss :
  {
    __bss_start = . ;
    __bss_start__ = . ;
    *(.bss)
    *(COMMON)
  } >DATA
  . = ALIGN(4);
  __bss_end__ = . ;
  __bss_end__ = . ;
   _end = .;
  PROVIDE (end = .);

  .stack ORIGIN(DATA) + LENGTH(DATA) - 0x0a0c : 
  {
    __stack_start__ = . ;
    *(.stack)
    . = ALIGN(4);
    __stack_end__ = . ;
  } >DATA


 
  __heap_start__ = __bss_end__ ;
  __heap_end__ = __stack_start__ ;

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
}
