TIMEOUT(300000);

/* conf */
ADDRESS_ROUTER = "aaaa::212:7401:1:101";
ADDRESS_SERVER = "aaaa::212:7402:2:202";
NR_PINGS = 10;
CMD_PING_PREFIX = "ping6 -c " + NR_PINGS + " -I tun0 ";
CMD_TUNNEL = "make connect-router-cooja";
CMD_WGET_ROUTER = "wget -t 1 -T 10 -O - http:\/\/[" + ADDRESS_ROUTER + "]";
CMD_WGET_SERVER = "wget -t 1 -T 10 -O - http:\/\/[" + ADDRESS_SERVER + "]";
COAP_SAMPLECLIENT_JAR = "/home/user/Californium/ExampleClient.jar";

/* delay */
msg = "";
GENERATE_MSG(5000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* override simulation speed limit to realtime */
sim.setSpeedLimit(1.0);

/* create tunnel interface */
log.log("create tunnel interface\n");
launcher = new java.lang.ProcessBuilder["(java.lang.String[])"](['sh','-c',CMD_TUNNEL]);
launcher.directory(new java.io.File("../../examples/er-rest-example"));
launcher.redirectErrorStream(true);
tunProcess = launcher.start();
tunRunnable = new Object();
tunRunnable.run = function() {
  var stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(tunProcess.getInputStream()));
  while ((line = stdIn.readLine()) != null) {
    if (line != null && !line.trim().equals("")) {
      log.log("TUN> " + line + "\n");
    }
  }
  tunProcess.destroy();
}
new java.lang.Thread(new java.lang.Runnable(tunRunnable)).start();


/* delay */
msg = "";
GENERATE_MSG(5000, "continue");
WAIT_UNTIL(msg.equals("continue"));


/* help function for executing commands */
processOutput = "";
executeAndWait = function(cmd) {
  log.log("EXEC>> " + cmd + "\n");

  launcher = new java.lang.ProcessBuilder["(java.lang.String[])"](['sh','-c',cmd]);
  launcher.redirectErrorStream(true);
  process = launcher.start();
  runnable = new Object();
  runnable.run = function() {
    var stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(process.getInputStream()));
    while ((line = stdIn.readLine()) != null) {
      if (line != null && !line.trim().equals("")) {
        log.log("PROCESS> " + line + "\n");
        processOutput += line + "\n";
      }
    }
    process.destroy();
  }
  t = new java.lang.Thread(new java.lang.Runnable(runnable));
  t.start();

  while (t.isAlive()) {
    msg = "";
    GENERATE_MSG(1000, "continue");
    WAIT_UNTIL(msg.equals("continue"));
  }
}

/* PERFORM TESTS */
testSummary = "";
testFailed = false;

/* ping tests */
testname = "ping border router";
log.log(testname + "\n");
testSummary += testname;
processOutput = "";
executeAndWait(CMD_PING_PREFIX + ADDRESS_ROUTER);
if (processOutput.indexOf("64 bytes from") != -1) {
  testSummary += ": OK\n";
} else {
  testSummary += ": FAILED\n";
  testFailed = true;
}

testname = "ping rest router";
log.log(testname + "\n");
testSummary += testname;
processOutput = "";
executeAndWait(CMD_PING_PREFIX + ADDRESS_SERVER);
if (processOutput.indexOf("64 bytes from") != -1) {
  testSummary += ": OK\n";
} else {
  testSummary += ": FAILED\n";
  testFailed = true;
}



/* wget tests */
testname = "wget border router";
log.log(testname + "\n");
testSummary += testname;
processOutput = "";
executeAndWait(CMD_WGET_ROUTER);
if (processOutput.indexOf("<title>ContikiRPL</title>") != -1) {
  testSummary += ": OK\n";
} else {
  testSummary += ": FAILED\n";
  testFailed = true;
}


/* coap tests: uses pre-downloaded dapaulid/JCoAP project's SampleClient.jar */
testname = "coap discovery";
log.log(testname + "\n");
testSummary += testname;
processOutput = "";
executeAndWait("java -jar " + COAP_SAMPLECLIENT_JAR + " DISCOVER coap:\/\/[" + ADDRESS_SERVER + "]");
if (processOutput.indexOf("+[.well-known/core]") != -1) {
  testSummary += ": OK\n";
} else {
  testSummary += ": FAILED\n";
  testFailed = true;
}

testname = "coap get hello";
log.log(testname + "\n");
testSummary += testname;
processOutput = "";
executeAndWait("java -jar " + COAP_SAMPLECLIENT_JAR + " GET coap:\/\/[" + ADDRESS_SERVER + "]/hello");
if (processOutput.indexOf("Hello World!") != -1) {
  testSummary += ": OK\n";
} else {
  testSummary += ": FAILED\n";
  testFailed = true;
}


log.log("\n\nTest finished. Summary:\n");
log.log(testSummary + "\n");

tunProcess.destroy();
if (testFailed) {
  log.testFailed();
} else {
  log.testOK();
}

