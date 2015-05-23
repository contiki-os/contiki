<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/collect-view</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <simulation>
    <title>My simulation</title>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/multi-threading/multi-threading.c</source>
      <commands EXPORT="discard">make clean TARGET=sky
make multi-threading.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/multi-threading/multi-threading.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>83.20518861404864</x>
        <y>11.060511519885651</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>265</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <viewport>0.9090909090909091 0.0 0.0 0.9090909090909091 49.35891944177396 108.94498952737668</viewport>
    </plugin_config>
    <width>263</width>
    <z>2</z>
    <height>292</height>
    <location_x>1</location_x>
    <location_y>202</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
    </plugin_config>
    <width>865</width>
    <z>0</z>
    <height>209</height>
    <location_x>3</location_x>
    <location_y>701</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
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
  </plugin>
</simconf>

