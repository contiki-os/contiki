RE-Mote + Node Red README
========================

A very basic example of how to use MQTT-Demo + Mosquitto + Node Red

Install Mosquitto
-----------------
* Install and run [mosquitto](http://mosquitto.org/). Default configuration
  options should work.

Fire up a Re-Mote
-----------------
* Compile the MQTT demo example from `../../../cc2538dk/mqtt-demo.c` following
  the instructions of the README.md therein.
* Program your RE-Mote.
* If you are running mosquitto with `-v`, a few seconds later you should see
  the Re-Mote connect, subscribe and start publishing.

Deploy your Node Red
--------------------
* Install and run [Node Red](https://github.com/node-red/node-red) as per the
  instructions
* Open `mqtt-remote-demo.json` from this directory with your favourite text
  editor, select all and copy. 
* Once you have the opened the Node Red page, click the menu icon on the top
  right and go to Import -> Clipboard. Paste the copied text.
* Double click the `MQTT input from Re-Mote` box on the top left. Set `Broker`
  IP and port to those corresponding to your running mosquitto.
* Hit Deploy
* Optionally, export the flow to a file for future use

Browse
------
Fire up a browser and browse to `http://<Node-Red's IP or hostname>/remote`

Do more cool stuff
------------------
Come up with more cool flows and share!
