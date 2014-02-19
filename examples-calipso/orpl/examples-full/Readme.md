This is a full-fledged example using ORPL, close to that used for experiments in the SenSys'13 paper "Let the Tree Bloom: Scalable Opportunistic Routing with ORPL".
The directory "tools" includes a number of utility modules, for energy tracing or logging. It also overrides some Contiki core files, namely:
* clock.c and rtimer-arch.c: 32-bit rtimer, allowing to run contikimac with wakeup interval greater than 500ms
* uip-ds6.c: re-implements uip_ds6_set_addr_iid for simple nodeid<->macaddress<->ipaddress mapping
* node-id.c: uses nodeids as defined in the deployment module

