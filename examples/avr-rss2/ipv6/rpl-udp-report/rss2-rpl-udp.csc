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
    <title>UDP sink/report simulation</title>
    <randomseed>123456</randomseed>
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
      se.sics.cooja.avrmote.RSS2MoteType
      <identifier>mote1</identifier>
      <description>Mote1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/platform/avr-rss2/examples/ipv6/rpl-udp-report/report.c</source>
      <commands EXPORT="discard">make report.avr-rss2 TARGET=avr-rss2 MCU=atmega128rfr2</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/platform/avr-rss2/examples/ipv6/rpl-udp-report/report.avr-rss2</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraClock</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraMoteID</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraUsart0</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraUsart1</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.RFR2Radio</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraADC</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraLED</moteinterface>
    </motetype>
    <motetype>
      se.sics.cooja.avrmote.RSS2MoteType
      <identifier>mote2</identifier>
      <description>Mote2</description>
      <source EXPORT="discard">[CONTIKI_DIR]/platform/avr-rss2/examples/ipv6/rpl-udp-report/sink.c</source>
      <commands EXPORT="discard">make sink.avr-rss2 TARGET=avr-rss2 MCU=atmega128rfr2</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/platform/avr-rss2/examples/ipv6/rpl-udp-report/sink.avr-rss2</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraClock</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraMoteID</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraUsart0</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraUsart1</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.RFR2Radio</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraADC</moteinterface>
      <moteinterface>se.sics.cooja.avrmote.interfaces.AvroraLED</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>100.0</x>
        <y>100.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.avrmote.interfaces.AvroraMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>mote1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>110.0</x>
        <y>100.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.avrmote.interfaces.AvroraMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>mote2</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>280</width>
    <z>9</z>
    <height>160</height>
    <location_x>400</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <moterelations>true</moterelations>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <viewport>3.908924509090908 0.0 0.0 3.908924509090908 -216.43707345454544 -217.89245090909074</viewport>
    </plugin_config>
    <width>400</width>
    <z>5</z>
    <height>400</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>1269</width>
    <z>8</z>
    <height>240</height>
    <location_x>400</location_x>
    <location_y>160</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Notes
    <plugin_config>
      <notes>Enter notes here</notes>
      <decorations>true</decorations>
    </plugin_config>
    <width>989</width>
    <z>6</z>
    <height>160</height>
    <location_x>680</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>0</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>508</width>
    <z>4</z>
    <height>389</height>
    <location_x>13</location_x>
    <location_y>413</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.MoteInterfaceViewer
    <mote_arg>1</mote_arg>
    <plugin_config>
      <interface>Serial port</interface>
      <scrollpos>0,0</scrollpos>
    </plugin_config>
    <width>545</width>
    <z>3</z>
    <height>392</height>
    <location_x>531</location_x>
    <location_y>413</location_y>
  </plugin>
</simconf>
