TIMEOUT(100000, log.log("Node 1: " + nr_packets[1] + ".\nNode 2: " + nr_packets[2] + ".\n"));

nr_packets = new Array();
nr_packets[1] = 0;
nr_packets[2] = 0;

while (true) {
  /* Only handle receive messages */
  YIELD_THEN_WAIT_UNTIL(msg.contains('received'));

  /* Count received packets */
  nr_packets[id]++;
  //log.log("Node " + id + " received " + nr_packets[id] + " messages\n");

  if (nr_packets[1] >= 30 && nr_packets[2] >= 30) {
    log.log("Node 1: " + nr_packets[1] + ".\nNode 2: " + nr_packets[2] + ".\n");
    log.testOK(); /* Report test success */
  }

}


