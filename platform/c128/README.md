Commodore 128
=============

The platform/c128/ directory is used for targeting a Commodore 128 computer.
Most things are shared between the 6502-based targets so please consult
cpu/6502/README.md for further details.

The following C64 Ethernet cards are supported:

- RR+RR-Net: Use driver cs8900a.eth  with address $DE08.
- TFE:       Use driver cs8900a.eth  with address $DE00.
- ETH64:     Use driver lan91c96.eth with address $DE00.

In most cases it is desirable to use an emulator for the development and
testing of a Contiki application. VICE is especially well suited as it emulates
both the RR-Net and TFE Ethernet cards. It is available at
[http://www.viceteam.org](http://www.viceteam.org).

The c128 target supports a PFS that requires less RAM than the POSIX file
system and converts UNIX path names to CMD syntax for CMD drives.
