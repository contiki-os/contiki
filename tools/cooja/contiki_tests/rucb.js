if (msg.startsWith("Completion time ")) {
  log.log("TEST OK\n");
  mote.getSimulation().getGUI().doQuit(false);
}
