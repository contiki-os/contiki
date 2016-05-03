This is an attempt to create a Cortex-M3 bootloader that can jump to Contiki firmware binaries located in the internal EEPROM.  Motivation is that binaries may come OTA to be stored at a later date.

We have in one folder the `bootloader`, and in the other an example firmware image for testing (`ota-image-example`).

# Dependencies
Tools used to compile this FW

```bash
# srecord will allow us to easily merge binaries
sudo add-apt-repository ppa:pmiller-opensource/ppa

# cc26xx requires a specific gcc-arm compiler
sudo apt-get remove binutils-arm-none-eabi gcc-arm-none-eabi
ppa:terry.guo/gcc-arm-embedded

sudo apt-get update
sudo apt-get install srecord gcc-arm-none-eabi
```

# Clone Code
It is critical to pull in the GIT submodules which contain TI's RTOS drivers.

```bash
  git clone https://github.com/msolters/contiki
  git submodule update --recursive --init
```

# Build Bootloader
```
  cd bootloader
  make bootloader.hex
```

# Build "target" OTA image
```
  cd ota-image-example
  make 
```

# Merging Binaries
To start, we create a binary that looks like this:

Binary | Space Allotted | Starting Position in EEPROM (bytes)
--- | --- | ---
bootloader.bin | 0x1000 | 0x0000
ota-image-example.bin | 0x19000 | 0x1000
