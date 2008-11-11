/* Script is called once for every node log output. */
/* Input variables: Mote mote, int id, String msg. */

/* Contiki test script example */
if (msg.startsWith('Contiki')) {
  log.testOK(); /* Report test success */
} else {
  log.testFailed(); /* Report test failure */
}
