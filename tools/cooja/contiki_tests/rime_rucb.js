if (msg.startsWith('Completion time')) {
  log.log(msg + "\n");
  log.testOK(); /* Report test success and quit */
}
