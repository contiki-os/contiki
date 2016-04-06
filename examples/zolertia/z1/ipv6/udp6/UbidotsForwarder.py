#! /usr/bin/env python
#------------------------------------------------------------#
# UDP example to forward data from a local IPv6 DODAG
# Antonio Lignan <alinan@zolertia.com>
#------------------------------------------------------------#
import sys
import json
import datetime
from socket import*
from socket import error
from time import sleep
import struct
from ctypes import *
from ubidots import ApiClient
#------------------------------------------------------------#
ID_STRING      = "V0.1"
#------------------------------------------------------------#
PORT              = 5678
CMD_PORT          = 8765
BUFSIZE           = 1024
#------------------------------------------------------------#
ENABLE_CLOUD      = 1
ENABLE_LOG        = 1
#------------------------------------------------------------#
DEBUG_PRINT_JSON  = 1
#------------------------------------------------------------#
# Ubidots constants
API_KEY     = "fd6c3eb63433221e0a6840633edb21f9ec398d6a"
TMP102_KEY  = "56fe8bc07625421d106f5d9f"
ACCEL_X_KEY = "56fe8bd57625421d302181da"
ACCEL_Y_KEY = "56fe8bec7625421d4e18f81a"
ACCEL_Z_KEY = "56fe8bf47625421e8694cb0e"
#------------------------------------------------------------#
# Message structure
#------------------------------------------------------------#
class SENSOR(Structure):
    _pack_   = 1
    _fields_ = [
                 ("id",                         c_uint16),
                 ("counter",                    c_uint16),
                 ("temperature",                c_int16 ),
                 ("x_axis",                     c_int8  ),
                 ("y_axis",                     c_int8  ),
                 ("z_axis",                     c_int8  ),
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
  print "\n***"
# -----------------------------------------------------------#
# Ubidots specifics
# -----------------------------------------------------------#
def get_ubidots_holders(api, a_key):
  try:
    my_specific_variable = api.get_variable(a_key)
  except UbidotsError400 as e:
    sys.exit("Ubidots error: %s: %s" % (e.message, e.detail))
  except UbidotsForbiddenError as e:
    sys.exit("Ubidots error: %s: %s" % (e.message, e.detail))
  return my_specific_variable
#------------------------------------------------------------#
# UDP6 session
#------------------------------------------------------------#
def start_client():
  now = datetime.datetime.now()
  print "UDP6 server side application "  + ID_STRING
  print "Started " + str(now)
  try:
    s = socket(AF_INET6, SOCK_DGRAM)
    s.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
    s.bind(('', PORT))

  except Exception:
    print "ERROR: Server Port Binding Failed"
    return
  print 'UDP server ready: %s'% PORT
  print "msg structure size: ", sizeof(SENSOR)
  print

  if ENABLE_CLOUD:
    try:
      client = ApiClient(API_KEY)
    except Exception as error:
      print error
      raise

    print "Connected to Ubidots, retrieving variables..."

    tmp102_var  = get_ubidots_holders(client, TMP102_KEY)
    accel_x_var = get_ubidots_holders(client, ACCEL_X_KEY)
    accel_y_var = get_ubidots_holders(client, ACCEL_Y_KEY)
    accel_z_var = get_ubidots_holders(client, ACCEL_Z_KEY)

    print "Done, looping forever now\n"

  while True:
    data, addr = s.recvfrom(BUFSIZE)
    now = datetime.datetime.now()
    print str(now)[:19] + " -> " + str(addr[0]) + ":" + \
          str(addr[1]) + " " + str(len(data))
    msg_recv = SENSOR(data)

    if ENABLE_LOG:
      print_recv_data(msg_recv)

    if ENABLE_CLOUD:
      tmp102_var.save_value(  {'value':int(msg_recv.temperature)} )
      accel_x_var.save_value( {'value':int(msg_recv.x_axis)}      )
      accel_y_var.save_value( {'value':int(msg_recv.y_axis)}      )
      accel_z_var.save_value( {'value':int(msg_recv.z_axis)}      )

  sleep(0.2)

#------------------------------------------------------------#
# MAIN APP
#------------------------------------------------------------#
if __name__ == "__main__":
  start_client()

