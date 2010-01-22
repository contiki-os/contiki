<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>100.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>0.95</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #sky1</description>
      <source>[CONTIKI_DIR]/examples/sky-shell/sky-checkpoint.c</source>
      <commands>make sky-checkpoint.sky TARGET=sky</commands>
      <firmware>[CONTIKI_DIR]/examples/sky-shell/sky-checkpoint.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyByteRadio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
    </motetype>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>-16.136777727943556</x>
        <y>85.85963282955879</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>11.712359274594053</x>
        <y>84.42239042889724</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>268</width>
    <z>5</z>
    <height>201</height>
    <location_x>0</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>378</width>
    <z>2</z>
    <height>485</height>
    <location_x>2</location_x>
    <location_y>201</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>1</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>395</width>
    <z>3</z>
    <height>486</height>
    <location_x>379</location_x>
    <location_y>201</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.LogVisualizerSkin</skin>
      <viewport>10.082791180977951 0.0 0.0 10.082791180977951 266.248788516359 -780.5960558101059</viewport>
    </plugin_config>
    <width>505</width>
    <z>1</z>
    <height>200</height>
    <location_x>269</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(150000, log.log("last msg: " + msg + "\n"));

/* Wait for nodes to boot */
mote1 = null;
mote2 = null;
while (mote1 == null || mote2 == null) {
  if (id == 1) {
    mote1 = mote;
  }
  if (id == 2) {
    mote2 = mote;
  }
  YIELD();
}
GENERATE_MSG(2000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Generate initial file */
fileID = 10;
command = "echo *mote1data" + fileID + " | write file" + fileID + ".txt";
log.log("mote1&gt; " + command + "\n");
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));

/* Download and append files */
while (fileID &lt; 30) {
  /* Mote 1 -&gt; Mote 2 */
  srcFile = "file" + fileID + ".txt";
  fileID++;
  dstFile = "file" + fileID + ".txt";
  command = "download 1.0 " + srcFile + " | write " + dstFile + " | null";
  log.log("mote2&gt; " + command + "\n");
  write(mote2, command);
  YIELD_THEN_WAIT_UNTIL(id == 2 &amp;&amp; msg.contains("Contiki&gt;"));
  command = "echo *mote2data" + fileID + " | append " + dstFile + " | null";
  log.log("mote2&gt; " + command + "\n");
  write(mote2, command);
  YIELD_THEN_WAIT_UNTIL(id == 2 &amp;&amp; msg.contains("Contiki&gt;"));

  /* Mote 2 -&gt; Mote 1 */
  srcFile = "file" + fileID + ".txt";
  fileID++;
  dstFile = "file" + fileID + ".txt";
  command = "download 2.0 " + srcFile + " | write " + dstFile + " | null";
  log.log("mote1&gt; " + command + "\n");
  write(mote1, command);
  YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));
  command = "echo *mote1data" + fileID + " | append " + dstFile + " | null";
  log.log("mote1&gt; " + command + "\n");
  write(mote1, command);
  YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));
}

/* List files, verify contents */
log.log("Locating file30.txt on mote 1\n");
write(mote1, "ls");
YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("file30.txt"));
if (!msg.contains("273 ")) {
  log.log("Bad file size, should be 273 bytes: " + msg + "\n");
  log.testFailed();
}
YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));

/* Wait 3 seconds (3000ms) */
GENERATE_MSG(3000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

fileID = 10;
command = "read file30.txt 0 13";
log.log("mote1&gt; " + command + "\n");
write(mote1, command);
while (fileID &lt; 30) {
  YIELD_THEN_WAIT_UNTIL(msg.contains("mote1data" + fileID));
  fileID++;
  log.log("&gt; " + msg + "\n");
  YIELD_THEN_WAIT_UNTIL(msg.contains("mote2data" + fileID));
  fileID++;
  log.log("&gt; " + msg + "\n");
}

log.log("Test finished at time: " + mote.getSimulation().getSimulationTime() + "\n");

log.testOK(); /* Report test success and quit */</script>
      <active>true</active>
    </plugin_config>
    <width>729</width>
    <z>0</z>
    <height>811</height>
    <location_x>774</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <showRadioRXTX />
      <showRadioHW />
      <split>1</split>
      <zoom>12</zoom>
    </plugin_config>
    <width>772</width>
    <z>7</z>
    <height>125</height>
    <location_x>2</location_x>
    <location_y>687</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>1503</width>
    <z>4</z>
    <height>245</height>
    <location_x>0</location_x>
    <location_y>812</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

