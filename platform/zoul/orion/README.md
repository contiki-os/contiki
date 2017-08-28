Zolertia Orion Ethernet Router
============================================

![Zolertia Orion Ethernet Router][zolertia-orion-router]

The Zolertia Orion Ethernet Router allows to create IoT applications out-of-the-box, connecting any IPv4/IPv6 service or application to 6LoWPAN wireless networks.

The Orion Router is a capable IPv4/IPv6 routing device, with an Ethernet interface and dual wireless radio, powered either via micro-USB or Power Over Ethernet (POE).  The device integrates the ENC28J60 ethernet module, and an external POE module, supporting up to 48VDC.

Orion is compatible with [6lbr](https://github.com/cetic/6lbr/wiki), more information is available at our [Wiki](https://github.com/Zolertia/Resources/wiki/6lbr).

In a nutshell the Orion Router packs the following features:

* ARM Cortex-M3 with 512KB flash and 32KB RAM (16KB retention), 32MHz.
* ISM 2.4-GHz IEEE 802.15.4 & Zigbee compliant.
* ISM 868-, 915-, 920-, 950-MHz ISM/SRD Band.
* RP-SMA connector for a 2.4GHz external antenna
* SMA connector for a 868/915MHz external antenna
* RJ45 ethernet connector
* Ethernet 10BASE-T IPv4/IP64
* AES-128/256, SHA2 Hardware Encryption Engine.
* ECC-128/256, RSA Hardware Acceleration Engine for Secure Key Exchange.
* On-board CP2104/PIC to flash over its micro-USB connector
* User and reset buttons.
* Power on/off button and LED to show power state
* RGB LED to allow more than 7 colour combinations.
* Indoor enclosure
* Layout 40.29 x 73.75 mm

There are ready-to-use examples at `examples/zolertia/zoul/orion`, showing how to deploy an IP64 border router, and connect to services such as [IFTTT (If This Then That)](https://ifttt.com).

Orion Technical documentation
=============

* [Datasheet](https://github.com/Zolertia/Resources/tree/master/Orion%20Ethernet%20Router/Hardware/Revision%20A/Datasheets)
* [Schematics](https://github.com/Zolertia/Resources/tree/master/Orion%20Ethernet%20Router/Hardware/Revision%20A/Schematics)

Orion Ethernet Router pin-out
=============

![Zolertia Orion Ethernet Router pin-out][zolertia-orion-pinout]

[zolertia-orion-pinout]: ../images/orion-pinout.png "Zolertia Orion Router pin-out"
[zolertia-orion-router]: ../images/orion-router-front.png "Zolertia Orion Ethernet Router"
