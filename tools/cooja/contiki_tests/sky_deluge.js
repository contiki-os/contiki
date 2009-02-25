TIMEOUT(100000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

WAIT_UNTIL(id == 3 && msg.contains("version 1"));
log.log("Node 3 got version 1\n");

WAIT_UNTIL(id == 5 && msg.contains("version 1"));
log.log("Node 5 got version 1\n");

log.testOK(); /* Report test success and quit */
