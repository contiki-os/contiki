OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

MEMORY
{
    flash (rx)  : ORIGIN = 0x0,     LENGTH = 256K
    sram_l (rwx)    : ORIGIN = 0x1fff8000,  LENGTH = 32K /* Only accessible via code bus. */
    sram_u (rwx)    : ORIGIN = 0x20000000,  LENGTH = 32K /* Only accessible via system bus. */
}

/* RAM limits */
__sram_u_start  = 0x20000000;
__sram_u_end    = __sram_u_start + 32K;
__sram_l_end    = 0x20000000;
__sram_l_start  = __sram_l_end - 32K;

SECTIONS
{
    /* Start at address 0x00. */
    . = 0x00;

    /* Interrupt vectors 0x00-0x3ff. */
    .vector_table :
    {
        _vector_rom = .;
        KEEP(*(.vector_table))
    } >flash
    ASSERT (. == 0x1bc, "Interrupt vector of invalid size.")

    /* Flash security 0x400-0x40f. */
    .security 0x400:
    {
        /* Backdoor key. */
        LONG (0x01234567) LONG (0x89abcdef)

        /* FPROT0-3: no region of the flash is protected. */
        LONG (0xffffffff)

        /* FSEC: place MCU in unsecure mode. */
        BYTE (0xfe)

        /* FOPT: disable EzPort. */
        BYTE (0xfd)

        /* FEPROT: default. */
        BYTE (0xff)

        /* FDPROT: default. */
        BYTE (0xff)
    } >flash
    ASSERT (. == 0x410, "Flash security sector of invalid size or position (linker-script error?).")

    /* Program code 0x410-. */
    .text :
    {
        . = ALIGN(4);
        _text_load = LOADADDR(.text);
        _text_start = .;
        /* Default ISR handlers */
        KEEP(*(.default_handlers))
        *(.text*)
        . = ALIGN(4);
        _rodata_start = .;
        *(.rodata*)
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

    .ramvect :
    {
        . = ALIGN(1024);
        _vector_ram_start = .;
        . = _vector_ram_start + 0x400;
        _vector_ram_end = .;
    } > sram_u

    /* The exidx section is used for C++ exception handling */
    __exidx_start = .;
    .ARM.exidx   : { *(.ARM.exidx*) }
    __exidx_end = .;

    /* Program data, values stored in flash and loaded upon init. */
    .data :
    {
        . = ALIGN(4);
        _data_load  = LOADADDR(.data);
        _data_start = .;
        *(.data*)
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
        . = ALIGN(4);
        _bss_end = .;
        __bss_end = .;
        *(.COMMON)
        . = ALIGN(4);
        _heap_start = .;
        __heap_start = .;
    } >sram_u

    /* Make sure we set _end, in case we want dynamic memory management... */
    __end = .;
    _end = .;
    PROVIDE(end = .);

    /* Startup code want to know where the stack should start... */
    __stack_init = __sram_u_end;
    STACK_SIZE = 2K;
    _heap_end = ALIGN (ORIGIN(sram_u) + LENGTH(sram_u) - STACK_SIZE, 8);
    _stack_start = ALIGN (ORIGIN(sram_u) + LENGTH(sram_u), 8);
    _stack_end = _stack_start - STACK_SIZE;
}
