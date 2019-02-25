SECTIONS
{
  .text :
  {
    . = ALIGN(4);
    *(.text.*)
    . = ALIGN(4);
  }

  .data :
  {
    . = ALIGN(4);
    *(.data.*)
    . = ALIGN(4);
  }

  .rodata :
  {
    . = ALIGN(4);
    *(.rodata.*)
    . = ALIGN(4);
  }

  .bss :
  {
    . = ALIGN(4);
    *(.bss.*)
    . = ALIGN(4);
  }
}
