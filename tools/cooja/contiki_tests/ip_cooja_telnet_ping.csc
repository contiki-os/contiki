<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>1</delaytime>
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
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype511</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>../../../examples/telnet-server/telnet-server.c</contikiapp>
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
      <commstack>uIPv4</commstack>
    </motetype>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype511</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>44.40540999693696</x>
        <y>81.18952332401878</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>1</z>
    <height>200</height>
    <location_x>2</location_x>
    <location_y>2</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Addresses: IP or Rime</skin>
      <skin>Mote IDs</skin>
      <skin>Radio environment (UDGM)</skin>
      <skin>LEDs</skin>
    </plugin_config>
    <width>247</width>
    <z>3</z>
    <height>110</height>
    <location_x>3</location_x>
    <location_y>203</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(30000);

/* conf */
nrReplies = 0;
ipAddress = "172.16.0.1";
osName = java.lang.System.getProperty("os.name").toLowerCase();
if (osName.startsWith("win")) {
  pingCmd = "ping -n 10 " + ipAddress;
} else {
  pingCmd = "ping -c 10 " + ipAddress;
}
replyMsg = "from " + ipAddress;

/* mote startup */
WAIT_UNTIL(msg.contains('Telnet server'));

/* override simulation delay, test times out is too fast otherwise */
mote.getSimulation().setDelayTime(1);

GENERATE_MSG(5000, "continue");
WAIT_UNTIL(msg.equals("continue"));
log.log("Starting ping process\n");

/* start ping process */
var runnableObj = new Object();
runnableObj.run = function() {
  pingProcess  = new java.lang.Runtime.getRuntime().exec(pingCmd);
  log.log("cmd&gt; " + pingCmd + "\n");

  stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(pingProcess.getInputStream()));
  while ((line = stdIn.readLine()) != null) {
    log.log("&gt; " + line + "\n");
    if (line.contains(replyMsg)) {
      nrReplies++;
      //log.log("reply #" + nrReplies + "\n");
    }
  }
  pingProcess.destroy();

  if (nrReplies &gt; 5) {
    log.testOK(); /* Report test success and quit */
  } else {
    log.log("Only " + nrReplies + "/10 ping replies was received\n");
    log.testFailed();
  }
}
var thread = new java.lang.Thread(new java.lang.Runnable(runnableObj));
thread.start();</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>700</height>
    <location_x>251</location_x>
    <location_y>1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.NativeIPGateway
    <mote_arg>0</mote_arg>
    <plugin_config>
      <network_interface>\Device\NPF_{6AE59250-8082-40D4-B2F4-89EC57DD3321}</network_interface>
      <register_routes>true</register_routes>
    </plugin_config>
    <width>388</width>
    <z>2</z>
    <height>331</height>
    <location_x>0</location_x>
    <location_y>313</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

