6top Example Description
------------------------

A RPL+TSCH node will act as basic node by default, but can be configured at startup
using the user button and following instructions from the log output. Every press
of a button toggles the mode as 6ln, 6dr or 6dr-sec (detailled next). After 10s with
no button press, the node starts in the last setting. 

The modes are:

* 6ln (default): 6lowpan node, will join a RPL+TSCH network and act as router.
* 6dr: 6lowpan DAG Root, will start its own RPL+TSCH network. Note this is not a
       border router, i.e. it does not have a serial interface with connection to
       the Internet. For a border router, see ../border-router.
* 6dr-sec: 6lowpan DAG Root, starting a RPL+TSCH network with link-layer security
       enabled. 6ln nodes are able to join both non-secured or secured networks.


6top Operation
---------------

If the mode is 6ln (node)

* The application triggers a 6P Add Request to 6dr (neighbor)
* Following this the application triggers another 6P Add Request to 6dr
* After an interval, the application triggers a 6P Delete Request to 6dr

For the Cooja simulation, you may use the rpl-tsch-sixtop-z1.csc file in this folder.
Once you run the simulation, "Mote output" window of Cooja simulator displays the
following messages.

For a 6P Add transaction,
		ID:1 TSCH-sixtop: Sixtop IE received
		ID:1 TSCH-sixtop: Send Link Response to node 2
		ID:2 TSCH-sixtop: Sixtop IE received
		ID:2 TSCH-sixtop: Schedule link x as RX with node 2
		ID:2 TSCH-sixtop: Schedule link x as TX with node 1

Similarly for a 6P Delete transaction.
