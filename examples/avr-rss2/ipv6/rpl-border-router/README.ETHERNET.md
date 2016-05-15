Document version: 2015-12-29

HOWTO connect the Microchip enc28j60 module
===========================================

It's possible to to add Ethernet to most microcontrollers having a SPI inter-
face. In the Atmel world the SPI pins are used for the ISP programming. Usually 
a 6-pin header. Named MOSI, MISO, SCK, CS (Chup Select), Vcc, GND

One popular Ethernet chip is Microchip ENC28J60. It been used for quite some 
time and is easy to program and are also supported by many platforms and 
projects. Contiki has files under:

Most focus fon AVR platforms. The SPI is used by Atmel ISP-progammings
interface. In addition to those pins you to selects a pin for chip
select (CS).

dev/enc28j60/
The low-level SPI drives is platform or rather CPU-specific. The avr-rss2
platform adds the low-level drivers for Atmel.

platform/avr-rss2/enc28j60_avr.[ch]
This should be usable on most 8-bit avr platforms.

The power consumption might be a problem. The enc28j60 consumes 160mA at 3.3V 
when transmitting. The LDO (voltage reg. LP2950) on the mote can only handle 
100mA. So external power is needed. Below is a description of a prototype.
 
Used components:
----------------
Externel power supply 5V.
Small breadboard
DC-jack 
Voltage Reg. LCP1700 3.3V
Capacitor 4.7uF
enc28j60 (From Ebay)
Pin-headers
Cables   2x5 (enc28j60)  2x3 SPI (AtMega node) 

Used colors:
Black GND
Red VCC 3.3V
Yellow SCK
Green MISO
White MOSI
Blue CS (Chip select)

SPI (Connector view)
--------------------
GREEN  YELLOW   NC     
RED    WHITE    BLACK  

CS is connected to external temp sensor middle pin. Consequently the
external temp sensor can not be used in this application.
 
Pictures
--------
![Module] ( http://www.radio-sensors.com/pictures/enc28j60.jpg)
![Connector] (http://www.radio-sensors.com/pictures/enc28c60-cable.jpg)


References
-----------



