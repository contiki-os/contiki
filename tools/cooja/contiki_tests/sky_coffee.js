/* Script is called once for every node log output. */
/* Input variables: Mote mote, int id, String msg. */

if (msg.startsWith('Coffee consistency test: 0')) {
  log.log('TEST OK');
  mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
} else if (msg.startsWith('Coffee consistency test')) {
  log.log('TEST FAILED');
  mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/
}
