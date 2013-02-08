<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
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
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/sky-shell-webserver/sky-shell-webserver.c</source>
      <commands EXPORT="discard">make clean TARGET=sky
make sky-shell-webserver.sky TARGET=sky DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/sky-shell-webserver/sky-shell-webserver.sky</firmware>
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
        <x>16.08449565170202</x>
        <y>26.59851562245121</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>42.81612608990057</x>
        <y>41.95359330906976</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>70.00470747576063</x>
        <y>60.92771287468932</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>99.0932428018975</x>
        <y>84.52903598560302</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>4</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>2.228374754476065 0.0 0.0 2.228374754476065 17.670417676175877 -52.816915265062036</viewport>
    </plugin_config>
    <width>305</width>
    <z>3</z>
    <height>201</height>
    <location_x>248</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>3</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>551</width>
    <z>1</z>
    <height>258</height>
    <location_x>2</location_x>
    <location_y>471</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>553</width>
    <z>2</z>
    <height>272</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(90000, log.log("last message: " + msg + "\n"));

mote1 = null;
mote4 = null;
nr_boot = 0;

/* Wait until all nodes have booted */
while (true) {
  if (msg.contains('Starting')) {
    nr_boot++;
    if (id == 1) mote1 = mote;
    if (id == 4) mote4 = mote;
  }
  if (nr_boot == 4) break;
  YIELD();
}
log.log("All motes booted\n");

/* Wait one second */
GENERATE_MSG(1000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* Create file on node 1 */
log.log("Node 1 creating data... ");
write(mote1, "echo FROM1 FROM1 | write node1file.txt\n");
log.log("done!\n");

/* Wait one second */
GENERATE_MSG(1000, "continue2");
WAIT_UNTIL(msg.equals("continue2"));

/* Download node1file.txt to node 4 */
log.log("Node 4 downloading from node 1... ");
write(mote4, "wget 172.16.1.0/node1file.txt | write node4file.txt\n");
WAIT_UNTIL(id == 4 &amp;&amp; msg.contains("FROM1"));
log.log("done!\n");

/* Wait one second */
GENERATE_MSG(1000, "continue3");
WAIT_UNTIL(msg.equals("continue3"));

/* Extend data */
log.log("Node 4 appending data... ");
write(mote4, "echo FROM4 FROM4 | append node4file.txt\n");
log.log("done!\n");

/* Wait one second */
GENERATE_MSG(1000, "continue4");
WAIT_UNTIL(msg.equals("continue4"));

/* Download node4file.txt at node 1 */
log.log("Node 1 downloading from node 4..");
write(mote1, "wget 172.16.4.0/node4file.txt\n");
WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("FROM1"));
WAIT_UNTIL(id == 1 &amp;&amp; msg.contains("FROM4"));
log.log("done!\n");

log.testOK();</script>
      <active>true</active>
    </plugin_config>
    <width>610</width>
    <z>0</z>
    <height>730</height>
    <location_x>552</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

