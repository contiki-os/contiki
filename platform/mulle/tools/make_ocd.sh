#!/bin/bash
sudo apt-get install libusb-1.0-0-dev
wget -O openocd.tar.bz2 http://downloads.sourceforge.net/project/openocd/openocd/0.7.0/openocd-0.7.0.tar.bz2
tar -xf openocd.tar.bz2
rm openocd.tar.bz2
mv openocd-0.7.0 openocd
cd openocd/
./configure --enable-ft2232_libftdi --enable-usbprog --enable-presto_libftdi --enable-stlink --enable-arm-jtag-ew --enable-jlink --enable-rlink --enable-vsllink --enable-usb_blaster_libftdi --enable-remote-bitbang --enable-vsllink --enable-ulink --enable-osbdm --enable-opendous --disable-shared --enable-ftdi
make
#sudo make install
