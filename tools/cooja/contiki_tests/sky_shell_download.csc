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
      <transmitting_range>100.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>0.95</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #sky1</description>
      <source>../../../examples/sky-shell/sky-checkpoint.c</source>
      <commands>make sky-checkpoint.sky TARGET=sky</commands>
      <firmware>../../../examples/sky-shell/sky-checkpoint.sky</firmware>
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
        <x>-16.136777727943556</x>
        <y>85.85963282955879</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
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
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>268</width>
    <z>4</z>
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
    <width>359</width>
    <z>3</z>
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
    <width>384</width>
    <z>2</z>
    <height>488</height>
    <location_x>390</location_x>
    <location_y>199</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Radio environment (UDGM)</skin>
      <skin>Mote IDs</skin>
    </plugin_config>
    <width>126</width>
    <z>1</z>
    <height>198</height>
    <location_x>649</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(150000); /* print last msg at timeout */

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
fileID = 1;
command = "echo *mote1data" + fileID + " | write file" + fileID + ".txt\n";
log.log("mote1&gt; " + command);
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));

/* Download and append files */
while (fileID &lt; 20) {
  /* Mote 1 -&gt; Mote 2 */
  srcFile = "file" + fileID + ".txt";
  fileID++;
  dstFile = "file" + fileID + ".txt";
  command = "download 1.0 " + srcFile + " | write " + dstFile + " | null\n";
  log.log("mote2&gt; " + command);
  write(mote2, command);
  YIELD_THEN_WAIT_UNTIL(id == 2 &amp;&amp; msg.contains("Contiki&gt;"));
  command = "echo *mote2data" + fileID + " | append " + dstFile + " | null\n";
  log.log("mote2&gt; " + command);
  write(mote2, command);
  YIELD_THEN_WAIT_UNTIL(id == 2 &amp;&amp; msg.contains("Contiki&gt;"));

  /* Mote 2 -&gt; Mote 1 */
  srcFile = "file" + fileID + ".txt";
  fileID++;
  dstFile = "file" + fileID + ".txt";
  command = "download 2.0 " + srcFile + " | write " + dstFile + " | null\n";
  log.log("mote1&gt; " + command);
  write(mote1, command);
  YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));
  command = "echo *mote1data" + fileID + " | append " + dstFile + " | null\n";
  log.log("mote1&gt; " + command);
  write(mote1, command);
  YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));
}

/* List files, verify contents */
/* XXX Beware of strange line breaks! (generated by shell's read command) */
log.log("Locating file21.txt on mote 1\n");
write(mote1, "ls\n");
YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("file21.txt"));
if (!msg.contains("264 ")) {
  log.log("Bad file size, should be 264 bytes: " + msg + "\n");
  log.testFailed();
}
YIELD_THEN_WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("Contiki&gt;"));
log.log("Verifying file21.txt contents on mote 1\n");
write(mote1, "read file21.txt\n");
YIELD_THEN_WAIT_UNTIL(msg.contains("mote1data1"));
YIELD_THEN_WAIT_UNTIL(msg.contains("mote2data2"));
// ..
YIELD_THEN_WAIT_UNTIL(msg.contains("mote2data16") || msg.contains("mote2data18"));
YIELD_THEN_WAIT_UNTIL(msg.contains("mote1data19") || msg.contains("mote1data21"));

log.log("Test finished at time: " + mote.getSimulation().getSimulationTime() + "\n");

log.testOK(); /* Report test success and quit */</script>
      <active>true</active>
    </plugin_config>
    <width>573</width>
    <z>0</z>
    <height>688</height>
    <location_x>80</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

