/*
 * Example showing how to reference and interact with surrounding
 * COOJA plugins from a test script.
 * The code looks up three common plugins and, if found, performs some
 * simple plugin-specific task.
 */

/* Started plugins are available from the GUI object */

TIMEOUT(60000);

counter=0;
plugins=0;

timeout_function = function my_fun() {
 log.log("Script timed out.\n");
 log.log(plugins + " plugins were referenced\n");
}

while (counter<10) {
  counter++;
  
  GENERATE_MSG(1000, "wait");
  YIELD_THEN_WAIT_UNTIL(msg.equals("wait"));

  /* Toggle Log Listener filter */
  plugin = mote.getSimulation().getCooja().getStartedPlugin("org.contikios.cooja.plugins.LogListener");
  if (plugin != null) {
    plugins++;
    log.log("LogListener: Setting filter: " + plugin.getFilter() + "\n");
    if (plugin.getFilter() == null || !plugin.getFilter().equals("Contiki")) {
      plugin.setFilter("Contiki");
    } else {
      plugin.setFilter("MAC");
    }
  }

  GENERATE_MSG(1000, "wait");
  YIELD_THEN_WAIT_UNTIL(msg.equals("wait"));

  /* Extract PowerTracker statistics */
  plugin = mote.getSimulation().getCooja().getStartedPlugin("PowerTracker");
  if (plugin != null) {
    plugins++;
    stats = plugin.radioStatistics();
    if (stats.length() > 40) {
      /* Stripping */
      stats = stats.substring(0, 40) + "...";
    }
    log.log("PowerTracker: Extracted statistics:\n" + stats + "\n");
  } else {
    log.log("No PowerTracker plugin\n");
  }

  GENERATE_MSG(1000, "wait");
  YIELD_THEN_WAIT_UNTIL(msg.equals("wait"));

  /* Select time in Radio Logger */
  plugin = mote.getSimulation().getCooja().getStartedPlugin("org.contikios.cooja.plugins.RadioLogger");
  if (plugin != null) {
    plugins++;
    log.log("RadioLogger: Showing logged radio packet at mid simulation\n");
    plugin.trySelectTime(time/2);
  }

}
