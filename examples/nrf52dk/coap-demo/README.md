A CoAP demo for nRF52 DK
========================
This demo contains two applications: coap-server and coap-client which are similar to 
[Coap Observable Server] and [Coap Observer Client] examples provided by the nRF5 IoT SDK.

Note that before any CoAP requests can be made you'll need to configure an IPv6 connection 
to the device and assign a routable IPv6 address.

For details how to do this please refer to sections 'Establishing an IPv6 connection'
and 'Distributing routable IPv6 prefix' in `platform/nrf52dk/README-BLE-6LoWPAN.md`.

CoAP Server
===========
The server exposes the following resources:

	host
    |-- .well-known
    |   `-- core
    `-- lights
        `-- led3
        
The state of LED 3 can be set and queried via CoAP through the observable resource `lights/led3`. Current
state of LED 3 is returned as a text string in the payload. The value 0 means that LED 3 is off, 1 otherwise.

Button 1 can be used to toggle state of the LED 3. This will cause a notification to be sent to 
any subscriber observing `lights/led3`. The state of the resource can also be changed by using POST/PUT to
the resource with desired state encoded as a text in the payload. Send 1 to switch on and 0 to switch off.

In order to compile and flash the CoAP server to a DK execute:

	make TARGET=nrf52dk coap-server.flash
	
Note, if you haven't previously used a given device with Contiki it is recommended
to erase the device and flash SoftDevice before flashing CoAP application, i.e.,

    make TARGET=nrf52dk erase
    make TARGET=nrf52dk softdevice.flash

Please refer to the *Testing* and *Python Example* sections of [Coap Observable Server] tutorial for detailed description how to query the Coap Server using a PC.

CoAP Client
===========
CoAP client compliments the CoAP server application. When Button 1 on the DK is pressed the the 
client subscribes to `lights/led3` resource. If successful the LED 4 will blink briefly. From this moment
any change of the `lights/led3` resource will be automatically reflected by the client's LED 3. 

Note that the client must know the server's IPv6 address. The address is specified as a make variable
during compliation.
  
In order to compile and flash the CoAP client to DK execute:

	make TARGET=nrf52dk SERVER_IPV6_ADDR=<full-ip6-address> coap-client.flash
	
Note, that you can use `NRF52_JLINK_SN=<SN>` to select a particular devkit in a case when
you have more than one boards connected to PC. Please refer to `platform/README.md` for 
details.
	
Please refer to the *Testing* and *Python Server Example* sections of [Coap Observer Client] tutorial for detailed description how to use CoAP client demo with a PC.

Resources
=========
[Coap Observable Server] http://developer.nordicsemi.com/nRF5_IoT_SDK/doc/0.9.0/html/a00054.html
[Coap Observer Client] http://developer.nordicsemi.com/nRF5_IoT_SDK/doc/0.9.0/html/a00051.html
