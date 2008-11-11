/* Wait until node has booted */
if (msg.startsWith('Starting')) {
  log.log("Shell started\n");
  global.put("started", true);
}
result = global.get("started");
if (result == null || result == false) {
  /*log.log("Shell did not yet start\n");*/
  return;
}

/* Send command ps */
result = global.get("command_ps");
if (result == null || result == false) {
  log.log("Sending 'ps'\n");
  global.put("command_ps", true);
  node.write("ps");
  return;
}

/* Wait for ps response */
if (msg.startsWith('Event timer')) {
  log.log("Reponse from 'ps'\n");
  global.put("response_ps", true);
}
result = global.get("response_ps");
if (result == null || result == false) {
  /*log.log("Waiting for 'ps' response\n");*/
  return;
}

/* Send command help */
result = global.get("command_help");
if (result == null || result == false) {
  log.log("Sending 'help'\n");
  global.put("command_help", true);
  node.write("help");
  return;
}

/* Wait for help response */
if (msg.startsWith('write <filename>')) {
  log.log("Reponse from 'help'\n");
  global.put("response_help", true);
}
result = global.get("response_help");
if (result == null || result == false) {
  /*log.log("Waiting for 'help' response\n");*/
  return;
}

log.testOK(); /* We are done! */