# Lesson 1: basics examples and features

The following examples will show how to start working with Contiki and Zolertia devices.

The examples are written to be followed in order, so be sure to start from the very first and make your way to the last in an ordered fashion.

## Lesson objectives

The objective of the lessons are:

* Show you how a Contiki's applications are written and implemented
* How to use LEDs (light emitting diodes) and the user button
* Show how to use the different timers in Contiki
* Learn how Contiki processes are created and communicate amongst each other
* Learn about the on-board sensors in the Zolertia nodes
* Show how to use the analogue to digital converter, and implement your own analogue sensors
* Control the General Purpose Input/Output (GPIO) pins

We assume you have a working Contiki development environment, if you need help configuring the toolchain and others, please visit this page:

https://github.com/Zolertia/Resources/wiki

## Some handy commands

The following commands will make your life easier when working with Contiki and the Zolertia nodes:

### Print a list of connected devices:

To get a list of the devices currently connected over USB, and be able to address individually upon flashing or open a terminal for debugging, use the command below:

````
$ make motelist
using saved target 'z1'
../../../../tools/zolertia/motelist-zolertia
-------------- ---------------- ---------------------------------------------
Reference      Device           Description
-------------- ---------------- ---------------------------------------------
ZOL-RM01-A0935 /dev/ttyUSB0     Silicon Labs Zolertia RE-Mote platform
Z1RC5174       /dev/ttyUSB1     Silicon Labs Zolertia Z1

````

### Open a serial connection for debugging

As you will learn in the first lesson `01-hello-world`, you can print debug or general information to the screen using the serial-to-USB connection.  To open a connection you need to know the USB port to which the device is connected (use the command above).

````
make login MOTES=/dev/ttyUSB0
````

If you don't include the port, as default it will try to use the first port it founds, or the `/dev/ttyUSB0`.  If you have an USB connection already established, opening a second one will scramble both!

### Restart a Zolertia Z1 device without pressing the reset button

From the command line just type:

````
make z1-reset MOTES=/dev/ttyUSB0
````

If you don't specify an USB port, it will reset ALL devices connected!

### Restart a Zolertia RE-Mote device without pressing the reset button

This option is not available for the RE-Mote platforms and alike

## Compiling and flashing applications

Let's start with the first example `01-hello-world`.  To compile and flash the application to a Zolertia Z1 node type the following:

````
make TARGET=z1 01-hello-world.upload
````

If you execute as above it will use the default compilation target and first-found USB port.

You will notice the examples have a `Makefile.target`, enabling as default the `z1` as our compilation target.  This allows to compile without specifying the `TARGET` argument as follows:

````
make 01-hello-world.upload
````

To change the current `Makefile.target` and use for example `zoul` boards (like the `RE-Mote`), just run:

````
make TARGET=zoul savetarget
````

To explicitely define a target and USB port at compilation time use:

````
make TARGET=z1 01-hello-world.upload MOTES=/dev/ttyUSB0
````

You can also combine commands and in a single instruction compile, flash and debug with:

````
make TARGET=z1 01-hello-world.upload MOTES=/dev/ttyUSB0 && make login MOTES=/dev/ttyUSB0
````

If you have more than one Z1 connected to the USB ports at the same time, and do not specify a port when flashing, it will programm ALL devices connected! This is useful when you need to program several devices with the same code, but cumbersome if you forgot you are running an application on another device, so take note!

If you have a `RE-Mote` you can use as above:

````
make TARGET=zoul 01-hello-world.upload MOTES=/dev/ttyUSB0 && make login MOTES=/dev/ttyUSB0
````

If you have more than one `RE-Mote` connected at the same time, it will only flash the first it finds if no USB port is specified.

But of course, saving the `TARGET` in `Makefile.target` as described before will save you a lot of writting!

## Working without writting a line of code: using pre-compiled images and list connected devices

Notice there is a directory named `Binaries`, in here you will find already compiled images/binaries for you to flash directly to the Zolertia Z1 nodes.  These binaries are useful in case you need to test with a proven working binary, or avoid compiling and start working right away.

These following tools will come handy.

### List connected devices

Use directly the same script the `make login` command uses:

````
./motelist
````

### Program a binary to the Zolertia Z1 nodes

````
./flash-z1 --z1 -c /dev/ttyUSB0 -r -e -I -p 01-basics/Binaries/z1/01-hello-world.ihex
````

The `flash-z1` script takes as arguments the USB port and the binary to flash.

You should see something like:

````
MSP430 Bootstrap Loader Version: 1.39-goodfet-8
Mass Erase...
Transmit default password ...
Invoking BSL...
Transmit default password ...
Current bootstrap loader version: 2.13 (Device ID: f26f)
Changing baudrate to 38400 ...
Program ...
43059 bytes programmed.
Reset device ...
````


### Program a binary to the Zolertia RE-Mote nodes

The flashing script is located in `tools/cc2538-bsl`, to flash a device connected i.e in port `/dev/ttyUSB0` just type:

````
python flash-zoul.py -e -w -v -p /dev/ttyUSB0 -a 0x00202000 01-basics/Binaries/zoul/01-hello-world.bin
````

You should see something like:

````
Opening port /dev/ttyUSB0, baud 500000
Reading data from 01-basics/Binaries/zoul/01-hello-world.bin
Firmware file: Raw Binary
Connecting to target...
CC2538 PG2.0: 512KB Flash, 32KB SRAM, CCFG at 0x0027FFD4
Primary IEEE Address: 06:15:AB:25:00:12:4B:00
Erasing 524288 bytes starting at address 0x00200000
    Erase done
Writing 516096 bytes starting at address 0x00202000
Write 8 bytes at 0x0027FFF8F00
    Write done
Verifying by comparing CRC32 calculations.
    Verified (match: 0x6ba3e86c)
````

## Changing the Zolertia Z1 Node and MAC addresses

As default the Zolertia Z1 nodes uses the reference ID number (see the above `motelist` command result) to generate both its Node and MAC addresses.  To change to a different one use:

````
make clean && make burn-nodeid.upload nodeid=5072 nodemac=5072 MOTES=/dev/ttyUSB0 && make z1-reset MOTES=/dev/ttyUSB0 && make login MOTES=/dev/ttyUSB0
````

Where `5072` is just a made up number.

Note: DO NOT use 0 at the beginning of the address, as this is actually a compiler flag will give you an error, for example instead of `0158` just type `158`.

You will see an output like this:

````
Starting 'Burn node id'
Burning node id 158
Restored node id 158
````
