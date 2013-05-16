webserver IPv6 raven
====================

This example features a simple webserver running on top of the IPv6 contiki
stack. It differs from the generic webserver6 example in that wherever possible
data is stored in AVR flash memory instead of RAM.  In addition are cgi scripts
for displaying temperature and voltage sensors, and sleep and radio statistics,
on web pages.

A bare $make will use the avr-raven platform. Alternate compatible platforms
can be specified, e.g. $make TARGET=avr-atmega128rfa1. In that case the
webserver process and web content located in that platform directory will be
used.

The perl script /tools/makefsdata converts web content from (by default)
all the files in the /httpd-fs subdirectory into the c source file /httpd-fsdata.c.
This file is not deleted in a $make clean so that a rebuild can be done without
perl in the toolchain (e.g. from a Windows cmd window). If any web file is
changed the dependencies will attempt to update httpd-fsdata.c. If perl
is not present, touching httpd-fsdata.c will give it a newer modification date
and allow the build to continue.

Two alternate web contents Huginn and Muninn are included in this directory.
Use e.g. $make WEBDIR=Huginn to generate a webserver6-huginn.elf file.
Different ipv6 address suffix and server names can be specified in the makefsdata.h
file of each directory so that mote addresses will not conflict.

webserver6.elf is always generated. It is also copied to another file whose
name indicates the platform and web directory. $make clean operates only the
specified target and webdir.  For example:

    make # builds webserver6.elf and webserver6-avr-raven.elf
    make WEBDIR=huginn # builds webserver6.elf and webserver6-huginn-avr-raven.elf
    make WEBDIR=muninn TARGET=avr-atmega128rfa1 # (builds webserver6.elf and webserver6-muninn-avr-atmega128rfa1.elf)
    make clean WEBDIR=huginn TARGET=avr-atmega128rfa1 # deletes the avr-atmega128rfa1 objects and webserver6-huginn-avr-atmega128rfa1.elf
    make clean # deletes the avr-raven objects and webserver6-avr-raven.elf


    make WEBDIR=xxx

always forces regeneration of web content into httpd-fsdata.c and so requires
PERL. A bare $make after that will not regenerate httpd-fsdata.c. Use

    make WEBDIR=default

to switch back to the default /http-fs/ content.

See Makefile.webserver for optional switches for RPL or a coffee file system.

    make UIP_CONF_RPL=1  # for a RPL node, or if rpl has become the contiki default,
    make UIP_CONF_RPL=0 # to override

Much head banging can result if you do not _make clean_ when changing make
options, as the normal build dependencies are bypassed and the needed object
modules may not be rebuilt.

Connect
-------

    make connect

will invoke the /tools/serial-log.pl perl script to connect to your serial
debug port. This will log to the console and optional log file, adding useful
time stamps. Edit the makefile for your serial port configuration.
