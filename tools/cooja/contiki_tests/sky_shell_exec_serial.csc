<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #sky1</description>
      <source>../../../examples/sky-shell-exec/sky-shell-exec.c</source>
      <commands>echo COMPILING CONTIKI EXECUTABLE
make hello-world.ce TARGET=sky

echo COMPILING FIRMWARE WITH CORE
make sky-shell-exec.sky TARGET=sky
make sky-shell-exec.sky CORE=sky-shell-exec.sky TARGET=sky
make sky-shell-exec.sky CORE=sky-shell-exec.sky TARGET=sky</commands>
      <firmware>../../../examples/sky-shell-exec/sky-shell-exec.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyByteRadio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkySerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
    </motetype>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>56.18151486126417</x>
        <y>93.20004013966208</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>545</width>
    <z>1</z>
    <height>551</height>
    <location_x>3</location_x>
    <location_y>347</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.mspmote.plugins.MspStackWatcher
    <mote_arg>0</mote_arg>
    <width>566</width>
    <z>2</z>
    <height>201</height>
    <location_x>247</location_x>
    <location_y>-1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(50000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

helloworld = new java.io.File("../../../examples/sky-shell-exec/hello-world.ce");
log.log("Running test on:  " + helloworld.getAbsolutePath() + "\n");
if (!helloworld.exists()) {
  log.log("hello-world.ce does not exist\n");
  log.testFailed();
}

log.log("Waiting for node startup\n");
WAIT_UNTIL(msg.contains('1.0: Contiki&gt;'));

log.log("Preparing node for incoming data\n");
write(mote, "write hello-world.b64");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Uploading hello-world.b64:\n");
cmdarr = "sh -c xxx".split(" ");
cmdarr[2] = "../../tools/base64-encode &lt; hello-world.ce";
sb = new java.lang.StringBuilder();
process = new java.lang.Runtime.getRuntime().exec(cmdarr, null, helloworld.getParentFile());
stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(process.getInputStream()));
while ((line = stdIn.readLine()) != null) {
  write(mote, line + "\n");
  sb.append(line);

  YIELD();

  if (msg.startsWith(line)) {
    log.log("UPLOADED: " + msg + "\n");
  } else {
    log.log("Serial port upload failed:\n" + line + "\n" + msg + "\n");
    log.testFailed();
  }
}
process.destroy();
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
write(mote, "~K\n");
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Reading and verifying hello-world.b64 from flash\n");
write(mote, "read hello-world.b64");
YIELD();
while (!msg.contains("Contiki&gt;")) {
  if (!sb.toString().startsWith(msg.trim())) {
    log.log("Verify failed: '" + msg + "' does not start '" + sb.toString() + "'\n");
    log.testFailed();
  } else {
    log.log("VERIFIED: " + msg + "\n");
    sb.replace(0, msg.length()-1, new String(""));
  }

  YIELD();
}
if (sb.length()&gt;0) {
  log.log("Verify failed: remaining buffer: " + sb);
  log.testFailed();
}
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));


log.log("Converting base64 to binary hello-world.ce\n");
write(mote, "read hello-world.b64 | dec64 | write hello-world.ce | null");
GENERATE_MSG(3000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Listing filesystem to make sure hello-world.ce exists\n");
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
write(mote, "ls\n");
WAIT_UNTIL(msg.contains("hello-world.ce"));

log.log("Starting hello world\n");
GENERATE_MSG(500, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
write(mote, "exec hello-world.ce\n");
while (true) {
  YIELD();
  if (msg.contains("OK")) {
    log.log("&gt; ELF loader returned OK\n");
  }
  if (msg.contains("Hello, world")) {
    log.log("&gt; Hello world process started\n");
    log.testOK();
  }
  if (msg.contains("Symbol not found")) {
    log.log("&gt; ELF loader error: " + msg +"\n");
    log.testFailed();
  }
}
</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>700</height>
    <location_x>215</location_x>
    <location_y>199</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

