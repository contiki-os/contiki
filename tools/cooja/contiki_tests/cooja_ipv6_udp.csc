<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project>[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
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
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype783</identifier>
      <description>Receiver</description>
      <contikiapp>[CONTIKI_DIR]/examples/udp-receiver-ipv6/example-udp-receiver.c</contikiapp>
      <commands>make example-udp-receiver.cooja TARGET=cooja DEFINES=UDP_ADDR_A=0xfe80,UDP_ADDR_B=0,UDP_ADDR_C=0,UDP_ADDR_D=0,UDP_ADDR_E=0x2,UDP_ADDR_F=0x2ff,UDP_ADDR_G=0xfe02,UDP_ADDR_H=0x0202</commands>
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
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype512</identifier>
      <description>Sender</description>
      <contikiapp>[CONTIKI_DIR]/examples/udp-sender-ipv6/example-udp-sender.c</contikiapp>
      <commands>make example-udp-sender.cooja TARGET=cooja DEFINES=UDP_ADDR_A=0xfe80,UDP_ADDR_B=0,UDP_ADDR_C=0,UDP_ADDR_D=0,UDP_ADDR_E=0x301,UDP_ADDR_F=0x1ff,UDP_ADDR_G=0xfe01,UDP_ADDR_H=0x101</commands>
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
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>98.76075470611741</x>
        <y>30.469519951198897</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>mtype783</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>58.59043340181549</x>
        <y>22.264557758786697</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>mtype512</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>2</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter>ID:1</filter>
    </plugin_config>
    <width>851</width>
    <z>1</z>
    <height>187</height>
    <location_x>1</location_x>
    <location_y>521</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.TrafficVisualizerSkin</skin>
      <viewport>2.565713585691764 0.0 0.0 2.565713585691764 -91.30090099174814 -28.413835696190525</viewport>
    </plugin_config>
    <width>246</width>
    <z>3</z>
    <height>121</height>
    <location_x>1</location_x>
    <location_y>201</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>150</split>
    </plugin_config>
    <width>246</width>
    <z>4</z>
    <height>198</height>
    <location_x>0</location_x>
    <location_y>323</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(100000, log.log("last msg: " + msg + "\n")); /* print last msg at timeout */

WAIT_UNTIL(msg.contains("Created connection"));
YIELD_THEN_WAIT_UNTIL(msg.contains("Created connection"));

log.log("Both nodes booted\n");

count = 0;
while (count++ &lt; 5) {
  /* Message from sender process to receiver process */
  YIELD_THEN_WAIT_UNTIL(msg.contains("Sender sending"));
  YIELD_THEN_WAIT_UNTIL(msg.contains("Receiver received"));
  log.log(count + ": Sender -&gt; Receiver OK\n");

  /* Message from receiver process to sender process */
  YIELD_THEN_WAIT_UNTIL(msg.contains("Receiver sending"));
  YIELD_THEN_WAIT_UNTIL(msg.contains("Sender received"));
  log.log(count + ": Receiver -&gt; Sender OK\n");
}

log.testOK(); /* Report test success and quit */</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>520</height>
    <location_x>250</location_x>
    <location_y>-1</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

