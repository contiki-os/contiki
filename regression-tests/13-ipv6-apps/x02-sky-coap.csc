<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <simulation>
    <title>REST with RPL router</title>
    <delaytime>-2147483648</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>50.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>rplroot</identifier>
      <description>Sky RPL Root</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/ipv6/rpl-border-router/border-router.c</source>
      <commands EXPORT="discard">make border-router.sky TARGET=sky DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver,NULLRDC_CONF_802154_AUTOACK=0,CC2420_CONF_AUTOACK=0,ENERGEST_CONF_ON=0,PROCESS_CONF_NO_PROCESS_NAMES=1</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/ipv6/rpl-border-router/border-router.sky</firmware>
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
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>skyweb</identifier>
      <description>Rest server</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/er-rest-example/er-example-server.c</source>
      <commands EXPORT="discard">make er-example-server.sky TARGET=sky DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_driver,NULLRDC_CONF_802154_AUTOACK=0,CC2420_CONF_AUTOACK=0,ENERGEST_CONF_ON=0,PROCESS_CONF_NO_PROCESS_NAMES=1</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/er-rest-example/er-example-server.sky</firmware>
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
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>33.260163187353555</x>
        <y>30.643217359962595</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>rplroot</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>35.100895239785295</x>
        <y>39.70574552287428</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>skyweb</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>259</width>
    <z>6</z>
    <height>179</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.AttributeVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.AddressVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.GridVisualizerSkin</skin>
      <viewport>3.3323852179491644 0.0 0.0 3.3323852179491644 -30.392247168885415 -60.79227000363299</viewport>
    </plugin_config>
    <width>176</width>
    <z>5</z>
    <height>173</height>
    <location_x>259</location_x>
    <location_y>3</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <coloring />
    </plugin_config>
    <width>576</width>
    <z>0</z>
    <height>492</height>
    <location_x>12</location_x>
    <location_y>260</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.RadioLogger
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
    org.contikios.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <showRadioRXTX />
      <showLEDs />
      <split>23</split>
      <zoomfactor>24060.2737326431</zoomfactor>
    </plugin_config>
    <width>579</width>
    <z>2</z>
    <height>152</height>
    <location_x>6</location_x>
    <location_y>758</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Notes
    <plugin_config>
      <notes>Nightly test exercising Contiki's Erbium CoAP implementation:
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

The final test uses the CoAP Java implementation by Matthias Kovatsch, downloaded from:
https://github.com/mkovatsc/Californium/blob/master/run/ExampleClient.jar
* $ java -jar ExampleClient.jar DISCOVER coap://[aaaa::212:7402:2:202]
* $ java -jar ExampleClient.jar GET coap://[aaaa::212:7402:2:202]/hello</notes>
      <decorations>true</decorations>
    </plugin_config>
    <width>751</width>
    <z>3</z>
    <height>369</height>
    <location_x>439</location_x>
    <location_y>3</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <scriptfile>[CONFIG_DIR]/rest_rpl_coap.js</scriptfile>
      <active>true</active>
    </plugin_config>
    <width>596</width>
    <z>1</z>
    <height>725</height>
    <location_x>591</location_x>
    <location_y>225</location_y>
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

