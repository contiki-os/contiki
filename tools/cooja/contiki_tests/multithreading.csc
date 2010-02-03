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
      <description>Sky Mote Type #1</description>
      <source>../../../examples/multi-threading/multi-threading.c</source>
      <commands>make clean TARGET=sky
make multi-threading.sky TARGET=sky</commands>
      <firmware>../../../examples/multi-threading/multi-threading.sky</firmware>
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
        <x>83.20518861404864</x>
        <y>11.060511519885651</y>
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
    <width>265</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config />
    <width>263</width>
    <z>2</z>
    <height>292</height>
    <location_x>1</location_x>
    <location_y>202</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>865</width>
    <z>0</z>
    <height>209</height>
    <location_x>3</location_x>
    <location_y>701</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(60000, log.log("nrLowAlpha: " + nrLowAlpha + "\nnrHighAlpha: " + nrHighAlpha + "\nnrLowCount: " + nrLowCount + "\nnrHighCount: " + nrHighCount + "\n"));

lowAlpha = "BA";
highAlpha = "JIHGFEDCBA";
lowCount = "10";
highCount = "9876543210";

nrLowAlpha = 0;
nrHighAlpha = 0;
nrLowCount = 0;
nrHighCount = 0;

while (true) {

  if (msg.equals(lowCount)) {
    //log.log("Count is low\n");
    nrLowCount++;
  } else if (msg.equals(lowAlpha)) {
    //log.log("Alpha is low\n");
    nrLowAlpha++;
  } else if (msg.equals(highCount)) {
    //log.log("Count is high\n");
    nrHighCount++;
  } else if (msg.equals(highAlpha)) {
    //log.log("Alpha is high\n");
    nrHighAlpha++;
  }

  if (nrLowCount &gt;= 5 &amp;&amp;
      nrLowAlpha &gt;= 5 &amp;&amp;
      nrLowCount &gt;= 5 &amp;&amp;
      nrHighCount &gt;= 5 &amp;&amp;
      nrHighAlpha &gt;= 5) {
     log.testOK();
  }
  YIELD();
}</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>1</z>
    <height>700</height>
    <location_x>267</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

