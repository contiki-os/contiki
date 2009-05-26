TIMEOUT(120000);

var nr_packets = new Array();
for (i=1; i <= 10; i++) {
  nr_packets[i] = 0;
}

while (true) {

  /* Listen for receive notifications */
  if (msg.contains('abc message received')) {

    /* Log receiving node */
    nr_packets[id] ++;
    log.log("Node " + id + " received message: " + nr_packets[id] + "\n");

    log.log("TEST STATUS: ");
    for (i = 1; i <= 10; i++) {
      log.log(nr_packets[i] + " ");
    }
    log.log("\n");
  }

  /* Did all nodes (1-10) receive at least one message? */
  for (i = 1; i <= 10; i++) {
    if (nr_packets[i] < 1) break;
    if (i == 10) log.testOK();
  }

  YIELD();
}
