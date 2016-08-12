A RPL node. Will act as basic node by default, but can be configured at startup
using the user button and following instructions from the log output. Every press
of a button toggles the mode as 6ln and 6dr. After 10s with no button press,
the node starts in the last setting. The modes are:
* 6ln (default): 6lowpan node, will join a RPL network and act as router.
* 6dr: 6lowpan DAG Root, will start its own RPL network. Note this is not a
border router, i.e. it does not have a serial interface with connection to
the Internet. For a border router, see ../border-router.  

To indicate the hardware target for the node, add one of the following
options in the command line:
If rpl-border-router runs on dongle:
JN516x_WITH_DONGLE=1
If rpl-border-router runs on DR1174:
JN516x_WITH_DR1174=1
If building for a new platform, first execute : make clean

For more information, see ../README.md.
