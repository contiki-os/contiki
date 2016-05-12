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
    <speedlimit>1.0</speedlimit>
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
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>mtype503</identifier>
      <description>ip64-router/ip64-router.c TARGET=cooja-ip64</description>
      <source>[CONTIKI_DIR]/examples/ip64-router/ip64-router.c</source>
      <commands>make ip64-router.cooja-ip64 TARGET=cooja-ip64</commands>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>mtype555</identifier>
      <description>examples/http-socket/http-example.c</description>
      <source>[CONTIKI_DIR]/examples/http-socket/http-example.c</source>
      <commands>make http-example.cooja TARGET=cooja</commands>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>55.719691912311305</x>
        <y>37.8697579181178</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype503</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>65.60514720922419</x>
        <y>33.88871867406431</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype555</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>280</width>
    <z>1</z>
    <height>160</height>
    <location_x>606</location_x>
    <location_y>15</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.LEDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>8.230641272440463 0.0 0.0 8.230641272440463 -395.6087959411366 -239.69239249818943</viewport>
    </plugin_config>
    <width>219</width>
    <z>3</z>
    <height>171</height>
    <location_x>29</location_x>
    <location_y>27</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>888</width>
    <z>2</z>
    <height>603</height>
    <location_x>34</location_x>
    <location_y>307</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>/* conf */&#xD;
var travis = java.lang.System.getenv().get("TRAVIS");&#xD;
if (travis == null) {&#xD;
    /* Instant Contiki */&#xD;
    CMD_TUNNEL = "echo '-vj' &gt; ~/.slirprc &amp;&amp; make Connect.class &amp;&amp; java Connect 'nc localhost 60001' 'script -t -f -c slirp'";&#xD;
    CMD_PING = "ping -c 5 8.8.8.8";&#xD;
    CMD_DIR = "../../tools/wpcapslip";&#xD;
} else {&#xD;
    /* Travis */&#xD;
    CMD_TUNNEL = "cd $TRAVIS_BUILD_DIR/tools/wpcapslip &amp;&amp; sudo apt-get install slirp &amp;&amp; echo '-vj' &gt; ~/.slirprc &amp;&amp; make Connect.class &amp;&amp; java Connect 'nc localhost 60001' 'script -t -f -c slirp'";&#xD;
    CMD_PING = "ping -c 5 8.8.8.8";&#xD;
    CMD_DIR = ".";&#xD;
}&#xD;
&#xD;
/* delay */&#xD;
GENERATE_MSG(1000, "continue");&#xD;
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));&#xD;
&#xD;
/* realtime speed */&#xD;
sim.setSpeedLimit(1.0);&#xD;
&#xD;
/* tunnel interface */&#xD;
log.log("opening tunnel interface: " + CMD_TUNNEL + "\n");&#xD;
launcher = new java.lang.ProcessBuilder["(java.lang.String[])"](['sh','-c',CMD_TUNNEL]);&#xD;
launcher.directory(new java.io.File(CMD_DIR));&#xD;
launcher.redirectErrorStream(true);&#xD;
tunProcess = launcher.start();&#xD;
tunRunnable = new Object();&#xD;
tunRunnable.run = function() {&#xD;
  var stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(tunProcess.getInputStream()));&#xD;
  while ((line = stdIn.readLine()) != null) {&#xD;
    if (line != null &amp;&amp; !line.trim().equals("")) {&#xD;
      log.log("TUN&gt; " + line + "\n");&#xD;
    }&#xD;
  }&#xD;
  tunProcess.destroy();&#xD;
}&#xD;
new java.lang.Thread(new java.lang.Runnable(tunRunnable)).start();&#xD;
&#xD;
GENERATE_MSG(2000, "continue");&#xD;
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));&#xD;
&#xD;
/* ping */&#xD;
log.log("pinging: " + CMD_PING + "\n");&#xD;
launcher = new java.lang.ProcessBuilder["(java.lang.String[])"](['sh','-c',CMD_PING]);&#xD;
launcher.directory(new java.io.File(CMD_DIR));&#xD;
launcher.redirectErrorStream(true);&#xD;
tunProcess = launcher.start();&#xD;
tunRunnable = new Object();&#xD;
tunRunnable.run = function() {&#xD;
  var stdIn = new java.io.BufferedReader(new java.io.InputStreamReader(tunProcess.getInputStream()));&#xD;
  while ((line = stdIn.readLine()) != null) {&#xD;
    if (line != null &amp;&amp; !line.trim().equals("")) {&#xD;
      log.log("PING&gt; " + line + "\n");&#xD;
    }&#xD;
  }&#xD;
  tunProcess.destroy();&#xD;
}&#xD;
new java.lang.Thread(new java.lang.Runnable(tunRunnable)).start();&#xD;
&#xD;
GENERATE_MSG(2000000, "stop");&#xD;
while(!msg.equals("stop")) {&#xD;
    if (!msg.startsWith("#L")) {&#xD;
        log.log(mote + ": " + msg + "\n");&#xD;
    }&#xD;
    if (id == 2 &amp;&amp; msg.startsWith("HTTP socket closed")) {&#xD;
        if (msg.split(' ')[3] &gt; 0) {&#xD;
            log.testOK();&#xD;
        } else {&#xD;
            log.testFailed();    &#xD;
        }&#xD;
    }&#xD;
    YIELD();&#xD;
}&#xD;
log.testFailed();</script>
      <active>true</active>
    </plugin_config>
    <width>960</width>
    <z>0</z>
    <height>682</height>
    <location_x>528</location_x>
    <location_y>192</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.serialsocket.SerialSocketServer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <port>60001</port>
      <bound>true</bound>
    </plugin_config>
    <width>362</width>
    <z>4</z>
    <height>116</height>
    <location_x>234</location_x>
    <location_y>101</location_y>
  </plugin>
</simconf>

