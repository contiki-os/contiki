
TIMEOUT(2400000); /* 40 minutes */

var NR_FEATHERS = mote.getSimulation().getMotesCount() - 1;

/* conf */
var travis = java.lang.System.getenv().get("TRAVIS");
if (travis == null) {
    /* Instant Contiki */
    CMD_TUNNEL = "echo '-vj' > ~/.slirprc && make Connect.class && java Connect 'nc localhost 60001' 'script -t -f -c slirp'";
    CMD_PING = "ping -c 5 8.8.8.8";
    CMD_DIR = "../../wpcapslip";
} else {
    /* Travis */
    CMD_TUNNEL = "cd $TRAVIS_BUILD_DIR/tools/wpcapslip && sudo apt-get install slirp && echo '-vj' > ~/.slirprc && make Connect.class && java Connect 'nc localhost 60001' 'script -t -f -c slirp'";
    CMD_PING = "ping -c 5 8.8.8.8";
    CMD_DIR = ".";
}

/* delay */
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* realtime speed */
sim.setSpeedLimit(2.0);

/* tunnel interface */
log.log("opening tunnel interface: " + CMD_TUNNEL + "\n");
launcher = new java.lang.ProcessBuilder["(java.lang.String[])"](['sh','-c',CMD_TUNNEL]);
launcher.directory(new java.io.File(CMD_DIR));
launcher.redirectErrorStream(true);
tunProcess = launcher.start();
tunRunnable = new Object();
tunRunnable.run = function() {
  var stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(tunProcess.getInputStream()));
  while ((line = stdIn.readLine()) != null) {
    if (line != null && !line.trim().equals("")) {
      //log.log("TUN> " + line + "\n");
    }
  }
  tunProcess.destroy();
}
new java.lang.Thread(new java.lang.Runnable(tunRunnable)).start();

GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* ping */
log.log("pinging: " + CMD_PING + "\n");
launcher = new java.lang.ProcessBuilder["(java.lang.String[])"](['sh','-c',CMD_PING]);
launcher.directory(new java.io.File(CMD_DIR));
launcher.redirectErrorStream(true);
tunProcess = launcher.start();
tunRunnable = new Object();
tunRunnable.run = function() {
  var stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(tunProcess.getInputStream()));
  while ((line = stdIn.readLine()) != null) {
    if (line != null && !line.trim().equals("")) {
      log.log("PING> " + line + "\n");
    }
  }
  tunProcess.destroy();
}
new java.lang.Thread(new java.lang.Runnable(tunRunnable)).start();

var completed = {};
while(Object.keys(completed).length < NR_FEATHERS) {
    if (!msg.startsWith("#L") && !msg.startsWith("E")) {
        //log.log(mote + ": " + msg + "\n");
    }
    if (id != 1 && msg.startsWith("HTTP socket closed")) {
        completed[id] = id;
        log.log("Data compelete " + id + ", heard " + Object.keys(completed).length + " in total\n");
    }
    YIELD();
}

log.testOK();
