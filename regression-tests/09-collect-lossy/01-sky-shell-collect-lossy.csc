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
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>80.0</transmitting_range>
      <interference_range>0.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>0.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>shell</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/collect/collect-view-shell.c</source>
      <commands EXPORT="discard">make collect-view-shell.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/collect/collect-view-shell.sky</firmware>
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
        <x>51.083635845134815</x>
        <y>52.18027797603351</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>87.37621589982353</x>
        <y>69.01745044943294</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>0.3225524247130407</x>
        <y>99.67744560167213</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>52.99553499162932</x>
        <y>44.55947520113671</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>64.36007081217727</x>
        <y>7.922505931377522</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>46.937655527278906</x>
        <y>34.37401121375584</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>6</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>29.606117317748925</x>
        <y>59.7062771702808</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>7</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>64.60462597715014</x>
        <y>65.32875118919438</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>8</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>84.91966612667193</x>
        <y>21.61064185087591</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>9</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>24.128119535736893</x>
        <y>14.27770805377394</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>10</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>36.23919862128766</x>
        <y>23.421151622254555</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>11</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>53.689973725385855</x>
        <y>92.47281715616484</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>12</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>21.499980846738108</x>
        <y>86.31913226282572</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>13</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>23.896054282937385</x>
        <y>6.502957157635625</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>14</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>86.99986531287792</x>
        <y>45.74381748881159</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>15</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>68.50722882135574</x>
        <y>50.25930042782911</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>16</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>22.03851735367126</x>
        <y>57.304977718401084</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>17</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>16.094855623021655</x>
        <y>20.15220518337424</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>18</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>91.46540103964149</x>
        <y>63.949352956656554</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>19</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>71.92141571796324</x>
        <y>39.70157072422388</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        org.contikios.cooja.mspmote.interfaces.MspMoteID
        <id>20</id>
      </interface_config>
      <motetype_identifier>sky1</motetype_identifier>
    </mote>
  </simulation>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>318</width>
    <z>1</z>
    <height>172</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.AttributeVisualizerSkin</skin>
      <viewport>4.028431381533795 0.0 0.0 4.028431381533795 114.84980283087096 -0.016939876572727552</viewport>
    </plugin_config>
    <width>631</width>
    <z>2</z>
    <height>545</height>
    <location_x>809</location_x>
    <location_y>0</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter>timedout</filter>
      <formatted_time />
      <coloring />
    </plugin_config>
    <width>1440</width>
    <z>3</z>
    <height>275</height>
    <location_x>1</location_x>
    <location_y>556</location_y>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>function
print_stats()
{
  log.log("Time " + time + "\n");
  log.log("Received " + total_received  + " messages, " +
	  (total_received / nrNodes) + " messages/node, " +
	  total_reorder + " reordered, " +
	  total_lost + " lost, " +
	  (total_lost / nrNodes) + " lost/node, " +
	  total_dups + " dups, " +
	  (total_dups / nrNodes) + " dups/node, " +
	  (total_hops / total_received) + " hops/message\n");
  log.log("Received:\n");
  for(i = 1; i &lt;= nrNodes; i++) {
      log.log("Node " + i + " ");
      if(i == sink) {
          log.log("sink\n");
      } else {
          log.log("received: " + received[i] + " hops: " + hops[i] + "\n");
      }
  }
  log.log("Stats: cpu " + 100 * total_cpu / (total_cpu + total_lpm) +
	  "% lpm " + 100 * total_lpm / (total_cpu + total_lpm) +
	  "% rx " + 100 * total_listen / (total_cpu + total_lpm) +
	  "% tx " + 100 * total_transmit / (total_cpu + total_lpm) +
  	  "% average latency " + total_latency / (4096 * total_received) +
	  " ms \n");
}

TIMEOUT(700000);


/* Conf. */
booted = new Array();
received = new Array();
hops = new Array();
nrNodes = 20;
toReceive = 8;
total_received = 0;
total_lost = 0;
total_hops = 0;
total_dups = 0;
total_reorder = 0;

total_cpu = total_lpm = total_listen = total_transmit = 0;

total_latency = 0;

nodes_starting = true;
for(i = 1; i &lt;= nrNodes; i++) {
  booted[i] = false;
  received[i] = '';
  for(var j = 0; j &lt; toReceive; j++) {
     received[i] += '_';
  }
  hops[i] = received[i];
}

/* Wait until all nodes have started */
while(nodes_starting) {
  YIELD_THEN_WAIT_UNTIL(msg.startsWith('Star'));
  
  log.log("Node " + id + " booted\n");
  booted[id] = true;

  for(i = 1; i &lt;= nrNodes; i++) {
    if(!booted[i]) {
      break;
    }
    if(i == nrNodes) {
      nodes_starting = false;
    }
  }
}

/* Create sink */
log.log("All nodes booted, creating sink at node " + id + "\n");
sink = id;
sink_node = node;
/* Wait for prompt */
YIELD_THEN_WAIT_UNTIL(id == sink);
log.log("Writing collect command\n");
node.write("collect | timestamp | blink | binprint &amp;");
GENERATE_MSG(40000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
node = sink_node;
log.log("Writing netcmd\n");
node.write("netcmd { repeat 10 60 { randwait 60 collect-view-data | blink | send 31 } }");

while(true) {
  YIELD();

  /* Count sensor data packets */

  if (msg.contains("ÿ")) {

    msg = msg.replace("ÿ", "");
  }

  log.log(time + " " + id + " " + msg + "\n");
  data = msg.split(" ");

  if(data[24] &amp;&amp; !msg.contains("P")) {

    len = parseInt(data[0]);
    timestamp1 = parseInt(data[1]);
    timestamp2 = parseInt(data[2]);
    timesynched_timestamp = parseInt(data[3]);
    node_id = parseInt(data[4]);
    seqno = parseInt(data[5]);
    hop = parseInt(data[6]);
    latency = parseInt(data[7]);
    data_len2 = parseInt(data[8]);
    clock = parseInt(data[9]);
    timesyncedtime = parseInt(data[10]);
    time_cpu = parseInt(data[11]);
    time_lpm = parseInt(data[12]);
    time_transmit = parseInt(data[13]);
    time_listen = parseInt(data[14]);
    best_neighbor = parseInt(data[15]);
    best_neighbor_etx = parseInt(data[16]);
    best_neighbor_rtmetrix = parseInt(data[17]);

    total_cpu += time_cpu;
    total_lpm += time_lpm;
    total_transmit += time_transmit;
    total_listen += time_listen;

    total_latency += latency;
    
    source = node_id;
    dups = received[source].substr(seqno, 1);
    if(dups == "_") {
        dups = 1;
    } else if(dups &lt; 9) {
        dups++;
    }
    received[source] = received[source].substr(0, seqno) + dups +
        received[source].substr(seqno + 1, toReceive - seqno);

    if(hop &gt; 9) {
        hop = "+";
    }
    hops[source] = hops[source].substr(0, seqno) + hop +
        hops[source].substr(seqno + 1, toReceive - seqno);

    total_received++;
    total_hops += hop;
    
    print_stats();
  }
  /* Signal OK if all nodes have reported toReceive messages. */
  num_reported = 0;
  for(i = 1; i &lt;= nrNodes; i++) {
      if(i != sink) {
          if(!isNaN(received[i])) {
              num_reported++;
          }
      }
  }

  if(num_reported == nrNodes - 1) {
      print_stats();
      log.testOK();
  }
}</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>775</height>
    <location_x>304</location_x>
    <location_y>5</location_y>
  </plugin>
</simconf>

