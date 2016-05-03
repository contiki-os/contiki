This is an attempt to create a Cortex-M3 bootloader that can jump to Contiki firmware binaries located in the internal EEPROM.  Motivation is that binaries may come OTA to be stored at a later date.

We have in one folder the `bootloader`, and in the other an example firmware image for testing (`ota-image-example`).

# Clone Code
At this step, it is critical to pull in the GIT submodules which contain TI's RTOS drivers.

```bash
  git clone https://github.com/msolters/contiki
  git submodule update --recursive --init
```

# Build Bootloader
```
  cd bootloader
  make
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
