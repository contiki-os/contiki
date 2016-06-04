# CC26XX OTA
This is an example Contiki application that demonstrates how to send complete firmware updates over-the-air (OTA) to a Texas Instruments CC2650.  (In this example, assumed platform is a Sensortag.)

The OTA system depends on compiling two separate binaries, which are both placed into the CC2650's internal flash.

*  A **bootloader**, which remains permanently fixed in the CC2650's flash.  It manages, verifies, and repairs the CC2650 firmware but is never overwritten.
*  The actual **OTA image**, which is your Contiki application.  This will change in the future as you compile and release new versions.  The very first OTA image is actually flashed manually from a .bin file.

## Quick Start
Please note that there are a handful of changes made to the Contiki tree outside of this example folder.  You cannot simply copy this example into the vanilla Contiki tree.  You must clone this fork and and compile using it.

```bash
git clone https://github.com/msolters/contiki
git submodule update --recursive --init
```

### Compile Firmware
To use this example out-of-the-box, just run the `./make-master-hex.sh` script, and flash the resulting `firmware.hex` to your Sensortag.  

`make-master-hex.sh` will compile the `/bootloader`, and then compile `/ota-image-example` as your initial "OTA image".  `ota-image-example` is a very simple Contiki app that simply blinks the red LED, and then starts an OTA download request.

The script will then generate some metadata for you (version number, UUID value, etc.), and finally fuse the bootloader and initial OTA image all into one big `firmware.hex` that you can conveniently flash to the Sensortag via e.g SmartRF Programmer 2.

### Start a Firmware Server
Now, you just need an OTA image server.  You can clone my working OTA server (written in NodeJS) [from here](https://github.com/msolters/ota-server).

This server can be run by simply invoking

```
node ota-server.js
```

The `ota-server` repo includes another compiled Contiki app, `ota-image-example.bin`.  This is merely a variant of the `ota-image-example` found here, except it blinks the *other* LED.

The only rule with the OTA server is that it must be run on a host that your Sensortag can resolve!  Using 6LBR as a BR, `bbbb::` is typically a decent choice for subnet.  Therefore,  `ota-server.js` comes out of the box with a default IP address of `bbbb::1` and will listen on port `3003`.  If your network is considerably different, these values can be very easily changed by looking at the first few lines of `ota-server.js`.  Please note that you do not have to use IPv6 for this.  You can use IPv4 values such as `http://10.0.0.1:80` or whatever you want!

>  Note: If you change the IP/port host configuration of `ota-server.js`, make sure to make the same changes to the `#define OTA_IMAGE_SERVER` inside `ota/ota-download.h`.  Then recompile with `./make-master-hex.sh` and reflash your Sensortag!

Once your OTA image server is up, the Sensortag should begin downloading the new `ota-image-example.bin`.  This can take several minutes!  You can check the progress via UART.  When the download is complete, the Sensortag should reboot, and the bootloader should update the internal OTA image with the new download.  Then, you should see the Sensortag start to blink the green LED!  Congratulations!

## Overview of OTA Update Mechanism
Internal Flash Memory Map

Description | Starting Position in EEPROM (bytes) | Space Allotted
--- | --- | ---
Bootloader | 0x00000000 | 0x00002000
OTA image | 0x00002000 | 0x00019000
bootloader ccfg | 0x0001FFA8 | 0x58

The overall concept of the OTA update mechanism is that the CC2650 will *always* power-up to the **bootloader**.

1.  The bootloader will first check the internal flash to see if the current **OTA image** is valid.
1.  The bootloader will next check the external flash chip (required, included by default in the Sensortag boards) to see if there are any newer **OTA images** that have been downloaded from the server.
1.  If there (a) are newer OTA images, or (b) if the current OTA image is invalid, the bootloader will overwrite the internal OTA image with the most recent download.
1.  Finally, the bootloader continues on to allow the internal OTA image firmware to execute as usual.

>  Note:  The bootloader does not do any downloading!  This is the job of the OTA images themselves.

## Making your own Contiki App OTA-ready
So you already have your Contiki app, and you just want to make it work like the `ota-image-example` here.  This is actually very easy.  By following the steps here, you can use the same exact `bootloader` provided here to send your own custom firmware over-the-air to your motes from an HTTP OTA image server.

>  Note:  First, make sure you are working inside this fork; these methods will not work in vanilla Contiki.

### Copy the OTA feature source
Copy `/ota` into your project's root folder.  We will need the source inside.

### Update Makefile
Make sure you have the following lines in your project's makefile:

```
CONTIKI_WITH_IPV6 = 1
MODULES += core/net/http-socket

OTA=1
OTA_SOURCE = ../ota
vpath %.c $(OTA_SOURCE)
CFLAGS += -I$(OTA_SOURCE)
PROJECT_SOURCEFILES += ota.c ota-download.c
```

The `OTA_SOURCE` variable should point to the location of the `/ota` folder you copied.

### Configure your OTA Image server URL
Make sure the IP address and port that your `ota-server.js` is running on is reflected in the `#define OTA_IMAGE_SERVER` value found inside `ota/ota-download.h` before you compile.  Default value is below:

```c
#define OTA_IMAGE_SERVER  "http://[bbbb::1]:3003"
```

### Include the OTA Download Headers
Make sure your Contiki app uses the following headers:

```c
#include "ota-download.h"
```

### Start an OTA Download in your app
Provided the above steps have been followed, you can use the following code to start the OTA download process in your Contiki app:

```c
process_start(ota_download_th_p, NULL);
```




## Clone Code
It is critical to pull in the GIT submodules which contain TI's RTOS drivers.


## Dependencies
Assuming a Debian machine there are a few specific tools needed to compile Contiki 3.0 and merge the resulting Intel-format .hex files.

```bash
# cc26xx requires a very specific gcc-arm compiler version
sudo apt-get remove binutils-arm-none-eabi gcc-arm-none-eabi
sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded

sudo apt-get update
sudo apt-get install srecord gcc-arm-none-eabi
```

## Building
Here are the steps to compiling the bootloader, a simple OTA image example, and then merging them into a single flashable image (called `firmware.hex`).

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

Equivalently, just execute `make-all.sh` from the `/cc26xx-ota-bootloader` directory.

## Flashing
The resulting firmware.hex can be flashed to the cc26xx using TI's SmartRF Flash Programmer 2.
