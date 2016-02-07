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
  * Ethernet

Contiki APIs:
  * Clock module
  * Timer, Stimer, Etimer, Ctimer, and Rtimer libraries

Standard APIs:
  * Stdio library (stdout and stderr only). Console output through UART 1
    device (connected to Galileo Gen2 FTDI header)

Building
--------

Prerequisites on all Ubuntu Linux systems include texinfo and uuid-dev.
Additional prerequisites on 64-bit Ubuntu Linux systems include
gcc-multilib and g++-multilib.

To build applications for this platform you should first build newlib (in
case it wasn't already built). To build newlib you can run the following
command:
```
$ ./platform/galileo/bsp/libc/build_newlib.sh
```

Once newlib is built, you are ready to build applications.  By default, the
following steps will use gcc as the C compiler and to invoke the linker.  To
use LLVM clang instead, change the values for both the CC and LD variables in
cpu/x86/Makefile.x86_common to 'clang'.

To build applications for the Galileo platform you should set the TARGET
variable to 'galileo'.  For instance, building the hello-world application
should look like this:
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

To also generate an '<application>.galileo.efi' file which is a UEFI [4] image,
you can run the following command prior to building applications:
```
$ cpu/x86/uefi/build_uefi.sh
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

#### Approach for Multiboot-compliant ELF Image

Copy Contiki binary image to sdcard
```
$ cp examples/hello-world/hello-world.galileo /mnt/sdcard
```

Copy grub binary to sdcard
```
$ cp platform/galileo/bsp/grub/bin/grub.efi /mnt/sdcard
```

#### Approach for UEFI Image

Copy Contiki binary image to sdcard
```
$ cp examples/hello-world/hello-world.galileo.efi /mnt/sdcard
```

### Connect to the console output

Connect the serial cable to your computer as shown in [2].

Choose a terminal emulator such as PuTTY. Make sure you use the SCO keyboard
mode (on PuTTY that option is at Terminal -> Keyboard, on the left menu).
Connect to the appropriate serial port using a baud rate of 115200.

### Boot Contiki Image

Turn on your board. After a few seconds you should see the following text
in the screen:
```
Press [Enter] to directly boot.
Press [F7]    to show boot menu options.
```

Press <F7> and select the option "UEFI Internal Shell" within the menu.

#### Boot Multiboot-compliant ELF Image

Once you have a shell, run the following commands to run grub application:
```
$ fs0:
$ grub.efi
```

You'll reach the grub shell. Now run the following commands to boot Contiki
image:
```
$ multiboot /hello-world.galileo
$ boot
```

#### Boot UEFI Image

Once you have a shell, run the following commands to boot Contiki image:
```
$ fs0:
$ hello-world.galileo.efi
```

### Verify that Contiki is Running

This should boot the Contiki image, resulting in the following messages being
sent to the serial console:
```
Starting Contiki
Hello World
```

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

[2] https://software.intel.com/en-us/articles/intel-galileo-gen-2-board-assembly-using-eclipse-and-intel-xdk-iot-edition

[3] https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

[4] http://www.uefi.org/
