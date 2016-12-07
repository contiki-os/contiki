Zolertia Firefly Revision A platform
============================================

![Zolertia Firefly Revision A breakout board][firefly-reva]

The Firefly is a breakout board designed to inspire.

It exposes the most basic Zoul features, sporting only the most down-to-core features to work with the Zoul, providing the following:

* ARM Cortex-M3 with 512KB flash and 32KB RAM (16KB retention), 32MHz.
* ISM 2.4-GHz IEEE 802.15.4 & Zigbee compliant.
* ISM 868-, 915-, 920-, 950-MHz ISM/SRD Band.
* On-board printed PCB sub-1GHz antenna and 2.4Ghz ceramic chip antenna.
* AES-128/256, SHA2 Hardware Encryption Engine.
* ECC-128/256, RSA Hardware Acceleration Engine for Secure Key Exchange.
* Compatible with breadboards and protoboards.
* On-board CP2104/PIC to flash over USB-A connector.
* User and reset buttons.
* RGB LED to allow more than 7 colour combinations.
* Small form factor (68.78 x 25.80mm).

The Firefly can be seen as the "small brother" of the RE-Mote, with a slick design and a lower cost.

To work out of the box, the firefly includes a PCB antenna for the Sub-1GHz interface, and a ceramic chip antenna for the 2.4GHz radio interface.  Optionally 2 x u.Fl connectors for 2.4GHz and sub-1GHz external antennas can be mounted.

The firefly can be programmed and debugged over JTAG and USB.  The board has a CP2104 USB to serial converter with a PIC, it allows to program the CC2538 without having to manually to put the device in bootloader mode.

The most prominent changes respect to the previous Firefly release are:

* On-board ceramic chip 2.4GHz antenna
* Pin-out changes and CC1200 GPIO2 pin is exposed
* RF matching improvements
* Printed PCB USB A connector instead of Micro-USB connector

Firefly pin-out
=============

![Firefly Revision A pin-out (front)][firefly-reva-pinout-front]

[firefly-reva-pinout-front]: ../images/firefly-pinout-front.png "Firefly Revision A pin-out (front)"
[firefly-reva]: ../images/firefly-reva.png "Zolertia Firefly Revision A breakout board"
