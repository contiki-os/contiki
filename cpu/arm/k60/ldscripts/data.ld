    /* Program data, values stored in flash and loaded upon init. */
    .data :
    {
        . = ALIGN(4);
        _data_load  = LOADADDR(.data);
        _data_start = .;

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

        /* fini data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
        . = ALIGN(4);

        _data_end = .;
    } >sram_u AT >flash

    /* Program bss, zeroed out during init. */
    .bss :
    {
        . = ALIGN(4);
        _bss_start = .;
        __bss_start = .;
        *(.bss*)
        *(.COMMON)
        . = ALIGN(4);
        _bss_end = .;
        __bss_end = .;
        . = ALIGN(4);
        _heap_start = .;
        __heap_start = .;
    } >sram_u

    /* Make sure we set _end, in case we want dynamic memory management... */
    __end = .;
    _end = .;
    PROVIDE(end = .);
