# IEEE 802.15.4e TSCH (TimeSlotted Channel Hopping)

## Overview

TSCH is a MAC layer of the [IEEE 802.15.4e-2012 amendment][ieee802.15.4e-2012],
currently being integrated as part of the new IEEE 802.15.4-2015.
[6TiSCH][ietf-6tisch-wg] is an IETF Working Group focused on IPv6 over TSCH.
This is a Contiki implementation of TSCH and the 6TiSCH so-called "minimal configuration",
which defines how to run a basic RPL+TSCH network.

It was developped by:
* Simon Duquennoy, SICS, simonduq@sics.se, github user: [simonduq](https://github.com/simonduq)
* Beshr Al Nahas, SICS (now Chalmers University), beshr@chalmers.se, github user: [beshrns](https://github.com/beshrns)
* Atis Elsts, Univ. Bristol, atis.elsts@bristol.ac.uk, github user: [atiselsts](https://github.com/atiselsts)

You can find an extensive evaluation of this implementation in our paper [*Orchestra: Robust Mesh Networks Through Autonomously Scheduled TSCH*](http://www.simonduquennoy.net/papers/duquennoy15orchestra.pdf), ACM SenSys'15.

## Features

This implementation includes:
  * Standard IEEE 802.15.4e-2012 frame version 2
  * Standard TSCH joining procedure with Enhanced Beacons with the following Information Elements:
    * TSCH synchronization (join priority and ASN) 
    * TSCH slotframe and link (basic schedule)
    * TSCH timeslot (timeslot timing template)
    * TSCH channel hopping sequence (hopping sequence template)
  * Standard TSCH link selection and slot operation (10ms slots by default)
  * Standard TSCH synchronization, including with ACK/NACK time correction Information Element
  * Standard TSCH queues and CSMA-CA mechanism
  * Standard TSCH security
  * Standard 6TiSCH TSCH-RPL interaction (6TiSCH Minimal Configuration and Minimal Schedule)
  * A scheduling API to add/remove slotframes and links
  * A system for logging from TSCH timeslot operation interrupt, with postponed printout
  * Orchestra: an autonomous scheduler for TSCH+RPL networks
  * A drift compensation mechanism

It has been tested on the following platforms:
  * NXP JN516x (`jn516x`, tested on hardware)
  * Tmote Sky (`sky`, tested on hardware and in cooja)
  * Zolertia Z1 (`z1`, tested in cooja only)
  * CC2538DK (`cc2538dk`, tested on hardware)
  * Zolertia Zoul (`zoul`, tested on hardware)
  * OpenMote-CC2538 (`openmote-cc2538`, tested on hardware)
  * CC2650 (`srf06-cc26xx`, tested on hardware)

This implementation was present at the ETSI Plugtest
event in Prague in July 2015, and did successfully inter-operate with all
four implementations it was tested against.

We have designed this implementation with IPv6 and RPL in mind, but the code is fully independent
from upper layers (with the exception of the optional `tsch-rpl.[ch]`), and has been
also tested with Rime (currently only with 64-bit link-layer addresses).

## Code structure

The IEEE 802.15.4e-2012 frame format is implemented in:
* `core/net/mac/frame802154.[ch]`: handling of frame version 2 
* `core/net/mac/frame802154-ie.[ch]`: handling of Information Elements

TSCH is implemented in:
* `core/net/mac/tsch/tsch.[ch]`: TSCH management (association, keep-alive), processes handling pending
outgoing and incoming packets, and interface with Contiki's upper layers as a MAC driver. TSCH does not
require a RDC (nordc is recommended).
* `tsch-slot-operation.[ch]`: TSCH low-level slot operation, fully interrupt-driven. Node wake up at every active
slot (according to the slotframes and links installed), transmit or receive frames and ACKs. Received packets are
stored in a ringbuf for latter upper-layer processing. Outgoing packets that are dequeued (because acknowledged
or dropped) are stored in another ringbuf for upper-layer processing. 
* `tsch-asn.h`: TSCH macros for Absolute Slot Number (ASN) handling.
* `tsch-packet.[ch]`: TSCH Enhanced ACK (EACK) and enhanced Beacon (EB) creation and parsing.
* `tsch-queue.[ch]`: TSCH  per-neighbor queue, neighbor state, and CSMA-CA.
* `tsch-schedule.[ch]`: TSCH slotframe and link handling, and API for slotframe and link installation/removal.
* `tsch-security.[ch]`: TSCH security, i.e. securing frames and ACKs from interrupt with ASN as part of the Nonce.
Implements the 6TiSCH minimal configuration K1-K2 keys pair.
* `tsch-rpl.[ch]`: used for TSCH+RPL networks, to align TSCH and RPL states (preferred parent -> time source,
rank -> join priority) as defined in the 6TiSCH minimal configuration.
* `tsch-log.[ch]`: logging system for TSCH, including delayed messages for logging from slot operation interrupt.
* `tsch-adaptive-timesync.c`: used to learn the relative drift to the node's time source and automatically compensate for it.

Orchestra is implemented in:
* `apps/orchestra`: see `apps/orchestra/README.md` for more information.

## Using TSCH

A simple TSCH+RPL example is included under `examples/ipv6/rpl-tsch`.
To use TSCH, first make sure your platform supports it.
Currently, `jn516x`, `sky`, `z1`, `cc2538dk`, `zoul`, `openmote-cc2538`, and `srf06-cc26xx` are the supported platforms.
To add your own, we refer the reader to the next section.

To add TSCH to your application, first include the TSCH module from your makefile with:

`MODULES += core/net/mac/tsch`

Then, enable TSCH from your project conf with the following:

```
/* Netstack layers */
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     tschmac_driver
#undef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nordc_driver
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER  framer_802154

/* IEEE802.15.4 frame version */
#undef FRAME802154_CONF_VERSION
#define FRAME802154_CONF_VERSION FRAME802154_IEEE802154E_2012
```

If you are running with RPL, it is recommended to enable the `tsch-rpl` module with:

```
/* TSCH and RPL callbacks */
#define RPL_CALLBACK_PARENT_SWITCH tsch_rpl_callback_parent_switch
#define RPL_CALLBACK_NEW_DIO_INTERVAL tsch_rpl_callback_new_dio_interval
#define TSCH_CALLBACK_JOINING_NETWORK tsch_rpl_callback_joining_network
#define TSCH_CALLBACK_LEAVING_NETWORK tsch_rpl_callback_leaving_network
```

On CC2420-based platforms, enable SFD timestamps with:

```
/* Disable DCO calibration (uses timerB) */
#undef DCOSYNCH_CONF_ENABLED
#define DCOSYNCH_CONF_ENABLED			 0

/* Enable SFD timestamps (uses timerB) */
#undef CC2420_CONF_SFD_TIMESTAMPS
#define CC2420_CONF_SFD_TIMESTAMPS       1
```

To configure TSCH, see the macros in `.h` files under `core/net/mac/tsch/` and redefine your own in your `project-conf.h`.

## Using TSCH with Security

To include TSCH standard-compliant security, set the following:
```
/* Enable security */
#undef LLSEC802154_CONF_ENABLED
#define LLSEC802154_CONF_ENABLED 1
/* TSCH uses explicit keys to identify k1 and k2 */
#undef LLSEC802154_CONF_USES_EXPLICIT_KEYS
#define LLSEC802154_CONF_USES_EXPLICIT_KEYS 1
/* TSCH uses the ASN rather than frame counter to construct the Nonce */
#undef LLSEC802154_CONF_USES_FRAME_COUNTER
#define LLSEC802154_CONF_USES_FRAME_COUNTER 0
```

The keys can be configured in `net/mac/tsch/tsch-security.h`.
Nodes handle security level and keys dynamically, i.e. as specified by the incoming frame header rather that compile-time defined.

By default, when including security, the PAN coordinator will transmit secured EBs.
Use `tsch_set_pan_secured` to explicitly ask the coordinator to secure EBs or not.

When associating, nodes with security included can join both secured or non-secured networks.
Set `TSCH_CONF_JOIN_SECURED_ONLY` to force joining secured networks only.
Likewise, set `TSCH_JOIN_MY_PANID_ONLY` to force joining networks with a specific PANID only.

## TSCH Scheduling

By default (see `TSCH_SCHEDULE_WITH_6TISCH_MINIMAL`), our implementation runs a 6TiSCH minimal schedule, which emulates an always-on link on top of TSCH.
The schedule consists in a single shared slot for all transmissions and receptions, in a slotframe of length `TSCH_SCHEDULE_DEFAULT_LENGTH`.

As an alternative, we provide Orchestra (under `apps/orchestra`), an autonomous scheduling solution for TSCH where nodes maintain their own schedule locally, solely based on their local RPL state.
Orchestra can be simply enabled and should work out-of-the-box with its default settings as long as RPL is also enabled.
See `apps/orchestra/README.md` for more information.

Finally, one can also implement his own scheduler, centralized or distributed, based on the scheduling API provides in `core/net/mac/tsch/tsch-schedule.h`.

## Porting TSCH to a new platform

Porting TSCH to a new platform requires a few new features in the radio driver, a number of timing-related configuration paramters.
The easiest is probably to start from one of the existing port: `jn516x`, `sky`, `z1`, `cc2538dk`, `zoul`, `openmote-cc2538`, `srf06-cc26xx`.

### Radio features required for TSCH

The main new feature required for TSCH is the so-called *poll mode*, a new Rx mode for Contiki radio drivers.
In poll mode, radio interrupts are disabled, and the radio driver never calls upper layers.
Instead, TSCH will poll the driver for incoming packets, from interrupt, exactly when it expects one.

TSCH will check when initializing (in `tsch_init`) that the radio driver supports all required features, namely:
* get and set Rx mode (`RADIO_PARAM_RX_MODE`) as follows:
  * enable address filtering with `RADIO_RX_MODE_ADDRESS_FILTER`
  * disable auto-ack with `RADIO_RX_MODE_AUTOACK`
  * enable poll mode with `RADIO_RX_MODE_POLL_MODE`
* get and set Tx mode (`RADIO_PARAM_TX_MODE`) as follows: 
  * disable CCA-before-sending with `RADIO_TX_MODE_SEND_ON_CCA`
* set radio channel with `RADIO_PARAM_CHANNEL`
* get last packet timestamp with `RADIO_PARAM_LAST_PACKET_TIMESTAMP`
* optionally: get last packet RSSI with `RADIO_PARAM_LAST_RSSI`
* optionally: get last packet LQI with `RADIO_PARAM_LAST_LQI`

### Timing macros required for TSCH

The following macros must be provided:
* `US_TO_RTIMERTICKS(US)`: converts micro-seconds to rtimer ticks
* `RTIMERTICKS_TO_US(T)`: converts rtimer ticks to micro-seconds
* `RADIO_DELAY_BEFORE_TX`: the delay between radio Tx request and SFD sent, in rtimer ticks
* `RADIO_DELAY_BEFORE_RX`: the delay between radio Rx request and start listening, in rtimer ticks
* optionally, `TSCH_CONF_DEFAULT_TIMESLOT_LENGTH`: the default TSCH timeslot length, useful i.e. for platforms
too slow for the default 10ms timeslots.

## Additional documentation

1. [IEEE 802.15.4e-2012 ammendment][ieee802.15.4e-2012]
2. [IETF 6TiSCH Working Group][ietf-6tisch-wg]
3. [A test procedure for Contiki timers in TSCH][tsch-sync-test]

[ieee802.15.4e-2012]: http://standards.ieee.org/getieee802/download/802.15.4e-2012.pdf
[ietf-6tisch-wg]: https://datatracker.ietf.org/wg/6tisch
[tsch-sync-test]: https://github.com/abbypjoby/Contiki-Synchronisation-Test
