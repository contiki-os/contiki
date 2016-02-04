MQTT Demo
=========
The MQTT client can be used to:

* Publish sensor readings to an MQTT broker.
* Subscribe to a topic and receive commands from an MQTT broker

The demo will give some visual feedback with the green LED:
* Very fast blinking: Searching for a network
* Fast blinking: Connecting to broker
* Slow, long blinking: Sending a publish message

Publishing
----------
By default the example will attempt to publish readings to an MQTT broker
running on the IPv6 address specified as `MQTT_DEMO_BROKER_IP_ADDR` in
`project-conf.h`. This functionality was tested successfully with
[mosquitto](http://mosquitto.org/).

The publish messages include sensor readings but also some other information,
such as device uptime in seconds and a message sequence number. The demo will
publish to topic `iot-2/evt/status/fmt/json`. The device will connect using
client-id `d:quickstart:cc2538:<device-id>`, where `<device-id>` gets
constructed from the device's IEEE address.

Subscribing
-----------
You can also subscribe to topics and receive commands, but this will only
work if you use "Org ID" != 'quickstart'. To achieve this, you will need to
change 'Org ID' (`DEFAULT_ORG_ID`). In this scenario, the device will subscribe
to:

`iot-2/cmd/+/fmt/json`

You can then use this to toggle LEDs. To do this, you can for example
use mosquitto client to publish to `iot-2/cmd/leds/fmt/json`. So, to change
the state of an LED, you would do this:

`mosquitto_pub -h <broker IP> -m "1" -t iot-2/cmd/leds/fmt/json`

Where `broker IP` should be replaced with the IP address of your mosquitto
broker (the one where you device has subscribed). Replace `-m "1'` with `-m "0"`
to turn the LED back off.

Bear in mind that, even though the topic suggests that messages are of json
format, they are in fact not. This was done in order to avoid linking a json
parser into the firmware. This comment only applies to parsing incoming
messages, outgoing publish messages use proper json payload.

IBM Quickstart Service
----------------------
It is also possible to publish to IBM's quickstart service. To do so, you need
to undefine `MQTT_DEMO_BROKER_IP_ADDR`.

The device will then try to connect to IBM's quickstart over NAT64, so you will
need a NAT64 gateway in your network to make this work. A guide on how to
setup NAT64 is out of scope here.

If you want to use IBM's cloud service with a registered device, change
'Org ID' (`DEFAULT_ORG_ID`) and provide the 'Auth Token' (`DEFAULT_AUTH_TOKEN`),
which acts as a 'password', but bear in mind that it gets transported in clear
text.
