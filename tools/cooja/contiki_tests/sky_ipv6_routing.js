/* Make test automatically fail (timeout) after 100 simulated seconds */
TIMEOUT(100000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

log.log("waiting for server response at mote 2\n");
WAIT_UNTIL(id == 2 && msg.startsWith("Response from the server"));

log.log("ok, reporting success now\n");
log.testOK(); /* Report test success and quit */
