## Configuration

We tested the `adaptivesec_driver` with ContikiMAC and nullrdc on TelosB, Re-Motes, and OpenMotes.

For using the `adaptivesec_driver`, first add this to your makefile:
`MODULES += core/net/llsec/adaptivesec`

The security levels of unicast and broadcast frames can be configured individually:
```c
#undef ADAPTIVESEC_CONF_UNICAST_SEC_LVL
#define ADAPTIVESEC_CONF_UNICAST_SEC_LVL 2
#undef ADAPTIVESEC_CONF_BROADCAST_SEC_LVL
#define ADAPTIVESEC_CONF_BROADCAST_SEC_LVL 2
```

The table below lists what each security level does by default:

| Security Level | Description                   | MIC length  |
| -------------- |:-----------------------------:| -----------:|
| 1              | Authentication only           | 6           |
| 2              | Authentication only           | 8           |
| 3              | Authentication only           | 10          |
| 5              | Authentication and encryption | 6           |
| 6              | Authentication and encryption | 8           |
| 7              | Authentication and encryption | 10          |

To reduce the security-related per frame overhead add:
```c
#undef LLSEC802154_CONF_USES_AUX_HEADER
#define LLSEC802154_CONF_USES_AUX_HEADER 0
```
If RAM is scarce, lower the maximum number of neighbors:
```c
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 14
```

Depending on whether you want to use group session keys or pairwise session keys, either add:
```c
#include "net/llsec/adaptivesec/noncoresec-autoconf.h"
```
or
```c
#include "net/llsec/adaptivesec/coresec-autoconf.h"
```
, respectively.

Also, set a network-wide key by adding:
```c
#define AKES_SINGLE_CONF_KEY { 0x00 , 0x01 , 0x02 , 0x03 , \
                               0x04 , 0x05 , 0x06 , 0x07 , \
                               0x08 , 0x09 , 0x0A , 0x0B , \
                               0x0C , 0x0D , 0x0E , 0x0F }
```

Finally, configure a `seeder` like so
```c
#undef CSPRNG_CONF_SEEDER
#define CSPRNG_CONF_SEEDER iq_seeder
```

## Troubleshooting

### nullrdc

When using `nullrdc`, acknowledgement frames should be disabled:
```c
#undef NULLRDC_CONF_802154_AUTOACK
#define NULLRDC_CONF_802154_AUTOACK 0
#undef NULLRDC_CONF_802154_AUTOACK_HW
#define NULLRDC_CONF_802154_AUTOACK_HW 0
#undef NULLRDC_CONF_SEND_802154_ACK
#define NULLRDC_CONF_SEND_802154_ACK 0
```
This is because the `HELLOACK-ACK` negotiation of AKES seems too fast for the current implementation of `nullrdc`. Enabling CSMA is an alternative solution:
```c
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC csma_driver
```

### ContikiMAC

When using ContikiMAC with phase-lock optimization, ensure that CSMA is also enabled. Otherwise, the frame counters of outgoing frames may not be in ascending order, which causes replay protection to filter out fresh frames sometimes.

```c
#undef CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 1
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC csma_driver
```

To improve ContikiMAC's reliability, switch on CSMA:
```c
#undef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC csma_driver
```
and/or the ContikiMAC framer:
```c
#undef NETSTACK_CONF_FRAMER
#define NETSTACK_CONF_FRAMER contikimac_framer
#undef CONTIKIMAC_FRAMER_CONF_ENABLED
#define CONTIKIMAC_FRAMER_CONF_ENABLED 1
#undef CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER
#define CONTIKIMAC_FRAMER_CONF_DECORATED_FRAMER adaptivesec_framer
#undef ADAPTIVESEC_CONF_DECORATED_FRAMER
#define ADAPTIVESEC_CONF_DECORATED_FRAMER framer_802154
```

## Reading

* [Paper](http://dl.acm.org/citation.cfm?id=2818002)