/* Script is called once for every node log output. */
/* Input variables: Mote mote, int id, String msg. */

if (msg.startsWith('Hello, world')) {
  log.log('TEST OK\n'); /* Report test success */
  mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
}
