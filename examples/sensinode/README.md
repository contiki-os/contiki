Sensinode platform example and test applications
================================================

by Zach Shelby <zach@sensinode.com>

Some more examples by George Oikonomou <oikonomou@users.sourceforge.net> -
Loughborough University

cc2431-location-engine, udp-ipv6, broadcast-rime blink-hello, event-post,
timer-test

This directory contains example and test applications for Sensinode CC2430
based devices. By default it is set to use the sensinode platform:

/platform/sensinode
/cpu/cc2430

To build an application:

    make [app_name]
    make hello_world

To build and upload an application using the Sensinode nano_programmer included
under /tools (default /dev/ttyUSB0):

    make [app_name].upload
    make hello_world.upload

To dump the serial port output (default /dev/ttyUSB0):

    make sensinode.serialdump

To configure the hardware model, you can include a make option e.g. for the
N601 (N100 is assumed by default):

    make hello_world DEFINES=MODEL_N601

These make options are defined in /platform/sensinode/Makefile.sensinode

Descriptions of applications:

- udp-ipv6: UDP client-server example over uIPv6. Uses link-local and global
  addresses. Button 1 on the client will send an echo request.

- broadcast-rime: Just a broadcast rime example, slightly modified

- sensors: Demonstrating button and ADC functionality

- cc2431-location-engine: Example demonstrating the usage cc2431 location
  engine (blind node) N.B. Not all sensinode devides have a cc2431

- event-post: Demonstrating the interaction between two processes with custom
  events

- blink-hello: Hello World with LED blinking.

- timer-test: Same as clock_test above + testing the rtimer-arch code

- border-router: 802.15.4 to SLIP bridge example. The node will forward packets
  from the 15.4 network to its UART (and thus a connected PC over SLIP)
