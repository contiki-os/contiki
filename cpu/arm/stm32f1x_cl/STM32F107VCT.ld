MEMORY
{
  CODE (rx) : ORIGIN = 0x08000000, LENGTH = 256K
  DATA (xrw) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Section Definitions */

EXTERN (vector_table)
ENTRY(reset_handler)
SECTIONS
{

/* Make sure the vector table is at address 0 */

   .text : {
   	   *(.vectors)
	   *(.text*)
	   . = ALIGN(4);
	   *(.rodata*)
	   . = ALIGN(4);
   } >CODE

   . = ALIGN(4);
   _etext = . ;
   PROVIDE (etext = .);

  .data :
  {
    _data = . ;
    *(.data*)
    . = ALIGN(4);
    _edata = . ;
    PROVIDE (_edata = .);
  } >DATA AT >CODE
  _data_loadaddr = LOADADDR(.data);

/* .bss section which is used for uninitialized data */

  .bss :
  {
    __bss_start = . ;
    __bss_start__ = . ;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = . ;
  } >DATA
  . = ALIGN(4);

   _end = .;
  PROVIDE (_end = .);
}
PROVIDE(_stack = ORIGIN(DATA) + LENGTH(DATA));
