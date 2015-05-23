<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>My simulation</title>
    <delaytime>-2147483648</delaytime>
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
      <description>Telnet server example</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/sky-ip/sky-telnet-server.c</source>
      <commands EXPORT="discard">make sky-telnet-server.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/sky-ip/sky-telnet-server.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>86.60672552430381</x>
        <y>5.401254433278691</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspSerial
        <history></history>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>261</width>
    <z>3</z>
    <height>206</height>
    <location_x>7</location_x>
    <location_y>4</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.NativeIPGateway
    <mote_arg>0</mote_arg>
    <plugin_config>
      <network_interface>lo</network_interface>
      <register_routes>true</register_routes>
    </plugin_config>
    <width>388</width>
    <z>1</z>
    <height>350</height>
    <location_x>75</location_x>
    <location_y>599</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>550</width>
    <z>0</z>
    <height>357</height>
    <location_x>39</location_x>
    <location_y>223</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.LEDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.GridVisualizerSkin</skin>
      <viewport>1.473063973063973 0.0 0.0 1.473063973063973 21.959789842145003 40.69175483313324</viewport>
    </plugin_config>
    <width>313</width>
    <z>4</z>
    <height>169</height>
    <location_x>294</location_x>
    <location_y>27</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(180000);

/* conf */
nrReplies = 0;
ipAddress = "172.16.1.0";
osName = java.lang.System.getProperty("os.name").toLowerCase();
if (osName.startsWith("win")) {
  pingOnceCmd = "ping -n 1 " + ipAddress;
  pingCmd = "ping -w 10000 -n 10 " + ipAddress;
} else {
  pingOnceCmd = "ping -c 1 " + ipAddress;
  pingCmd = "ping -i 4 -c 10 " + ipAddress;
}
replyMsg = "from " + ipAddress;

/* wait for mote startup */
GENERATE_MSG(5000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* make gateway */
pingOnceProcess  = new java.lang.Runtime.getRuntime().exec(pingOnceCmd);
GENERATE_MSG(5000, "continue");
WAIT_UNTIL(msg.equals("continue"));

/* override simulation speed limit to realtime */
mote.getSimulation().setSpeedLimit(1.0);

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
    <width>589</width>
    <z>2</z>
    <height>900</height>
    <location_x>631</location_x>
    <location_y>38</location_y>
  </plugin>
</simconf>

