


# 6TiSCH Operation Sublayer (6top)

## **Overview**

6TiSCH Operation Sublayer (6top), a logical link layer in the 6TiSCH architecture,  provides the abstraction of an IP link over a TSCH MAC layer (IEEE 802.15.4e). The functionality of 6top is to facilitate the dynamic cell negotiation with one-hop neighbors.

This is a Contiki implementation of 6top, which defines how to handle a 2-step 6top transaction on a basic RPL+TSCH network.

Developed by:

- Shalu R, CDAC, shalur@cdac.in, github user:  [shalurajendran](https://github.com/shalurajendran)
- Lijo Thomas, CDAC, lijo@cdac.in

## **Features**

This implementation includes:

- 6top protocol(6P)
  - 6P Add
  - 6P Delete
- 2-step 6top Transaction

The code has been tested on Zolertia Z1 (z1, tested in cooja only) platform.

## **Code structure**

The 6top frame format is included in:

- core/net/mac/frame802154-ie.[ch]: Modified to include Sixtop Information Elements

6top is implemented in:

- core/net/mac/tsch/sixtop/sixtop.[ch]: 6top interface and 6top protocol(6P addition and deletion), handling of 6P Requests and Response

TSCH file modified:

- core/net/mac/tsch/tsch.[ch]: Modified to include Sixtop module


## **Using 6top**

A simple 6top+TSCH+RPL example is included under examples/ipv6/rpl-tsch-sixtop. Currently this application triggers the 6P transactions, later the 6top Scheduling Function (SF) may be implemented for the purpose.

To use 6top, make sure your platform supports TSCH and have sufficient memory.

To add 6top to your application, first include the Sixtop module in your Makefile with:

MODULES += core/net/mac/tsch/sixtop

Also enable the sixtop functionalities in your project-conf.h file with

	#undef TSCH_CONF_WITH_SIXTOP
	
	#define TSCH_CONF_WITH_SIXTOP 1

## **Additional documentation**

1. [IETF 6TiSCH Working Group](https://datatracker.ietf.org/wg/6tisch)
2. [6TiSCH 6top Protocol (6P)](https://tools.ietf.org/pdf/draft-ietf-6tisch-6top-protocol-02.pdf)


