IPSO Objects Example
============================================

This is an example of how to make use of the IPSO Object and LWM2M
implementation in Contiki.

The LWM2M implementation is based on the Erbium CoAP implementation
and consists of two apps: lwm2m-engine and ipso-objects. The
lwm2m-engine handle the specifics of LWM2M including bootstrapping and
how read/writes of objects and resources are handled. The ipso-objects
contains implementations of some of the IPSO Smart Objects.

The implementation was used during the IPSO Interop in May 2015,
Kista, Sweden, and was successfully tested with other
implementations.

The examples use some of the basic IPSO object for controlling LEDs on
Contiki devices and for reading out temperature.

##Testing IPSO-objects with Leshan

First program a device with the examples/ipso-objects/example-ipso-objects.c

```bash
>make example-ipso-objects.upload TARGET=zoul
>...
```

After that start up a native-border router or other border router on fd00::1/64
or another prefix - NOTE: if you use another prefix you will need to change LWM2M_SERVER_ADDRESS for which the device will register - in project-conf.h:
```
#define LWM2M_SERVER_ADDRESS "fd00::1"
```

Then when everything is setup you can download a Leshan and use that to
test controlling LEDs of the device.

###Starting Leshan
```bash
wget https://hudson.eclipse.org/leshan/job/leshan/lastSuccessfulBuild/artifact/leshan-standalone.jar
java -jar ./leshan-standalone.jar
```
Browse to leshans device page with http://127.0.0.1:8080 .

When you have started the border-router and also Leshan you should now
start (or reboot) your IPSO Object enabled device. Within 30 seconds
you should be able to see it on the Leshan device page.

