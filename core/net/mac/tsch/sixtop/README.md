# [EXPERIMENTAL] 6TiSCH Operation Sublayer (6top)

## Overview

6TiSCH Operation Sublayer (6top), a logical link layer in the 6TiSCH
architecture, provides the abstraction of an IP link over a TSCH MAC layer (IEEE
802.15.4e). The functionality of 6top is to facilitate the dynamic cell
negotiation with one-hop neighbors.

This is a Contiki implementation of 6top, developed by:

- Shalu R, CDAC, shalur@cdac.in, github user: [shalurajendran](https://github.com/shalurajendran)
- Lijo Thomas, CDAC, lijo@cdac.in
- Yasuyuki Tanaka

It supports 6P (6top Protocol) of [draft-03](draft-ietf-6tisch-6top-protocol-03)
except for concurrent 6P transactions described in [Section
4.3.3](https://tools.ietf.org/html/draft-ietf-6tisch-6top-protocol-03#section-4.3.3).

## Code Structure

| File            | Contents                       |
|-----------------|--------------------------------|
| sixtop.[ch]     | 6top external APIs             |
| sixp.[ch]       | 6P I/O APIs                    |
| sixp-nbr.[ch]   | 6P Neighbor Management APIs    |
| sixp-pkt.[ch]   | 6P Packet Manipulation APIs    |
| sixp-trans.[ch] | 6P Transaction Management APIs |
| sixp-conf.h     | 6P Configuration Parameters    |

## Configuration Paramters

| Parameter                            | Meaning                                      | Default Value |
|--------------------------------------|----------------------------------------------|---------------|
| SIXTOP_CONF_MAX_SCHEDULING_FUNCTIONS | The maximum number of SFs installed in 6top  |             1 |
| SIXTOP_CONF_MAX_TRANSACTIONS         | The maximum number of transactions at a time |             1 |

## Caveat

At this moment, allocated 6P neighbor states never be deleted since 6P has no
way to decide to delete them. 6P neighbor state consists of SeqNum to be used
for a next reqeust, GTX, and GRX (defined in sixp-nbr.c).

Once the number of 6P neighbor states reaches to the `SIXTOP_6P_MAX_NEIGHBORS`,
a transaction with a new neighbor may fail due to failure in allocating a 6P
neighbor state or schedule generation inconsistency.

## Example

An example using 6top is provided under `examples/ipv6/rpl-tsch-sxitop/`.

A simple Scheduling Function (SF) is implemented in `sf-simple.[ch]`. This SF
does not have bandwidth monitoring mechanism nor bandwidth adaptation
mechanism. Instead, the SF provides APIs so that a user process can add or
remove cells dynamically. A sample user process implementation using the SF is
found in `node-sixtop.c`, which is tested with `rpl-tsch-sixtop-z1.csc`.

## Test

6top or 6P related tests are implemented under `regression-tests/26-6tisch`.

## Using 6top

In order to use 6top with your system, `sixtop` module must be built together in
addition to [TSCH](../).

Enable TSCH, then set 1 to `TSCH_CONF_WITH_SIXTOP`, typically, in your
`project-conf.h`:

```C
#undef TSCH_CONF_WITH_SIXTOP
#define TSCH_CONF_WITH_SIXTOP 1
```

Next, add `sixtop` into `MODULES` in your Makefile:

```Makefile
MODULES += core/net/mac/tsch/sixtop
```

For now, the simple SF mentioned above is only one SF shipped in the code
base. You may have to implement your own SF.

## Implementing a Scheduling Function

Scheduling Function (SF) is represented with `sixtop_sf_t`, which defined in
`sixtop.h`. An SF can be added into 6top by `sixtop_add_sf()`. After the
addition, a handler defined in `sixtop_sf_t` is invoked on a correspondent
event, for example, reception of a 6P request or timeout of an ongoing
transaction.

SF can send a 6P packet by `sixp_output()`. After every Add or Delete operation,
SF needs to advance schedule generation counter associated with the peer by
`sixp_advance_generation()`, counter which is maintaeind by 6P. Otherwise, SF
will not able to perform any command request other than Clear due to schedule
generation inconsistency.

## References

1. [IETF 6TiSCH Working Group](https://datatracker.ietf.org/wg/6tisch)
2. [6top Protocol (6P)](https://tools.ietf.org/html/draft-ietf-6tisch-6top-protocol)


