Document version: 2016-02-22

An ethernet-to-802.15.4 NAT/GW IP64
===================================

This project is heavily based on the rpl-border-router in the example directory
using the IP64 framework. This work is done Contiki/Thingsquare as the driver 
for enc28j60.

But rather than using slip as in the original example an ethernet interface 
the Microchip enc28j60 is used. The needed SPI-glue for AtMega's is added by 
this platform. (avr-rss2)

See the README.ETHERNET.md for ethernet module info.



