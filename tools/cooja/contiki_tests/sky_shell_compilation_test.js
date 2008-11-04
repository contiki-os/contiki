/* Script is called once for every node log output. */
/* Input variables: Mote mote, int id, String msg. */

/* Contiki test script example */
if (msg.startsWith('Contiki')) {
  log.log('TEST OK\n'); /* Report test success */
  
  /* To increase test run speed, close the simulator when done */
  mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
} else {
  log.log('TEST FAIL\n'); /* Report test failure */
  mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
}
