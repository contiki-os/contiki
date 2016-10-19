### Taken from https://pypi.python.org/pypi/paho-mqtt
### Requires Paho-MQTT package, install by:
### pip install paho-mqtt
#-----------------------------------------------------------------------------#
import argparse
import json
import paho.mqtt.client as mqtt
#-----------------------------------------------------------------------------#
MQTT_URL              = "mqtt.thethings.io"
#-----------------------------------------------------------------------------#
CMD_PUB_STRING_LEDS   = {"key":"leds_toggle","value":"0"}
CMD_PUB_STRING_REBOOT = {"key":"reboot","value":"0"}
CMD_PUB_STRING_SENSOR = {"key":"enable_sensor","value":"1"}
CMD_PUB_STRING_PERIOD = {"key":"update_period","value":"45"}

CMD_PUB_STRING = {"values":[
                              CMD_PUB_STRING_LEDS,
                              CMD_PUB_STRING_REBOOT,
                              CMD_PUB_STRING_SENSOR,
                              CMD_PUB_STRING_PERIOD
                           ]}

data_token = ""
cmd_token  = ""
#-----------------------------------------------------------------------------#
# When connected to the broker subscribe to the data topic and if the CMD
# Thing-Token is present, publish to the CMD topic the default configuration
# keys
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

    # Subscribe to the data topic
    client.subscribe('v2/things/' + data_token)
    print("Subscribed to data topic v2/things/" + data_token)

    # If we got a cmd topic, populate the topic with QoS=0
    if cmd_token:
      aux = json.dumps(CMD_PUB_STRING, separators=(',', ':'), sort_keys=True)
      client.publish('v2/things/' + cmd_token, aux, 0)
      print("Publishing {0} to {1}".format('v2/things/' + cmd_token, aux))

    # Subscribe to the default command topic
    if cmd_token:
      client.subscribe('v2/things/' + cmd_token)
    else:
      client.subscribe('v2/things/' + data_token + '/cmd')

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

def print_help():
  print("things-cmd-config.py -d <data token> -c <command token>")

#-----------------------------------------------------------------------------#
# This is where the application actually starts
#-----------------------------------------------------------------------------#
# Retrieve arguments
parser = argparse.ArgumentParser(description="TheThings.io MQTT configuration")

parser.add_argument('-d', '--data', action="store", nargs=1, required=True,
                    help="Thing-Token to publish data", type=str)
parser.add_argument('-c', '--cmd', action="store", nargs=1, required=False,
                    help="Thing-Token to publish commands", default="", type=str)

opts = parser.parse_args()

data_token = opts.data[0]
cmd_token  = opts.cmd[0]

# Create the MQTT connection
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("connecting to " + MQTT_URL + " with Token " + data_token)
client.username_pw_set(data_token, "")

client.connect(MQTT_URL, 1883, 60)
client.loop_forever()
