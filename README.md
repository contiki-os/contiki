# RICH Contiki Code

[![Build Status](https://travis-ci.org/EIT-ICT-RICH/contiki.svg?branch=rich-3.x)](https://travis-ci.org/EIT-ICT-RICH/contiki/branches)

This repository contains open source contributions to the Conitki OS developped in the framework of the EIT Digital **RICH** Activity.

It features a **TSCH** implementation with 6TiSCH minimal and Orchestra support, and a port to NXP's **JN516X** platform.
See `examples/rich` for application examples.

## JN516x

This repository includes a comprehensive port of the
[jn516x chip family](http://www.nxp.com/products/microcontrollers/product_series/jn516x)
to Contiki.
It supports all Contiki networking stacks (MAC and NET) and was tested at scale in
several testbeds including a 25 node testbed running our 6TiSCH stack
(also part of this repository).

More information on how to get started with the port at:
https://github.com/EIT-ICT-RICH/contiki/blob/rich-3.x/platform/jn516x/README.md

The jn516x port was merged in the official Contiki repository via this pull request:
https://github.com/contiki-os/contiki/pull/1219

## TSCH, 6TiSCH, and Orchestra

This repository features a
[IEEE 802.15.4e TSCH](http://standards.ieee.org/getieee802/download/802.15.4e-2012.pdf)
+
[6TiSCH](https://datatracker.ietf.org/wg/6tisch)
implementation that is the result of two years of work,
was tested extensively in different testbeds and on different hardware platforms, and checked
for interoperability at the
[6TiSCH plugtest](http://www.etsi.org/news-events/events/942-6tisch-plugtests)
organized by ETSI in Prague in Julty 2015.
It supports a rich feature set including standard link-layer security, dynamic join process with
runtime dissemination of hopping sequence and timing settings, and a scheduling API.
We also provide our autonomous scheduler for TSCH + RPL networks,
[Orchestra](http://www.simonduquennoy.net/papers/duquennoy15orchestra.pdf).

More information, documentation on how to use and port to your own platform are available at:
https://github.com/EIT-ICT-RICH/contiki/blob/rich-3.x/core/net/mac/tsch/README.md

TSCH is currently being considered for inclusion in the official Contiki repository through this PR:
https://github.com/contiki-os/contiki/pull/1285
