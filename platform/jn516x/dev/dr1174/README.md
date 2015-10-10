This directory contains the contiki driver for the button sensor on the DR1174 baseboard.
When used with an extention board, sensors from the dr1175 anf dr1179 directories are used in addition to this.
leds-arch.c implements the led driver for leds D3 and D6 on the board.
Mapping of LEDs on JN516x DR1174:
    leds.h:          led on DR1174:
    LEDS_GP0        LED D3
    LEDS_GP1        LED D6
Note: LEDS_GPx definitions included in leds.h via platform-conf.h
