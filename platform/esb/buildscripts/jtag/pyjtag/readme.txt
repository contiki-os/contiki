pyJTAG
------

Software to talk to the parallel port JTAG PCB as seen with the FET kits.
It is released under a free software license,
see license.txt for more details.

(C) 2002-2003 Chris Liechti <cliechti@gmx.net>

Features
--------

- understands TI-Text and Intel-hex
- download to Flash and/or RAM, erase, verify
- reset device
- load addres into R0/PC and run
- upload a memory block MSP->PC (output as binary data or hex dump)
- written in Python, runs on Win32, Linux, BSD
- use per command line, or in a Python script

Requirements
------------
- Linux, BSD, Un*x or Windows PC
- Python 2.0 or newer, 2.2 recomeded
- Parallel JTAG hardware with an MSP430 device connected

Installation
------------
Python installations are available from www.python.org. On Windows simply
use the installer. The win32all package has an installer too. These
installations should run fine with the deafults.

On Linux just Python is needed. On some distributions is Python 1.5.2
installed per default. You may meed to change the first line in the script
from "python" to "python2". Maybe Python 2.x is in a separate package that
has to be installed. There are rpm and deb binary packages and a source
tarball available through the Python homepage.

The pyjtag archive can simply be unpacked to a directory, Windows users
can use WinZip or WinRar among others to extract the gzipped tar file.
If you want to run it from everywhere the directory where the file jtag.py
is, should be added to the PATH.
Look at "~/.profile", "/etc/profile" on Linux, "autoexec.bat" on Win9x/ME,
System Properties/Environment in Win2000/NT/XP.

_parjtag.so/dll from the jtag archive must be copied to the same directory as
jtag.py. On Windows also MSP430mspgcc.dll and HIL.dll must be located in the
same dir or somewhere in the PATH.

Short introduction
------------------
This software uses the JTAG hardware that comes with the FET kits. It is
connected to the parallel port.

The program can be started by typing "python jtag.py" in a console. Often
it works also with just "jtag.py" or "./jtag.py".

USAGE: jtag.py [options] [file]
If "-" is specified as file the data is read from the stdinput.
A file ending with ".txt" is considered to be in TIText format all
other filenames are considered IntelHex.

General options:
  -h, --help            Show this help screen.
  -l, --lpt=name        Specify an other parallel port.
                        (defaults to LPT1 (/dev/parport0 on unix)
  -D, --debug           Increase level of debug messages. This won't be
                        very useful for the average user...
  -I, --intelhex        Force fileformat to IntelHex
  -T, --titext          Force fileformat to be TIText
  -f, --funclet         The given file is a funclet (a small program to
                        be run in RAM)
  -R, --ramsize         Specify the amont of RAM to be used to program
                        flash (default 256).

Program Flow Specifiers:

  -e, --masserase       Mass Erase (clear all flash memory)
  -m, --mainerase       Erase main flash memory only
  --eraseinfo           Erase info flash memory only (0x1000-0x10ff)
  --erase=address       Selectively erase segment at the specified address
  -E, --erasecheck      Erase Check by file
  -p, --program         Program file
  -v, --verify          Verify by file

The order of the above options matters! The table is ordered by normal
execution order. For the options "Epv" a file must be specified.
Program flow specifiers default to "p" if a file is given.
Don't forget to specify "e" or "eE" when programming flash!
"p" already verifies the programmed data, "v" adds an additional
verification though uploading the written data for a 1:1 compare.
No default action is taken if "p" and/or "v" is given, say specifying
only "v" does a check by file of a programmed device.

Data retreiving:
  -u, --upload=addr     Upload a datablock (see also: -s).
  -s, --size=num        Size of the data block do upload. (Default is 2)
  -x, --hex             Show a hexadecimal display of the uploaded data.
                        (Default)
  -b, --bin             Get binary uploaded data. This can be used
                        to redirect the output into a file.
  -i, --ihex            Uploaded data is output in Intel HEX format.
                        This can be used to clone a device.

Do before exit:
  -g, --go=address      Start programm execution at specified address.
                        This implies option "w" (wait)
  -r, --reset           Reset connected MSP430. Starts application.
                        This is a normal device reset and will start
                        the programm that is specified in the reset
                        interrupt vector. (see also -g)
  -w, --wait            Wait for <ENTER> before closing parallel port.


Examples
--------
These examples assume that you have added the installation directory to
the PATH. Type the full path to jtag.py otherwise and maybe use
"python jtag.py". Depending on installation it may also appear under the
name "msp430-jtag".

jtag.py -e
        Only erase flash.

jtag.py -eErw 6port.a43
        Erase flash, erase check, download an executable, run it (reset)
        and wait.

jtag.py -mS -R 2048 6port.a43
        Use ramsize option on a device with 2k RAM to speed up
        download. Of course any value from 128B up to the maximum
        a device has is allowed.
        The progress and mainerase options are also activated.
        Only erasing the main memory is useful to keep calibration
        data in the information memory.

jtag.py 6port.a43
        Download of an executable to en empty (new or erased) device.
        (Note that in new devices some of the first bytes in the
        information memory are random data. If data should be
        downloaded there, specify -eE.)

jtag.py --go=0x220 ramtest.a43
        Download a program into RAM and run it, may not work
        with all devices.

jtag.py -f blinking.a43
        Download a program into RAM and run it. It must be
        a special format with "startadr", "entrypoint",
        "exitpoint" as the first three words in the data
        and it must end on "jmp $". See MSP430debug sources
        for more info.

jtag.py -u 0x0c00 -s 1024
        Get a memory dump in HEX, from the bootstrap loader.
        or save the binary in a file:
          "python jtag.py -u 0x0c00 -s 1024 -b >dump.bin"
        or as an intel-hex file:
          "python jtag.py -u 0x0c00 -s 1024 -i >dump.a43"

jtag.py -r
        Just start the user program (with a reset).

cat 6port.a43|jtag.py -e -
        Pipe the data from "cat" to jtag.py to erase and program the
        flash. (un*x example, don't forget the dash at the end of the
        line)

History
-------
1.0     public release
1.1     fix of verify error
1.2     use the verification during programming
1.3     meinerase, progress options, ihex output

References
----------
- Python: http://www.python.org

- Texas Instruments MSP430 Homepage, links to Datasheets and Application
  Notes: http://www.ti.com/sc/msp430

