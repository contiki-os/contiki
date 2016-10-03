### Taken from https://pypi.python.org/pypi/paho-mqtt
### Requires Paho-MQTT package, install by:
### pip install paho-mqtt

import paho.mqtt.client as mqtt

MQTT_URL         = ""
MQTT_USERID      = ""
MQTT_PASSWD      = ""

MQTT_TOPIC_EVENT = ''
MQTT_TOPIC_PUB   = ''
MQTT_PUB_STRING  = ''

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

    if MQTT_TOPIC_EVENT:
      client.subscribe(MQTT_TOPIC_EVENT)
      print("Subscribed to " + MQTT_TOPIC_EVENT)

    if MQTT_PUB_STRING:
      client.publish(MQTT_TOPIC_PUB, MQTT_PUB_STRING, 0)
      print("Published " + MQTT_PUB_STRING + " to " + MQTT_TOPIC_PUB)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("connecting to " + MQTT_URL)
client.username_pw_set(MQTT_USERID, MQTT_PASSWD)

client.connect(MQTT_URL, 1883, 60)
client.loop_forever()
