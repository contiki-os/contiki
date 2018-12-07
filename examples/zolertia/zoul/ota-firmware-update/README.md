# Simple OTA Firmware Update Using CoAP
The demo shows how easy it is to update the firmware on TI CC2538 MCU using CoAP as an application protocol.
> Note: All tests were performed on the Zolertia RE-Mote BOARD, but same should apply on any TI CC2538 MCU board

## Content
The demo consists of a _bootloader_ and the _hello-world_ applications, each having a single CoAP resource `/firmware` and `/bootloader`, respectively. Python script `coap_firmware_update.py` is located under `zolertia/tools`, and it is used for downloading the application HEX file to a specific target.

## Bootloader app
The _bootloader_ is executed immediately after the first boot, and it's located in flash just after the `COFFEE-FS` (`COFFEE_START + COFFEE_SIZE`). It's purpose is to download the new firmware and save it in flash as well as change the execution address to a newly downloaded firmware, in case of successful OTA download.

## Hello World app
_hello-world_ is the main application. The location in the flash is defined by the `FLASH_CONF_FW_ADDR` and it needs to sit after the the _bootloader_. The application contains `/bootloader` resource which is used to give back the execution to the _bootloader_ in case OTA firmware update needs to be performed.

## Flash Memory Structure

| Location                  ||
|---------------------------|---------------|
| `0x00200000 - 0x00202000` | `COFFEE-FS`   |
| `0x00202000 - 0x00211000` | `bootloader`  |
| `0x00211000 - 0x0027F800` | `application` |

# Quick Start
## Flashing the bootloader
To build the bootloader go to `bootloader` directory and execute:

    make BOARD=remote bootloader
To upload the bootloader make sure target is connected over USB and execute:

    make BOARD=remote bootloader.upload
> Note: If the connection port is not automatically detected, export add `PORT` variable pointing to the USB device node, for example `PORT=/dev/ttyUSB0`

After successful upload, the red LED will blink, and the device will be ready for the OTA firmware update.

## Flashing the hello-world
To build the `hello-world` HEX file for OTA firmware update execute:

    make BOARD=remote hello-world.hex

In order to flash the newly build application use the `coap_firmware_update.py` located under `zolertia/tools`

    python coap_firmware_update.py <node address> hello-world.hex
> Note: The python script requires the following modules:  
> CRCMOD: `sudo pip install -U crcmod`  
> CoAPthon: https://github.com/Tanganelli/CoAPthon  
> IntelHex: `sudo pip install -U intelhex`

After successful flash is performed the device will reboot and the newly flashed application will be executed.

## Kicking it back to _bootloader_
Once the application is running, `/bootloader` resource is used to kick the target back in the bootloader mode. This is done by simply performing PUT method on the resource. If the variable `address` is passed with the HEX value of the _bootloader_ the target will reboot and execute from the specified address, otherwise the default address will be used.
