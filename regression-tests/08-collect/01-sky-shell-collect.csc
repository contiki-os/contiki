<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mrm</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/mspsim</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/avrora</project>
  <project EXPORT="discard">[CONTIKI_DIR]/tools/cooja/apps/native_gateway</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>150.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/collect/collect-view-shell.c</source>
      <commands EXPORT="discard">make collect-view-shell.sky TARGET=sky</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/collect/collect-view-shell.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
    </motetype>
    <mote>
      <breakpoints />
      <interface_config>
        org.contikios.cooja.interfaces.Position
        <x>69.8193406818502</x>
        <y>86.08116624448307</y>
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
        <x>23.73597351424919</x>
        <y>23.64085389583863</y>
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
        <x>96.89503278354498</x>
        <y>61.516110156918224</y>
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
        <x>7.611970631754317</x>
        <y>50.863062569941086</y>
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
        <x>97.77577457011573</x>
        <y>36.50885983165134</y>
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
        <x>81.84280607291373</x>
        <y>12.262433268451778</y>
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
        <x>48.76918142113213</x>
        <y>76.28996665071358</y>
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
        <x>10.516199800941727</x>
        <y>71.39959931668729</y>
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
        <x>69.48672858021564</x>
        <y>2.274435761561955</y>
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
        <x>84.25868612469665</x>
        <y>32.943146693468975</y>
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
        <x>13.670969901144792</x>
        <y>63.99238378992226</y>
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
        <x>72.51554571631638</x>
        <y>47.00560695436694</y>
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
        <x>9.789480819347663</x>
        <y>73.70566372866651</y>
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
        <x>32.19085060633389</x>
        <y>72.59300816076136</y>
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
        <x>80.2677099635723</x>
        <y>98.0702168139253</y>
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
        <x>9.946705912815235</x>
        <y>52.10151176834845</y>
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
        <x>2.43737538721972</x>
        <y>56.151002617425625</y>
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
        <x>27.435525284930186</x>
        <y>61.81996286556931</y>
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
        <x>70.60927462351833</x>
        <y>98.32577014155726</y>
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
        <x>43.3203771155477</x>
        <y>11.948622865702085</y>
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

TIMEOUT(500000);


/* Conf. */
booted = new Array();
received = new Array();
hops = new Array();
nrNodes = 20;
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
  received[i] = "___________";
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
GENERATE_MSG(20000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.equals("continue"));
node = sink_node;
log.log("Writing netcmd\n");
node.write("netcmd { repeat 11 30 { randwait 30 collect-view-data | blink | send } }");

while(true) {
  YIELD();

  /* Count sensor data packets */

  if (msg.contains("ÿ")) {
    log.log("WARN: Detected bad character in: '" + msg + "'\n");
    msg = msg.replace("ÿ", "");
  }

  data = msg.split(" ");

  if(data[24]) {

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
        received[source].substr(seqno + 1, 10 - seqno);

    if(hop &gt; 9) {
        hop = "+";
    }
    hops[source] = hops[source].substr(0, seqno) + hop +
        hops[source].substr(seqno + 1, 10 - seqno);

    total_received++;
    total_hops += hop;
    
    print_stats();
  }
  /* Signal OK if all nodes have reported 10 messages. */
  num_reported = 0;
  for(i = 1; i &lt;= nrNodes; i++) {
      if(i != sink) {
          if(received[i].split("_").length -1 &lt;= 1) {
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
    <width>602</width>
    <z>0</z>
    <height>508</height>
    <location_x>257</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>259</width>
    <z>5</z>
    <height>200</height>
    <location_x>4</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>2.2620479837704246 0.0 0.0 2.2620479837704246 11.65652309586307 5.218753534979797</viewport>
    </plugin_config>
    <width>260</width>
    <z>3</z>
    <height>296</height>
    <location_x>0</location_x>
    <location_y>197</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.SimControl
    <width>259</width>
    <z>4</z>
    <height>200</height>
    <location_x>4</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.Visualizer
    <plugin_config>
      <skin>org.contikios.cooja.plugins.skins.IDVisualizerSkin</skin>
      <skin>org.contikios.cooja.plugins.skins.UDGMVisualizerSkin</skin>
      <viewport>3.1695371670945955 0.0 0.0 3.1695371670945955 -64.4008177427222 -14.683213177997528</viewport>
    </plugin_config>
    <width>260</width>
    <z>4</z>
    <height>296</height>
    <location_x>0</location_x>
    <location_y>197</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>720</width>
    <z>2</z>
    <height>486</height>
    <location_x>695</location_x>
    <location_y>2</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.TimeLine
    <plugin_config>
      <mote>0</mote>
      <mote>1</mote>
      <mote>2</mote>
      <mote>3</mote>
      <mote>4</mote>
      <mote>5</mote>
      <mote>6</mote>
      <mote>7</mote>
      <mote>8</mote>
      <mote>9</mote>
      <mote>10</mote>
      <mote>11</mote>
      <mote>12</mote>
      <mote>13</mote>
      <mote>14</mote>
      <mote>15</mote>
      <mote>16</mote>
      <mote>17</mote>
      <mote>18</mote>
      <mote>19</mote>
      <showRadioRXTX />
      <showRadioHW />
      <split>118</split>
      <zoom>9</zoom>
    </plugin_config>
    <width>1440</width>
    <z>1</z>
    <height>425</height>
    <location_x>0</location_x>
    <location_y>405</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

