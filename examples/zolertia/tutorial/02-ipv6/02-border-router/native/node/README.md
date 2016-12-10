# Placeholder

This project is intended to run on a mode that is connected to a native host system by SLIP.  The SLIP is really SERIAL LINE 15.4, as this just turns the mote into a smart radio, running the RPL and 6lowpan stack on the Host.  This goes with native-border-router.

To compile and program:

````
make TARGET=z1 slip-radio.upload
````




