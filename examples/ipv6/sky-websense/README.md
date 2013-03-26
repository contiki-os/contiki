Sky websense
============

This example features a simple webserver running on top of the IPv6 contiki
stack on Sky motes to provide sensor values, and with a RPL border router to
bridge the sensor network to Internet.

To test the example in COOJA under Linux
----------------------------------------

1. Start COOJA and load the simulation "example-sky-websense.csc"

    make TARGET=cooja example-sky-websense.csc

2. Connect to the COOJA simulation using tunslip6:

    make connect-router-cooja

3. You should now be able to browse to the nodes using your web browser:
   Router: http://[aaaa::0212:7401:0001:0101]/
   Node 2: http://[aaaa::0212:7402:0002:0202]/


To run the example on real nodes under Linux
--------------------------------------------

1. Program the nodes with the websense application

    make TARGET=sky sky-websense.upload

2. Disconnect the nodes and program one node with the RPL border router

    cd ../rpl-border-router && make TARGET=sky border-router.upload

3. Connect to the border router using tunslip6:

    make connect-router

4. Reboot the router and note the router IP address

5. You should now be able to browse to your router node using your web browser:
   http://[<ROUTER IPv6 ADDRESS>]/. On this page you should see a list of all
   accessible nodes with their IP adresses.
