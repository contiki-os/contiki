/* Initiate trickle from node 1 by clicking button (once) */
result = global.get("recv_1");
if (result == null) {
  if (id != 1) {
    return;
  }

  if (!msg.contains('Starting')) {
    return;
  }

  log.log("Clicking node 1 button\n");
  mote.getInterfaces().getButton().clickButton()
  global.put("recv_1", "ok");
  return;
}

/* Only handle receive messages */
if (!msg.contains('received')) {
  return;
}

/* Remember receiver */
global.put("recv_" + id, "ok");
log.log(id + " received a message\n");

/* Did all nodes (1-10) receive a message? */
for (i = 1; i <= 10; i++) {
  result = global.get("recv_" + i);
  if (result == null) {
    return;
  }
}

log.log("TEST OK\n"); /* Report test success */
mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
