Coffee filesystem (CFS) examples
========================

Coffee is a very simple, relatively small and easy to use file system that you
are most likely going to be very familiar with if you have done any C file
access in the past. The notion is the same as on a normal PC: you open a file,
read and write to it and close it. Contiki will take care of the underlying
flash memory, giving you more time to focus on the real issues.

An extended explanation on CFS internals and how it works can be found at the
[CFS wiki page](https://github.com/contiki-os/contiki/wiki/Coffee-filesystem-example)

Supported hardware (tested or known to work)
--------------------------------------------------
* sky
* cc2538dk
* z1
* zoul
* wismote
* avr-raven

Coffee is known to work for the `sky` platform, however the examples currently
fails to compile due to ROM overflow.  Tweaking the platform configuration may
be necessary.
