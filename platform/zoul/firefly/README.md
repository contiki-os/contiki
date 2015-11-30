Zolertia Firefly platform
============================================

![Zolertia Firefly breakout board][firefly]

The Firefly is a breakout board designed to inspire.

It exposes the most basic Zoul features, sporting only the most down-to-core features to work with the Zoul, providing the following:

* ARM Cortex-M3 with 512KB flash and 32KB RAM (16KB retention), 32MHz.
* ISM 2.4-GHz IEEE 802.15.4 & Zigbee compliant.
* ISM 868-, 915-, 920-, 950-MHz ISM/SRD Band.
* On-board printed PCB sub-1GHz antenna, alternatively u.FL for sub-1GHz/2.4GHz external antennas.
* AES-128/256, SHA2 Hardware Encryption Engine.
* ECC-128/256, RSA Hardware Acceleration Engine for Secure Key Exchange.
* Compatible with breadboards and protoboards.
* On-board CP2104/PIC to flash over USB.
* User and reset buttons.
* RGB LED to allow more than 7 colour combinations.
* Small form factor (53x25mm).

The Firefly can be seen as the "small brother" of the RE-Mote, with a slick design and a lower cost.

To work out of the box, the firefly includes a PCB antenna for the Sub-1GHz interface, as well as 2 x u.Fl connectors for 2.4GHz and sub-1GHz external antennas.

The firefly can be programmed and debugged over JTAG and USB.  The board has a CP2104 USB to serial converter with a PIC, it allows to program the CC2538 without having to manually to put the device in bootloader mode.

[firefly]: ../images/firefly.png "Zolertia RE-Firefly breakout board"
