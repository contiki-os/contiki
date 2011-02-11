/*
 * Example showing how to reference and interact with surrounding
 * COOJA plugins from a test script.
 * The code looks up three common plugins and, if found, performs some
 * simple plugin-specific task.
 */

/* Started plugins are available from the GUI object */

while (true) {

  GENERATE_MSG(1000, "wait");
  YIELD_THEN_WAIT_UNTIL(msg.equals("wait"));

  /* Toggle Log Listener filter */
  plugin = mote.getSimulation().getGUI().getStartedPlugin("se.sics.cooja.plugins.LogListener");
  if (plugin != null) {
    log.log("LogListener: Setting filter: " + plugin.getFilter() + "\n");
    if (plugin.getFilter() == null || !plugin.getFilter().equals("Contiki")) {
      plugin.setFilter("Contiki");
    } else {
      plugin.setFilter("MAC");
    }
  }

  GENERATE_MSG(1000, "wait");
  YIELD_THEN_WAIT_UNTIL(msg.equals("wait"));

  /* Extract Timeline statistics */
  plugin = mote.getSimulation().getGUI().getStartedPlugin("se.sics.cooja.plugins.TimeLine");
  if (plugin != null) {
    stats = plugin.extractStatistics();
    if (stats.length() > 40) {
      /* Stripping */
      stats = stats.substring(0, 40) + "...";
    }
    log.log("Timeline: Extracted statistics:\n" + stats + "\n");
  }

  GENERATE_MSG(1000, "wait");
  YIELD_THEN_WAIT_UNTIL(msg.equals("wait"));

  /* Select time in Radio Logger */
  plugin = mote.getSimulation().getGUI().getStartedPlugin("se.sics.cooja.plugins.RadioLogger");
  if (plugin != null) {
    log.log("RadioLogger: Showing logged radio packet at mid simulation\n");
    plugin.trySelectTime(time/2);
  }

}
