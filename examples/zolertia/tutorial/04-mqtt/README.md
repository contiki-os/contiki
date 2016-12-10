# Lesson 4: MQTT example

The following examples will show how to create a MQTT node able to publish and subscribe to a MQTT broker

For more information about MQTT visit the following page:

http://mqtt.org/

## Lesson objectives

The objective of the lessons are:

* Familiarize with the MQTT API
* Publish sensor data and node statistics to a MQTT broker
* Control the on-board LEDs remotely by publishing to the MQTT broker and receiving the command notification
* Implement a local MQTT broker (mosquitto), or publish to one on Internet


## Requirements

We assume you have a working Contiki development environment, if you need help configuring the toolchain and others, please visit this page:

https://github.com/Zolertia/Resources/wiki

It is advisable to install [mosquitto](http://mosquitto.org/), on Ubuntu:

````
sudo apt-add-repository ppa:mosquitto-dev/mosquitto-ppa
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients
````

You will also need two Zolertia devices: a Border Router and a node acting as the MQTT client.

This example works for both `Z1` nodes and `zoul` platforms like the `RE-Mote`, each one will publish its on-board sensors data.

Keep in mind the `Z1` has lesser number of `resources` enabled due to RAM constrains.

## Instructions

Flash the application:

````
make mqtt-example.upload
````

The green LED will blink at a faster pace while the MQTT node is trying to connect to the MQTT broker, then when connected it will stop blinking.

Whenever the node publishes to the MQTT broker it will turn on the green LED, and off when it finishes publishing.

````
Rime started with address 193.12.0.0.0.0.19.208
MAC c1:0c:00:00:00:00:13:d0 Ref ID: 43981
Contiki-3.x-2577-gea0738b started. Node id is set to 5072.
CSMA nullrdc, channel check rate 128 Hz, radio channel 26
Tentative link-local IPv6 address fe80:0000:0000:0000:c30c:0000:0000:13d0
Starting 'MQTT Demo'
MQTT Demo Process
Subscription topic zolertia/cmd/leds
Init
Registered. Connect attempt 1
APP - MQTT Disconnect. Reason 3
Disconnected
Disconnected. Attempt 2 in 1024 ticks
Registered. Connect attempt 2
Connecting (2)
APP - Application has a MQTT connection
APP - Subscribing to zolertia/cmd/leds
APP - Application is subscribed to topic successfully
Publishing
APP - Publish to zolertia/evt/status: {"d":{"myName":"Zolertia Z1 Node","Seq no":1,"Uptime (sec)":63,"Def Route":"fe80::212:4b00:615:ab25","Temp":"2.768","X axis":"86"}}
````

## Subscribing to the topic

There is a python script named `mqtt-client.py` you could use to subscribe to the MQTT broker and topic, and receive notifications whenever the MQTT node publishes.

In case of a `Z1` node publishing to mosquitto's MQTT broker:

````
$ python mqtt-client.py 
connecting to fd00::1
Connected with result code 0
Subscribed to zolertia/evt/status
Subscribed to zolertia/cmd/leds
zolertia/evt/status {"d":{"myName":"Zolertia Z1 Node","Seq no":1,"Uptime (sec)":63,"Def Route":"fe80::212:4b00:615:ab25","Temp":"27.68","X axis":"86"}}
````

For the `RE-Mote`, in this case using a local mosquitto broker:

````
$ python mqtt-client.py 
connecting to fd00::1
Connected with result code 0
Subscribed to zolertia/evt/status
Subscribed to zolertia/cmd/leds
zolertia/evt/status {"d":{"myName":"Zolertia RE-Mote platform","Seq #":5,"Uptime (sec)":239,"Def Route":"fe80::212:4b00:615:ab25","Core Temp":"34.523","ADC1":"2280","ADC3":"1452"}}

````

## Sending a remote command to the MQTT node

The MQTT node subscribes to the following topic:

`zolertia/cmd/led`

Sending as payload `1` will turn on the red LED, and a `0` off.

Execute this from the command line:

````
mosquitto_pub -h "test.mosquitto.org"  -t "zolertia/cmd/leds" -m "1" -q 0
````

The above command will publish to the `cmd` topic, all nodes subscribed to it will turn its red LED on.

````
APP - Application received a publish on topic 'zolertia/cmd/leds'. Payload size is 1 bytes. Content:
Pub Handler: topic='zolertia/cmd/leds' (len=17), chunk_len=1
Turning LED RED on!

APP - Application received a publish on topic 'zolertia/cmd/leds'. Payload size is 1 bytes. Content:
Pub Handler: topic='zolertia/cmd/leds' (len=17), chunk_len=1
Turning LED RED off!
````
