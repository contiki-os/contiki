Atari
=====

The platform/atari/ directory is used for targeting an Atari 8-bit computer.
Most things are shared between the 6502-based targets so please consult
cpu/6502/README for further details.


The following Atari 8-bit Ethernet card is supported:

- Atari 8-bit Ethernet Project: Use driver cs8900a.eth with address $D500.

The 'disk' make goal requires HiassofT's dir2atr program. It is available at
http://www.horus.com/~hias/atari/ - either as source code (being part of the
'AtariSIO driver and utilities for Linux') or as Win32 binary program (being
part of the 'Atari Tools for Win32').
