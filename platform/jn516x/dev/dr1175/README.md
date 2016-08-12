This directory contains the contiki driver for the sensors (light, humidity and temperature sensor) available on the 
NXP DR1175 board. This board is part of the NXP JN516X Evaluation Kit (see http://www.nxp.com/documents/leaflet/75017368.pdf). 

The dr1175 sensor code interfaces to the contiki `core/lib/sensors.c` framework.
The code is specificaly for the JN516X platform, because it makes use of the platform\DK4 libraries 
of this JN516X SDK.
`examples/jn516x/rpl/coap-dr1175-node.c` shows an example on using this contiki driver.

Mapping of LEDs on JN516x DR1175/DR1174:
    leds.h:         led on DR1175/DR1174:
DR1174+DR1175:
    LEDS_RED        Red led in RGB-led with level control on DR1175    
    LEDS_GREEN      Green led in RGB-led with level control on DR1175    
    LEDS_BLUE       Blue led in RGB-led with level control on DR1175    
    LEDS_WHITE      White power led with level control on DR1175
    LEDS_GP0        LEDS D3 on DR1174
    LEDS_GP1        LEDS D6 on DR1174
Note: LEDS_GPx and LEDS_WHITE definitions included in leds.h via platform-conf.h
