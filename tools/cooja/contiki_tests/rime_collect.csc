<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
  <simulation>
    <title>Rime collect test</title>
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
      se.sics.cooja.contikimote.ContikiMoteType
      <identifier>mtype528</identifier>
      <description>Contiki Mote Type #1</description>
      <contikiapp>../../../examples/rime/example-collect.c</contikiapp>
      <commands>make example-collect.cooja TARGET=cooja</commands>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Battery</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiVib</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiMoteID</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRS232</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiBeeper</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiIPAddress</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiRadio</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiButton</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiPIR</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiClock</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiLED</moteinterface>
      <moteinterface>se.sics.cooja.contikimote.interfaces.ContikiCFS</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <symbols>false</symbols>
      <commstack>Rime</commstack>
    </motetype>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>50.829139451170256</x>
        <y>62.671405251575884</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>1</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>66.70779372167401</x>
        <y>87.6357288540325</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>2</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>41.129223698257974</x>
        <y>11.860747327849797</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>3</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>3.896453344832884</x>
        <y>57.94647173289612</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>4</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>66.34456950462831</x>
        <y>23.965576279871247</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>5</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>70.88189625059834</x>
        <y>87.6624950776213</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>6</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>19.09803704982651</x>
        <y>38.15109163392978</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>7</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>89.63745297998314</x>
        <y>5.041038966356015</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>8</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>13.348832186584092</x>
        <y>21.93390094486253</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>9</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>1.674784139064478</x>
        <y>14.130140721379835</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>10</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>47.87142051729898</x>
        <y>7.506518941902218</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>11</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>26.88271893793631</x>
        <y>45.005087461409865</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>12</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>56.02971812708909</x>
        <y>26.88551057316878</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>13</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>95.69470045280002</x>
        <y>71.59310205157577</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>14</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>31.731698652367335</x>
        <y>39.71639147226488</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>15</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>2.8792353689463424</x>
        <y>51.72896355140204</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>16</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>51.07177717821779</x>
        <y>90.91445330727116</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>17</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>43.42318694224158</x>
        <y>91.44323457482095</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>18</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>87.22205782780289</x>
        <y>48.33830904569607</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>19</id>
      </interface_config>
    </mote>
    <mote>
      se.sics.cooja.contikimote.ContikiMote
      <motetype_identifier>mtype528</motetype_identifier>
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>1.6332448872152683</x>
        <y>32.64258506789177</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.interfaces.Battery
        <infinite>false</infinite>
      </interface_config>
      <interface_config>
        se.sics.cooja.contikimote.interfaces.ContikiMoteID
        <id>20</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>223</width>
    <z>3</z>
    <height>227</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Addresses: IP or Rime</skin>
      <skin>Mote IDs</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>200</width>
    <z>0</z>
    <height>225</height>
    <location_x>222</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>420</width>
    <z>2</z>
    <height>489</height>
    <location_x>0</location_x>
    <location_y>225</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(300000, log.log("Timeout\n"));

function
print_stats()
{
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
}

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
mote.getInterfaces().getButton().clickButton()

while(true) {
  YIELD();

  /* Count sensor data packets */
  if(msg.startsWith("Sink")) {
    node_text = msg.split(" ")[4];
    seqno_text = msg.split(" ")[6];
    hops_text = msg.split(" ")[8];
    if(node_text) {
      source = parseInt(node_text);
      seqno = parseInt(seqno_text);
      hop = parseInt(hops_text);
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
      if(count[i] &gt; 10) {
	print_stats();
	log.testFailed(); /* We are done! */
      }
    }
    
    /* Wait until the sink have received at least two messages from every node */
    for(i = 1; i &lt;= nrNodes; i++) {
      if(count[i] &lt; 2) {
	break;
      }
      if(i == nrNodes) {
	print_stats();
	log.testOK();
      }
    }
    print_stats();
  }
}</script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>1</z>
    <height>714</height>
    <location_x>420</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

