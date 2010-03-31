<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
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
      <identifier>mtype611</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>[CONTIKI_DIR]/examples/telnet-server/telnet-server.c</contikiapp>
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
      <motetype_identifier>mtype611</motetype_identifier>
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
      <motetype_identifier>mtype611</motetype_identifier>
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
      <motetype_identifier>mtype611</motetype_identifier>
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
      <motetype_identifier>mtype611</motetype_identifier>
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
      <motetype_identifier>mtype611</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>5</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
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
      <viewport>6.0123843024302435 0.0 0.0 6.0123843024302435 81.22165451665171 95.03179722772279</viewport>
      <hidden />
    </plugin_config>
    <width>659</width>
    <z>4</z>
    <height>198</height>
    <location_x>246</location_x>
    <location_y>2</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(300000);

/* conf */
NR_MOTES = 5;
NR_PINGS = 20;
NR_PINGS_MIN = 10;
osName = java.lang.System.getProperty("os.name").toLowerCase();
if (osName.startsWith("win")) {
  pingCmd = "ping -n " + NR_PINGS + " "; /* + ip */
} else {
  pingCmd = "ping -c " + NR_PINGS + " "; /* + ip */
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

/* override simulation delay to realtime */
mote.getSimulation().setDelayTime(java.lang.Integer.MIN_VALUE);

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
    <width>600</width>
    <z>2</z>
    <height>702</height>
    <location_x>903</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>904</width>
    <z>1</z>
    <height>538</height>
    <location_x>0</location_x>
    <location_y>519</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.NativeIPGateway
    <mote_arg>0</mote_arg>
    <plugin_config>
      <network_interface>\Device\NPF_{8ABD0E53-3824-4541-B397-5FAF535206CF}</network_interface>
      <register_routes>true</register_routes>
    </plugin_config>
    <width>388</width>
    <z>6</z>
    <height>321</height>
    <location_x>1</location_x>
    <location_y>200</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
    </plugin_config>
    <width>517</width>
    <z>3</z>
    <height>321</height>
    <location_x>389</location_x>
    <location_y>200</location_y>
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
      <showRadioRXTX />
      <split>1</split>
      <zoom>11</zoom>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>355</height>
    <location_x>903</location_x>
    <location_y>702</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

