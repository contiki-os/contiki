TIMEOUT(90000, log.log("last message: " + msg + "\n"));

mote1 = null;
mote4 = null;
nr_boot = 0;

/* Wait until all nodes have booted */
while (true) {
  if (msg.contains('Starting')) {
    nr_boot++;
    if (id == 1) mote1 = mote;
    if (id == 4) mote4 = mote;
  }
  if (nr_boot == 4) break;
  YIELD();
}
log.log("All motes booted\n");

/* Wait one second */
GENERATE_MSG(1000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* Create file on node 1 */
log.log("Node 1 creating data\n");
write(mote1, "echo FROM1 | write node1file.txt\n");

/* Download node1file.txt to node 4 */
log.log("Node 4 downloading from node 1\n");
write(mote4, "wget 172.16.1.0/node1file.txt | write node4file.txt\n");
WAIT_UNTIL(id == 4 && msg.contains("FROM1"));

/* Wait one second */
GENERATE_MSG(1000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* Extend data */
log.log("Node 4 appending data\n");
write(mote4, "echo FROM4 | append node4file.txt\n");

/* Download node2file.txt at node 1 */
log.log("Node 1 downloading from node 4\n");
write(mote1, "wget 172.16.4.0/node4file.txt\n");

WAIT_UNTIL(id == 1 && msg.contains("FROM4"));

log.testOK();
