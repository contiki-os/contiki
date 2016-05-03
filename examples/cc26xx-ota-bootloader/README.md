# Cortex-M3 OTA Example

This is an attempt to create a Cortex-M3 bootloader that can jump to (Contiki) firmware binaries located elsewhere in the internal EEPROM.  Motivation is that binaries may come OTA to be stored at a later date.  Assumed system is a Texas Instruments CC2650 Sensortag.

We have in one folder the `bootloader`, and in the other an example firmware image for testing (`ota-image-example`).

# Overview & MMap
The idea behind this OTA mechanism is that the cc26xx always boots to the bootloader located at the origin of the EEPROM (`0x0000`).  It is the bootloader's purpose to locate and verify other firmware images (herein referred to as OTA images) stored in the flash, and then to branch processor execution to the most recent valid image.

For this example, we consider only the bootloader and a single example OTA image.

Description | Starting Position in EEPROM (bytes) | Space Allotted
--- | --- | ---
bootloader | 0x00000000 | 0x00001000
ota-image-example | 0x00001000 | 0x0001EFA8
ccfg (bootloader) | 0x0001FFA8 | 0x58

> NB: 0x1000 = 1 flash sector (4096 bytes)

*  Each of these binaries is statically linked with the Starting Position of each pre-determined and baked into the linker script.  E.G. `ota-image-example` has the flash origin set to `0x1000` in its linker file.
*  Each binary also has its own Vector table placed at the very start of each image.  This seems to be the method recommended by TI's OAD literature.

The core idea behind branching code execution to a different firmware image is to branch to the RESET vector in the target image's Vector table:

```asm
  __asm(" LDR R0, =0x1004 ");
  __asm(" BX R0 ");
```

0x1004 is the start address of the target image 0x1000 + 0x4 bytes to get to the RESET vector.

# What Should Happen?
If this example functions properly, we should see:

1.  CC26XX boots up into the Vector table and bootloader located at 0x0000.
1.  Bootloader should then initialize the GPIO system, illuminate an LED, and then jump to the OTA image located at 0x1000.
1.  *If* the OTA image begins executing, the CC26XX should then start strobing its LED at a frequency of 1Hz.

So far, the bootloader boots, configures GPIO, but the OTA image does not begin executing.

# Get Started

## Clone Code
It is critical to pull in the GIT submodules which contain TI's RTOS drivers.

```bash
git clone https://github.com/msolters/contiki
git submodule update --recursive --init
```

## Dependencies
Assuming a Debian machine there are a few specific tools needed to compile Contiki 3.0 and merge the resulting Intel-format .hex files.

```bash
# srecord will allow us to easily merge binaries
sudo add-apt-repository ppa:pmiller-opensource/ppa

# cc26xx requires a very specific gcc-arm compiler version
sudo apt-get remove binutils-arm-none-eabi gcc-arm-none-eabi
sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded

sudo apt-get update
sudo apt-get install srecord gcc-arm-none-eabi
```

## Building
Here are the steps to compiling the bootloader, a simple OTA image example, and then merging them into a single flashable image.

```bash
  cd examples/cc26xx-ota-bootloader

  # (1) Build the bootloader
  cd bootloader
  make bootloader.hex

  # (2) Build the target (OTA) image
  cd ../ota-image-example
  make

  # (3) Merge the binaries
  cd ..
  srec_cat bootloader/bootloader.hex -intel -exclude 0x1000 0x1FFA8 ota-image-example/ota-image-example.hex -intel -crop 0x1000 0x1FFA8 -o firmware.hex -intel
```

## Flashing
The resulting firmware.hex can be flashed to the cc26xx using TI's SmartRF Flash Programmer 2.
