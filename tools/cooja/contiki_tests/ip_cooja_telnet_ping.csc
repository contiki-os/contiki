<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>My simulation</title>
    <delaytime>-2147483648</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>25.0</transmitting_range>
      <interference_range>0.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype696</identifier>
      <description>Telnet server example</description>
      <source>[CONTIKI_DIR]/examples/telnet-server/telnet-server.c</source>
      <commands>make telnet-server.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>0.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>mtype696</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>20.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>mtype696</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>40.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>mtype696</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>60.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>mtype696</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>80.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>mtype696</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>273</width>
    <z>5</z>
    <height>201</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.LEDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <viewport>3.9267917714869 0.0 0.0 3.9267917714869 52.173676425591204 89.17520347194386</viewport>
      <hidden />
    </plugin_config>
    <width>443</width>
    <z>2</z>
    <height>203</height>
    <location_x>271</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(900000);

/* conf */
NR_MOTES = 5;
NR_PINGS = 20;
NR_PINGS_MIN = 10;
osName = java.lang.System.getProperty("os.name").toLowerCase();
if (osName.startsWith("win")) {
  pingCmd = "ping -w 10000 -n " + NR_PINGS + " "; /* + ip */
} else {
  pingCmd = "ping -i 4 -c " + NR_PINGS + " "; /* + ip */
}
var nrReplies = new Array();
replyMsg = "from "; /* + ip */

/* wait for all motes to boot */
currentMote = 1;
while (currentMote &lt;= NR_MOTES) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('Telnet server'));
  log.log("Mote " + id + " booted\n");

  currentMote++;
}

GENERATE_MSG(1000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* override simulation speed limit to realtime */
mote.getSimulation().setSpeedLimit(1.0);

/* ping motes */
currentMote = 1;
while (currentMote &lt;= NR_MOTES) {
  nrReplies[currentMote] = 0;
  ip = mote.getSimulation().getMoteWithID(currentMote).getInterfaces().getIPAddress().getIPString();
  log.log("Pinging mote " + currentMote + "\n");
  motePingCmd = pingCmd + ip;
  log.log("&gt; " + motePingCmd + "\n");

  /* ping process */
  var pingRunnable = new Object();
  pingRunnable.run = function() {
    pingProcess  = new java.lang.Runtime.getRuntime().exec(motePingCmd);
    stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(pingProcess.getInputStream()));
    while ((line = stdIn.readLine()) != null) {
      if (line != null &amp;&amp; !line.trim().equals("")) {
        log.log("&lt; " + line + "\n");
      }
      if (line.contains(replyMsg + ip)) {
        nrReplies[currentMote]++;
      }
    }
    pingProcess.destroy();
  }
  var thread = new java.lang.Thread(new java.lang.Runnable(pingRunnable));
  thread.start();

  while (thread.isAlive()) {
    GENERATE_MSG(500, "continue");
    YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
  }

  currentMote++;
}

log.log("\nTEST SUMMARY:\n")
testOK = 1;
currentMote = 1;
while (currentMote &lt;= NR_MOTES) {
  log.log("Ping replies from mote " + currentMote + ": " + nrReplies[currentMote] + "/" + NR_PINGS + "\n")
  if (nrReplies[currentMote] &lt; NR_PINGS_MIN) {
    testOK = 0;
  }

  currentMote++;
}

if (testOK == 1) {
  log.testOK();
}
log.testFailed();</script>
      <active>true</active>
    </plugin_config>
    <width>565</width>
    <z>0</z>
    <height>517</height>
    <location_x>715</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <coloring />
    </plugin_config>
    <width>1280</width>
    <z>4</z>
    <height>310</height>
    <location_x>0</location_x>
    <location_y>519</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.NativeIPGateway
    <mote_arg>0</mote_arg>
    <plugin_config>
      <network_interface>lo</network_interface>
      <register_routes>true</register_routes>
    </plugin_config>
    <width>388</width>
    <z>3</z>
    <height>321</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
    </plugin_config>
    <width>324</width>
    <z>6</z>
    <height>321</height>
    <location_x>389</location_x>
    <location_y>200</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <mote>2</mote>
      <mote>3</mote>
      <mote>4</mote>
      <showRadioRXTX />
      <split>26</split>
      <zoomfactor>16662.274975375385</zoomfactor>
    </plugin_config>
    <width>1278</width>
    <z>1</z>
    <height>149</height>
    <location_x>1</location_x>
    <location_y>831</location_y>
  </plugin>
</simconf>

