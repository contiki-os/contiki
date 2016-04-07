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
import paho.mqtt.client as mqtt
#------------------------------------------------------------#
ID_STRING      = "V0.1"
#------------------------------------------------------------#
PORT              = 5678
CMD_PORT          = 8765
BUFSIZE           = 1024
#------------------------------------------------------------#
ENABLE_MQTT       = 1
ENABLE_LOG        = 0
#------------------------------------------------------------#
DEBUG_PRINT_JSON  = 1
#------------------------------------------------------------#

MQTT_URL          = "mqtt.relayr.io"
MQTT_PORT         = 1883
MQTT_KEEPALIVE    = 60
MQTT_URL_TOPIC    = "v2/zolertia/tutorialthings/"
MQTT_URL_PUB      = "/v1/3a0a6fc7-e797-463a-9a5c-a7ce2f81cde4/data"
MQTT_USER         = "3a0a6fc7-e797-463a-9a5c-a7ce2f81cde4"
MQTT_PASSWORD     = "s1iLfyEpXzHs"
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
    print "{0}:{1} -".format(f_name, getattr(msg, f_name)),
  print
  print "***"
# -----------------------------------------------------------#
def publish_recv_data(data, pubid, conn, addr):
  try:
    res, mid = conn.publish(MQTT_URL_PUB, payload=data, qos=1)
    print "MQTT: Publishing to {0}... " + "{1} ({2})".format(mid, res, str(pubid))
  except Exception as error:
    print error
  
# -----------------------------------------------------------#
def jsonify(keyval, val):
  return json.dumps(dict(value=val, key=keyval))
# -----------------------------------------------------------#
def jsonify_recv_data(msg):

  # [{"meaning":"analog0","value":3},{"meaning":"test_counter","value":5}]
  sensordata = '[{"meaning":"acceleration","value":{'
  sensordata += '"x":' + str(msg.x_axis) + ','
  sensordata += '"y":' + str(msg.y_axis) + ','
  sensordata += '"z":' + str(msg.z_axis) + '}}]'
  return sensordata

# -----------------------------------------------------------#
# MQTT related functions
# -----------------------------------------------------------#
def on_connect(client, userdata, rc):
  print("MQTT: Connected ({0}) ").format(str(rc))
  client.subscribe(MQTT_URL_PUB + MQTT_URL_TOPIC)
#------------------------------------------------------------#
def on_message(client, userdata, msg):
  print("MQTT: RX: " + msg.topic + " : " + str(msg.payload))
#------------------------------------------------------------#
def on_publish(client, packet, mid):
  print("MQTT: Published {0}").format(mid)
#------------------------------------------------------------#
# UDP6 and MQTT client session
#------------------------------------------------------------#
def start_client():
  now = datetime.datetime.now()
  print "UDP6 server side application "  + ID_STRING
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
  print 'UDP server ready: %s'% PORT
  print "msg structure size: ", sizeof(SENSOR)
  print

  if ENABLE_MQTT:
    # Initialize MQTT connection
    try:
      client = mqtt.Client()
    except Exception as error:
      print error
      raise

    client.on_connect = on_connect
    client.on_message = on_message
    client.on_publish = on_publish

    # Set user and password
    client.username_pw_set(MQTT_USER, MQTT_PASSWORD)

    try:
      client.connect(MQTT_URL, MQTT_PORT, MQTT_KEEPALIVE)
    except Exception as error:
      print error
      raise

    # Start the MQTT thread and handle reconnections, also ensures the callbacks
    # being triggered
    client.loop_start()

  while True:
    data, addr = s.recvfrom(BUFSIZE)
    now = datetime.datetime.now()
    print str(now)[:19] + " -> " + str(addr[0]) + ":" + str(addr[1]) + " " + str(len(data))

    msg_recv = SENSOR(data)
    if ENABLE_LOG:
      print_recv_data(msg_recv)
    sensordata = jsonify_recv_data(msg_recv)

    if ENABLE_MQTT:
      publish_recv_data(sensordata, msg_recv.id, client, addr[0])

  client.loop_stop()

#------------------------------------------------------------#
# MAIN APP
#------------------------------------------------------------#
if __name__ == "__main__":
  start_client()

