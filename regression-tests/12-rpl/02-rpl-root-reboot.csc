<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/collect-view</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <simulation>
    <title>My simulation</title>
    <randomseed>123456</randomseed>
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
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype748</identifier>
      <description>Sender</description>
      <source>[CONFIG_DIR]/code/sender-node.c</source>
      <commands>make TARGET=cooja clean
make sender-node.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype442</identifier>
      <description>RPL root</description>
      <source>[CONFIG_DIR]/code/root-node.c</source>
      <commands>make TARGET=cooja clean
make root-node.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype236</identifier>
      <description>Receiver</description>
      <source>[CONFIG_DIR]/code/receiver-node.c</source>
      <commands>make TARGET=cooja clean
make receiver-node.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>-22.5728586847096</x>
        <y>123.9358664968653</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype236</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>116.13379149678028</x>
        <y>88.36698920455684</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype748</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>-1.39303771455413</x>
        <y>100.21446701029119</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>4</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype236</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>95.25095618820441</x>
        <y>63.14998053005015</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>5</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype236</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>66.09378990830604</x>
        <y>38.32698761608261</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>6</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype236</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>29.05630841762433</x>
        <y>30.840688165838436</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>7</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype236</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>10.931583432822638</x>
        <y>69.848248459216</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>8</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype236</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>0.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>3</id>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype442</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>280</width>
    <z>0</z>
    <height>160</height>
    <location_x>400</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>se.sics.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>se.sics.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <viewport>2.5379695437350276 0.0 0.0 2.5379695437350276 75.2726010197627 15.727272727272757</viewport>
    </plugin_config>
    <width>400</width>
    <z>2</z>
    <height>400</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
    </plugin_config>
    <width>1184</width>
    <z>3</z>
    <height>240</height>
    <location_x>402</location_x>
    <location_y>162</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Notes
    <plugin_config>
      <notes>Enter notes here</notes>
      <decorations>true</decorations>
    </plugin_config>
    <width>904</width>
    <z>4</z>
    <height>160</height>
    <location_x>680</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>GENERATE_MSG(0000000, "add-sink");&#xD;
GENERATE_MSG(1000000, "remove-sink");&#xD;
GENERATE_MSG(1000000, "add-sink");&#xD;
&#xD;
lostMsgs = 0;&#xD;
&#xD;
TIMEOUT(10000000, if(lastMsg != -1 &amp;&amp; lostMsgs == 0) { log.testOK(); } );&#xD;
&#xD;
lastMsg = -1;&#xD;
packets = "_________";&#xD;
hops = 0;&#xD;
&#xD;
while(true) {&#xD;
    YIELD();&#xD;
    if(msg.equals("remove-sink")) {&#xD;
        m = sim.getMoteWithID(3);&#xD;
        sim.removeMote(m);&#xD;
        log.log("removed sink\n");&#xD;
    } else if(msg.equals("add-sink")) {&#xD;
        if(!sim.getMoteWithID(3)) {&#xD;
            m = sim.getMoteTypes()[1].generateMote(sim);&#xD;
            m.getInterfaces().getMoteID().setMoteID(3);&#xD;
            sim.addMote(m);&#xD;
            log.log("added sink\n");&#xD;
         } else {&#xD;
            log.log("did not add sink as it was already there\n");      &#xD;
         }&#xD;
    } else if(msg.startsWith("Sending")) {&#xD;
        hops = 0;&#xD;
    } else if(msg.startsWith("#L")) {&#xD;
        hops++;&#xD;
    } else if(msg.startsWith("Data")) {&#xD;
//        log.log("" + msg + "\n");    &#xD;
        data = msg.split(" ");&#xD;
        num = parseInt(data[14]);&#xD;
        packets = packets.substr(0, num) + "*";&#xD;
        log.log("" + hops + " " + packets + "\n");&#xD;
//        log.log("Num " + num + "\n");&#xD;
        if(lastMsg != -1) {&#xD;
          if(num != lastMsg + 1) {&#xD;
            numMissed = num - lastMsg;&#xD;
            lostMsgs += numMissed;&#xD;
            log.log("Missed messages " + numMissed + " before " + num + "\n");            &#xD;
            for(i = 0; i &lt; numMissed; i++) {&#xD;
                packets = packets.substr(0, lastMsg + i) + "_";    &#xD;
            }&#xD;
          }    &#xD;
        }&#xD;
        lastMsg = num;&#xD;
    }&#xD;
}</script>
      <active>true</active>
    </plugin_config>
    <width>962</width>
    <z>1</z>
    <height>596</height>
    <location_x>603</location_x>
    <location_y>43</location_y>
  </plugin>
</simconf>

