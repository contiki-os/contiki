if (msg.startsWith("Callback function called at time 2000000 (counter=4000)")) {
  log.log("TEST OK\n");
  mote.getSimulation().getGUI().doQuit(false);
}
