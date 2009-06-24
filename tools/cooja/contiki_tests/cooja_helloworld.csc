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
      <identifier>mtype82</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>../../../examples/hello-world/hello-world.c</contikiapp>
      <commands>make hello-world.cooja TARGET=cooja</commands>
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
      <symbols>false</symbols>
      <commstack>Rime</commstack>
    </motetype>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype82</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>69.64867743029201</x>
        <y>69.2570131081022</y>
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
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Log output: printf()'s</skin>
    </plugin_config>
    <width>259</width>
    <z>1</z>
    <height>198</height>
    <location_x>2</location_x>
    <location_y>203</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>259</width>
    <z>2</z>
    <height>217</height>
    <location_x>2</location_x>
    <location_y>403</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>259</width>
    <z>3</z>
    <height>200</height>
    <location_x>2</location_x>
    <location_y>3</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(2000, log.log("last message: " + msg + "\n"));

WAIT_UNTIL(msg.equals('Hello, world'));
log.testOK();</script>
      <active>true</active>
    </plugin_config>
    <width>592</width>
    <z>0</z>
    <height>618</height>
    <location_x>264</location_x>
    <location_y>3</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

