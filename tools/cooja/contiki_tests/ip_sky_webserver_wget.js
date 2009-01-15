TIMEOUT(30000, log.log("last message: " + msg + "\n"));

mote1 = null;
mote4 = null;

/* Wait until both node 1 and 4 have booted */
while (true) {
  if (msg.contains('Starting')) {
    if (id == 1) mote1 = mote;
    if (id == 4) mote4 = mote;
  }
  if (mote1 != null && mote4 != null) break;
  YIELD();
}
log.log("Mote 1 and mote 4 booted\n");

/* Wait one second */
GENERATE_MSG(1000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* Create file on node 1 */
log.log("Creating data on node 1\n");
write(mote1, "echo data generated at node 1 | write node1file.txt\n");

/* Fetch file to node 2 */
log.log("Fetching data to node 4\n");
write(mote4, "wget 172.16.1.0/node1file.txt | write node2file.txt\n");

WAIT_UNTIL(id == 4 && msg.contains("data generated at node 1"));
log.log("Node 4 received data!\n");

log.testOK();
