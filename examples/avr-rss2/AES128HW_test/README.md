AES128HW_test
=============

Scope
-----
Simple demo to use the AES crypto engine in the Atmel radios.

Build
-----
make TARGET=avr-rss2

Program output
--------------
*******Booting Contiki-3.x-3252-g0783591*******
I2C: AT24MAC
MAC address 7d:c2:
PAN=0xABCD, MAC=nullmac, RDC=nullrdc, NETWORK=Rime, channel=26, check-rate-Hz=128, tx-power=0
Routing Enabled

54657374737472696E675F5F5F5F5F5F Uncrypted 
6FDE14E8F9453C6714B7B45B24876CBF AES-128 EBC Crypted
54657374737472696E675F5F5F5F5F5F Decrypted 
215C9836DCDB443F15AFED576E9F7F72 AES-128 MIC

