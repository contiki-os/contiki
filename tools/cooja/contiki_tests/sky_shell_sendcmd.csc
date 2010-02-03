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
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
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
      <commands>make clean TARGET=sky
make sky-checkpoint.sky TARGET=sky</commands>
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
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
    </motetype>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>85.95630230780736</x>
        <y>-3.1198604890271326</y>
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
        <x>49.907666469432186</x>
        <y>13.102030707681678</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>63.205651488651284</x>
        <y>29.26179327208891</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>30.60420340759257</x>
        <y>44.54754287360952</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>2</z>
    <height>200</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.LogVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <viewport>2.126765951505188 0.0 0.0 2.126765951505188 -7.584821059232853 59.46165934250099</viewport>
    </plugin_config>
    <width>246</width>
    <z>6</z>
    <height>243</height>
    <location_x>1</location_x>
    <location_y>202</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter>Sending</filter>
    </plugin_config>
    <width>1022</width>
    <z>1</z>
    <height>209</height>
    <location_x>0</location_x>
    <location_y>446</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(150000);

/* Wait for nodes to boot */
mote1 = null;
while (mote1 == null) {
  if (id == 1) {
    mote1 = mote;
  }
  YIELD();
}
GENERATE_MSG(20000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Send command: mote 1 -&gt; mote 2*/
command = "sendcmd 2.0 echo hello";
log.log("mote1&gt; " + command + "\n");
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 2 &amp;&amp; msg.contains("hello"));
log.log("mote2: " + msg + "\n");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Send command: mote 1 -&gt; mote 2*/
command = "sendcmd 2.0 {echo hello again}";
log.log("mote1&gt; " + command + "\n");
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 2 &amp;&amp; msg.contains("hello again"));
log.log("mote2: " + msg + "\n");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

/* Send command: mote 1 -&gt; mote 2 -&gt; mote 3 -&gt; mote 4 */
command = "sendcmd 2.0 {sendcmd 3.0 {sendcmd 4.0 echo multihop}}";
log.log("mote1&gt; " + command + "\n");
write(mote1, command);
YIELD_THEN_WAIT_UNTIL(id == 4 &amp;&amp; msg.contains("multihop"));
log.log("mote4: " + msg + "\n");
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));

log.log("Test finished at time: " + mote.getSimulation().getSimulationTime() + "\n");
log.testOK(); /* Report test success and quit */</script>
      <active>true</active>
    </plugin_config>
    <width>775</width>
    <z>3</z>
    <height>445</height>
    <location_x>246</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <mote>2</mote>
      <mote>3</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <split>109</split>
      <zoom>12</zoom>
    </plugin_config>
    <width>1021</width>
    <z>0</z>
    <height>150</height>
    <location_x>1</location_x>
    <location_y>655</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

