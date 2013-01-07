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
    <motedelay_us>5000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>0.95</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>[CONTIKI_DIR]/examples/rime/example-abc.c</source>
      <commands>make clean TARGET=sky
make example-abc.sky TARGET=sky</commands>
      <firmware>[CONTIKI_DIR]/examples/rime/example-abc.sky</firmware>
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
        <x>33.53152221759984</x>
        <y>15.078801911773077</y>
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
        <x>54.67966631314053</x>
        <y>31.0253850381183</y>
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
    <width>313</width>
    <z>3</z>
    <height>199</height>
    <location_x>-1</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>312</width>
    <z>2</z>
    <height>123</height>
    <location_x>0</location_x>
    <location_y>198</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>311</width>
    <z>0</z>
    <height>377</height>
    <location_x>1</location_x>
    <location_y>320</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(100000, log.log("Node 1: " + nr_packets[1] + ".\nNode 2: " + nr_packets[2] + ".\n"));

nr_packets = new Array();
nr_packets[1] = 0;
nr_packets[2] = 0;

while (true) {
  /* Only handle receive messages */
  YIELD_THEN_WAIT_UNTIL(msg.contains('received'));

  /* Count received packets */
  nr_packets[id]++;
  //log.log("Node " + id + " received " + nr_packets[id] + " messages\n");

  if (nr_packets[1] &gt;= 4 &amp;&amp; nr_packets[2] &gt;= 4) {
    log.log("Node 1: " + nr_packets[1] + ".\nNode 2: " + nr_packets[2] + ".\n");
    log.testOK(); /* Report test success */
  }

}</script>
      <active>true</active>
    </plugin_config>
    <width>601</width>
    <z>1</z>
    <height>697</height>
    <location_x>312</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

