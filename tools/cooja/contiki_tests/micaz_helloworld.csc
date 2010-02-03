<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
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
      se.sics.cooja.avrmote.MicaZMoteType
      <identifier>micaz1</identifier>
      <description>MicaZ Mote Type #micaz1</description>
      <source>../../../examples/hello-world/hello-world.c</source>
      <commands>make clean TARGET=micaz
make hello-world.elf TARGET=micaz</commands>
      <firmware>../../../examples/hello-world/hello-world.elf</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.MicaZID</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.MicaZLED</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.MicaZRadio</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.MicaClock</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.MicaSerial</moteinterface>
    </motetype>
    <mote>
      se.sics.cooja.avrmote.MicaZMote
      <motetype_identifier>micaz1</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>68.44103812985554</x>
        <y>35.6791174319418</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.avrmote.interfaces.MicaZID
        <id>1</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>259</width>
    <z>4</z>
    <height>184</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.LogVisualizerSkin</skin>
      <viewport>0.9090909090909091 0.0 0.0 0.9090909090909091 81.78087442740406 87.56443869823474</viewport>
    </plugin_config>
    <width>300</width>
    <z>1</z>
    <height>300</height>
    <location_x>456</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>756</width>
    <z>2</z>
    <height>150</height>
    <location_x>0</location_x>
    <location_y>286</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <showRadioRXTX />
      <showRadioHW />
      <showLEDs />
      <split>109</split>
      <zoom>9</zoom>
    </plugin_config>
    <width>756</width>
    <z>3</z>
    <height>150</height>
    <location_x>0</location_x>
    <location_y>436</location_y>
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
    <width>600</width>
    <z>0</z>
    <height>305</height>
    <location_x>89</location_x>
    <location_y>211</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

