# CC26xx IPv6-over-BLE server demo

This example provides a simple implementation of a UDP server that communicates
with a UDP client via IPv6-over-BLE [RFC 7668](https://tools.ietf.org/html/rfc7668).

After booting, the server waits for a UDP packet to be received.
When a packet is received, the server prints its content in the console and responds with
a UDP packet with content `Hello client <counter>!`, where <counter> is a packet counter that
is incremented every time a UDP response is sent.

## Requirements
To run this example, you will need a [RFC 7668](https://tools.ietf.org/html/rfc7668)-compliant node.
A simple IPv6-over-BLE node is implemented under `examples/cc26xx/cc26xx-ble-client-demo`.

## Configuration

## Running the example
Deploy your border router and create the IPv6-over-BLE connection to the node device.
After node and router perform IPv6 neighbor discovery, the node will start to send UDP packets to the server.
