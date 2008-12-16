if (!msg.contains('Sky telnet process')) {
  return;
}

ipAddress = "172.16.1.0";
osName = java.lang.System.getProperty("os.name").toLowerCase();
if (osName.startsWith("win")) {
  pingCmd = "ping -n 10 " + ipAddress;
} else {
  pingCmd = "ping -c 10 " + ipAddress;
}
replyMsg = "from " + ipAddress;

ok = global.get('started_ping');
if (ok == true) {
  //log.log("ping already started\n");
  return;
}
global.put('started_ping', true);
global.put('replies', "");

var runnableObj = new Object();
runnableObj.run = function() {
  pingProcess  = new java.lang.Runtime.getRuntime().exec(pingCmd);
  log.log("cmd> " + pingCmd + "\n");

  stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(pingProcess.getInputStream()));
  while ((line = stdIn.readLine()) != null) {
    log.log("> " + line + "\n");
    if (line.contains(replyMsg)) {
      global.put('replies', global.get('replies') + "1");
      //log.log("reply #" + global.get('replies').length() + "\n");
    }
  }
  pingProcess.destroy();

  if (global.get('replies').length() > 5) {
    log.testOK(); /* Report test success and quit */
  } else {
    log.log("Only " + global.get('replies').length() + "/10 ping replies was received\n");
    log.testFailed();
  }
}
var thread = new java.lang.Thread(new java.lang.Runnable(runnableObj));
thread.start();
