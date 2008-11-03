/* Only handle receive messages */
if (!msg.contains('received')) {
  return;
}

/* Log receiver */
count = global.get("count_" + id);
if (count == null) {
  count = 0;
}
count++;
global.put("count_" + id, count);

log.log("Node " + id + " received message: " + count + "\n");

/* Did all nodes (1-10) receive a message? */
for (i = 1; i <= 10; i++) {
  result = global.get("count_" + i);
  if (result == null || result == 0) {
    log.log("Node " + i + " did not yet receive a message\n");
    return;
  }
}

/* Report test info */
log.log("TEST INFO: ");
for (i = 1; i <= 10; i++) {
  result = global.get("count_" + i);
  log.log(java.lang.Integer.toString(result) + " ");
}
log.log("\n");

log.log("TEST OK\n"); /* Report test success */
mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
