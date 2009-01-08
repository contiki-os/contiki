TIMEOUT(120000, log.log("received/node: " + count[1] + " " + count[2] + " " + count[3] + " " + count[4] + " " + count[5] + " " + count[6] + " " + count[7] + "\n"));

/* Conf. */
booted = new Array();
count = new Array();
nrNodes = 7;
nodes_starting = true;
for (i = 1; i <= nrNodes; i++) {
  booted[i] = false;
  count[i] = 0;
}

/* Wait until all nodes have started */
while (nodes_starting) {
  WAIT_UNTIL(msg.startsWith('Starting'));
  log.log("Node " + id + " booted\n");
  booted[id] = true;

  for (i = 1; i <= nrNodes; i++) {
    if (!booted[i]) break;
    if (i == nrNodes) nodes_starting = false;
  }
}

/* Create sink */
log.log("All nodes booted, creating sink at node " + id + "\n");
mote.getInterfaces().getButton().clickButton()

while (true) {
  YIELD();

  /* Count sensor data packets */
  source = msg.split(" ")[0];
  log.log("Got data from node " + source + "\n");
  count[source]++;

  /* Fail if any node has transmitted more than 20 packets */
  for (i = 1; i <= nrNodes; i++) {
    if (count[i] > 20) {
      log.log("received/node: " + count[1] + " " + count[2] + " " + count[3] + " " + count[4] + " " + count[5] + " " + count[6] + " " + count[7] + "\n");
      log.testFailed(); /* We are done! */
    }
  }

  /* Wait until we have received data from all nodes */
  for (i = 1; i <= nrNodes; i++) {
    if (count[i] < 5) break;
    if (i == nrNodes) log.testOK();
  }

}
