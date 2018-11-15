# CC26xx IPv6 over BLE - UDP client

This example demonstrates a UDP client using IPv6 over BLE.
The node operates as a UDP client connected via IPv6 over BLE to a border router.

After booting the node starts to send ICMPv6 echo requests to an UDP server
(IPv6: aaaa::1) and waits for a valid ICMPv6 echo response from the server.

When an echo response was received, the node periodically sends UDP messages 
to the UDP server and prints every UDP message received from the server.

## Requirements

To run this example you will need:

* A IPv6 over BLE border router.
  See [nRF5 IoT SDK] (http://developer.nordicsemi.com/nRF5_IoT_SDK/doc/0.9.0/html/a00092.html)
  for instructions on how to setup a border router on a Raspberry Pi.
