#!/bin/bash

rm firmware.hex

printf "Making OTA Bootloader\n"
cd ./bootloader
make clean
make bootloader.hex

printf "\nMaking OTA Example Image\n"
cd ../ota-image-example
make clean
make ota-image-example

printf "\nAdding metadata to OTA Example Image"
cd ..
rm firmware-with-metadata.bin
node generate-metadata.sh ota-image-example/ota-image-example.bin 0x0 0xabcd

printf "\nMerging OTA Example Image with Bootloader"
#srec_cat bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 ota-image-example/ota-image-example.hex -intel -crop 0x2000 0x1FFA8 -o firmware.hex -intel
srec_cat bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 firmware-with-metadata.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o firmware.hex -intel
