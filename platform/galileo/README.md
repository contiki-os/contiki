Intel Galileo Board
===================

This README file contains general information about the Intel Galileo board
support. In the following lines you will find information about supported
features as well as instructions on how to build, run and debug applications
for this platform. The instructions were only test in Linux environment.

Requirements
------------

In order to build and debug the following packages must be installed in your
system:
  * gcc
  * gdb
  * openocd

Moreover, in order to debug via JTAG or serial console, you will some extra
devices as described in [1] and [2].

Features
--------

This section presents the features currently supported (e.g. device drivers
and Contiki APIs) by the Galileo port.

Device drivers:
  * Programmable Interrupt Controller (PIC)
  * Programmable Intergal Timer (PIT)
  * Real-Time Clock (RTC)
  * UART

Contiki APIs:
  * Clock module
  * Timer, Stimer, Etimer, Ctimer, and Rtimer libraries

Standard APIs:
  * Stdio library (stdout and stderr only). Console output through UART 1
    device (connected to Galileo Gen2 FTDI header)

Building
--------

To build applications for this platform you should first build newlib (in
case it wasn't already built). To build newlib you can run the following
command:
```
$ ./platform/galileo/bsp/libc/build_newlib.sh
```

Once newlib is built, you are ready to build applications. To build
applications for Galileo platform you should set TARGET variable to 'galileo'.
For instance, building the hello-world application should look like this:
```
$ cd examples/hello-world/ && make TARGET=galileo
```

This will generate the 'hello-world.galileo' file which is a multiboot-
compliant [3] ELF image. This image contains debugging information and it
should be used in your daily development.

You can also build a "Release" image by setting the BUILD_RELEASE variable to
1. This will generate a Contiki stripped-image optimized for size.
```
$ cd examples/hello-world/ && make TARGET=galileo BUILD_RELEASE=1
```

Running
-------

In order to boot the Contiki image, you will need a multiboot-compliant
bootloader. In the bsp directory, we provide a helper script which builds the
Grub bootloader with multiboot support. To build the bootloader, just run the
following command:
```
$ platform/galileo/bsp/grub/build_grub.sh
```

Once Grub is built, we have three main steps to run Contiki applications:
prepare SDcard, connect to console, and boot image. Below follows
detailed instructions.

### Prepare SDcard

Mount the sdcard in directory /mnt/sdcard.

Copy Contiki binary image to sdcard
```
$ cp examples/hello-world/hello-world.galileo /mnt/sdcard
```

Copy grub binary to sdcard
```
$ cp platform/galileo/bsp/grub/bin/grub.efi /mnt/sdcard
```

### Connect to the console output

Connect the serial cable to your computer as showed in [2].

Choose one terminal emulator such as screen, putty or minicom. Make sure you
use keyboard SCO mode (on putty that option is at Terminal -> Keyboard, on
the left menu). Connect to /dev/ttyUSB0, use 115200 speed.

### Boot Contiki Image

Turn on your board. After a few seconds you should see the following text
in the screen:
```
Press [Enter] to directly boot.
Press [F7]    to show boot menu options.
```

Press <F7> and select the option "UEFI Internal Shell" within the menu. Once
you have a shell, run the following commands to run grub application:
```
$ fs0:
$ grub.efi
```

You'll reach de grub shell. Now run the following commands to boot Contiki
image:
```
$ multiboot /hello-world.galileo
$ boot
```

For now, we lack of UART support so you won't see any output. However, you can
use JTAG (see next section) to verify that the Contiki is running.

Debugging
---------

This section describes how to debug Contiki via JTAG. The following
instructions consider you have the devices: Flyswatter2 and ARM-JTAG-20-10
adapter (see [1]).

Attach the Flyswatter2 to your host computer with an USB cable. Connect the
Flyswatter2 and ARM-JTAG-20-10 adapter using the 20-pins head. Connect the
ARM-JTAG-20-10 adapter to Galileo Gen2 JTAG port using the 10-pins head.

Once everything is connected, run Contiki as described in "Running" section,
but right after loading Contiki image (multiboot command), run the following
command:
```
$ make TARGET=galileo debug
```

The 'debug' rule will run OpenOCD and gdb with the right parameters. OpenOCD
will run in background and its output will be redirected to a log file in the
application's path called LOG_OPENOCD. Once gdb client is detached, OpenOCD
is terminated.

If you use a gdb front-end, you can define the "GDB" environment
variable and your gdb front-end will be used instead of default gdb.
For instance, if you want to use cgdb front-end, just run the command:
```
$ make BOARD=galileo debug GDB=cgdb
```

References
----------

[1] https://communities.intel.com/message/211778

[2] http://www.intel.com/support/galileo/sb/CS-035124.htm

[3] https://www.gnu.org/software/grub/manual/multiboot/multiboot.html
