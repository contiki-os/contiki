### Taken from https://pypi.python.org/pypi/paho-mqtt
### Requires Paho-MQTT package, install by:
### pip install paho-mqtt

import paho.mqtt.client as mqtt

# Change accordingly to the MQTT Broker and topic you want to subscribe
# In the example it would be either "test.mosquitto.org" or "fd00::1" if
# running a mosquitto broker locally
MQTT_URL         = "fd00::1"
MQTT_TOPIC_EVENT = "zolertia/evt/status"
MQTT_TOPIC_CMD   = "zolertia/cmd/leds"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC_EVENT)
    print("Subscribed to " + MQTT_TOPIC_EVENT)
    client.subscribe(MQTT_TOPIC_CMD)
    print("Subscribed to " + MQTT_TOPIC_CMD)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("connecting to " + MQTT_URL)
client.connect(MQTT_URL, 1883, 60)
client.loop_forever()
