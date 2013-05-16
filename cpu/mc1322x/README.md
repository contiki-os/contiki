MC1322x
=======

libmc1322x is a library, build system, test code, and utilities for using the
mc13224v from Freescale.

Getting Started
---------------

    cd tests
    make

this will build all the test files in libmc1322x/tests for each board
defined in libmc1322x/board. You will have programs like:

rftest-tx_redbee-dev.bin
rftest-tx_redbee-r1.bin

rftest-rx_redbee-dev.bin
rftest-rx_redbee-r1.bin

if you only wanted to build binaries for one board you can do:

    make BOARD=redbee-dev

You can use mc1322x-load.pl in libmc1322x/tools to run your code:

    ../tools/mc1322x-load.pl -f rftest-tx_redbee-dev.bin

Incorporating libmc1322x into your own code
-------------------------------------------

The best way to incorporate libmc1322x into your code is as a git submodule:

    mkdir newproject
    cd newproject
    git init

Initialized empty Git repository in /home/malvira/newproject/.git/

    git submodule add git://git.devl.org/git/malvira/libmc1322x.git

This will add libmc1322x to your repository. Now to setup the Makefile:

    cp libmc1322x/tests/Makefile .

You need to edit the Makefile to point MC1322X to your libmc1322x submodule:

Change line 1

   MC1322X := ..

to

   MC1322X := libmc1322x

and edit COBJS and TARGETS accordings. COBJS are all of your common code for
any of your programs. TARGETS are the names of your programs.

For instance, you can have a common routine that prints a welcome message that
is used by two programs a and b. You would add common.o to COBJS:

   COBJS:= common.o

and your target line would read:

   TARGETS := a b

COBJS are made for each board so it is ok to have board specific code in there.
As an example, tests uses this in tests.c to print the name of the board in the
welcome message. You could also use this to change your GPIO mappings between
boards.
