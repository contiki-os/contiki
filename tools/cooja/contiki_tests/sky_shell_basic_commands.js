TIMEOUT(2000, log.log("last message: " + msg + "\n"));

/* Wait until node has booted */
WAIT_UNTIL(msg.startsWith('Starting'));
log.log("Shell started\n");

/* Test command: ps */
node.write("ps");
WAIT_UNTIL(msg.startsWith('Event timer'));

/* Test command: help */
node.write("help");
WAIT_UNTIL(msg.startsWith('write <filename>'));

log.testOK(); /* We are done! */
