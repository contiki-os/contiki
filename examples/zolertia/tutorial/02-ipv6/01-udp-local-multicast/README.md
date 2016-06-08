# Link-Local UDP multicast example

The following examples will show how to read and set radio parameters, such as:

* RSSI (Received signal strength indication) and LQI (Link quality indicator)
* Radio channel
* PAN ID (network identifier)

We will also learn how to use the Simple-UDP library, which allows to create wireless applications on top of IPv6/UDP, and transmit data such as sensor readings and system information.

## Requirements

You will need at least two Zolertia motes, the `RE-Mote` and `Z1` can be used together in the same network.

## Compile and program the example

````
make 01-udp-local-multicast.upload && make login
````

The command above should open a serial connection right after thenode is flashed, showing (in the case of the Z1):

````
Node id is not set, using Z1 product ID
Rime started with address 193.12.0.0.0.0.19.200
MAC c1:0c:00:00:00:00:13:c8 Ref ID: 5064
Contiki-3.x-2162-g709d3d5 started. Node id is set to 5064.
CSMA nullrdc, channel check rate 128 Hz, radio channel 26
Tentative link-local IPv6 address fe80:0000:0000:0000:c30c:0000:0000:13c8
Starting 'UDP multicast example process'

* Radio parameters:
   Channel 15 (Min: 11, Max: 26)
   Tx Power   0 dBm (Min: -25 dBm, Max:   0 dBm)
   PAN ID: 0xABCD

***
Message from: fe80::c30c:0:0:13c2
Data received on port 8765 from port 8765 with length 14
CH: 15 RSSI: -55dBm LQI: 82
ID: 171, temp: 2400, x: -245, y: 71, z: 76, batt: 3012, counter: 11

***
Sending packet to multicast adddress ff02::1
ID: 171, temp: 2475, x: -85, y: 240, z: 34, batt: 2985, counter: 1

````

In the case of the `RE-Mote`:

````
Contiki-3.x-2576-g4499d80
Zolertia RE-Mote platform
Rime configured with address 00:12:4b:00:06:15:ab:25
 Net: sicslowpan
 MAC: CSMA
 RDC: nullrdc

* Radio parameters:
   Channel 15 (Min: 11, Max: 26)
   Tx Power   3 dBm (Min: -24 dBm, Max:   7 dBm)
   PAN ID: 0xABCD

***
Sending packet to multicast adddress ff02::1
ID: 171, core temp: 37.619, ADC1: 2332, ADC2: 0, ADC3: 1496, batt: 3300, counter: 1
````

Notice when restarting the Z1 it prints its addressing and networking information:

````
MAC c1:0c:00:00:00:00:13:c8 Ref ID: 5064
````

The MAC address is derived from the `platform/z1/contiki-z1-main.c` address scheme, using the Z1 Node ID in the last two octets to create a MAC address.

The `RE-Mote` MAC address is printed at the following line:

````
00:12:4b:00:06:15:ab:25
````

For the `Z1` mote this is the `Node ID`:

````
Contiki-3.x-2162-g709d3d5 started. Node id is set to 5064.
````

The Node ID as commented in `Lesson 1` is derived either from the Product Reference ID (as seen when using the `make motelist` command), or either preconfigured by flashing this to the external flash memory of the Z1 node (also explained in the previous lesson).

In the case of the `RE-Mote`, the `Node ID` corresponds to the last 2 bytes of the MAC address, in our example `0xab25`.

The example uses `CSMA` (Carrier Sense Multiple Access) MAC protocol (default in Contiki).  We are using `NullRDC` which prevents Contiki from duty-cycling the radio to save energy, but allow to keep receivers always on all the time.

````
CSMA nullrdc, channel check rate 128 Hz, radio channel 26
Tentative link-local IPv6 address fe80:0000:0000:0000:c30c:0000:0000:13c8
````

The Link-Local IPv6 address is derived from the MAC address.


