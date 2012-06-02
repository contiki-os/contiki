Short description on how to set-up a sensor network for global IPv6 addresses.
NOTE: this assumes that you do not have a native IPv6 connection.

You will need:
* PC with Ubuntu (Linux) - 11 or 12 versions
* A node for the RPL-Border-Router (examples/ipv6/rpl-border-router)
* A node for the json webservice (examples/ipv6/json-ws)

Set-up IPv6 tunnel and Border Router
------------------------------------
1. Ensure that you have gogo6c installed.

> sudo apt-get install gogoc

2. Register an account at gogo6 and Freenet6 (http://www.gogo6.com).
   The account at Freenet6 is needed by the gogo6c client.

3. Edit the gogoc.conf and set your own Freenet6 user and password by
   changing the lines with "userid" and "passwd".

4. Start gogoc at command line

> cd contiki/examples/ipv6/json-ws
> sudo gogoc -f gogoc.conf -n

This will print your prefix - TSP_PREFIX.
In my case TSP_PREFIX=2001:05c0:1517:e400 (prefixlen is 56).

5. Connect one of the nodes to the PC (via USB or serial) and program
   it with the RPL-border-router (assumes Z1 node).

> cd contiki/examples/ipv6/rpl-border-router
> make DEFINES=DEFINES=NETSTACK_RDC=nullrdc_driver,NULLRDC_CONF_802154_AUTOACK=1 TARGET=z1 border-router.upload

6. Run tunslip6 which will forward IP from the RPL network to
   the IPv6 tunnel (and to the Internet).

> cd contiki/examples/ipv6/rpl-border-router
> make connect-router PREFIX=<TSP_PREFIX>::1/64

   When you start this you should get a printout from the border-router
   which give you the IPv6 address of it.

Server IPv6 addresses:
 2001:05c0:1517:e400:c30c::10a
 fe80::c30c:0:0:10a

7. Browse using Mozilla Firefox (or any other browser) to the IPv6 address
   given by the border router. This will show you the list of other nodes
   connected to the RPL network.

   http://[2001:05c0:1517:e400:c30c::10a]/

   NOTE: this is a global IPv6 address so it should also be reachable from
   any machine on the Internet.

Configuration of COSM submission
--------------------------------
1. Register a COSM account at https://cosm.com/
   Set-up a feed and create an API key for the feed.

2. Program the sensor node with (assumes Z1)

> cd contiki/examples/ipv6/json-ws
> make websense-z1.upload WITH_COSM=1 TARGET=z1

3. Check the IPv6 address of the node via the RPL-border router or
   by looking at printouts when booting (make login TARGET=z1)

4. You need to configure the node to push data to the COSM feed and
   this can be done in several ways. For convenience a Python script
   is included that pushes the configuration to the nodes.

   Edit the file 'setcosm.py' and replace "<your-key>" and
   "<your-feed>" with your COSM API key and COSM feed id. You can then
   use this Python script to configure your nodes.

   This is an example that configures the node with IP address
   2001:05c0:1517:e400:c30c::10b to push data to the COSM feed with
   stream 1:

> cd contiki/examples/ipv6/json-ws
> ./setcosm.py [2001:05c0:1517:e400:c30c::10b] 1

   Another way to configure the nodes is to use a REST add-on for the
   web browser to post a COSM configuration to the node. "REST Client"
   for Mozilla Firefox is an example of such add-on.

   POST a JSON expression to your node with the following data:
   This assumes that you have the feed with id 55180 and want to post
   to stream 1 in that feed. The field 'appdata' should be set to the
   API key you created at the COSM web site for the feed.

{
 "host":"[2001:470:1f10:333::2]",
 "port":80,
 "path":"/v2/feeds/55180/datastreams/1",
 "appdata":"<insert your COSM API key>",
 "interval":120,
 "proto":"cosm"
}

   This will configure the node to periodically push temperature data
   every other minute. You can use GET to retrieve the data to se that
   the node has been successfully configured (the COSM API key will be
   visualized as a number of stars).
