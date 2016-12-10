# Lesson 2: IPv6/6LoWPAN networking basics

The following examples will show how to start developing wireless applications in Contiki, using UDP and TCP over IPv6/6LoWPAN.

The examples are written to be followed in order, so be sure to start from the very first and make your way to the last in an ordered fashion.

## Lesson objectives

The objective of the lessons are:

* Learn how to change the radio frequency channel, transmission power and PAN ID
* Assest the link quality and range using the RSSI (Received signal strength indication) and LQI (Link quality indicator)
* Create link-local wireless applications
* Learn how to send data (such as sensor readings) wirelessly
* Create IPv6 global wireless connected networks using a Border Router
* Learn how to create an UDP Server and client application, forwarding data over MQTT to a cloud platform
* Create a embedded Webserver and show sensor data over a webbrowser
* Debug a wireless application using the packet sniffer

We assume you have a working Contiki development environment, if you need help configuring the toolchain and others, please visit this page:

https://github.com/Zolertia/Resources/wiki
