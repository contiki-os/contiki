TIMEOUT(120000, log.log(nr_packets[1] + ", " + nr_packets[2] + ", " + nr_packets[3] + ", " + nr_packets[4] + ", " + nr_packets[5] + ", " + nr_packets[6] + ", " + nr_packets[7] + ", " + nr_packets[8] + ", " + nr_packets[9] + ", " + nr_packets[10] + "\n"));

nr_packets = new Array();
for (i=1; i <= 10; i++) {
  nr_packets[i] = 0;
}

WAIT_UNTIL(id == 1 && msg.contains('Starting'));
log.log("Node 1 started. Clicking node button.\n");
mote.getInterfaces().getButton().clickButton()

while (true) {
  /* Only handle receive messages */
  WAIT_UNTIL(msg.contains('received'));

  /* Remember receiving node */
  log.log(id + " received a message\n");
  nr_packets[id]++;

  /* Did all nodes (2-10) receive a message? */
  for (i = 2; i <= 10; i++) {
    if (nr_packets[i] < 1) break;
    if (i == 10) log.log("TEST OK\n"); /* Report test success */
  }
}
