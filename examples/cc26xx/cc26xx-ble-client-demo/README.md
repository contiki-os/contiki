# CC26xx IPv6-over-BLE client demo

This example provides a simple implementation of a UDP client that connects
to a UDP server via IPv6-over-BLE [RFC 7668](https://tools.ietf.org/html/rfc7668).

After booting, the node sends an ICMPv6 echo request (ping) to the configured server address
and waits for an ICMPv6 echo response.
When a response is received, the client start to send a UDP packet with content
`Hello server <counter>!` to the server, where <counter> is a packet counter that is incremented every time a UDP packet is sent.
If the server respondes with a UDP packet, the content of the packet is printed in the console.

## Requirements
To run this example, you will need a [RFC 7668](https://tools.ietf.org/html/rfc7668)-compliant border router.
A simple IPv6-over-BLE border router is implemented under `examples/cc26xx/cc26xx-ble-server-demo`.
Another possible border router is the Raspberry Pi 3 with the Raspbian OS
(for detailed instructions see the [Nordic Semiconductor guide](http://developer.nordicsemi.com/nRF5_IoT_SDK/doc/0.9.0/html/a00092.html)).

## Configuration
To successfully communicate with a UDP server, the `SERVER_IP` define in `client.c` needs to be configured 
to the IPv6 address of the UDP server.
For example:
```
#define SERVER_IP     					"fe80::1"
```
## Running the example
Deploy your border router and create the IPv6-over-BLE connection to the node device.
After node and router perform IPv6 neighbor discovery, the node will start to send UDP packets to the server.
