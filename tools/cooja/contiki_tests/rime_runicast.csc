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
    <motedelay_us>10000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>0.0</interference_range>
      <success_ratio_tx>0.9</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/rime/example-runicast.c</source>
      <commands>make clean TARGET=sky
make example-runicast.sky TARGET=sky</commands>
      <firmware>../../../examples/rime/example-runicast.sky</firmware>
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
        <x>3.0783332685337617</x>
        <y>38.39795740836801</y>
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
        <x>1.1986251808192212</x>
        <y>53.65838347315817</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>34.432838059195255</x>
        <y>38.26541658684913</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>150.85510197745134</x>
        <y>141.37553211643905</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>265</width>
    <z>4</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter>received</filter>
    </plugin_config>
    <width>539</width>
    <z>0</z>
    <height>319</height>
    <location_x>0</location_x>
    <location_y>325</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>263</width>
    <z>2</z>
    <height>125</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <width>276</width>
    <z>1</z>
    <height>324</height>
    <location_x>264</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
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
    <minimized>false</minimized>
  </plugin>
</simconf>

