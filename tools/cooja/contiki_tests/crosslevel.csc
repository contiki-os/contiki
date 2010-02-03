<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>5000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>50.0</interference_range>
      <success_ratio_tx>0.5</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>[CONTIKI_DIR]/examples/rime/example-abc.c</source>
      <commands>make example-abc.sky TARGET=sky DEFINES=MAC_DRIVER=nullmac_driver</commands>
      <firmware>[CONTIKI_DIR]/examples/rime/example-abc.sky</firmware>
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
    <motetype>
      se.sics.cooja.mspmote.ESBMoteType
      <identifier>esb1</identifier>
      <description>ESB Mote Type #1</description>
      <source>[CONTIKI_DIR]/examples/rime/example-abc.c</source>
      <commands>make example-abc.esb TARGET=esb DEFINES=MAC_DRIVER=nullmac_driver</commands>
      <firmware>[CONTIKI_DIR]/examples/rime/example-abc.esb</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.ESBLED</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.ESBButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.TR1001Radio</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
    </motetype>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype686</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>[CONTIKI_DIR]/examples/rime/example-abc.c</contikiapp>
      <commands>make example-abc.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <symbols>false</symbols>
      <commstack>Rime</commstack>
    </motetype>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>-2.7427179779871587</x>
        <y>-17.498426286221605</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>mtype686</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>-1.3167654732910574</x>
        <y>21.002291340573123</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>mtype686</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>99.92586236013211</x>
        <y>-18.924378790917704</y>
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
        <x>102.77776736952431</x>
        <y>18.150386331180922</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.ESBMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>198.31658518416305</x>
        <y>-18.924378790917704</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.ESBMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>196.890632679467</x>
        <y>19.57633883587702</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>6</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>295.28135550349793</x>
        <y>-18.924378790917704</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>7</id>
      </interface_config>
      <motetype_identifier>mtype686</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>293.8554029988019</x>
        <y>16.72443382648482</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>8</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>399.0339262783647</x>
        <y>-17.46710863775396</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>mtype686</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.ESBMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>401.18433133198937</x>
        <y>15.878266080338506</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>10</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>479.0051389972142</x>
        <y>-15.553186635152876</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>11</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      se.sics.cooja.mspmote.ESBMote
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>478.3433974354351</x>
        <y>18.789928952166647</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>12</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>265</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>460</width>
    <z>2</z>
    <height>178</height>
    <location_x>400</location_x>
    <location_y>568</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <viewport>0.6868927097814259 0.0 0.0 0.6868927097814259 19.429407529520386 57.09333478544578</viewport>
    </plugin_config>
    <width>401</width>
    <z>4</z>
    <height>177</height>
    <location_x>0</location_x>
    <location_y>569</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <width>265</width>
    <z>5</z>
    <height>371</height>
    <location_x>0</location_x>
    <location_y>199</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(120000);

var nr_packets = new Array();
for (i=1; i &lt;= 12; i++) {
  nr_packets[i] = 0;
}

while (true) {

  /* Listen for receive notifications */
  if (msg.contains("abc message received 'Hello'")) {

    /* Log receiving node */
    nr_packets[id] ++;
    log.log("Node " + id + " received message: " + nr_packets[id] + "\n");

    log.log("TEST STATUS: ");
    log.log(" C-C " + nr_packets[1] + "-" + nr_packets[2]);
    log.log(" S-S " + nr_packets[3] + "-" + nr_packets[4]);
    log.log(" E-E " + nr_packets[5] + "-" + nr_packets[6]);
    log.log(" C-S " + nr_packets[7] + "-" + nr_packets[8]);
    log.log(" C-E " + nr_packets[9] + "-" + nr_packets[10]);
    log.log(" S-E " + nr_packets[11] + "-" + nr_packets[12]);
    log.log("\n");
  }

  /* Did all nodes (1-12) receive at least three messages? */
  for (i = 1; i &lt;= 12; i++) {
    if (nr_packets[i] &lt; 3) break;
  }
  if (i == 13) {
    log.testOK();
  }

  YIELD();
}</script>
      <active>true</active>
    </plugin_config>
    <width>596</width>
    <z>0</z>
    <height>569</height>
    <location_x>264</location_x>
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
      <mote>4</mote>
      <mote>5</mote>
      <mote>6</mote>
      <mote>7</mote>
      <mote>8</mote>
      <mote>9</mote>
      <mote>10</mote>
      <mote>11</mote>
      <showRadioRXTX />
      <showRadioHW />
      <split>109</split>
      <zoom>12</zoom>
    </plugin_config>
    <width>862</width>
    <z>1</z>
    <height>219</height>
    <location_x>-1</location_x>
    <location_y>746</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

