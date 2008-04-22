if (msg.startsWith("Hello, world")) {
  log.log("TEST OK\n");
  mote.getSimulation().getGUI().doQuit(false);
}
