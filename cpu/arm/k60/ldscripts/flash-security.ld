    /* Flash security 0x400-0x40f. */
    .security 0x400 :
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
    ASSERT (SIZEOF(.security) == 0x10, "Flash security sector of invalid size (linker-script error?).")
    ASSERT (. == 0x410, "Flash security sector of invalid size or position (linker-script error?).")
