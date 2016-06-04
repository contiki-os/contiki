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

printf "\nAdding metadata to OTA Example Image\n"
cd ..
rm -f firmware-metadata.bin
rm -f generate-metadata
make generate-metadata
#CRC=`./generate-crc ota-image-example/ota-image-example.bin`
#node generate-metadata.js ota-image-example/ota-image-example.bin 0x0 0xabcd $CRC
./generate-metadata ota-image-example/ota-image-example.bin 0x1 0xabcd
srec_cat firmware-metadata.bin -binary ota-image-example/ota-image-example.bin -binary -offset 0x100 -o firmware-with-metadata.bin -binary

printf "\nMerging OTA Example Image with Bootloader\n"
srec_cat bootloader/bootloader.hex -intel -crop 0x0 0x2000 0x1FFA8 0x20000 firmware-with-metadata.bin -binary -offset 0x2000 -crop 0x2000 0x1B000 -o firmware.hex -intel
