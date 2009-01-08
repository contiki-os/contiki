TIMEOUT(120000);

nr_recv = 0;
nr_timedout = 0;
nr_sent = 0;

while (nr_sent < 10) {

/* Count received packets */
if (msg.contains('received')) {
  nr_recv++;
  log.log("Received packets count now: " + nr_recv + "\n");
}

/* Count timed out packets */
else if (msg.contains('timed out')) {
  nr_timedout++;
  log.log("Timed out packets count now: " + nr_timedout + "\n");
}

/* Count sent packets */
else if (msg.contains('sent to')) {
  nr_sent++;
  log.log("Sent packets count now: " + nr_sent + "\n");
}

YIELD();
}

/* Make sure received counter matches sent counter */
if (nr_recv < nr_sent) {
  log.log("Received < Sent: " + nr_recv + " < " + nr_sent + "\n");
  log.log("Received packets less than acked sent packets!\n");
  log.testFailed();
}

/* Make sure some packets timed out (all from node 4) */
if (nr_timedout < 2) {
  log.log("Timed out: " + nr_timedout + "\n");
  log.log("Too few packets timed out!\n");
  log.testFailed();
}

log.log("Received / Sent: " + nr_recv + " / " + nr_sent + "\n");
log.log("Timed out: " + nr_timedout + "\n");
log.testOK();
