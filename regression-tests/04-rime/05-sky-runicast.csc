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
    <randomseed>1</randomseed>
    <motedelay_us>10000000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>0.0</interference_range>
      <success_ratio_tx>0.9</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/rime/example-runicast.c</source>
      <commands EXPORT="discard">make clean TARGET=sky
make example-runicast.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/rime/example-runicast.sky</firmware>
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
        <x>3.0783332685337617</x>
        <y>38.39795740836801</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>1.1986251808192212</x>
        <y>53.65838347315817</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>34.432838059195255</x>
        <y>38.26541658684913</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>150.85510197745134</x>
        <y>141.37553211643905</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>265</width>
    <z>4</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter>received</filter>
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>539</width>
    <z>0</z>
    <height>319</height>
    <location_x>0</location_x>
    <location_y>325</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>0.6259856679816412 0.0 0.0 0.6259856679816412 77.4082730178659 -21.226329635441804</viewport>
    </plugin_config>
    <width>263</width>
    <z>2</z>
    <height>125</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
      <formatted_time />
      <showdups>false</showdups>
      <hidenodests>false</hidenodests>
    </plugin_config>
    <width>276</width>
    <z>1</z>
    <height>324</height>
    <location_x>264</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(240000);

nr_recv = 0;
nr_timedout = 0;
nr_sent = 0;

while (nr_sent &lt; 10) {

/* Count received packets */
if (msg.contains('received') &amp;&amp; !msg.contains('DUPLICATE')) {
  nr_recv++;
  log.log("RECV=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
}

/* Count timed out packets */
else if (msg.contains('timed out')) {
  nr_timedout++;
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", TIMEDOUT=" + nr_timedout + "\n");
}

/* Count sent packets */
else if (msg.contains('sent to')) {
  nr_sent++;
  log.log("recv=" + nr_recv + ", SENT=" + nr_sent + ", timedout=" + nr_timedout + "\n");
}

YIELD();
}

/* Make sure received counter matches sent counter */
if (nr_recv &lt; nr_sent-3) {
  log.log("Error: Received &lt;&lt; Sent!\n");
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testFailed();
}

else if (nr_recv &gt; nr_sent+3) {
  log.log("Error: Received &gt;&gt; Sent!\n");
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testFailed();
}

/* Make sure some packets timed out (all from node 4) */
else if (nr_timedout &lt; 2) {
  log.log("Error: Too few packets timed out!\n");
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testFailed();
}

else {
  log.log("recv=" + nr_recv + ", sent=" + nr_sent + ", timedout=" + nr_timedout + "\n");
  log.testOK();
}</script>
      <active>true</active>
    </plugin_config>
    <width>503</width>
    <z>3</z>
    <height>643</height>
    <location_x>539</location_x>
    <location_y>1</location_y>
  </plugin>
</simconf>

