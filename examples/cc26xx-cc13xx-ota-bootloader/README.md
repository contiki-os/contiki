# CC26XX/CC13xx OTA
This is an example Contiki application that demonstrates how to send complete firmware updates over-the-air (OTA) to a Texas Instruments CC26xx/CC13xx.  (Default board is a sensortag/cc2650, can be set using BOARD)

The OTA system depends on compiling two separate binaries, which are both placed into the CC26xx/CC13xx's internal flash.

*  A **bootloader**, which remains permanently fixed in the CC26xx/CC13xx's flash.  It manages, verifies, and repairs the CC26xx/CC13xx firmware but is never overwritten.
*  The actual **OTA image**, which is your Contiki application.  This will change in the future as you compile and release new versions.  The very first OTA image is actually flashed manually from a .bin file.

You can also read my [article on OTA](http://marksolters.com/programming/2016/06/07/contiki-ota.html) for more detail about this example.

## Quick Start
Please note that there are a handful of changes made to the Contiki tree outside of this example folder.  You cannot simply copy this example into the vanilla Contiki tree.  You must clone this fork and and compile using it.  For example, the `apps/er-coap` used in this fork is derived from that found in the [6LBR](https://github.com/cetic/6lbr/tree/develop/apps/er-coap) tree.

```bash
git clone https://github.com/msolters/contiki
git submodule update --recursive --init
```

### Compile Firmware
To use this entire example out-of-the-box, just run

```bash
make master-hex
```

This will produce a single file called `firmware.hex`, which you can flash to your Sensortag.  

Building the master `firmware.hex` involves compiling the `/bootloader`, and then compiling `/ota-image-example` as your initial "OTA image" firmware.  `ota-image-example` is a very simple Contiki app that simply blinks the red LED, and then starts an OTA download request.

The script will then generate some metadata (see below for more about OTA metadata) for your OTA image (version number `0`, UUID value `0x0000abcd`, CRC checksum, etc.).  Finally, `srec_cat` is used to merge the bootloader and initial OTA image binaries into one big `firmware.hex`.  This allows you to flash a ready-to-use bootloader/firmware combo in one step using e.g SmartRF Programmer 2.

### Start a Firmware Server
Now, you just need an OTA image server.  You can clone my working OTA server (written in NodeJS) [from here](https://github.com/msolters/ota-server).

This server can be run by simply invoking

```
node ota-server.js ota-image-example.bin
```

The `ota-server` repo includes another compiled Contiki app, `ota-image-example.bin`.  This is merely a variant of the `ota-image-example` found here, except it blinks the *other* LED.

The only rule with the OTA server is that it must be run on a host that your Sensortag can resolve!  You can enter your OTA server's IP address by filling in the values of `#define OTA_SERVER_IP()` inside `ota/ota-download.h`.  Then recompile with `make master-hex` and reflash your Sensortag!

Once your OTA image server is up, the Sensortag should begin downloading the new `ota-image-example.bin`.  This can take several minutes!  You can check the progress via UART.  When the download is complete, the Sensortag should reboot, and the bootloader should update the internal OTA image with the new download.  Then, you should see the Sensortag start to blink the green LED!  Congratulations!

## Overview of OTA Update Mechanism
Internal Flash Memory Map

Description | Starting Position in EEPROM (bytes) | Space Allotted
--- | --- | ---
Bootloader | 0x00000000 | 0x00002000
OTA image | 0x00002000 | 0x0001B000
bootloader ccfg | 0x0001FFA8 | 0x58

The overall concept of the OTA update mechanism is that the CC26xx/CC13xx will *always* power-up to the **bootloader**.

1.  The bootloader will first check the internal flash to see if the current **OTA image** is valid.
1.  The bootloader will next check the external flash chip (required, included by default in the Sensortag boards) to see if there are any newer **OTA images** that have been downloaded from the server.
1.  If there (a) are newer OTA images, or (b) if the current OTA image is invalid, the bootloader will overwrite the internal OTA image with the most recent download.
1.  Finally, the bootloader continues on to allow the internal OTA image firmware to execute as usual.

>  Note:  The bootloader does not do any downloading!  This is the job of the OTA images themselves.

## Building your own OTA-Ready Contiki App
So you already have your Contiki app, and you just want to make it work like the `ota-image-example` here.  By following the steps here, you can use the same exact `bootloader` provided here to send your own custom firmware over-the-air to your motes from an HTTP OTA image server.

>  Note:  First, make sure you are working inside this fork; these methods will not work in vanilla Contiki.

### Get You a Bootloader!
To build *just* the bootloader, run

```
make bootloader
```

This will leave a `bootloader.hex` in the `/bootloader` folder.  You'll need this for the final step when we merge your custom firmware binary with the bootloader binary.

### Copy the OTA feature source
Copy `apps/ota` into your Contiki example's root folder.  We will need the source inside.

### Update Makefile
Make sure you have the following lines in your project's makefile:

```
PROJECTDIR?=.
APPDIRS += $(PROJECTDIR)/apps
APPS += er-coap rest-engine ota
OTA=1

CONTIKI_WITH_IPV6 = 1
```

The `APPDIRS` variable should point to the `apps` folder you copied in the previous step.

### Update `project.conf`
For CoAP to work correctly, we have to make some defines.  I've found the bare minimum is as follows in my project's `project.conf`:

```c
/*---------------------------------------------------------------------------*/
/* COAP                                                                      */
/*---------------------------------------------------------------------------*/
#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE           1280

/* Disabling TCP on CoAP nodes. */
#undef UIP_CONF_TCP
#define UIP_CONF_TCP                   0

/* Increase rpl-border-router IP-buffer when using more than 64. */
#undef REST_MAX_CHUNK_SIZE
#define REST_MAX_CHUNK_SIZE            256

/* Multiplies with chunk size, be aware of memory constraints. */
#undef COAP_MAX_OPEN_TRANSACTIONS
#define COAP_MAX_OPEN_TRANSACTIONS     2

/* Filtering .well-known/core per query can be disabled to save space. */
#undef COAP_LINK_FORMAT_FILTERING
#define COAP_LINK_FORMAT_FILTERING     0
#undef COAP_PROXY_OPTION_PROCESSING
#define COAP_PROXY_OPTION_PROCESSING   0
```

### Configure your OTA Image server URL
Make sure the IP address of `ota-server.js` is reflected in the `#define OTA_SERVER_IP()` line found inside `apps/ota/ota-download.h` before you compile.  Default value is to look for an OTA server at `bbbb::1`.  

```c
#define OTA_SERVER_IP() uip_ip6addr(&ota_server_ipaddr, 0xbbbb, 0, 0, 0, 0, 0, 0, 0x1)
```

Remember, the CoAP port is by default `5683`.

### Include the OTA Download Headers
Make sure your Contiki app uses the following headers:

```c
#include "ota-download.h"
```

### Start an OTA Download Thread to Update
To actually trigger a CoAP update attempt in Contiki:

```c
process_start(ota_download_th_p, NULL);
```

The logic of when to trigger this process is entirely up to you.  For example, you could trigger the download process in the callback of a COAP request which would be sent to the Contiki node when the server receives new firmware.

Just keep in mind:  when this process is complete, the device will reboot!  Once you start the `ota_download_th` thread, assume the device could reboot at any time.  Also, this version will continue to attempt to complete the download theoretically forever.  Feel free to implement an error catching or retry counting scheme if you like.

### Compile
With the above changes, you should be able to simply compile your app as per usual.  One important rule to follow when compiling firmware as an OTA image though -- it must be a .bin!  That's because we still need to add the OTA metadata to the firmware image, and the `generate-metadata` tool only works with .bin files.

>  Note:  It's important to use a .bin target because when producing & injecting OTA metadata, we need the raw byte-by-byte firmware image.  An Intel-format .hex file would have to be parsed into a memory buffer first, which introduces significant opportunity for error.

### Add OTA Metadata
OTA metadata consists of the following:

Metadata Property | Size | Description | Example
Version | `uint16_t` | This is an integer used to represent the version of the firmware.  This is the value used to determine how "new" a firmware update is.  The bootloader will always prefer OTA images with higher version numbers.  You should use a value of 0x0 for your initial factory-given firmware. | 0x0, 0x1, ... 0xffff
UUID | `uint32_t` | This is a unique integer used as an identifier for the firmware release.  This is primarily of use internally, to index software changes or to use as a hash of e.g. the commit # the firmware is based off. | 0xdeadbeef

There are two other OTA metadata properties -- CRC16 value (computed by the provided tool), CRC16 shadow value (computed by the recipient device to verify the image integrity) and firmware size (in bytes).  However, none of these require your direct input.

#### Creating OTA Metadata from the Firmware .bin
I have included a C program that will allow you to easily create OTA metadata, called `generate-metadata`.  When running `make master-hex`, it's automatically built from `generate-metadata.c`.  In case it's not, you can simply run `make generate-metadata`.

`generate-metadata` accepts 3 arguments; the path to the firmware .bin, the version number and the UUID.

>  Note:  Both version number and UUID should be given as **hex** integers.  Example usage:

```bash
make generate-metadata
./generate-metadata ota-image-example/ota-image-example.bin 0x0 0xdeadbeef
```

After running the program, you will get a `firmware-metadata.bin` file in the same directory as the `generate-metadata` executable.

#### Merging OTA Metadata with your Firmware Binary
Now, to complete the construction of your custom OTA image, all you need to do is merge the `firmware-metadata.bin` with the firmware .bin.  To do that, use the `srec_cat` utility:

```bash
srec_cat firmware-metadata.bin -binary ota-image-example/ota-image-example.bin -binary -offset 0x100 -o firmware-with-metadata.bin -binary
```

Obviously, you should replace `ota-image-example/ota-image-example.bin` with your own firmware binary.  This command will then ouput a final .bin file, `firmware-with-metadata.bin`.  This is a complete OTA image!  This is the type of .bin file that can be loaded using e.g. `ota-server.js`.  It contains appropriate metadata header and a Contiki app compiled with the `OTA=1` flag.

### Merge the Bootloader and the OTA Image
For the initial flash operation, we will need our Sensortag to have both a working bootloader and an initial OTA image.  To do this, we need to merge the bootloader and OTA image binaries into one .hex file.  To do that, we can once again use the `srec_cat` command:

```bash
srec_cat bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 firmware-with-metadata.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o firmware.hex -intel
```

While you may change the location of your `bootloader.hex` or `firmware-with-metadata.bin`, it is important to keep all numeric arguments as they are!  The final file produced by this operation is `firmware.hex`.  This will represent a working bootloader, as well as your own firmware compiled as an OTA image with OTA metadata!
