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

cd ..
srec_cat bootloader/bootloader.hex -intel -crop 0x0 0x1000 0x1FFA8 0x20000 ota-image-example/ota-image-example.hex -intel -crop 0x1000 0x1FFA8 -o firmware.hex -intel
