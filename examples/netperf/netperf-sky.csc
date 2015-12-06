<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
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
      <description>netperf shell</description>
      <source>[CONTIKI_DIR]/examples/netperf/netperf-shell.c</source>
      <commands>make netperf-shell.sky TARGET=sky</commands>
      <firmware>[CONTIKI_DIR]/examples/netperf/netperf-shell.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkySerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
    </motetype>
    <mote>
      org.contikios.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>49.48292285385544</x>
        <y>97.67000744426045</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
    <mote>
      org.contikios.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>80.21380569499377</x>
        <y>98.51039574575084</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>290</width>
    <z>2</z>
    <height>172</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>1024</width>
    <z>0</z>
    <height>377</height>
    <location_x>0</location_x>
    <location_y>171</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <split>118</split>
      <zoom>9</zoom>
    </plugin_config>
    <width>1024</width>
    <z>1</z>
    <height>150</height>
    <location_x>0</location_x>
    <location_y>548</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(100000);
started = 0;
while(true) {
  YIELD(); /* wait for another mote output */
  log.log(time + " " + id + " " + msg + "\n");
  if(msg.startsWith("Done")) {
    log.testOK();
  }
  if(msg.startsWith("netperf control connection failed")) {
    log.testFailed();
  }
  if(id == 1 &amp;&amp; msg.startsWith("1.0: Contiki") &amp;&amp; started == 0) {
    write(mote, "netperf -bups 2.0 20\n"); /* Write to mote serial port */
    started = 1;
  }
}
//log.testOK(); /* Report test success and quit */
//log.testFailed(); /* Report test failure and quit */</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>-1</z>
    <height>476</height>
    <location_x>399</location_x>
    <location_y>154</location_y>
    <minimized>true</minimized>
  </plugin>
</simconf>

