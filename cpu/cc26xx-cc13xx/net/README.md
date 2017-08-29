# BLEach: a fully open-source IPv6-over-BLE stack for Constrained Embedded IoT Devices

## Overview
In 2015, the IETF released the [RFC 7668][rfc7668] that specifies how IPv6 packets
can be exchanged using BLE connections (IPv6 over BLE).
This Contiki extenstion implements [BLEach][bleachWeb], a fully open-source IPv6-over-BLE stack for Contiki.
BLEach can be used for node (BLE slave) and border router (BLE master) devices.

It was developed by 
* Michael Spoerk, Graz University of Technology, michael.spoerk@tugraz.at, github user: [spoerk](https://github.com/spoerk)

This IPv6-over-BLE stack is presented and evaluated in the paper:
[BLEach: Exploiting the Full Potential of IPv6 over BLE in Constrained Embedded IoT Devices](http://sensys.acm.org/2017/), ACM SenSys'17.

## Features
This implementation includes:
  * IPv6-over-BLE node implementation compliant to [RFC 7668][rfc7668]
  	* connect to a single IPv6-over-BLE border router
  	* maximum IPv6 packet length of 1280 bytes
  	* BLE L2CAP channels in LE credit-based flow control mode
  * IPv6-over-BLE border router implementation compliant to [RFC 7668][rfc7668]
    * connect to up to 4 different IPv6-over-BLE nodes
  	* maximum IPv6 packet length of 1280 bytes
  	* BLE L2CAP channels in LE credit-based flow control mode
  * BLE link layer support for version [4.1][bleSpec]:
  	* BLE advertisement
  	* BLE initiating
  	* BLE connection slave
  	* BLE connection master
  	
It has been tested on the TI CC2650 SensorTag and the TI CC2650 LaunchPad hardware.

## Modules
The IPv6-over-BLE stack comes with the following modules:

### BLE radio
The implementation of the BLE radio for the TI CC26xx platform is implemented in `cpu/cc26xx-cc13xx/rf-core/ble-cc2650.c`
and `cpu/cc26xx-cc13xx/rf-core/ble-hal/*.[ch]`.
These files contain all the hardware specific code for supporting BLE as a link layer.

### BLE parametrization layer
A minimal version of the BLE parametrization layer is implemented in `cpu/cc26xx-cc13xx/net/ble-null-par.[ch]`.
In this minimal version the parametrization does not perform any parameter adaptation.

### BLE L2CAP layer
The L2CAP LE credit-based flow control support is implemented in `cpu/cc26xx-cc13xx/net/ble-l2cap.c`.
Besides implementing rudimentary L2CAP support, this module handles fragmentation of large IPv6 packets.

## Using BLEach
A simple IPv6-over-BLE UDP-client and UDP-server example is included under `examples/cc26xx/cc26xx-ble-client-demo`
and `examples/cc26xx/cc26xx-ble-server-demo`, respectively.

For IPv6-over-BLE support, IPv6 needs to be enabled and RPL needs to disabled in Contiki:
```
CONTIKI_WITH_RPL = 0
CONTIKI_WITH_IPV6 = 1
```

Currently, BLEach is only available for the Texas Instruments CC2650 hardware platform.

The following sections describe how to configure BLEach for IPv6-over-BLE nodes and border routers.

### IPv6-over-BLE node (BLE slave)
To enable IPv6 over BLE, the project conf needs to contain:
```
#define PACKETBUF_CONF_SIZE                  	1280
#define QUEUEBUF_CONF_NUM                       1
#define UIP_CONF_BUFFER_SIZE                 	1280

#define CC26XX_CONF_RADIO_MODE					CC26XX_RADIO_MODE_BLE
#define NETSTACK_CONF_RADIO             		ble_cc2650_driver
#define NETSTACK_CONF_RDC               		ble_null_par_driver
#define NETSTACK_CONF_MAC               		ble_l2cap_driver

#define RTIMER_CONF_MULTIPLE_ACCESS 			1

/* 6LoWPAN settings */
#define SICSLOWPAN_CONF_MAC_MAX_PAYLOAD      	1280
#define SICSLOWPAN_CONF_COMPRESSION          	SICSLOWPAN_COMPRESSION_HC06
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD   0  /* always use compression */
#define SICSLOWPAN_CONF_FRAG                    0
#define SICSLOWPAN_FRAMER_HDRLEN                0

/* network stack settings */
#define UIP_CONF_ROUTER                         0
#define UIP_CONF_ND6_SEND_NA 					1

```

The following optional parameter can be used to configure that BLE advertisement behaviour:
```
#define BLE_CONF_DEVICE_NAME					"TI CC26xx device"
#define BLE_CONF_ADV_INTERVAL					25
```
`BLE_CONF_DEVICE_NAME` holds the device name that is used for advertisement, `BLE_CONF_ADV_INTERVAL`
specifies the used advertisement interval in milliseconds.

### IPv6-over-BLE border-router (BLE master)
To enable IPv6 over BLE, the project conf needs to contain:
```
#define PACKETBUF_CONF_SIZE                  	1280
#define QUEUEBUF_CONF_NUM                       1
#define UIP_CONF_BUFFER_SIZE                 	1280

#define CC26XX_CONF_RADIO_MODE					CC26XX_RADIO_MODE_BLE
#define NETSTACK_CONF_RADIO             		ble_cc2650_driver
#define NETSTACK_CONF_RDC               		ble_null_par_driver
#define NETSTACK_CONF_MAC               		ble_l2cap_driver

#define RTIMER_CONF_MULTIPLE_ACCESS 			1

/* BLE radio settings */
#define BLE_MODE_CONF_INIT_PEER_ADDR			0xAABBCCDDEEFF

/* 6LoWPAN settings */
#define SICSLOWPAN_CONF_MAC_MAX_PAYLOAD      	1280
#define SICSLOWPAN_CONF_COMPRESSION          	SICSLOWPAN_COMPRESSION_HC06
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD   0  /* always use compression */
#define SICSLOWPAN_CONF_FRAG                    0
#define SICSLOWPAN_FRAMER_HDRLEN                0

/* network layer settings */
#define UIP_CONF_ROUTER                         1
#define UIP_CONF_ND6_SEND_NA                    1
#define UIP_CONF_ND6_SEND_RA					1
#define UIP_CONF_IP_FORWARD                     0
```
`BLE_MODE_CONF_INIT_PEER_ADDR` specifies the public BLE address of the node
to which a connection should be established (0xAA is the MSB of the device address).
Currently, if the BLE master should needs to support more that a single BLE slave, 
`BLE_MODE_CONF_MAX_CONNECTIONS` needs to be configured to the desired number of slaves and
the peer addresses of the slaves need to be configured in the `ble_cc2650.c` file.

The following optional defines can be used to configure the BLE connection parameters:
```
#define BLE_MODE_CONF_CONN_CHANNEL_MAP			0x1FFFFFFFFFULL
#define BLE_CONF_CONNECTION_INTERVAL			250
#define BLE_CONF_CONNECTION_SLAVE_LATENCY		1
```

`BLE_MODE_CONF_CONN_CHANNEL_MAP` defines the channel map used for the BLE connection (see [BLE specification][bleSpec] for further info).
`BLE_CONF_CONNECTION_INTERVAL` specifies the used connection interval and `BLE_CONF_CONNECTION_SLAVE_LATENCY` specifies the
used slave latency of the BLE connection.



[rfc7668]: https://tools.ietf.org/html/rfc7668
[bleSpec]: https://www.bluetooth.com/specifications/bluetooth-core-specification/legacy-specifications
[bleachWeb]: http://www.iti.tugraz.at/BLEach

