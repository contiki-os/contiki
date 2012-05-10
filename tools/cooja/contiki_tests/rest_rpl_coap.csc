<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>REST with RPL router</title>
    <delaytime>-2147483648</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>50.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>rplroot</identifier>
      <description>Sky RPL Root</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/ipv6/rpl-border-router/border-router.c</source>
      <commands EXPORT="discard">make border-router.sky TARGET=sky DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver,NULLRDC_CONF_802154_AUTOACK=0,CC2420_CONF_AUTOACK=0,ENERGEST_CONF_ON=0,PROCESS_CONF_NO_PROCESS_NAMES=1</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/ipv6/rpl-border-router/border-router.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyByteRadio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>skyweb</identifier>
      <description>Rest</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/er-rest-example/rest-server-example.c</source>
      <commands EXPORT="discard">make rest-server-example.sky TARGET=sky DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver,NULLRDC_CONF_802154_AUTOACK=0,CC2420_CONF_AUTOACK=0,ENERGEST_CONF_ON=0,PROCESS_CONF_NO_PROCESS_NAMES=1</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/er-rest-example/rest-server-example.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyByteRadio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>33.260163187353555</x>
        <y>30.643217359962595</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>rplroot</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>35.100895239785295</x>
        <y>39.70574552287428</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>skyweb</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>259</width>
    <z>6</z>
    <height>179</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AttributeVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <viewport>3.3323852179491644 0.0 0.0 3.3323852179491644 -30.392247168885415 -60.79227000363299</viewport>
    </plugin_config>
    <width>176</width>
    <z>5</z>
    <height>173</height>
    <location_x>259</location_x>
    <location_y>3</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <coloring />
    </plugin_config>
    <width>582</width>
    <z>2</z>
    <height>393</height>
    <location_x>6</location_x>
    <location_y>259</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.RadioLogger
    <plugin_config>
      <split>114</split>
      <analyzers name="6lowpan" />
    </plugin_config>
    <width>574</width>
    <z>-1</z>
    <height>471</height>
    <location_x>412</location_x>
    <location_y>190</location_y>
    <minimized>true</minimized>
  </plugin>
  <plugin>
    SerialSocketServer
    <mote_arg>0</mote_arg>
    <width>428</width>
    <z>4</z>
    <height>74</height>
    <location_x>7</location_x>
    <location_y>181</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <showRadioRXTX />
      <showLEDs />
      <split>23</split>
      <zoomfactor>24060.2737326431</zoomfactor>
    </plugin_config>
    <width>579</width>
    <z>3</z>
    <height>152</height>
    <location_x>13</location_x>
    <location_y>655</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Notes
    <plugin_config>
      <notes>Nightly test based on simulation in examples/er-rest-example/rest-server-example.csc:
* One REST server, and one RPL border router w. corresponding tun0 netif
* ContikiMAC is disabled to make firmwares fit on Tmote Sky nodes.
* Additional compile-time DEFINES used in this simulation:
NETSTACK_MAC=nullmac_driver
NETSTACK_RDC=nullrdc_driver
NULLRDC_CONF_802154_AUTOACK=0
CC2420_CONF_AUTOACK=0
ENERGEST_CONF_ON=0
PROCESS_CONF_NO_PROCESS_NAMES=1

The test script communicates with the REST server via the RPL border router using external commands.
(* $ make connect-router-cooja)
* $ ping6 -c 10 -I tun0 aaaa::212:7401:1:101
* $ ping6 -c 10 -I tun0 aaaa::212:7402:2:202
* $ wget -t 1 -T 10 -O - http://[aaaa::212:7402:2:202]

The final test uses CoAP to talk to the server, and requires an external Java library:
See: https://github.com/dapaulid/JCoAP
To download jar-file: $ wget --no-check-certificate http://github.com/dapaulid/JCoAP/raw/master/run/SampleClient.jar
</notes>
    <decorations>true</decorations>
    </plugin_config>
    <width>751</width>
    <z>1</z>
    <height>252</height>
    <location_x>439</location_x>
    <location_y>3</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <scriptfile>[CONFIG_DIR]/rest_rpl_coap.js</scriptfile>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>584</height>
    <location_x>592</location_x>
    <location_y>260</location_y>
  </plugin>
  <plugin>
    PowerTracker
    <width>400</width>
    <z>-1</z>
    <height>155</height>
    <location_x>132</location_x>
    <location_y>152</location_y>
    <minimized>true</minimized>
  </plugin>
</simconf>

