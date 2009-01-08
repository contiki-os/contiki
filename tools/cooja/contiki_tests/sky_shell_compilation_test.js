TIMEOUT(2000, log.log("last message: " + msg + "\n"));

WAIT_UNTIL(msg.startsWith('Contiki'));
log.testOK();
