### Taken from https://pypi.python.org/pypi/paho-mqtt
### Requires Paho-MQTT package, install by:
### pip install paho-mqtt

import paho.mqtt.client as mqtt

# Change accordingly to the MQTT Broker and topic you want to subscribe
# In the example it would be either "test.mosquitto.org" or "fd00::1" if
# running a mosquitto broker locally

MQTT_URL         = "fd00::1"
MQTT_TOPIC_EVENT = "zolertia/test"

# The callback upon connecting successfully
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC_EVENT)
    print("Subscribed to " + MQTT_TOPIC_EVENT)

# The callback when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print("RX: " + msg.topic + " " + str(msg.payload))

# Create the MQTT connection object
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect and loop forever
print("connecting to " + MQTT_URL)
client.connect(MQTT_URL, 1883, 60)
client.loop_forever()
