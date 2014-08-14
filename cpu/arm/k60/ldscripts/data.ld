    /* Program data, values stored in flash and loaded upon init. */
    .data :
    {
        . = ALIGN(4);
        _data_load  = LOADADDR(.data);
        _data_start = .;

        *(.data*)
        . = ALIGN(4);

        _data_end = .;
        /* edata is used instead of _data_end by some RTOSes startup code (e.g. ChibiOS) */
        PROVIDE(edata = .);
    } >sram_u AT >flash

    /* Program bss, zeroed out during init. */
    .bss :
    {
        . = ALIGN(4);
        _bss_start = .;
        __bss_start = .;
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
        __bss_end = .;
        __bss_end__ = .;
        . = ALIGN(4);
        _heap_start = .;
        __heap_start = .;
    } >sram_u

    /* Make sure we set _end, in case we want dynamic memory management... */
    _end = .;
    __end = .;
    __end__ = .;
    PROVIDE(end = .);
