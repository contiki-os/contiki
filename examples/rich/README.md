# HOWTO - Setting up the RPL Border router and other RICH nodes

**Important note: the CoAP scheduler interface under `tools/rich-scheduler-interface.[ch]` are not ported yet to Contiki 3.x and therefore not currently usable**

In this folder we have a fully functional demonstrator for RICH stack (CoAP + IPv6/RPL + TSCH on NXP) with the components:

1. **RPL border router**: to start the wireless network and connect it to other networks.  
2. and a **wireless node** that does nothing except acting as a wireless forwarder. It is possible to implement other meaningful processes on this node to make an embedded smart controller/actuator that utilizes RICH stack.

These components can be compiled for either NXP JN516X or sky platforms, and should work out-of-the-box.

## RICH RPL Border Router

Setup the UART flow-control mode for the router from rpl-border-router/project-conf.h

* Enable either **HW flow control**
```C
#define UART_HW_FLOW_CTRL  1
#define UART_XONXOFF_FLOW_CTRL 0
```
* or **SW flow control**
```C
#define UART_HW_FLOW_CTRL  0
#define UART_XONXOFF_FLOW_CTRL 1
```
* You can disable both, but it is not recommended.

Compile and flash a node with the rpl-border-router.jn516x.bin image. Either a USB dongle or a dev-board would work.

From a Linux terminal, go to `contiki-private/examples/rich/rpl-border-router` and do either
`make connect-router-hw` if you have **HW flow control**
or `make connect-router-sw` if you have **SW flow control**

This will start a tunnel interface (tun0) on the host machine.
All traffic towards our network (prefix `aaaa::/64`, address on the PC side `aaaa::1/64`) will now be routed to the border router.

## RICH Node

The directory contiki-private/examples/rich/node contains a basic RICH node running TSCH, RPL, and the CoAP scheduler resources.
You can compile and program more NXP nodes to run this, forming a larger network.
