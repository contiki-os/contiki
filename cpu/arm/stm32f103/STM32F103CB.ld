MEMORY
{
  CODE (rx) : ORIGIN = 0x8000000, LENGTH = 128K
  DATA (xrw) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Section Definitions */

SECTIONS
{

/* Make sure the vector table is at address 0 */

.vectrom :
{
	KEEP(*(.isr_vector))
} >CODE =0

.text :
{
	KEEP(*(.init))
	*(.text .text.*)
	KEEP(*(.fini))
} >CODE =0

. = ALIGN(4);

.rodata :
  {
   	*(.rodata .rodata.*)
	*(.gnu.linkonce.r.*)
  } >CODE

  _etext = . ;
  PROVIDE (etext = .);
   
  .data :
  {
    _data = . ;
    *(.data)
    _edata = . ;
    PROVIDE (edata = .);
  } >DATA AT >CODE
  . = ALIGN(4);


/* .bss section which is used for uninitialized data */

  .bss :
  {
    __bss_start = . ;
    __bss_start__ = . ;
    *(.bss)
    *(COMMON)
    __bss_end = . ;	
    __bss_end__ = . ;
    *(.noinit)
  } >DATA
  . = ALIGN(4);
  
   _end = .;
  PROVIDE (end = .);

Main_Stack_Size = 0x00000200;
Process_Stack_Size = 0x00000200;

Stack_Size = Main_Stack_Size + Process_Stack_Size;
  .stack ORIGIN(DATA) + LENGTH(DATA) - Stack_Size : 
  {
    __stack_start__ = . ;
    Main_Stack_Start = . ;
    . += Main_Stack_Size;
    Main_Stack_End = . ;	
    Process_Stack_Start = . ;
    . += Process_Stack_Size;
    Process_Stack_End = . ;	
    . = ALIGN(4);
    __stack_end__ = . ;
    Top_Stack = .;
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
