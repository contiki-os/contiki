#!/usr/bin/python

lladdr_6lbr='0212:7400:1466:5117'
lladdr_mote='0212:7400:146e:f407'
radio_dev='/dev/tty.usbserial-XBTOKIQP'
mote_dev='/dev/tty.usbserial-XBTO3BTS'
mote_baudrate=115200

from support import *

br=LocalNativeBR()
mote=TelosMote()
platform=MacOSX() #Linux()
