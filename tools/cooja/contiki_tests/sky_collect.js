/* Wait until all nodes have booted */
if (msg.startsWith('Starting')) {
  log.log("Node " + id + " booted\n");
  global.put("boot_" + id, true);
}
for (i = 1; i <= 7; i++) {
  result = global.get("boot_" + i);
  if (result == null || result == false) {
    /*log.log("Node " + i + " did not yet boot\n");*/
    return;
  }
}

/* Create sink */
result = global.get("created_sink");
if (result == null || result == false) {
  log.log("All nodes booted, creating sink at node " + id + "\n");
  mote.getInterfaces().getButton().clickButton()
  global.put("created_sink", true);
  return;
}

/* Log incoming sensor data */
source = msg.split(" ")[0];
count = global.get("count_" + source);
log.log("Got data from node " + source + "\n");
if (count == null) {
  count = 0;
}
count++;
global.put("count_" + source, count);

/* Fail if any node has transmitted more than 20 packets */
for (i = 1; i <= 7; i++) {
  result = global.get("count_" + i);
  if (result > 20) {

  log.log("FAILED: received/node: " + 
    global.get("count_1") + " " +
    global.get("count_2") + " " +
    global.get("count_3") + " " +
    global.get("count_4") + " " +
    global.get("count_5") + " " +
    global.get("count_6") + " " +
    global.get("count_7") + "\n");
    log.testFailed(); /* We are done! */
    return;
  }
}

/* Wait until we have received data from all nodes */
for (i = 1; i <= 7; i++) {
  result = global.get("count_" + i);
  if (result < 5) {
    /*log.log("Node " + i + " only sent " + result + " messages yet\n");*/
    return;
  }
}

log.testOK(); /* We are done! */
