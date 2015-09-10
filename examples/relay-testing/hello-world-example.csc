<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/serial_socket</project>
  <project EXPORT="discard">/home/user/contikiprojects/sics.se/mobility</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/collect-view</project>
  <project EXPORT="discard">/home/user/contikiprojects/sics.se/powertracker</project>
  <simulation>
    <title>Hello world example</title>
    <delaytime>0</delaytime>
    <randomseed>123456</randomseed>
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
      <description>Hello world sky</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/hello-world/hello-world.c</source>
      <commands EXPORT="discard">make hello-world.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/hello-world/hello-world.sky</firmware>
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
    <motetype>
      org.contikios.cooja.avrmote.MicaZMoteType
      <identifier>micaz1</identifier>
      <description>Hello world micaz</description>
      <source>[CONTIKI_DIR]/examples/hello-world/hello-world.c</source>
      <commands>make hello-world.elf TARGET=micaz</commands>
      <firmware>[CONTIKI_DIR]/examples/hello-world/hello-world.elf</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.avrmote.interfaces.MicaZID</moteinterface>
      <moteinterface>org.contikios.cooja.avrmote.interfaces.MicaZLED</moteinterface>
      <moteinterface>org.contikios.cooja.avrmote.interfaces.MicaZRadio</moteinterface>
      <moteinterface>org.contikios.cooja.avrmote.interfaces.MicaClock</moteinterface>
      <moteinterface>org.contikios.cooja.avrmote.interfaces.MicaSerial</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>93.1576842462532</x>
        <y>12.110654409141041</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>24.27184344020801</x>
        <y>51.304686914739605</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.avrmote.interfaces.MicaZID
        <id>2</id>
      </interface_config>
      <motetype_identifier>micaz1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>318</width>
    <z>0</z>
    <height>192</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <viewport>3.8271488095595365 0.0 0.0 3.8271488095595365 -79.7101385461893 -0.34997402775074654</viewport>
    </plugin_config>
    <width>300</width>
    <z>1</z>
    <height>300</height>
    <location_x>435</location_x>
    <location_y>4</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>738</width>
    <z>2</z>
    <height>356</height>
    <location_x>0</location_x>
    <location_y>224</location_y>
  </plugin>
</simconf>

