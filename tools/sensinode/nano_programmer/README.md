# Nano Programmer

Programming tool for the Sensinode Nano series using Dxxx development boards.

Copyright 2007-2008 Sensinode Ltd.

## Installation

### Linux

No external libraries required.

### Windows/Cygwin

See the nano_usb_programmer README file on how to install FTDI library
for nano_usb_programmer. The nano_programmer build system will fetch
the library from there.

## Usage

Usage info for the Nano Programmer is available with command

    ./nano_programmer --help

Note that use might require root/administrator privileges
 depending on system configuration.

## Known problems

Occasional timing failures resulting in "Reinit failed."-messages do come
up in some PC configurations. If you experience programming failures (the programmer
is not able to recover), please report your system configuration to Sensinode.
On Linux, it is known that the "brltty" program causes problems with the FTDI
serial driver. Uninstalling brltty resolves the problem.

## Version

v1.3 2008-01-31 Martti Huttunen	 Multi-platform build and created instructions
