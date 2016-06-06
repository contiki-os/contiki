Sniffer application mote side
=============================
This put the radio in sniff mode and should capure all traffic used
on the set channel. 

Default channel
---------------
26

Bulld
-----
make TARGET=avr-rss2

Default serial port speed
-------------------------
38400 bps

More info & uasage
------------------
Look in the host directory

Contiki support
---------------
The code promisc for support is needed. This also adds the sensniff 
format. 

References
----------
https://github.com/g-oikonomou/sensniff
