# OTA Bootloader

You can use this bootloader with your own OTA-ready applications on the CC2650 platform.  To build, simply enter the `/bootloader` directory and use:

```bash
make bootloader.hex
```

This bootloader is made to be as simple as it can be.  There's only a few things to be aware of regarding its configuration.

## Compiling
In order to strip away everything that wasn't strictly necessary, this bootloader does not compile through Contiki's master `Makefile.include`.  It has an independent `Makefile` which only depends on TI's cc26xxware (located under `/cpu/cc26xx-cc13xx/`), as well as its own linker script `cc26xx.bootloader.ld`.

The compiler has two optional flags.  They can be used together, alone, or not at all.

```bash
make bootloader.hex BURN_GOLDEN_IMAGE=1 CLEAR_OTA_SLOTS=1
```

Compiler Flag | Purpose
---|---
`BURN_GOLDEN_IMAGE=1` | The bootloader will copy the current firmware from internal flash and store it in OTA slot 0 before running any other code.  To flash firmware as the Golden Image, simply merge it with a bootloader thus configured.
`CLEAR_OTA_SLOTS=1` | Use this flag if you want the bootloader to erase all OTA slots in external flash before running any other code.  This will not erase the Golden Image.

### Contiki Location
Although the bootloader does not depend on any Contiki source code, it still must know where your Contiki directory is to find cc26xxware.  If you copy the `/bootloader` folder somewhere else in this Contiki tree, the bootloader's `Makefile` may not be able to find the correct relative location of the Contiki root directory.  To fix this, just modify the `CONTIKI` var inside the bootloader's `Makefile`:

```
CONTIKI = ../../..
```

### OTA Location
The bootloader `Makefile` will also look for the `/ota` folder, which contains `ota.c`, `ota.h`, etc.  If `/bootloader` is copied elsewhere, make sure the `/ota` directory is also copied to the same destination, or alternatively change the `OTA_SOURCE` variable inside `Makefile`:

```
OTA_SOURCE = ../ota
```

## Use of External Flash
The OTA bootloader stores and manages OTA image updates by reading and writing downloads to an external flash chip.

The external flash chip is expected to be connected via SPI -- see `ext-flash.c` driver in `/bootloader/ext-flash`.  This driver is designed for the [W25X40CL](https://www.winbond.com/resource-files/w25x40cl_e01.pdf) flash chip, the same as used in the CC2650 Sensortag board.

The SPI configuration can be modified using `/bootloader/ext-flash/spi-pins.h`:

```c
#ifndef SPI_PINS_H_
#define SPI_PINS_H_
/**
 *    How is the ext-flash hardware connected to your board?
 *    Enter pin definitions here.
 */
#include "ioc.h"

#define BOARD_IOID_FLASH_CS       IOID_14
#define BOARD_FLASH_CS            (1 << BOARD_IOID_FLASH_CS)
#define BOARD_IOID_SPI_CLK_FLASH  IOID_10
#define BOARD_IOID_SPI_MOSI       IOID_9
#define BOARD_IOID_SPI_MISO       IOID_8

#endif
```

If you want to use a different flash chip, you'll have to provide your own driver and rewrite most of `ota.c`.

## Memory Map (Internal)
The bootloader is designed to only inhabit the first two sectors and the last 88 bytes of internal flash.  You are free to overwrite anything between flash addresses [0x2000, 0x1FFA8) without damaging the bootloader.

The bootloader will expect to find and will also write new firmware starting at internal flash address 0x2000, and will only allow firmware images of 0x19000 (100Kb) long.  To change this configuration, you can edit the `OTA_IMAGE_OFFSET` and `OTA_IMAGE_LENGTH` variables in `/cpu/cc26xx-cc13xx/Makefile.cc26xx-cc13xx`:

```c
OTA_IMAGE_OFFSET=0x2000		#	Starts on flash page 2
OTA_IMAGE_LENGTH=0x19000 	# 25 flash pages long
```

Keep in mind the internal flash used by CC2650 modules from TI is only 0x20000 (128Kb).
