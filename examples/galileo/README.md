Galileo Specific Examples
=========================

This directory contains galileo-specific example applications to illustrate
how to use galileo APIs.

In order to build a application, you should set the EXAMPLE environment
variable to the name of the application you want to build. For instance, if
you want to build gpio-output application, run the following command:

```
$ make TARGET=galileo EXAMPLE=gpio-output
```

The corresponding EXAMPLE variable setting for each application is
listed to the right of its heading.

GPIO
----

### GPIO Output (EXAMPLE=gpio-output)

This application shows how to use the GPIO driver APIs to manipulate output
pins. This application sets the GPIO 5 pin as output pin and toggles its
state at every half second.

For a visual effect, you should wire shield pin IO2 to a led in a protoboard.
Once the application is running, you should see a blinking LED.

### GPIO Input (EXAMPLE=gpio-input)

This application shows how to use the GPIO driver APIs to manipulate
input pins. This application uses default galileo pinmux
initialization and sets the GPIO 5 (shield pin IO2) as output pin and
GPIO 6 (shield pin IO3) as input. A jumper should be used to connect
the two pins. The application toggles the output pin state at every
half second and checks the value on input pin.

### GPIO Interrupt (EXAMPLE=gpio-interrupt)

This application shows how to use the GPIO driver APIs to manipulate
interrupt pins. This application uses default galileo pinmux
initialization and sets the GPIO 5 (shield pin IO2) as output pin and
GPIO 6 (shield pin IO3) as interrupt. A jumper should be used to
connect the two pins. It toggles the output pin stat at every half
second in order to emulate an interrupt. This triggers an interrupt
and the application callback is called. You can confirm that though
the UART output.

I2C
---

### I2C LSM9DS0 (EXAMPLE=i2c-LSM9DS0)

This application shows how to use I2C driver APIs to configure I2C
Master controller and communicate with an LSM9DS0 sensor if one has
been connected as described below. At every 5 seconds, the application
reads the "who am I" register from gyroscope sensor and prints if the
register value matches the expected value described in the spec [1].

According to the sensor spec, to read the value in "who am I" register, we
should first perform an i2c write operation to select the register we want
to read from and then we perform the i2c read operation to actually read
the register contents.

The wiring setup is as follows (left column from Galileo and right column from LSM9DS0):
- 3.3v and Vin
- GND  and GND
- GND  and SDOG
- 3.3v and CSG
- SDA  and SDA
- SCL  and SCL

### I2C Callbacks (EXAMPLE=i2c-callbacks)

This application is very similar to the previous one in that it also
shows how to use I2C callback functionality, but it can be run without
attaching any additional sensors to the platform since it simply
communicates with a built-in PWM controller.

Every five seconds, the application reads the current value of the
MODE1 register, which should have previously been initialized to the
value 0x20. The test verifies that this expected value is returned by
the read.

Isolated Memory Regions
-----------------------

### Print IMR info (EXAMPLE=print-imr)

This application prints out information about the configuration of the
Intel Quark X1000 SoC Isolated Memory Regions (IMRs), the Host System
Management Mode Controls register, and the Host Memory I/O Boundary
register.

Protection Domains
------------------

### Protection Domain Switch Latency (EXAMPLE=prot-domain-switch-latency)

This application measures and prints the average latency of repeatedly
switching from one protection domain to another and back, in ping-pong
fashion.  It can optionally perform memory accesses to metadata
associated with the destination protection domain.  This feature can
be enabled by specifying SAMPLE_METADATA=1 on the build command line.

References
----------

[1] http://www.st.com/st-web-ui/static/active/en/resource/technical/document/datasheet/DM00087365.pdf

