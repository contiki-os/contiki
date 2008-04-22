if (msg.startsWith("Bye, world")) {
  log.log("TEST OK\n");
  mote.getSimulation().getGUI().doQuit(false);
}
