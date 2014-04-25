    /* Interrupt vectors 0x00-0x3ff. */
    .vector_table 0x00000000 :
    {
        _vector_rom = .;
        KEEP(*(.isr_vector))
        KEEP(*(.vector_table))
    } >flash
    /* Actually only 111 entries are used by the K60DNxxxVLL10. (111*4 == 444 == 0x1bc) */
    ASSERT (SIZEOF(.vector_table) == 444, "Interrupt vector table of invalid size.")
    ASSERT (. == 0x1bc, "Interrupt vector table at invalid location.")

    /*
     * Allocate space for interrupt vector in RAM
     * This can safely be removed to free up 0x400 bytes of RAM if the code does
     * not make use of this CPU feature.
     */
    .ramvect :
    {
        . = ALIGN(1024);
        _vector_ram_start = .;
        . = _vector_ram_start + 0x400;
        _vector_ram_end = .;
    } > sram_u

