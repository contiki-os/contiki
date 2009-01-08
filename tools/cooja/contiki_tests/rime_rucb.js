TIMEOUT(120000);

WAIT_UNTIL(msg.startsWith('Completion time'));
log.testOK();
