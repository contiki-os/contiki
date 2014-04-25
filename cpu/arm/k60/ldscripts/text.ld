    /* Program code 0x410-. */
    .text :
    {
        . = ALIGN(4);
        _text_load = LOADADDR(.text);
        _text_start = .;
        KEEP (*(SORT_NONE(.init)))
        KEEP (*(SORT_NONE(.fini)))
        /* Default ISR handlers */
        KEEP(*(.default_handlers))
        *(.text.unlikely .text.*_unlikely .text.unlikely.*)
        *(.text.exit .text.exit.*)
        *(.text.startup .text.startup.*)
        *(.text.hot .text.hot.*)
        *(.text .stub .text.* .gnu.linkonce.t.*)

        /* gcc uses crtbegin.o to find the start of
           the constructors, so we make sure it is
           first.  Because this is a wildcard, it
           doesn't matter if the user does not
           actually link against crtbegin.o; the
           linker won't look for a file to match a
           wildcard.  The wildcard also means that it
           doesn't matter which directory crtbegin.o
           is in.  */
        KEEP (*crtbegin.o(.ctors))
        KEEP (*crtbegin?.o(.ctors))
        KEEP (*crtbeginTS.o(.ctors))
        /* We don't want to include the .ctor section from
           the crtend.o file until after the sorted ctors.
           The .ctor section from the crtend file contains the
           end of ctors marker and it must be last */
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))

        KEEP (*crtbegin.o(.dtors))
        KEEP (*crtbegin?.o(.dtors))
        KEEP (*crtbeginTS.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
        . = ALIGN(4);
        _rodata_start = .;
        *(.rodata .rodata* .gnu.linkonce.r.*)
        . = ALIGN(4);
        _rodata_end = .;
        _text_end = .;
    } >flash

    .ramcode :
    {
        . = ALIGN(4);
        _ramcode_load = LOADADDR(.ramcode);
        _ramcode_start = .;
        *(.ramcode*)
        . = ALIGN(4);
        _ramcode_end = .;
    } > sram_l AT>flash

    /* The .extab, .exidx sections are used for C++ exception handling */
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >flash

    PROVIDE_HIDDEN (__exidx_start = .);
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >flash
    PROVIDE_HIDDEN (__exidx_end = .);
