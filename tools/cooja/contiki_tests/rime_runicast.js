TIMEOUT(240000);

nr_recv = 0;
nr_timedout = 0;
nr_sent = 0;

while (nr_sent < 10) {

/* Count received packets */
if (msg.contains('received')) {
  nr_recv++;
  log.log("RECV=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
}

/* Count timed out packets */
else if (msg.contains('timed out')) {
  nr_timedout++;
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", TIMEDOUT=" + nr_timedout + "\n");
}

/* Count sent packets */
else if (msg.contains('sent to')) {
  nr_sent++;
  log.log("recv=" + nr_recv + ", SENT=" + nr_sent + ", timedout=" + nr_timedout + "\n");
}

YIELD();
}

/* Make sure received counter matches sent counter */
if (nr_recv < nr_sent-3) {
  log.log("Error: Received << Sent!\n");
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testFailed();
}

if (nr_recv > nr_sent+3) {
  log.log("Error: Received >> Sent!\n");
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testFailed();
}

/* Make sure some packets timed out (all from node 4) */
if (nr_timedout < 2) {
  log.log("Error: Too few packets timed out!\n");
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testFailed();
}

log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
log.testOK();
