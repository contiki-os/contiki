#!/bin/bash
mkdir tools
cd tools
wget https://launchpad.net/gcc-arm-embedded/4.8/4.8-2013-q4-major/+download/gcc-arm-none-eabi-4_8-2013q4-20131204-linux.tar.bz2
tar xf gcc-arm-none-eabi-4_8-2013q4-20131204-linux.tar.bz2
rm gcc-arm-none-eabi-4_8-2013q4-20131204-linux.tar.bz2
mv gcc-arm-none-eabi-4_8-2013q4 gcc-arm
