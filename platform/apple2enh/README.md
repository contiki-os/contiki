Apple II
========

The platform/apple2enh/ directory is used for targeting an Enhanced Apple //e
(or compatible) computer. Most things are shared between the 6502-based targets
so please consult cpu/6502/README for further details.

The following Apple II Ethernet cards are supported:

- Uthernet: Use driver cs8900a.eth  with address $C0x0 (x = 8 + slot number).
- LANceGS:  Use driver lan91c96.eth with address $C0x0 (x = 8 + slot number).

In most cases it is desirable to use an emulator for the development and testing
of a Contiki application. AppleWin is especially well suited as it emulates the
Uthernet card in slot 3. It is available at http://applewin.berlios.de/.

The 'disk' make goal requires AppleCommander 1.3.5 or later. It is available at
http://applecommander.sourceforge.net/.

The apple2enh target supports a PFS that requires much less RAM than the POSIX
file system. However this benefit comes with the following restrictions:

- Only one file may be open at the same time.
- Only read operations are supported but no write operations.
- The files must be located in the same directory the application is located in
  (the ProDOS prefix may very well point to another directory).
- The network MTU size must be set at least to 1265 (the default is 1500).
- No network function may be called while a file is open.
