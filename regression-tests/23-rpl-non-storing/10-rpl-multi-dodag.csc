<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/collect-view</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <project EXPORT="discard">[APPS_DIR]/serial2pty</project>
  <project EXPORT="discard">[APPS_DIR]/radiologger-headless</project>
  <simulation>
    <title>My simulation</title>
    <randomseed>123456</randomseed>
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
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>mtype301</identifier>
      <description>Sender</description>
      <source>[CONTIKI_DIR]/regression-tests/12-rpl/code/sender-node.c</source>
      <commands>make sender-node.cooja TARGET=cooja</commands>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>mtype820</identifier>
      <description>RPL root</description>
      <source>[CONTIKI_DIR]/regression-tests/12-rpl/code/root-node.c</source>
      <commands>make root-node.cooja TARGET=cooja</commands>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <motetype>
      org.contikios.cooja.contikimote.ContikiMoteType
      <identifier>mtype306</identifier>
      <description>Receiver</description>
      <source>[CONTIKI_DIR]/regression-tests/12-rpl/code/receiver-node.c</source>
      <commands>make receiver-node.cooja TARGET=cooja</commands>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Battery</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>org.contikios.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <symbols>false</symbols>
    </motetype>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>9.767954940345236</x>
        <y>88.75813939592845</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype306</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>63.36720084537501</x>
        <y>75.88456991067605</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype301</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-20.684049350551753</x>
        <y>60.49767834794315</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>6</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype306</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>64.61229064867878</x>
        <y>39.88729002781773</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>3</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype306</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>37.157272454309606</x>
        <y>19.60335867526139</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>4</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype306</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>-21.976612887408603</x>
        <y>30.69884249204435</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>5</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype306</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>43</x>
        <y>98</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>7</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype820</motetype_identifier>
    </mote>
    <mote>
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>0.0</x>
        <y>0.0</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiMoteID
        <id>8</id>
      </interface_config>
      <interface_config>
        org.contikios.cooja.contikimote.interfaces.ContikiRadio
        <bitrate>250.0</bitrate>
      </interface_config>
      <motetype_identifier>mtype820</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>280</width>
    <z>3</z>
    <height>160</height>
    <location_x>400</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.GridVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.MoteTypeVisualizerSkin</skin>
      <viewport>1.7624788498159916 0.0 0.0 1.7624788498159916 97.6893062637241 8.72727272727273</viewport>
    </plugin_config>
    <width>400</width>
    <z>0</z>
    <height>400</height>
    <location_x>1</location_x>
    <location_y>1</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>1184</width>
    <z>2</z>
    <height>240</height>
    <location_x>402</location_x>
    <location_y>162</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Notes
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
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>GENERATE_MSG(0000000, "add-sink-1");&#xD;
GENERATE_MSG(0000000, "add-sink-2");&#xD;
GENERATE_MSG(0000000, "remove-sink-3");&#xD;
&#xD;
GENERATE_MSG(2000000, "remove-sink-1");&#xD;
GENERATE_MSG(4000000, "remove-sink-2");&#xD;
GENERATE_MSG(4000000, "add-sink-3");&#xD;
&#xD;
lostMsgs = 0;&#xD;
newDagOk = 0;&#xD;
&#xD;
TIMEOUT(6000000, if(newDagOk == 2) { log.testOK(); } );&#xD;
&#xD;
lastMsg = -1;&#xD;
newSink = 0;&#xD;
packets = "_________";&#xD;
hops = 0;&#xD;
&#xD;
while(true) {&#xD;
    YIELD();&#xD;
    if(msg.equals("remove-sink-1")) {&#xD;
        m = sim.getMoteWithID(7);&#xD;
        if(m) {&#xD;
            sim.removeMote(m);&#xD;
            log.log("removed sink 1\n");&#xD;
        }&#xD;
    } else if(msg.equals("remove-sink-2")) {&#xD;
        m = sim.getMoteWithID(8);&#xD;
        if(m) {&#xD;
            sim.removeMote(m);&#xD;
            log.log("removed sink 2\n");&#xD;
        }&#xD;
    } else if(msg.equals("remove-sink-3")) {&#xD;
        m = sim.getMoteWithID(9);&#xD;
        if(m) {&#xD;
            sim.removeMote(m);&#xD;
            log.log("removed sink 3\n");&#xD;
        }&#xD;
    } else if(msg.equals("add-sink-1")) {&#xD;
        newSink = 1;&#xD;
        if(!sim.getMoteWithID(7)) {&#xD;
            m = sim.getMoteTypes()[1].generateMote(sim);&#xD;
            m.getInterfaces().getMoteID().setMoteID(7);&#xD;
            sim.addMote(m);&#xD;
            m.getInterfaces().getPosition().setCoordinates(43, 98, 0);&#xD;
            log.log("added sink 1\n");&#xD;
         } else {&#xD;
            log.log("did not add sink 1 as it was already there\n");      &#xD;
         }&#xD;
    } else if(msg.equals("add-sink-2")) {&#xD;
        newSink = 1;&#xD;
        if(!sim.getMoteWithID(8)) {&#xD;
            m = sim.getMoteTypes()[1].generateMote(sim);&#xD;
            m.getInterfaces().getMoteID().setMoteID(8);&#xD;
            sim.addMote(m);&#xD;
            log.log("added sink 2\n");&#xD;
         } else {&#xD;
            log.log("did not add sink 2 as it was already there\n");      &#xD;
         }&#xD;
    } else if(msg.equals("add-sink-3")) {&#xD;
        newSink = 1;&#xD;
        if(!sim.getMoteWithID(9)) {&#xD;
            m = sim.getMoteTypes()[1].generateMote(sim);&#xD;
            m.getInterfaces().getMoteID().setMoteID(9);&#xD;
            sim.addMote(m);&#xD;
            log.log("added sink 3\n");&#xD;
         } else {&#xD;
            log.log("did not add sink 3 as it was already there\n");      &#xD;
         }&#xD;
    } else if(msg.startsWith("Sending")) {&#xD;
        hops = 0;&#xD;
    } else if(msg.startsWith("#L") &amp;&amp; msg.endsWith("1; red")) {&#xD;
        hops++;&#xD;
    } else if(msg.startsWith("Data")) {&#xD;
        data = msg.split(" ");&#xD;
        num = parseInt(data[14]);&#xD;
        if(newSink) {&#xD;
          newDagOk++;&#xD;
          newSink = 0;&#xD;
        }&#xD;
        if(lastMsg != -1) {&#xD;
          if(num != lastMsg + 1) {&#xD;
            numMissed = num - lastMsg - 1;&#xD;
            lostMsgs += numMissed;           &#xD;
            log.log("Missed messages " + numMissed + " before " + num + "\n");            &#xD;
            for(i = 0; i &lt; numMissed; i++) {&#xD;
                packets = packets.substr(0, lastMsg + i + 1).concat("_");    &#xD;
            }&#xD;
          }    &#xD;
        }&#xD;
        packets = packets.substr(0, num).concat("*");&#xD;
        log.log("" + hops + " " + packets + "\n");&#xD;
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

