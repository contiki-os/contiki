
/* Count received packets */
if (msg.contains('received')) {
  result = global.get("recv");
  if (result == null) {
    result = 0;
  }

  result++;
  global.put("recv", result);
  log.log("Received packets count now: " + result + "\n");
}

/* Count timed out packets */
if (msg.contains('timed out')) {
  result = global.get("timeout");
  if (result == null) {
    result = 0;
  }

  result++;
  global.put("timeout", result);
  log.log("Timed out packets count now: " + result + "\n");
}



/* Count sent packets */
if (msg.contains('sent to')) {
  result = global.get("sent");
  if (result == null) {
    result = 0;
  }

  result++;
  global.put("sent", result);
  log.log("Sent packets count now: " + result + "\n");
}

/* Look for test completion */
countSent = global.get("sent");
if (countSent >= 10) {

  /* Make sure received counter matches sent counter */
  countReceived = global.get("recv");
  if (countReceived < countSent) {
    log.log("Received < Sent: " + countReceived + " < " + countSent + "\n");
    log.log("Received packets less than acked sent packets!\n");
    log.testFailed();
  }

  /* Make sure some packets timed out (all from node 4) */
  countTimedOut = global.get("timeout");
  if (countTimedOut == null || countTimedOut < 2) {
    log.log("Timed out: " + countTimedOut + "\n");
    log.log("Too few packets timed out!\n");
    log.testFailed();
  }

  log.log("Received / Sent: " + countReceived + " / " + countSent + "\n");
  log.log("Timed out: " + countTimedOut + "\n");
  log.testOK();
}
