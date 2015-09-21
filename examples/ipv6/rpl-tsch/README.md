A RPL+TSCH node. Will act as basic node by default, but can be configured at startup
using the user button and following instructions from the log output. Every press
of a button toggles the mode as 6ln, 6dr or 6dr-sec (detailled next). After 10s with
no button press, the node starts in the last setting. The modes are:
* 6ln (default): 6lowpan node, will join a RPL+TSCH network and act as router.
* 6dr: 6lowpan DAG Root, will start its own RPL+TSCH network. Note this is not a
border router, i.e. it does not have a serial interface with connection to
the Internet. For a border router, see ../border-router.
* 6dr-sec: 6lowpan DAG Root, starting a RPL+TSCH network with link-layer security
enabled. 6ln nodes are able to join both non-secured or secured networks.  
