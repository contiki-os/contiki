This directory contains the contiki driver for the sensor (potentiometer) available on the 
NXP DR1199 board. This board is part of the NXP JN516x Evaluation Kit (see http://www.nxp.com/documents/leaflet/75017368.pdf).
The driver for the switches on the DR1199 are supported by `dev/dr1174` when compiled with the flag `SENSOR_BOARD_DR1199` set.

The dr1199 sensor code interfaces to contiki `core/lib/sensors.c` framework.
The code is specificaly for the JN516X platform, because it makes use of the platform\DK4 libraries 
of this JN516X SDK.
`examples/jn516x/rpl/coap-dr1199-node.c` shows an example on using this contiki driver.
leds-arch.c implements the led driver for leds D3 and D6 on the DR1174 base-board and the DR1199 board.
Mapping of LEDs on JN516x DR1199/DR1174:
    leds.h:          led on DR1174:
DR1174+DR1199:
    leds.h          physical leds
    LEDS_GREEN      LED D1 on DR1199                      
    LEDS_BLUE       LED D2 on DR1199
    LEDS_RED        LED D3 on DR1199
    LEDS_GP0        LED D3 on DR1174
    LEDS_GP1        LED D6 on DR1174
Note: LEDS_GPx definitions included in leds.h via platform-conf.h
