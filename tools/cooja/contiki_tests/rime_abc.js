/* Only handle receive messages */
if (!msg.contains('received')) {
  return;
}

/* Remember receiver */
global.put("recv_" + id, "ok");
log.log(id + " received a message\n");

/* Did all nodes (1 and 2) receive a message? */
for (i = 1; i <= 2; i++) {
  result = global.get("recv_" + i);
  if (result == null) {
    return;
  }
}

log.log("TEST OK\n"); /* Report test success */
mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
