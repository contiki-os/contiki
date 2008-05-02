if (msg.startsWith("Callback function called at time 50000 (counter=100)")) {
  log.log("TEST OK\n");
  mote.getSimulation().getGUI().doQuit(false);
}
