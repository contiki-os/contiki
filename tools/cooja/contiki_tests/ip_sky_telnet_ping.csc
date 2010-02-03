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
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/sky-ip/sky-telnet-server.c</source>
      <commands>make clean TARGET=sky
make sky-telnet-server.sky TARGET=sky</commands>
      <firmware>../../../examples/sky-ip/sky-telnet-server.sky</firmware>
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
        <x>86.60672552430381</x>
        <y>5.401254433278691</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>4</z>
    <height>200</height>
    <location_x>2</location_x>
    <location_y>2</location_y>
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
    <height>332</height>
    <location_x>2</location_x>
    <location_y>401</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>385</width>
    <z>1</z>
    <height>239</height>
    <location_x>2</location_x>
    <location_y>201</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>LEDs</skin>
      <skin>Addresses: IP or Rime</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>140</width>
    <z>3</z>
    <height>201</height>
    <location_x>249</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(30000);

/* conf */
nrReplies = 0;
ipAddress = "172.16.1.0";
osName = java.lang.System.getProperty("os.name").toLowerCase();
if (osName.startsWith("win")) {
  pingOnceCmd = "ping -n 1 " + ipAddress;
  pingCmd = "ping -n 10 " + ipAddress;
} else {
  pingOnceCmd = "ping -c 1 " + ipAddress;
  pingCmd = "ping -c 10 " + ipAddress;
}
replyMsg = "from " + ipAddress;

/* wait for mote startup */
WAIT_UNTIL(msg.contains('Sky telnet process'));

/* make gateway */
pingOnceProcess  = new java.lang.Runtime.getRuntime().exec(pingOnceCmd);
GENERATE_MSG(5000, "continue");
WAIT_UNTIL(msg.equals("continue"));
log.log("cont\n");

/* override simulation delay, test will time out is too fast otherwise */
mote.getSimulation().setDelayTime(1);

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
    <width>603</width>
    <z>0</z>
    <height>732</height>
    <location_x>389</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

