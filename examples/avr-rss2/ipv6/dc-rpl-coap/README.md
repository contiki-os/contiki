Example: mockup of DC converter functionality for IoT-grid.
===========================================
This example imitates DC converter functionality for IoT-grid.
We use standard IoT protocol stack with CoAP application-level protocol. 
The settings are as follows.

* APPLICATION:		CoAP
* TRANSPORT:		UDP
* NETWORK:		IPv6/RPL
* ADAPTATION:		6LoWPAN
* MAC:			nullmac_driver
* RADIO DUTY CYCLE:	nullrdc_driver
* PHYSICAL:		IEEE 802.15.4

CoAP resources
----------------
We define a CoAP resource for each functionality of DC converter.
Since each functionality may have several parameters, we define each resource
as a vector of parameters as follows.

* /dcdc/status	read-only parameters for power monitoring.
It also support periodic monitoring through CoAP observe option.
  ** 0	VOUT	Output voltage 
  ** 1	VIN	Input voltage
  ** 2	IOUT	Output current
  ** 3	IIN	Input current

* /dcdc/vdc	configurable parameters for voltage droop control function.
  ** 0	VGRID	Desired grid output voltage
  ** 1	SLOPE	Slope of voltage droop control function
  ** 2	PMAX	Maximum output power allowed

* /dcdc/hwcfg	configurable parameters for DC converter hardware.
  ** 0	VMAX	Maximum output voltage allowed
  ** 1	IMAX	Maximum output current allowed

Each functionality is implemented as a sensor type device.
They are located in "dev" folder.
* dc-status-sensor	for /dcdc/status
* dc-vdc-sensor		for /dcdc/vdc
* dc-hw-sensor		for /dcdc/hwcfg

The corresponding CoAP handler for each resource is defined in "resources" folder.
* res-dc-status-obs	for /dcdc/status
* res-dc-vdc		for /dcdc/vdc
* res-dc-hwcfg		for /dcdc/hwcfg

coap-server.c
----------------
The server acts as the RPL root node.
It has 3 CoAP resources as described above.

coap-client.c
----------------
The client periodically send a CoAP command (either GET or PUT) to monitor/update 
values of parameters of a resource. 

project-conf.h
----------------
This file contains definitions needed for this IoT-grid example as follows.
* Enable IPv6 network stack
* Set nullrdc_driver
* Set nullmac_driver
* Activate CoAP observe client library (COAP_OBSERVE_CLIENT = 1)
* Increase the maximum number of observee and maximum number of open transaction to 10
(COAP_MAX_OPEN_TRANSACTIONS and COAP_MAX_OBSERVEES = 10)

