<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/serial_socket</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/collect-view</project>
  <simulation>
    <title>test</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>0</motedelay_us>
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
      <description>Sky Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/sky/test-coffee.c</source>
      <commands EXPORT="discard">make clean TARGET=sky
make test-coffee.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/sky/test-coffee.sky</firmware>
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
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>97.11078411573273</x>
        <y>56.790978919276014</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>0</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.LogVisualizerSkin</skin>
      <viewport>0.9090909090909091 0.0 0.0 0.9090909090909091 28.717468985697536 3.3718373461127142</viewport>
    </plugin_config>
    <width>246</width>
    <z>3</z>
    <height>170</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>846</width>
    <z>2</z>
    <height>209</height>
    <location_x>2</location_x>
    <location_y>370</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(80000);

fileOK = null;
gcOK = null;

while (fileOK == null || gcOK == null) {
  YIELD();

  if(msg.contains("ERROR")) {
    log.log(msg);
    log.testFailed();
  }

  if (msg.startsWith('Coffee test finished')) {
    log.testOK();
  }
}</script>
      <active>true</active>
    </plugin_config>
    <width>601</width>
    <z>1</z>
    <height>370</height>
    <location_x>247</location_x>
    <location_y>0</location_y>
  </plugin>
</simconf>

