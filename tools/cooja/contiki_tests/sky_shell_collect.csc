<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
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
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/sky-shell/sky-shell.c</source>
      <commands>make clean TARGET=sky
make sky-shell.sky TARGET=sky</commands>
      <firmware>../../../examples/sky-shell/sky-shell.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
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
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>87.29845932913939</x>
        <y>60.286214311723164</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>94.30809966340686</x>
        <y>22.50388779326399</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>2</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>82.40423567500785</x>
        <y>39.56979106929553</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>3</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>26.185019854469438</x>
        <y>4.800834369523899</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>4</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>1.9530156130507015</x>
        <y>78.3175061800706</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>5</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>48.35216700543414</x>
        <y>80.36988713780997</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>6</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>24.825985087266833</x>
        <y>74.27809432062487</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>7</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>8.356165164293616</x>
        <y>94.33967355724187</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>8</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>45.11740613004886</x>
        <y>31.7059041432301</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>9</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>68.9908548386292</x>
        <y>55.01991960639596</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>10</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>13.181122543889046</x>
        <y>55.9636533130127</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>11</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>2.1749985906538427</x>
        <y>78.39666095789707</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>12</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>37.79795217518357</x>
        <y>7.164284163506062</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>13</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>64.4595177394984</x>
        <y>72.115414337433</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>14</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>81.85663737096085</x>
        <y>89.31412706434035</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>15</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>44.74952276297882</x>
        <y>18.78566116347574</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>16</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>96.11333426285873</x>
        <y>90.64560410751824</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>17</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>21.651464136783527</x>
        <y>7.1381043251259495</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>18</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>83.6006916200628</x>
        <y>26.97170140682981</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>19</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>1.3446070721664705</x>
        <y>7.340373220385176</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>20</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>690</width>
    <z>1</z>
    <height>207</height>
    <location_x>0</location_x>
    <location_y>493</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>
function
print_stats()
{
  log.log("Time " + time + "\n");
  log.log("Received " + total_received  + " messages, " +
	  (total_received / nrNodes) + " messages/node, " +
	  total_lost + " lost, " +
	  (total_lost / nrNodes) + " lost/node, " +
	  total_dups + " dups, " +
	  (total_dups / nrNodes) + " dups/node, " +
	  (total_hops / total_received) + " hops/message\n");
  log.log("Seqnos: ");
  for(i = 1; i &lt;= nrNodes; i++) {
    log.log(seqnos[i] + " ");
  }
  log.log("\n");
  log.log("Lost: ");
  for(i = 1; i &lt;= nrNodes; i++) {
    log.log(lost[i] + " ");
  }
  log.log("\n");
  log.log("Dups: ");
  for(i = 1; i &lt;= nrNodes; i++) {
    log.log(dups[i] + " ");
  }
  log.log("\n");
  log.log("Hops: ");
  for(i = 1; i &lt;= nrNodes; i++) {
    log.log(hops[i] + " ");
  }
  log.log("\n");
  log.log("Received: ");
  for(i = 1; i &lt;= nrNodes; i++) {
    log.log(count[i] + " ");
  }
  log.log("\n");
  log.log("Stats: cpu " + 100 * total_cpu / (total_cpu + total_lpm) +
	  "% lpm " + 100 * total_lpm / (total_cpu + total_lpm) +
	  "% rx " + 100 * total_listen / (total_cpu + total_lpm) +
	  "% tx " + 100 * total_transmit / (total_cpu + total_lpm) +
  	  "% average latency " + total_latency / (4096 * total_received) +
	  " ms \n");
}

TIMEOUT(500000, print_stats());

/* Conf. */
booted = new Array();
count = new Array();
hops = new Array();
seqnos = new Array();
lost = new Array();
dups = new Array();
nrNodes = 20;
total_received = 0;
total_lost = 0;
total_hops = 0;
total_dups = 0;

total_cpu = total_lpm = total_listen = total_transmit = 0;

total_latency = 0;

nodes_starting = true;
for(i = 1; i &lt;= nrNodes; i++) {
  booted[i] = false;
  count[i] = 0;
  hops[i] = 0;
  seqnos[i] = -1;
  lost[i] = 0;
  dups[i] = 0;
}

/* Wait until all nodes have started */
while(nodes_starting) {
  YIELD_THEN_WAIT_UNTIL(msg.startsWith('Starting'));
  
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

/* Wait for prompt */
YIELD_THEN_WAIT_UNTIL(id == sink);
/*mote.getInterfaces().getButton().clickButton()*/
log.log("Writing netcmd\n");
node.write("netcmd { repeat 0 40 { randwait 40 sky-alldata | blink | send } }");
YIELD_THEN_WAIT_UNTIL(id == sink);
log.log("Writing collect command\n");
node.write("collect | timestamp | blink | binprint &amp;");

while(true) {
  YIELD();

  /* Count sensor data packets */
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
    light1 = parseInt(data[11]);
    light2 = parseInt(data[12]);
    temperature = parseInt(data[13]);
    humidity = parseInt(data[14]);
    rssi = parseInt(data[15]);
    time_cpu = parseInt(data[16]);
    time_lpm = parseInt(data[17]);
    time_transmit = parseInt(data[18]);
    time_listen = parseInt(data[19]);
    best_neighbor = parseInt(data[20]);
    best_neighbor_etx = parseInt(data[21]);
    best_neighbor_rtmetrix = parseInt(data[22]);
    battery_voltage = parseInt(data[23]);
    battery_indicator = parseInt(data[24]);

    total_cpu += time_cpu;
    total_lpm += time_lpm;
    total_transmit += time_transmit;
    total_listen += time_listen;

    total_latency += latency;
    
    source = node_id;
    count[source]++;
    hops[source] = hop;
    seqno_gap = seqno - seqnos[source];
    seqnos[source] = seqno;
    
    total_received++;
    total_hops += hop;
    
    if(seqno_gap == 2) {
      total_lost += seqno_gap - 1;
      lost[source] += seqno_gap - 1;
    } else if(seqno_gap == 0) {
      total_dups += 1;
      dups[source] += 1;
    }
  }
  /* Fail if the sink has received more than 10 messages from any node. */
  for(i = 1; i &lt;= nrNodes; i++) {
    if(i != sink &amp;&amp; count[i] &gt; 10) {
      print_stats();
      log.testFailed(); /* We are done! */
    }
  }
  
  /* Wait until the sink have received at least two messages from
     every node, except the sink, which doesn't send packets. */  
  for(i = 1; i &lt;= nrNodes; i++) {
    if(i != sink) {
      if(count[i] &lt; 2) {
	break;
      }
      if(i == nrNodes) {
	print_stats();
	log.testOK();
      }
    }
  }
}</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>2</z>
    <height>700</height>
    <location_x>259</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>259</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>-1</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>260</width>
    <z>0</z>
    <height>296</height>
    <location_x>0</location_x>
    <location_y>197</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

