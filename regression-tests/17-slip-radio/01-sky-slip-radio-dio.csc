<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/collect-view</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <simulation>
    <title>slip radio 1</title>
    <randomseed>123456</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>15.0</transmitting_range>
      <interference_range>15.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>slip radio</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/ipv6/slip-radio/slip-radio.c</source>
      <commands EXPORT="discard">make slip-radio.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/ipv6/slip-radio/slip-radio.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky2</identifier>
      <description>wait-dag</description>
      <source EXPORT="discard">[CONTIKI_DIR]/regression-tests/17-slip-radio/code/wait-dag.c</source>
      <commands EXPORT="discard">make wait-dag.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/regression-tests/17-slip-radio/code/wait-dag.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>43.565500781711165</x>
        <y>14.697933087406794</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>53.849666651434326</x>
        <y>14.629826028666905</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky2</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>315</width>
    <z>1</z>
    <height>175</height>
    <location_x>433</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>14.682765905648006 0.0 0.0 14.682765905648006 -512.6620495401903 -96.80631081927221</viewport>
    </plugin_config>
    <width>432</width>
    <z>4</z>
    <height>291</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>758</width>
    <z>3</z>
    <height>289</height>
    <location_x>748</location_x>
    <location_y>159</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Notes
    <plugin_config>
      <notes>Slip-radio Tests, 01-sky-slip-radio-dio

Test that we can send a packet over a slip-radio.
In this basic test, we send a DIO from mote 1, and wait for "DAG Found" in mote 2.</notes>
      <decorations>true</decorations>
    </plugin_config>
    <width>928</width>
    <z>5</z>
    <height>159</height>
    <location_x>749</location_x>
    <location_y>-1</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(30000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

YIELD_THEN_WAIT_UNTIL(msg.contains("Slip Radio started..."));

//Wait for 5 sec
GENERATE_MSG(5000, "sleep"); 
YIELD_THEN_WAIT_UNTIL(msg.equals("sleep"));

//Configure 802.15.4 Channel (checks that we can detect commands on serial input)
write(sim.getMoteWithID(1), String.fromCharCode(0x21,67,26,192));
YIELD_THEN_WAIT_UNTIL(msg.contains("setting channel: 26"));

/*Send a DIO over SLIP with !S... ugly, but enough to test the radio
  note: 192 = SLIP_END */
write(sim.getMoteWithID(1), String.fromCharCode(192,0x21,0x53,0x1,0x3,0x9,0x0,0x5,0xa,0x0,0xd0,0xb,0x0,0x1,0x41,0xc8,0xd0,0xcd,0xab,0xff,0xff,0x1,0x1,0x1,0x0,0x1,0x74,0x12,0x0,0x7a,0x1b,0x3a,0x4,0x1,0x0,0xff,0xff,0x1,0x1,0x1,0x1a,0x9b,0x1,0x24,0x7c,0xa0,0x78,0x1,0x0,0x10,0xf0,0x0,0x0,0xfe,0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x2,0x12,0x74,0x1,0x0,0x1,0x1,0x1,0x2,0x6,0x7,0x4,0x0,0x2,0x0,0x0,0x4,0xe,0x0,0x8,0xc,0xa,0x7,0x0,0x1,0x0,0x0,0x1,0x0,0x1e,0x1,0x0,0x8,0x1e,0x40,0x40,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xaa,0xaa,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,192));

//Check that the other mote receives the DIO and joins the DAG
YIELD_THEN_WAIT_UNTIL(msg.equals("DAG Found"));

log.testOK();</script>
      <active>true</active>
    </plugin_config>
    <width>758</width>
    <z>2</z>
    <height>502</height>
    <location_x>749</location_x>
    <location_y>449</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
    </plugin_config>
    <width>746</width>
    <z>0</z>
    <height>657</height>
    <location_x>3</location_x>
    <location_y>294</location_y>
  </plugin>
</simconf>

