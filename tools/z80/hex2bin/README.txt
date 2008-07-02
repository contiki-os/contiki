hexameter : Convert Intel hex files to a binary file
version 2.0.0
                             Copyright (c) 2003-2008, Takahide Matsutsuka.

1/ What is it?

It converts Intel hex files, which emitted by SDCC-Z80 compiler, to
a binary file.  You can attach additional prefix and/or suffix file
to comply the file with arbitrary binary format.
It provides a development environment of C language on PC-6001 and
other old computers.


2/ Installation

Installation can be done in following steps.

a) Download and install SDCC from http://sdcc.sf.net/
   Version 2.8.0 is tested.
   SDCC is a cross-compiler for Z80 and other 8bit CPUs.
   Extract an archive on your disk, say "c:/sdcc".

b) Copy hexameter.exe to any path-available directory. for exmaple
   "c:/sdcc/bin".


3/ Use

a) Write your own C code.

b) Compile your code using SDCC.
   While linking, you need to specify options for SDCC so that the
   code can be worked on your machine.
   See sample/Makefile for actual usage.

   % sdcc -mz80 -c YOUR_CODE1.c
   % sdcc -mz80 -c YOUR_CODE2.c

   This step creates a file YOUR_CODE1.o and YOUR_CODE2.o respectively,
   which run on Z80-based machine.

   % for target in YOUR_CODE1.o YOUR_CODE2.o; do echo $@ >> YOUR_LIB.lib $@; done

   This step makes a library file.

   sdcc -mz80 --no-std-crt0 --code-loc 0x840f --data-loc 0 -o YOUR_APP.ihx crt0.o -lYOUR_LIB.lib

   -mz80 specifies the Z80 mode.
   --code-loc specifies the start address of your object code.
   Basically, it depends on the machine and RAM size.  As for PC-6001,
   0x840f for 32kB, or 0xc40f for 16kB.  As for PC-6001mkII, it would
   be 0x800f.
   If you are not sure what you are about to do, just leave it as
   default 0x840f.
   --data-loc 0 specifies the code is followed by data.
   You can specify an arbitrary address instead.
   --no-std-crt0 indicates that you use your own crt0 instead of sdcc's
   default crt0 object.  The customized crt0.o file is in lib directory
   of this release.

c) Convert ihx file to cas file using hex2cas.

   hexameter YOUR_CODE.ihx [YOUR_CODE2.ihx ...]
   The ihx files are just attached in the specified order.

   Here you can take some options:
     -p <filename>   specify prefix file name.
     -s <filename>   specify suffix file name.
     -n <name>       specify PC-6001 cassette file name,
                     must be 6 characters or less.
     -v              verbose output
     -o <filename>   specify output file name
     -b <size>       size of the output file in hexadecimal bytes.
                     only if the size of the output is less than the size
		     specified, the trailing bytes will filled by zeroes.
		     note that it doesn't mean the size of output is
		     restricted by the given size.
     -h		     displays simple usage

   Prefixes and suffixes are provided by files, which should be in the same
   directory as hexameter.exe.
   mode1
   mode2
   mode5
   n88
   rom60
   rom62
   suffix

d) Example

   The following is a typical example to convert from ihx files to
   PC-6001 loadable cassette format.
   % hexameter -p mode2 -s suffix mycode1.ihx mycode2.ihx -o myapp.p6

e) Load your cas file into your 6001 emulator.

   I've checked it working on the following emulators:

   VirtualTrek
     http://www.geocities.com/emucompboy/
   iP6Win
     http://www.retropc.net/mm/pc6001/
   PC-6001VW
     http://bernie.hp.infoseek.co.jp/develop/pc6001vw.html


4/ Note

- While I've tested the tool on Cigwin on Win32, I think it works
  on other environments with a little work.

- SDCC has many isuues regarding compilation.  Don't blame me about them! :)


5/ History

3/29/2003 1.0.0 First version
4/20/2003 1.0.1 Mode option has been added
9/01/2007 1.1.0 ROM-mode has been added
9/28/2007 1.2.0 Customized crt0 has been introduced to clear global data
3/15/2008 1.3.0 VRAM options introduced
4/28/2008 1.4.0 Header file option introduced

5/17/2008 2.0.0 Migrated to hexameter, to support more flexible configurations


Enjoy!

http://www.markn.org/
markn@markn.org
