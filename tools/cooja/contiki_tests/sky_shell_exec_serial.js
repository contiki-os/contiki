TIMEOUT(50000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

helloworld = new java.io.File("../../../examples/sky-shell-exec/hello-world.ce");
log.log("Running test on:  " + helloworld.getAbsolutePath() + "\n");
if (!helloworld.exists()) {
  log.log("hello-world.ce does not exist\n");
  log.testFailed();
}

log.log("Waiting for node startup\n");
WAIT_UNTIL(msg.contains('1.0: Contiki>'));

log.log("Preparing node for incoming data\n");
node.write("write hello-world.b64");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Uploading hello-world.b64:\n");
cmdarr = "sh -c xxx".split(" ");
cmdarr[2] = "../../tools/base64-encode < hello-world.ce";
process = new java.lang.Runtime.getRuntime().exec(cmdarr, null, helloworld.getParentFile());
stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(process.getInputStream()));
while ((line = stdIn.readLine()) != null) {
  node.write(line + "\n");

  YIELD();

  if (msg.startsWith(line)) {
    log.log("UPLOADED: " + line + "\n");
  } else {
    log.log("Serial port upload failed:\n" + line + "\n" + msg + "\n");
    log.testFailed();
  }
}
process.destroy();
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
node.write("~K\n");
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Converting base64 to binary hello-world.ce\n");
node.write("read hello-world.b64 | dec64 | write hello-world.ce | null");
GENERATE_MSG(3000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Listing filesystem to make sure hello-world.ce exists\n");
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
node.write("ls\n");
WAIT_UNTIL(msg.contains("hello-world.ce"));

log.log("Starting hello world\n");
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
node.write("exec hello-world.ce\n");
WAIT_UNTIL(msg.contains("OK"));
log.log("> ELF loader returned OK\n");
WAIT_UNTIL(msg.contains("Hello, world"));
log.log("> Hello world process started\n");

log.log("Finished!\n");
log.testOK();
