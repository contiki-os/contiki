This directory contains the contiki driver for the sensor (potentiometer) available on the 
NXP DR1199 board. This board is part of the NXP JN516x Evaluation Kit (see http://www.nxp.com/documents/leaflet/75017368.pdf).
The driver for the switches on the DR1199 are supported by `dev/dr1174` when compiled with the flag `SENSOR_BOARD_DR1199` set.

The dr1199 sensor code interfaces to contiki `core/lib/sensors.c` framework.
The code is specificaly for the JN516X platform, because it makes use of the platform\DK4 libraries 
of this JN516X SDK.
`examples/jn516x/rpl/coap-dr1199-node.c` shows an example on using this contiki driver.
