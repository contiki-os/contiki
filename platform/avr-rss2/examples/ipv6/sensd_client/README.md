Contiki client for sensd
========================

Sensd README describes the concept. sensd  code is on github.


sensd - A WSN Internet GW, hub, agent, proxy & cloud
====================================================

Authors
--------
Robert Olsson <robert@radio-sensors.com>
Jens Laas <jens.laas@uadm.uu.se>

Contributors
------------

Abstract 
--------
We've outlined, designed and implemented and very simple concept for WSN 
data sharing, including data collection, storage and retrieval using 
standard text tools. The concept restricts Internet access to WSN 
motes and acts agent not to expose motes directly for robustness and 
security reasons. Low level, physical or link WSN protocol can be used. 
including  6lowpan, RIME etc and any type of Radio Duty Cycling (RDC). 
sensd works on the application layer.  A TCP connection initiates an 
implicit "subscribe". The M2P model is currently supported.

Key concepts
------------

* Agent. sensd works as an agent and does not allow direct Internet
  access to motes. Recall motes are constrained in most aspects and 
  can not support many connections, has weak security etc.

* Hub. Share the data from the root or sink node over TCP. In effect sensor 
  data can be sent over Internet to be shared over among TCP active listeners. 
  The TCP connection initiates an implicit "subscribe".

* Proxy. The support proxy functions over ipv4 and well as ipv6. Sensd can 
  forward to a proxy on a public IP. The typical case is when GW is behind 
  a NAT.
 
* WSN RP "rendez-vous point". A concepts where data from various WSN nets  
  are merged. This models a "cloud service" functionality for WSN networks. 
  sensd can be used both to forward data to RP. It can also work as the RP.
  RP receiving WSN data and allowing multiple TCP listeners.

* All programs are written C, script in Java and bash. Designed for small
  footprint and with minimal dependencies. sensd runs on Raspberry Pi and 
  Openwrt.

* This work introduces a simple tag format for sensor data. The overall
  idea is that data tagging is an "agreement" between producers and consumer. 
  Tags are simple are description of the data. Example T=25.2. where T=
  is the tag 25.2 the value. Most likely this a temperature. But we 
  can't be sure since we don't know since this is an agreement between 
  the producer and the consumer. Tags are also used for identification.
  Example tags, E64= Where globally unique ID can used. Another more 
  relaxed example is TXT=  a user description. See docs.

* Geotagging and timestamping is supported via tags.

* Ecosystem support. There are telphone apps to for data monitoring and
  and plotting. Android app can act as WSN-agent and forward to proxy/RP.

* The concept also includes a mapping to URI (Unified Resource Identifier) 
  to form a WSN caching server similar to CoAP using http-proxy.

* Copyright. Open-Source via GPL. Projecet used github.com


Introduction
------------

This is collection of software to implement data monitoring and data collection
from WSN Wireless Sensor Networks. The goal is to have a very simple,
straight-forward and robust framework.

The scenario: One or several motes is connected to USB or serial port to gather
received information from connected WSN motes. Data can be visualized in
several ways.

*  Sensor data report can be transmitted and propagated throughout the
   Internet. sensd acts as server and sends incoming report to active
   listeners.

*  Data is kept in ASCII with tagging and ID information. Data is conveniently
   handled, copied and viewed with standard text utilities of your OS.

*  Last mote report is cached into the file system suitable for URI use. The
   Format is SID/TAG. Typical tags are EUI64 and unique serial numbers. The
   different TAGS are left for mote user to define. Although the TAGS used in
   our example setup are included in this draft for example purposes.


Both formats can easily be stored or linked directly in web tree to form a
URI to format WSN logging/datafile or caching service.

A daemon that reads WSN mote reports from USB/serial and stores data in a ASCII
data file. Default is located at _/var/log/sensors.dat_

Addtional components
--------------------

* seltag [More info] (https://github.com/herjulf/sensd/blob/master/seltag/README.md)

* js A set of Java-scripts can plot, print and visualize sensor data from 
  sensd directly in your web-browser.

* documentation and sample files.  [More info] (https://github.com/herjulf/sensd/blob/master/seltag/README.md)

* Read Sensors Android app.  [Source Code] (https://github.com/herjulf/Read-Sensors)


Datafile logging
----------------

Below is and example of the anatomy of a sensors.dat file we are currently using in our WSN
data collection networks.

	2012-05-22 14:07:46 UT=1337688466 ID=283c0cdd030000d7 PS=0 T=30.56  T_MCU=34.6  V_MCU=3.08 UP=2C15C V_IN=4.66

	2012-05-22 14:11:41 UT=1337688701 ID=28a9d5dc030000af PS=0 T=36.00  V_MCU=2.92 UP=12C8A0 RH=42.0 V_IN=4.13  V_A1=3.43  [ADDR=0.175 SEQ=33 RSSI=21 LQI=255 DRP=1.00]

Each line is a mote report. They start with date and time and are followed by a set of
tags. The tags is different for different motes. In other words they can
send different data. Essential is the ID which should be unique for each mote.

The information with brackets is information generated by the receiving mote
and is not a part the motes data. Typically RSSI (Receiver Signal Strength
Indicator) and LQI (Link Quality Indicator)


Internet sensor data
--------------------

Start sensd with the `-report` option. This enables reports to be transmitted
over IP to remote listeners. Default TCP port 1234.

Server side example:

	sensd -report -p 1234  -D /dev/ttyUSB0

Client side. Example using netcat:

	nc server-ip 1234

URI format
----------

URI (Unified Resource Identifier) displays the node ID and the tags in a file tree.
It is easy to export this into a web tree to form a URI similar to a CoAP gateway.

Example: In our case we have a unique sensor ID followed by the different data
fields represented by "tags".

	/tmp/WSN1-GW1/281a98d20200004a:
	DRP  ID  LQI  PS  RH  RSSI  SEQ  T  V_IN  V_MCU  ADDR

	/tmp/WSN1-GW1/28be51ce02000031:
	DRP  ID  LQI  PS  RH  RSSI  SEQ  T  UP  V_IN  V_MCU  ADDR

Read Temp from a sensor:

	cat /tmp/WSN1-GW1/281a98d20200004a/T
	19.44

And it's very easy to link this tree into a web-server.

GPS support
-----------

Positioning support has been added via GPS device connected to serial
or USB port. Tags added when enabled GWGPS_LON & GWGPS_LAT.
GPS code from. https://github.com/herjulf/gps_simple

Getting the source and building
-------------------------------

Code is stored in github. Typically procedure below is the very straight-
forward unix way:

	git clone http://github.com/herjulf/sensd
	cd sensd
	make

Put your binaries after your preference:

Pre-built binary versions
--------------------------

For x86:
Sensd and friends are available in Bifrost/Linux packages. Those packages are
statically linked and can be used on most x86 Linuxes. 32-bit compiled.

http://ftp.sunet.se/pub/Linux/distributions/bifrost/download/opt/opt-sensd-2.3-1.tar.gz


Use
---

The WSN data logging and caching concept is in actual use with Contiki, RIME
broadcast application.

Tips
----

One can use netcat to listen to reports:

Example:

	nc radio-sensors.com 1235

To save in file use nohup:

	nohup nc radio-sensors.com 1235 > /var/log/sensors.dat

As sensd used TCP and ASCII encoding. tetlnet and web-browsers can be used
as well.


