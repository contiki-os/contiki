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
  * I2C
  * GPIO (default pinmux configuration is listed in
    platform/galileo/drivers/galileo-pinmux.c)
  * Intel Quark X1000 SoC message bus
  * Isolated Memory Regions (IMRs)

Contiki APIs:
  * Clock module
  * Timer, Stimer, Etimer, Ctimer, and Rtimer libraries

Standard APIs:
  * Stdio library (stdout and stderr only). Console output through UART 1
    device (connected to Galileo Gen2 FTDI header)

Optional support for protection domains is also implemented and is
described in cpu/x86/mm/README.md.

Preparation
-----------

Prerequisites on all Ubuntu Linux systems include texinfo and uuid-dev.
Additional prerequisites on 64-bit Ubuntu Linux systems include
gcc-multilib and g++-multilib.

Docker can optionally be used to prepare an Ubuntu-based, containerized build
environment. This has been tested with Docker installed on Microsoft Windows 10.

If not using a containerized environment, proceed to the "Building" section
below.

Using a Docker-based environment on Windows requires that the repository has
been checked out with Git configured to preserve UNIX-style line endings. This
can be accomplished by changing the 'core.autocrlf' setting prior to cloning
the repository [5]:
```
git config --global core.autocrlf input
```
Note that this is a global setting, so it may affect other Git operations.

The drive containing the repository needs to be shared with Docker containers
for the following steps to work [6].  Note that this is a global setting that
affects other containers that may be present on the host.

Open Microsoft PowerShell and navigate to the base directory of the repository.
Run the following command to create the build environment:
```
docker build -t contiki-galileo-build platform/galileo/bsp/docker
```
This creates a container named 'contiki-galileo-build' based on Ubuntu and
installs development tools in the container.

The build commands shown below can be run within the newly-created container. To
obtain a shell, run the following command in PowerShell from the base directory
of the repository.
```
docker run -t -i -v ${Pwd}:/contiki contiki-galileo-build
```
This command mounts the current directory and its subdirectories at the path
'/contiki' within the container. Changes to those files in the container are
visible to the host and vice versa. However, changes to the container
filesystem are not automatically persisted when the container is stopped.

The containerized build environment does not currently support building the Grub
bootloader nor debugging using the instructions in this document.

See the Docker Overview for more information about working with containers [7].

Building
--------

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

To restrict DMA so that peripherals are blocked from accessing memory
regions that do not contain any data that needs to be DMA-accessible,
specify X86_CONF_RESTRICT_DMA=1 as a command-line argument to the make
command that is used to build the image.  This will configure and lock
the IMRs.

Galileo Gen. 2 is targeted by default.  Specify GALILEO_GEN=1 on the build
command line to target first generation boards.

Running
-------

You will need a multiboot-compliant bootloader to boot Contiki images in that
format. However, this is not needed for booting UEFI images.

In the bsp directory, we provide a helper script which builds the Grub
bootloader with multiboot support. To build the bootloader, just run the
following command:
```
$ platform/galileo/bsp/grub/build_grub.sh
```

Once Grub is built, we have three main steps to run Contiki applications:
prepare SDcard, connect to console, and boot image. Below follows
detailed instructions.

### Prepare SDcard

The instructions in this section are for a native Linux development environment,
so equivalent operations should be substituted when using some other environment
(e.g. Windows Explorer can be used to perform equivalent operations when using
Docker on Windows as a development environment).

Mount the sdcard in directory /mnt/sdcard.

Create UEFI boot directory:
```
$ mkdir -p /mnt/sdcard/efi/boot
```

#### Approach for Multiboot-compliant ELF Image

Copy Contiki binary image to sdcard
```
$ cp examples/hello-world/hello-world.galileo /mnt/sdcard
```

Copy grub binary to sdcard
```
$ cp platform/galileo/bsp/grub/bin/grub.efi /mnt/sdcard/efi/boot/bootia32.efi
```

#### Approach for UEFI Image

Copy Contiki binary image to sdcard:
```
$ cp examples/hello-world/hello-world.galileo.efi /mnt/sdcard/efi/boot/bootia32.efi
```

### Connect to the console output

Connect the serial cable to your computer as shown in [8] for first generation
boards and [2] for second generation boards.

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

Waiting for the system to select the default boot device may be sufficient.
However, if this does not boot Contiki or Grub (depending on what is installed
as the UEFI boot image) then perform the following procedure after rebooting
and waiting for the boot message to appear: Press <F7> and select the option
"UEFI Misc Device" within the menu.

No additional steps should be required to boot a Contiki UEFI image.

Run the following additional commands to boot a multiboot-compliant image:
```
$ multiboot /hello-world.galileo
$ boot
```

### Verify that Contiki is Running

This should boot the Contiki image, resulting in the following messages being
sent to the serial console:
```
Starting Contiki
Hello, world
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

[5] https://www.git-scm.com/book/en/v2/Customizing-Git-Git-Configuration

[6] https://docs.docker.com/docker-for-windows/#/shared-drives

[7] https://docs.docker.com/engine/understanding-docker/

[8] https://software.intel.com/en-us/articles/intel-galileo-gen-1-board-assembly-using-eclipse-and-intel-xdk-iot-edition
