* MSPSim version 0.9x

MSPSim is a Java-based instruction level emulator of the MSP430 series
microprocessor and emulation of some sensor networking
platforms. Supports loading of IHEX and ELF firmware files, and has
some tools for monitoring stack, setting breakpoints, and profiling.

* System requirements

You need a recent Java to run MSPSim. Java SE 1.5 or newer is
recommended. The current version of MSPSim also requires make for
compiling.

* Building MSPSim

You will build MSPSim by typing

>make

* Running examples

Run the default example on the ESB node emulator by typing:

>make runesb

(here you can pass the PIR with the mouse and click on the
 user button to get reaction from the example application).


Run the default example on the Sky node emulator by typing:

>make runsky

(this is a leds-blinker only and does not react to any mouse
 movements or button clicks).


* Main Features
- Instruction level emulation of MSP430 microprocessor
- Supports loading of ELF and IHEX files
- Easy to add external components that emulates external HW
- Supports monitoring of registers, adding breakpoints, etc.
- Built-in profiling of executed code
- Statistics for various components modes (on/off, LPM modes, etc).
- Emulates some external hardware such as TR1001 and CC2420.
- Command Line Interface, CLI, for setting up breakpoints and output
  to files or windows.
- GDB remote debugging support (initial)

* What is emulated of the MSP430
- CPU (instruction level simulation)
- Timer A/B subsystem
- USARTs
- Digital I/O
- Multiplication unit
- Basic A/D subsystem (not complete)
- Watchdog

* Limitations of the emulation (some of them) on version 0.9x
- currently the emulator runs as if it can use all memory as RAM
  (e.g. flash writes, etc not supported)
- no DMA implementation
- timer system not 100% emulated