TIMEOUT(100000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

WAIT_UNTIL(msg.contains("Created connection"));
YIELD_THEN_WAIT_UNTIL(msg.contains("Created connection"));

log.log("Both nodes booted\n");

count = 0;
while (count++ < 5) {
  /* Message from sender process to receiver process */
  YIELD_THEN_WAIT_UNTIL(msg.contains("Sender sending"));
  YIELD_THEN_WAIT_UNTIL(msg.contains("Receiver received"));
  log.log(count + ": Sender -> Receiver OK\n");

  /* Message from receiver process to sender process */
  YIELD_THEN_WAIT_UNTIL(msg.contains("Receiver sending"));
  YIELD_THEN_WAIT_UNTIL(msg.contains("Sender received"));
  log.log(count + ": Receiver -> Sender OK\n");
}

log.testOK(); /* Report test success and quit */
