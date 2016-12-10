### Taken from https://pypi.python.org/pypi/paho-mqtt
### Requires Paho-MQTT package, install by:
### pip install paho-mqtt

import paho.mqtt.client as mqtt

MQTT_URL   = "iot.eclipse.org"
MQTT_TOPIC = "v2/zolertia/tutorialthings/#"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    print("Subscribed to " + MQTT_TOPIC)
    client.subscribe(MQTT_TOPIC)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("connecting to " + MQTT_URL)
client.connect(MQTT_URL, 1883, 60)
client.loop_forever()
