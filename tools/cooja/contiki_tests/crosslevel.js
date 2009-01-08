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
  }

  /* Did all nodes (1-10) receive at least one message? */
  for (i = 1; i <= 10; i++) {
    if (nr_packets[id] < 1) break;
    if (nr_packets[id] == 10) log.testOK();
  }

  YIELD();
}
