Example to program the AVR Raven Jackdaw USB Stick

To program enter:
make clean
make ravenusbstick.eu
make ravenusbstick.u

To change the channel:
* Edit platform/avr-ravenusb/contiki-conf.h : line 290
#define CHANNEL_802_15_4          26
* Upload both eeprom (.eu) and flash (.u) again.

To change programmer:
* Edit platform/avr-ravenusb/Makefile.avr-ravenusb : line 48
AVRDUDE_PROGRAMMER=jtag2


To change jackdaw to enable MAC support
* Edit platform/avr-ravenusb/contiki-conf.h : line 198
#define USB_CONF_MACINTOSH 0