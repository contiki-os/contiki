#! /usr/bin/env python

#------------------------------------------------------------#
# UDP example to forward data from a local IPv6 DODAG
# Antonio Lignan <alinan@zolertia.com>
# Install: sudo pip install requests
#------------------------------------------------------------#
import sys
import json
import datetime
from socket import*
from socket import error
from time import sleep
import struct
from ctypes import *
import requests
#------------------------------------------------------------#
ID_STRING      = "V0.1"
#------------------------------------------------------------#
PORT              = 5678
CMD_PORT          = 8765
BUFSIZE           = 1024
#------------------------------------------------------------#
# If using a client based on the Z1 mote, then enable by equal to 1, else if
# using the RE-Mote equal to 0
EXAMPLE_WITH_Z1   = 1
#------------------------------------------------------------#
IFTTT_URL         = "https://maker.ifttt.com/trigger/"
IFTTT_EVENT       = "test"
IFTTT_KEY         = ""
#------------------------------------------------------------#
# Message structure
#------------------------------------------------------------#
if EXAMPLE_WITH_Z1:
  var1 = "temperature"
  var2 = "x_axis"
  var3 = "y_axis"
  var4 = "z_axis"
else:
  var1 = "core_temp"
  var2 = "ADC1"
  var3 = "ADC2"
  var4 = "ADC3"

class SENSOR(Structure):
    _pack_   = 1
    _fields_ = [
                 ("id",                         c_uint8),
                 ("counter",                    c_uint16),
                 (var1,                         c_int16),
                 (var2,                         c_int16),
                 (var3,                         c_int16),
                 (var4,                         c_int16),
                 ("battery",                    c_uint16)
               ]

    def __new__(self, socket_buffer):
        return self.from_buffer_copy(socket_buffer)

    def __init__(self, socket_buffer):
        pass
#------------------------------------------------------------#
# Helper functions
#------------------------------------------------------------#
def print_recv_data(msg):
  print "***"
  for f_name, f_type in msg._fields_:
    print "{0}:{1} ".format(f_name, getattr(msg, f_name)),
  print
  print "***"
# -----------------------------------------------------------#
def start_client():
  now = datetime.datetime.now()
  print "UDP6-IFTTT server side application "  + ID_STRING
  print "Started " + str(now)
  try:
    s = socket(AF_INET6, SOCK_DGRAM)
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)

    # Replace address below with "aaaa::1" if tunslip6 has
    # created a tun0 interface with this address
    s.bind(('', PORT))

  except Exception:
    print "ERROR: Server Port Binding Failed"
    return
  print 'UDP6-IFTTT server ready: %s'% PORT
  print "msg structure size: ", sizeof(SENSOR)
  print

  while True:
    data, addr = s.recvfrom(BUFSIZE)
    now = datetime.datetime.now()
    print str(now)[:19] + " -> " + str(addr[0]) + ":" + str(addr[1]) + " " + str(len(data))

    msg_recv = SENSOR(data)
    print_recv_data(msg_recv)

    # Create an empty dictionary and store the values to send
    report = {}
    report["value1"] = msg_recv.counter
    if EXAMPLE_WITH_Z1:
      report["value2"] = msg_recv.temperature
    else:
      report["value2"] = msg_recv.core_temp
    report["value3"] = msg_recv.battery
    requests.post(IFTTT_URL + IFTTT_EVENT + "/with/key/" + IFTTT_KEY, data=report) 

#------------------------------------------------------------#
# MAIN APP
#------------------------------------------------------------#
if __name__ == "__main__":
  start_client()

