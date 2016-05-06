#!/bin/bash

cd ./bootloader
make clean
make bootloader.hex

cd ../ota-image-simple
make clean
make ota-image-simple.hex

cd ..
srec_cat bootloader/bootloader.hex -intel -exclude 0x1000 0x1FFA8 ota-image-simple/ota-image-simple.hex -intel -crop 0x1000 0x1FFA8 -o firmware.hex -intel
