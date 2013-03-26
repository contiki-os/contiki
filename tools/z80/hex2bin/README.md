hexameter : Convert Intel hex files to a binary file
====================================================

version 2.1.0 Copyright (c) 2003-2008, Takahide Matsutsuka.

What is it?
-----------

It converts Intel hex files, which emitted by SDCC-Z80 compiler, to a binary
file.  You can attach additional prefix and/or suffix file to comply the file
with arbitrary binary format.  It provides a development environment of C
language on PC-6001 and other old computers.

Installation
------------

Installation can be done in the following steps.

1. Download and install SDCC from [http://sdcc.sf.net](http://sdcc.sf.net)
   Version 2.8.0 has been tested.  SDCC is a cross-compiler for Z80 and other
   8bit CPUs. Extract an archive on your disk, say "c:/sdcc".

2. Place hexameter.exe and prefix/suffix files to any path-available directory.

Use
---

1. Write your own C code.

2. Compile your code using SDCC. While linking, you need to specify options
   for SDCC so that the code can be worked on your machine.  See
   sample/Makefile for actual usage.

        sdcc -mz80 -c YOUR_CODE1.c
        sdcc -mz80 -c YOUR_CODE2.c

   This step creates a file YOUR_CODE1.o and YOUR_CODE2.o respectively, which
   run on Z80-based machine.

        for target in YOUR_CODE1.o YOUR_CODE2.o; do echo $@ >> YOUR_LIB.lib $@; done

   This step makes a library file.

        sdcc -mz80 --no-std-crt0 --code-loc 0x840f --data-loc 0 -o YOUR_APP.ihx crt0.o -lYOUR_LIB.lib

   - -mz80 specifies the Z80 mode.
   - --code-loc specifies the start address of your object code.  Basically, it
     depends on the machine and RAM size.  As for PC-6001, 0x840f for 32kB, or
     0xc40f for 16kB.  As for PC-6001mkII, it would be 0x800f.  If you are not
     sure what you are about to do, just leave it as default 0x840f.
   - --data-loc 0 specifies the code is followed by data.  You can specify an
     arbitrary address instead.
   - --no-std-crt0 indicates that you use your own crt0 instead of sdcc's
     default crt0 object.  The customized crt0.o file is in lib directory of
     this release.

3. Convert ihx file to cas file using hexameter

        hexameter <ihx_file|binary_file> [ihx_file|binary_file ...]

   The ihx files are just attached in the specified order.

   Here you can take some options:
     -o <filename>     specify output file name
     -d <name>=<value> define ihx-specific value replacement
     -v                verbose output
     -b <size>         size of the output file in hexadecimal bytes.
                       only if the size of the output is less than the size
       specified, the trailing bytes will filled by zeroes.
       note that it doesn't mean the size of output is
       restricted by the given size.
     -h       displays simple usage

   You can find various predefined library file in "lib" directory.
   Each of them may take its own optional value, which you can specify
   with -d option.

4. Example

   The following is a typical example to convert from ihx files to
   PC-6001 loadable cassette format.

        hexameter -v TAPE=myfile mode2.ihx mycode2.ihx -o myapp.p6

   Note that TAPE value is defined in mode2.ihx, defines cassette file name.

5. Load your cas file into your 6001 emulator.

   I've checked it working on the following emulators:

   [iP6Win](http://www.retropc.net/mm/pc6001)

IHX extentions
--------------

To support run-time user specified values in ihx files, Hexameter supports
extended ihx files.  Examples are located in ihx directory.

For example, mode2.ihx has the following line.

    :06000a02TAPE

As in normal ihx format, the first 9 characters conform to the following
format:

    :AABBBBCC
    AA    bytes encoded in this line
    BBBB  start address of this line
    CC    type of this line

The normal ihx file, which sdcc emits, has the type 00 (data) and 01 (end of
file).  In addition to this, Hexameter supports the following types.

    02    string
    03    byte
    04    word (encoded in little endian)

The rest of line defines a name of the definition.  In the example above, the
name TAPE is assigned to this line.  You can use latin alphabets, numbers, and
underscore for the name.  Letters are case-sensitive.

When you run Hexameter, you can specify a value for this definition like:

    hexameter -d TAPE=xxxx

Since this example defines 06 in the bytes field, TAPE can have up to six
characters.  This string will override the memory location 000a specified in
the second field.

For type 03 (byte) and 04 (word), bytes field has no effect.


Note
----

- Cygwin dependency has been removed.
- SDCC has many isuues regarding compilation.  Don't blame me about them! :)

History
-------

03/29/2003 1.0.0 First version
04/20/2003 1.0.1 Mode option has been added
09/01/2007 1.1.0 ROM-mode has been added
09/28/2007 1.2.0 Customized crt0 has been introduced to clear global data
03/15/2008 1.3.0 VRAM options introduced
04/28/2008 1.4.0 Header file option introduced

05/17/2008 2.0.0 Migrated to hexameter, to support more flexible configurations
07/16/2008 2.1.0 Support arguments in ihx file, remove prefix/suffix instead
07/18/2008 2.1.1 Cygwin dependency has been removed
10/02/2008 2.1.2 Template for sdos 1.1 has been added
12/14/2009 2.1.3 A minor bug in Linux environment has been fixed

Enjoy!

[http://www.markn.org](http://www.markn.org)
<markn@markn.org>
