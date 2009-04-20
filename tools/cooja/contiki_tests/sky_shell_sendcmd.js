TIMEOUT(150000);

log.log("Random seed: " + mote.getSimulation().getRandomSeed() + "\n");

/* Wait for nodes to boot */
mote1 = null;
while (mote1 == null) {
  if (id == 1) {
    mote1 = mote;
  }
  YIELD();
}
GENERATE_MSG(2000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Send command: mote 1 -> mote 2*/
command = "sendcmd 2.0 echo hello\n";
log.log("mote1> " + command);
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 2 && msg.contains("hello"));
log.log("mote2: " + msg + "\n");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Send command: mote 1 -> mote 2*/
command = "sendcmd 2.0 {echo hello again}\n";
log.log("mote1> " + command);
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 2 && msg.contains("hello again"));
log.log("mote2: " + msg + "\n");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Send command: mote 1 -> mote 2 -> mote 3 -> mote 4 */
command = "sendcmd 2.0 {sendcmd 3.0 {sendcmd 4.0 echo multihop}}\n";
log.log("mote1> " + command);
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 4 && msg.contains("multihop"));
log.log("mote4: " + msg + "\n");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Test finished at time: " + mote.getSimulation().getSimulationTime() + "\n");
log.testOK(); /* Report test success and quit */
