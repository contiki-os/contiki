# HOWTO - Setting up the RPL Border router and other nodes

In this folder we have a fully functional demonstrator with the components:

1. **RPL border router**: to start the wireless network and connect it to other networks.  
2. and a **wireless node** that acts as a basic RPL node by default (but can optionally be used configured as DAG Root)

## RICH RPL Border Router

Setup the UART flow-control mode for the router from border-router/project-conf.h

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

From a Linux terminal, go to `contiki/examples/jn516x/rpl/border-router` and do either
`make connect-router-hw` if you have **HW flow control**
or `make connect-router-sw` if you have **SW flow control**

This will start a tunnel interface (tun0) on the host machine.
All traffic towards our network (prefix fd00::1/64) will now be routed to the border router.

## RPL Node

The directory contiki-private/examples/jn516x/rpl/node contains a basic RICH node running TSCH and RPL.
You can compile and program more NXP nodes to run this, forming a larger network.
You should be able to ping the nodes (you can obtain their IPv6 address either directly from their log output
or by enabling DEBUG output in rpl-icmp6.c and looking at DAO prefixes being added.

## RPL+CoAP Nodes

coap-*-node are example nodes that expose their sensors as CoAP resources. See README.md files from the sub-directories
for more details.
