# RPL Security features 

## **Overview**

RPL defines in RFC 6550 security features that provide confidentiality, integrity, and authenticity. All the features are optional.
RPL defines three security modes of operation:

1. Unsecure: 
	In this security mode, RPL uses basic DIS, DIO, DAO, and DAO-ACK messages, which do not have Security sections.
2. Secure: 
	RPL uses secure messages. 
	To join a RPL Instance, a node must have a preinstalled key.
	Nodes use this to provide message confidentiality, integrity, and authenticity. 
	Nodes can be either hosts or routers.
3. Authenticated: 
	RPL uses secure messages. 
	To join a RPL Instance, a node must have a preinstalled key. 
	Nodes use this key to provide message confidentiality, integrity, and authenticity. 
	Using this preinstalled key, a node may join the network as a host only. 
	To join the network as a router, a node must obtain a second key from a key authority.

Messages are encrypted/authenticated by using CCM* library.

Developed by:
Antonio Arena, Universita'  di Pisa, rnantn@gmail.com, github user:[arenantonio92](https://github.com/arenantonio92)

## **Features**

This implementation includes:
	- RPL Secure mode of operation

The code has been tested on Zolertia Z1 (z2, tested in cooja only) platform.

## **Code Structure**

The Secure mode code is included in rpl-*.c files and is activated with a macro define in rpl-conf.h

## **Using RPL Secure**

A simple example wich uses Secure RPL is included under examples/ipv6/rpl-sec. 

In order to use Secure RPL messages, RPL must be enabled in every node.
Also enable secure mode functionalities in your project-conf.h file with

	#undef RPL_CONF_SECURITY
		
	#define RPL_CONF_SECURITY 1

You can also set the preinstalled key (128 bit, CCM* uses AES-128) in your project-conf.h with

	#undef RPL_CONF_SECURITY_K

	#define RPL_CONF_SECURITY_K { 0xF7, 0x8E, 0xBA, 0xC9, 0xED, 0xE3, 0xE6, 0x68, 0xDF, 0x15, 0x72, 0xE0, 0x78, 0xB7, 0x80, 0x9E }

In the end, you can also set the security level (see RFC6550 6.1) in your project-conf.h with

	#undef RPL_CONF_SEC_LVL
	#define RPL_CONF_SEC_LVL 3

## **Additional Documentation**

1. [RPL: IPv6 Routing Protocol for Low-Power and Lossy Networks] (https://tools.ietf.org/html/rfc6550)




