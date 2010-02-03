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
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/udp-sender-ipv6/example-udp-sender.c</source>
      <commands>make clean TARGET=sky
make example-udp-sender.sky TARGET=sky DEFINES=WITH_UIP6,UDP_ADDR_A=0xfe80,UDP_ADDR_B=0,UDP_ADDR_C=0,UDP_ADDR_D=0,UDP_ADDR_E=0x0212,UDP_ADDR_F=0x7402,UDP_ADDR_G=0x02,UDP_ADDR_H=0x202</commands>
      <firmware>../../../examples/udp-sender-ipv6/example-udp-sender.sky</firmware>
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
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky2</identifier>
      <description>Sky Mote Type #2</description>
      <source>../../../examples/udp-receiver-ipv6/example-udp-receiver.c</source>
      <commands>make clean TARGET=sky
make example-udp-receiver.sky TARGET=sky DEFINES=WITH_UIP6,UDP_ADDR_A=0xfe80,UDP_ADDR_B=0,UDP_ADDR_C=0,UDP_ADDR_D=0,UDP_ADDR_E=0x0212,UDP_ADDR_F=0x7401,UDP_ADDR_G=0x01,UDP_ADDR_H=0x101</commands>
      <firmware>../../../examples/udp-receiver-ipv6/example-udp-receiver.sky</firmware>
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
        <x>65.934608127183</x>
        <y>63.70462190529231</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky2</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>67.66105781539623</x>
        <y>63.13924301161143</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>816</width>
    <z>1</z>
    <height>333</height>
    <location_x>1</location_x>
    <location_y>365</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Addresses: IP or Rime</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>246</width>
    <z>0</z>
    <height>167</height>
    <location_x>0</location_x>
    <location_y>198</location_y>
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
    <width>572</width>
    <z>2</z>
    <height>700</height>
    <location_x>246</location_x>
    <location_y>-1</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

