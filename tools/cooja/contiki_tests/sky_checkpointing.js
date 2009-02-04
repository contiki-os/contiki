TIMEOUT(120000, log.log("timeout at phase " + phase + ". last message: " + msg + "\n"));
phase=0;

/* Wait until node has booted */
WAIT_UNTIL(msg.startsWith('Starting'));
log.log("Shell started\n");
phase++;

/* 1. BACKGROUND PROCESS - NO CHECKPOINTING */
node.write("repeat 10 1 echo bg process &");
log.log("Starting background process without checkpointing\n");
expected=10;
while (expected > 0) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('bg process'));
  expected--;
}

/* Make sure background process has exited */
GENERATE_MSG(3000, "continue");
while (!msg.contains('continue')) {
  YIELD();
  if (msg.contains('bg process')) {
    log.log("Too many bg messages at phase: " + phase + "\n");
    log.testFailed(); /* We are done! */
    while (true) YIELD();
  }
}
log.log("Background process without checkpointing done\n\n");
phase++;

/* 2. BACKGROUND PROCESS - CHECKPOINTING EVERY SECOND */
node.write("repeat 10 1 echo bg process &");
log.log("Starting background process with periodic checkpointing\n");
expected=10;
while (expected > 0) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('bg process'));
  expected--;
  node.write("checkpoint file" + expected);
}

/* Make sure background process has exited */
GENERATE_MSG(3000, "continue");
while (!msg.contains('continue')) {
  YIELD();
  if (msg.contains('bg process')) {
    log.log("Too many bg messages at phase: " + phase + "\n");
    log.testFailed(); /* We are done! */
    while (true) YIELD();
  }
}
log.log("Background process with periodic checkpointing done\n\n");
phase++;

/* 3. LIST ALL FILES */
node.write("ls");
YIELD_THEN_WAIT_UNTIL(msg.contains('file9'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file8'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file7'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file6'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file5'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file4'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file3'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file2'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file1'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file0'));
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.contains('continue'));
log.log("All checkpoints are stored in the filesystem\n\n");
phase++;

/* 4. ROLLBACK TO RESTORE BACKGROUND PROCESS */
node.write("rollback file7");
log.log("Rollingt back background process at count 7\n");
expected=7;
while (expected > 0) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('bg process'));
  expected--;
}

/* Make sure background process has exited */
GENERATE_MSG(3000, "continue");
while (!msg.contains('continue')) {
  YIELD();
  if (msg.contains('bg process')) {
    log.log("Too many bg messages at phase: " + phase + "\n");
    log.testFailed(); /* We are done! */
    while (true) YIELD();
  }
}
log.log("Background process was rolled back successfully\n\n");
phase++;


log.testOK(); /* We are done! */
