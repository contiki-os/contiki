REST example
============

Open a terminal and go to "<CONTIKI_HOME>/examples/rest-example/" directory.

MAIN EXAMPLE: rest-server-example.c : A RESTful server example showing how to
use the REST layer to develop server-side applications (possible to run it over
either COAP or HTTP) To use COAP as the underlying application protocol, one
should define WITH_COAP = 1 in rest-example/Makefile. Otherwise, HTTP is used.
Look at the source code to see which resources are available. (check the
RESOURCE macros in the code).  Each resource has a handler function which is
called by the REST layer to serve the request.  (i.e. "helloworld" resource has
a handler function named "helloworld_handler" which is called when a web
service request is received for "helloworld" resource.)


To run REST examples in COOJA on Linux
--------------------------------------------

Accessing the server from outside:

1. Start COOJA and load the simulation "rest-server-example.csc" by the following command.

        make TARGET=cooja rest-server-example.csc

2. After loading the COOJA file, open another another terminal pointing to the
   same directory and connect to the COOJA simulation using tunslip6:

        make connect-router-cooja

3. You need to use a COAP or HTTP client to interact with the COOJA nodes
   running REST code.  In this setting, two servers are available: IP addresses
   are aaaa::0212:7402:0002:0202 and aaaa::0212:7403:0003:0303. COAP uses
   61616, whereas HTTP uses 8080 port in default configuration.  First, ping
   the COOJA nodes to test the connectivity.

        ping6 aaaa::0212:7402:0002:0202
        ping6 aaaa::0212:7403:0003:0303

HTTP Examples
-------------

You can use curl as an http client to interact with the COOJA motes running
REST code.

    curl -H "User-Agent: curl" aaaa::0212:7402:0002:0202:8080/helloworld #get helloworld plain text
    curl -H "User-Agent: curl" aaaa::0212:7402:0002:0202:8080/led?color=green -d mode=off -i #turn off the green led
    curl -H "User-Agent: curl" aaaa::0212:7402:0002:0202:8080/.well-known/core -i
    curl -X POST -H "User-Agent: curl" aaaa::0212:7402:0002:0202:8080/helloworld #method not allowed

COAP Examples
-------------

You should run a COAP client on your computer. You can use the URLs and methods
provided above in HTTP examples to test the COAP Server.  For example, Matthias
Kovatsch has developed a CoAP Firefox plug-in which is accessible via
[http://people.inf.ethz.ch/mkovatsc/#pro](http://people.inf.ethz.ch/mkovatsc/#pro)

Accessing the server inside the sensor network: (Note: Provided only for COAP
implementation) Start COOJA and load the simulation
"coap-client-server-example.csc" by the following command.

    make TARGET=cooja coap-client-server-example.csc

coap-client-server-example.csc : Runs rest-server-example.c as the server (over
COAP) (IP:aaaa::0212:7401:0001:0101) in one node and coap-client-example.c as
the client (IP: aaaa::0212:7402:0002:0202) in another node.  Client
periodically accesses resources of server and prints the payload.

Note: If the generated binary is bigger than the MOTE code size, then you will
get a "region text is full" error.  Right now, REST+HTTP example uses (Contiki
& ContikiMAC & uIPv6 & RPL & HTTP Server & REST Layer) which does not fit in
Tmote Sky memory.  To save same code space and make the example fit, you can
define static routes rather than using RPL or use nullrdc rather than
ContikiMAC.  If border router does not fit, then first try to update the
Makefile of border router in <CONTIKI_HOME>/examples/ipv6/rpl-border-router by
setting WITH_WEBSERVER=0.

To run REST server on real nodes (i.e. tmote sky)
--------------------------------------------

1. Program the nodes with the rest-server-example

        make TARGET=sky rest-server-example.upload

2. Disconnect the nodes and program one node with the RPL border router

        cd ../ipv6/rpl-border-router && make TARGET=sky border-router.upload

3. Connect to the border router using tunslip6:

        make connect-router

4. Reconnect the motes, open new terminal for each mote and run the following
   command to note their IP addresses (after running the command reset the
   corresponding mote to get IP address printed)

        make login TARGET=sky MOTE=2   #Shows the prints for first mote
        make login TARGET=sky MOTE=3   #For second mote and so on.

5. Test the connectivity by pinging them.

        ping6 <IPv6 Address of the MOTE>

6. Remaining parts are the same with the COOJA example. (i.e. if it is a COAP
   Server, it's available at <NODE_IP_ADDR>:61616)


To run REST server with minimal-net on Linux
--------------------------------------------
1. Compile with minimal-net setting.

        make rest-server-example TARGET=minimal-net

2. Run the generated executable with sudo and note the IP address of the server
   which will be printed right after.

        sudo ./rest-server-example.minimal-net

3. How to access and test the server is same with the other settings. (i.e. if
   it is a COAP Server, it's available at <IP_ADDRESS_FROM_STEP_2>:61616 and if
   it's a HTTP Server it is available at <IP_ADDRESS_FROM_STEP_2>:8080)

TODO
----

- Better option handling needed - ex: critical options are not differentiated
  for now. Need to add support for some such as Tokens. Also, C/E difference
  should be added.
- Reilable message sending is missing. i.e. client example should resend
  request in case ACK does not arrive. Same for server pushing (in case of
  subscriptions)
- Add Block transfer example
- Add Subscription example
- Add an Android/Java COAP Client to Contikiprojects to be able to interact
  with Contiki.
- COAP-specific Method Codes
