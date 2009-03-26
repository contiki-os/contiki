<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <ticktime>1</ticktime>
    <randomseed>123456</randomseed>
    <motedelay>1000</motedelay>
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
      <description>Sky Mote Type #sky1</description>
      <source>../../../examples/sky-shell-exec/sky-shell-exec.c</source>
      <commands>make hello-world.ce TARGET=sky

make sky-shell-exec.sky TARGET=sky
make sky-shell-exec.sky CORE=sky-shell-exec.sky TARGET=sky
make sky-shell-exec.sky CORE=sky-shell-exec.sky TARGET=sky</commands>
      <firmware>../../../examples/sky-shell-exec/sky-shell-exec.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspIPAddress</moteinterface>
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
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>56.18151486126417</x>
        <y>93.20004013966208</y>
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
    <z>5</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>printf()'s + LEDs</skin>
    </plugin_config>
    <width>300</width>
    <z>4</z>
    <height>300</height>
    <location_x>1020</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <history>256</history>
    </plugin_config>
    <width>1347</width>
    <z>1</z>
    <height>675</height>
    <location_x>0</location_x>
    <location_y>337</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>845</width>
    <z>2</z>
    <height>551</height>
    <location_x>11</location_x>
    <location_y>249</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.mspmote.plugins.MspStackWatcher
    <mote_arg>0</mote_arg>
    <width>613</width>
    <z>3</z>
    <height>300</height>
    <location_x>247</location_x>
    <location_y>12</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

