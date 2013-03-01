#!/usr/bin/python

# Example of a native 6lbr interacting with a real slip-radio (radio_dev) and a real 6lbr-demo client (mote_dev)
lladdr_6lbr='0212:7400:1466:5117'
lladdr_mote='0212:7400:146e:f407'
radio_dev='/dev/tty.usbserial-XBTOKIQP'
mote_dev='/dev/tty.usbserial-XBTO3BTS'
mote_baudrate=115200
channel=24

from support import *

br=LocalNativeBR()
mote=TelosMote()
platform=MacOSX() #Linux()

""" 
This next configuration is an example of a COOJA-based test on Linux. A native 6lbr interacts with a simulated
slip-radio mote through a local socket and a simulated mote on a hardcoded pseudo terminal 
TODO: make /dev/pts/# dynamic, launch cooja simulation & socat automatically

lladdr_6lbr='0212:7402:0001:0101'
lladdr_mote='0212:7402:0002:0202'
radio_sock='localhost'
mote_dev='/dev/pts/3'
mote_baudrate=115200
channel=26

from support import *

br=LocalNativeBR()
mote=VirtualTelosMote()
platform=Linux() #MacOSX()

"""


