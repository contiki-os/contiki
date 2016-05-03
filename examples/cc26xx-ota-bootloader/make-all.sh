#!/bin/bash

cd ./bootloader
make bootloader.hex

cd ../ota-image-example
make ota-image-example

cd ..
srec_cat bootloader/bootloader.hex -intel -exclude 0x1000 0x1FFA8 ota-image-example/ota-image-example.hex -intel -crop 0x1000 0x1FFA8 -o firmware.hex -intel
