Nano USB Programmer
===================

An USB programmer for the Sensinode NanoRouter N600.

Copyright 2007-2008 Sensinode Ltd.

Installation
------------

### Linux

The installation is quite simple but requires the user to obtain the FTDI
development library. The installation also requires root privileges in some
phases (the ldconfig command to be more specific). Running the
Nano_USB_Programmer executable might also require root privileges.

- unpack the Nano_USB_Programmer-v[xxx].zip to a directory
- get the FTDI development library from
  [http://www.ftdichip.com/Drivers/D2XX/Linux/libftd2xx0.4.13.tar.gz](http://www.ftdichip.com/Drivers/D2XX/Linux/libftd2xx0.4.13.tar.gz)
- unpack the ftdi archive
- copy the library static_lib/libftd2xx.a.[version] into /usr/lib
- copy the library libftd2xx.so.[version] into /usr/lib
- make a symbolic link to the library, for example: ln -s
  /usr/lib/libftd2xx.so.0.4.13 /usr/lib/libftd2xx.so
- run ldconfig
- copy the header files into the nano_usb_programmer/ftdi_linux/ directory
- go to the programmer directory and run make

### Windows/Cygwin

- The FTDI library can be downloaded at:

[http://www.ftdichip.com/Drivers/CDM/CDM%202.02.04%20WHQL%20Certified.zip](http://www.ftdichip.com/Drivers/CDM/CDM%202.02.04%20WHQL%20Certified.zip)

- Copy header files (ftd2xx.h), ftd2xx.lib and ftd2xx.dll to nano_usb_programmer/ftdi_win32
- Copy the ftd2xx.dll to your windows system32 directory

Usage
-----

Usage info for the Nano_USB_Programmer is available with command
./nano_usb_programmer --help. Note that use might require root/administrator
privileges depending on system configuration.

Known problems (Linux)
----------------------

There's one known problem at the moment. The N600 must be unplugged and plugged
in again after it has been programmed or the MAC address has been read from it
before it can respond to the programmer again. The reason for this is the FTDI
library is not perfectly integrated with the Linux serial driver.

README Version
--------------

v1.0    2007-11-14  Mikko Saarnivala   Initial release
v1.1    2007-11-15  Mikko Saarnivala   A small error in the instructions fixed
v1.2    2007-11-19  Mikko Saarnivala   Added the FTDI CBUS2 value handling
v1.3    2008-01-31  Martti Huttunen    Multi-platform build and updated instructions
