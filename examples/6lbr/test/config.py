#!/usr/bin/python

# Example of a native 6lbr interacting with a real slip-radio (radio_dev) and a real 6lbr-demo client (mote_dev)
iid_6lbr='0212:7400:1466:5117'
iid_mote='0212:7400:146e:f407'
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

iid_6lbr='0212:7402:0001:0101'
iid_mote='0212:7402:0002:0202'
radio_sock='localhost'
mote_dev='/dev/pts/3'
mote_baudrate=115200
channel=26

from support import *

br=LocalNativeBR()
mote=VirtualTelosMote()
platform=Linux() #MacOSX()

"""

"""
scenarios
define 1 to run the scenario, 0 to skip
"""
S0=1
S1=0
S2=0
S3=0
S4=0
S5=0
S6=0
S7=0
S8=0
S9=0
S10=0
S11=0
S12=0

"""
modes
define 1 to run each selected scenario in that mode, 0 to skip the mode
"""
mode_SmartBridgeManual=0
mode_SmartBridgeAuto=0
mode_Router=1
mode_RouterNoRa=0

