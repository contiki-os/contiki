<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
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
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/sky/test-coffee.c</source>
      <commands>make clean TARGET=sky
make test-coffee.sky TARGET=sky</commands>
      <firmware>../../../examples/sky/test-coffee.sky</firmware>
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
        <x>97.11078411573273</x>
        <y>56.790978919276014</y>
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
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Log output: printf()'s</skin>
    </plugin_config>
    <width>246</width>
    <z>2</z>
    <height>170</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>846</width>
    <z>1</z>
    <height>209</height>
    <location_x>2</location_x>
    <location_y>370</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(80000);

fileOK = null;
gcOK = null;

while (fileOK == null || gcOK == null) {
  YIELD();

  if (msg.startsWith('Coffee file test: 0')) {
    fileOK = true;
  } else if (msg.startsWith('Coffee file test')) {
    fileOK = false;
  } else if (msg.startsWith('Coffee garbage collection test: 0')) {
    gcOK = true;
  } else if (msg.startsWith('Coffee garbage collection test')) {
    gcOK = false;
  }

}

if (gcOK == false) {
  log.log("coffee garbage collection failed\n");
}
if (fileOK == false) {
  log.log("coffee file test failed\n");
}
if (!fileOK || !gcOK) {
  log.testFailed();
} else {
  log.testOK();
}</script>
      <active>true</active>
    </plugin_config>
    <width>601</width>
    <z>0</z>
    <height>370</height>
    <location_x>247</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

