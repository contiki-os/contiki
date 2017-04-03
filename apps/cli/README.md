cli app
=======
cli is an simple attempt to add commands to an ipv6 project to
instruments ip6 and RPL to give have an easy tool for interactive 
monitoring and debugging. Uses serial line/UART. Note that Contiki 
has several cli efforts you might want look around. 

cli Code
--------
apps/cli/cli.c


Project code needs
------------------
#include "dev/serial-line.h"
extern void handle_serial_input(const char *line);
.

/* Parse serial events in a event loop */
  .
  if (ev == serial_line_event_message && data != NULL) {
      handle_serial_input((const char *) data);
  }

Example: examples/avr-rss2/ipv6/rpl-border-router/border-router.c


Build
-----
To include in project:
in Makefile
.
+APPS += cli

In project-conf.h
/* cli config */
#define CLI_CONF_COMMAND_PROMPT  "KTH-MQTT> "
#define CLI_CONF_PROJECT  "GreenIoT v1.0 2017-03-13"

Use
---
KTH-MQTT> sho neigh

Neighbors IPv6            Sec  State       Rank      RPL Flg   ETX  RSSI Freshness
fe80::212:4b00:616:1019   N/A  REACHABLE   1.00[128] P   0     1274  16    8
fe80::fec2:3d00:0:7ce     N/A  REACHABLE   2.00[256] P   0     261   14    9
fe80::fec2:3d00:0:1242    N/A  REACHABLE   3.34[428] P*  0     128   13    16

KTH-MQTT> 
DefRT: :: -> fe80::fec2:3d00:0:1242 (infinite lifetime)
Routes:

KTH-MQTT> show dag

DAG_ID: fd02::212:4b00:616:1019
DAG: rank:480 version:240
DAG: grounded:128 preferenc:7
DAG: used:1 joined:1
DAG: lifetime:2946
INSTANCE: instance_id: 29 used; 1
    dio_sent: 62 dio_recv: 36 dio_totint: 70
RPL: MOP 2 OCP 1 rank 480 dioint 14, nbr count 3
RPL: nbr  25   128,  1274 =>  1402 -- 14 f  (last tx 1 min ago)
RPL: nbr 206   256,   224 =>   480 -- 11 fp (last tx 0 min ago)
RPL: nbr  66   428,   274 =>   702 -- 16 f  (last tx 0 min ago)
RPL: end of list

KTH-MQTT> show stats
rpl.mem_overflows=0
rpl.local_repairs=0
rpl.global_repairs=0
rpl.malformed_msgs=0
rpl.resets=12
rpl.parent_switch=10
rpl.forward_errors=0
rpl.loop_errors=0
rpl.root_repairs=0

Authors
--------
See cli.c code is used from several projects.
