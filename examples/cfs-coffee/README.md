Contiki File System (CFS) and Coffee Examples
=============================================

Coffee is a very simple, relatively small and easy to use file system that you
are most likely going to be very familiar with if you have done any C file
access in the past. The notion is the same as on a normal PC: you open a file,
read and write to it and close it. Contiki will take care of the underlying
flash memory, giving you more time to focus on the real issues.

Coffee is a full implementation of the CFS API.

An extended explanation on CFS and Coffee internals and how they work can be
found at the [CFS](https://github.com/contiki-os/contiki/wiki/File-systems) and
[Coffee](https://github.com/contiki-os/contiki/wiki/Coffee-filesystem-example)
wiki pages.

Supported Hardware (tested or known to work)
--------------------------------------------
* sky
* z1
* wismote
* avr-raven
* cc2538dk
* openmote-cc2538
* zoul

The examples are known to build for the 'avr-raven' platform. However,
some of them currently fail at runtime due to file system overflow.
Tweaking the file sizes in the examples is necessary.
