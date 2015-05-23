<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>5000000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>50.0</interference_range>
      <success_ratio_tx>0.5</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/rime/example-abc.c</source>
      <commands EXPORT="discard">make clean TARGET=sky
make example-abc.sky TARGET=sky DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_noframer_driver,CC2420_CONF_AUTOACK=0</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/rime/example-abc.sky</firmware>
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
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
    </motetype>
    <motetype>
      org.contikios.cooja.mspmote.ESBMoteType
      <identifier>esb1</identifier>
      <description>ESB Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/rime/example-abc.c</source>
      <commands EXPORT="discard">make clean TARGET=esb
make example-abc.esb TARGET=esb DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_noframer_driver</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/rime/example-abc.esb</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.ESBLED</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.ESBButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.TR1001Radio</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
    </motetype>
    <motetype>
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>mtype343</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>[CONTIKI_DIR]/examples/rime/example-abc.c</contikiapp>
      <commands>make clean TARGET=cooja
make example-abc.cooja TARGET=cooja DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_noframer_driver</commands>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      org.contikios.cooja.motes.ImportAppMoteType
      <identifier>apptype1</identifier>
      <description>Application Mote Type #apptype1</description>
      <motepath>[CONTIKI_DIR]/tools/cooja/examples/appmote_rimeabc</motepath>
      <moteclass>RimeABC</moteclass>
    </motetype>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>199.49101781472754</x>
        <y>-95.62813700240686</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>6</id>
      </interface_config>
      <motetype_identifier>mtype343</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>98.81565434011203</x>
        <y>11.842061334190785</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>12</id>
      </interface_config>
      <motetype_identifier>mtype343</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-6.590009891330797</x>
        <y>-126.908292993505</y>
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
        <x>-2.909991539436897</x>
        <y>-95.62813700240686</y>
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
        <x>96.4505039616984</x>
        <y>-95.62813700240686</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-1.2018609210334443</x>
        <y>15.546413751270247</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>10</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-3.557828103468252</x>
        <y>112.32835838975996</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>16</id>
      </interface_config>
      <motetype_identifier>mtype343</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>197.65100863878064</x>
        <y>-130.5883113453989</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-3.0540371295731754</x>
        <y>-15.940581793905181</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>92.7704856098045</x>
        <y>-128.74830216945196</y>
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
        <x>98.81565434011203</x>
        <y>-17.792758002444913</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>11</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.motes.AbstractApplicationMoteType$SimpleMoteID
        <id>14</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>199.74274129227032</x>
        <y>4.324930898273846</y>
        <z>0.0</z>
      </interface_config>
      <motetype_identifier>apptype1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.motes.AbstractApplicationMoteType$SimpleMoteID
        <id>8</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>291.4419800282441</x>
        <y>-99.28851117819308</y>
        <z>0.0</z>
      </interface_config>
      <motetype_identifier>apptype1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.motes.AbstractApplicationMoteType$SimpleMoteID
        <id>19</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-5.495536655521387</x>
        <y>179.73520343003455</y>
        <z>0.0</z>
      </interface_config>
      <motetype_identifier>apptype1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.motes.AbstractApplicationMoteType$SimpleMoteID
        <id>20</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-3.8119538152129264</x>
        <y>201.62178035404452</y>
        <z>0.0</z>
      </interface_config>
      <motetype_identifier>apptype1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.motes.AbstractApplicationMoteType$SimpleMoteID
        <id>18</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>100.5701822839116</x>
        <y>109.02472413707922</y>
        <z>0.0</z>
      </interface_config>
      <motetype_identifier>apptype1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>289.58980381970434</x>
        <y>-130.77550672336852</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>7</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>196.5661698954619</x>
        <y>-22.675925974597682</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>13</id>
      </interface_config>
      <motetype_identifier>esb1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>100.5701822839116</x>
        <y>85.45456437276077</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>17</id>
      </interface_config>
      <motetype_identifier>mtype343</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-5.14611380187246</x>
        <y>83.73921581848423</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>15</id>
      </interface_config>
      <motetype_identifier>mtype343</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>265</width>
    <z>5</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter>ID:[12]</filter>
    </plugin_config>
    <width>953</width>
    <z>0</z>
    <height>184</height>
    <location_x>0</location_x>
    <location_y>524</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.RadioLogger
    <plugin_config>
      <split>110</split>
    </plugin_config>
    <width>695</width>
    <z>2</z>
    <height>152</height>
    <location_x>260</location_x>
    <location_y>369</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(120000);

NR_NODES = 20;
var nr_packets = new Array();
for (i=1; i &lt;= NR_NODES; i++) {
  nr_packets[i] = 0;
}

while (true) {

  /* Listen for receive notifications */
  if (msg.contains("abc message received 'Hello'")) {

    /* Log receiving node */
    nr_packets[id] ++;
    log.log("Node " + id + " received message: " + nr_packets[id] + "\n");

    log.log("TEST STATUS: ");
    log.log(" S-S " + nr_packets[1] + "-" + nr_packets[2]);
    log.log(" S-E " + nr_packets[3] + "-" + nr_packets[4]);
    log.log(" S-C " + nr_packets[5] + "-" + nr_packets[6]);
    log.log(" S-A " + nr_packets[7] + "-" + nr_packets[8]);
    log.log(" E-E " + nr_packets[9] + "-" + nr_packets[10]);
    log.log(" E-C " + nr_packets[11] + "-" + nr_packets[12]);
    log.log(" E-A " + nr_packets[13] + "-" + nr_packets[14]);
    log.log(" C-C " + nr_packets[15] + "-" + nr_packets[16]);
    log.log(" C-A " + nr_packets[17] + "-" + nr_packets[18]);
    log.log(" A-A " + nr_packets[19] + "-" + nr_packets[20]);
    log.log("\n");
  }

  /* Did all NR_NODES receive at least three messages? */
  for (i = 1; i &lt;= NR_NODES; i++) {
    if (nr_packets[i] &lt; 3) break;
  }
  if (i == NR_NODES+1) {
    log.testOK();
  }

  YIELD();
}</script>
      <active>true</active>
    </plugin_config>
    <width>692</width>
    <z>4</z>
    <height>367</height>
    <location_x>264</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.TimeLine
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
      <mote>12</mote>
      <mote>13</mote>
      <mote>14</mote>
      <mote>15</mote>
      <mote>16</mote>
      <mote>17</mote>
      <mote>18</mote>
      <mote>19</mote>
      <showRadioRXTX />
      <split>26</split>
      <zoomfactor>4.84134798580701</zoomfactor>
    </plugin_config>
    <width>956</width>
    <z>1</z>
    <height>276</height>
    <location_x>-1</location_x>
    <location_y>655</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <viewport>0.5766712294536613 0.0 0.0 0.5766712294536613 36.4700887534336 117.76935499005339</viewport>
    </plugin_config>
    <width>264</width>
    <z>3</z>
    <height>321</height>
    <location_x>0</location_x>
    <location_y>201</location_y>
  </plugin>
</simconf>

