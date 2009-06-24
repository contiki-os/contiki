<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
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
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype232</identifier>
      <description>Receiver</description>
      <contikiapp>../../../examples/udp-receiver-ipv6/example-udp-receiver.c</contikiapp>
      <commands>make example-udp-receiver.cooja TARGET=cooja DEFINES=WITH_UIP6=1,UIP_CONF_IPV6=1,UDP_ADDR_A=0xfe80,UDP_ADDR_B=0,UDP_ADDR_C=0,UDP_ADDR_D=0,UDP_ADDR_E=0x2,UDP_ADDR_F=0x2ff,UDP_ADDR_G=0xfe02,UDP_ADDR_H=0x0202</commands>
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
      <symbols>false</symbols>
      <commstack>uIPv6</commstack>
    </motetype>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype330</identifier>
      <description>Sender</description>
      <contikiapp>../../../examples/udp-sender-ipv6/example-udp-sender.c</contikiapp>
      <commands>make example-udp-sender.cooja TARGET=cooja DEFINES=WITH_UIP6=1,UIP_CONF_IPV6=1,UDP_ADDR_A=0xfe80,UDP_ADDR_B=0,UDP_ADDR_C=0,UDP_ADDR_D=0,UDP_ADDR_E=0x301,UDP_ADDR_F=0x1ff,UDP_ADDR_G=0xfe01,UDP_ADDR_H=0x101</commands>
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
      <symbols>false</symbols>
      <commstack>uIPv6</commstack>
    </motetype>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype232</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>98.76075470611741</x>
        <y>30.469519951198897</y>
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
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype330</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>58.59043340181549</x>
        <y>22.264557758786697</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
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
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter>ID:1</filter>
    </plugin_config>
    <width>249</width>
    <z>2</z>
    <height>181</height>
    <location_x>1</location_x>
    <location_y>521</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Radio environment (UDGM)</skin>
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
    <width>246</width>
    <z>1</z>
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
    <height>700</height>
    <location_x>250</location_x>
    <location_y>-1</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

