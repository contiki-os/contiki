TIMEOUT(300000, log.log("Timeout\n"));

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
  for(i = 1; i <= nrNodes; i++) {
    log.log(seqnos[i] + " ");
  }
  log.log("\n");
  log.log("Lost: ");
  for(i = 1; i <= nrNodes; i++) {
    log.log(lost[i] + " ");
  }
  log.log("\n");
  log.log("Dups: ");
  for(i = 1; i <= nrNodes; i++) {
    log.log(dups[i] + " ");
  }
  log.log("\n");
  log.log("Hops: ");
  for(i = 1; i <= nrNodes; i++) {
    log.log(hops[i] + " ");
  }
  log.log("\n");
  log.log("Received: ");
  for(i = 1; i <= nrNodes; i++) {
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
for(i = 1; i <= nrNodes; i++) {
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

  for(i = 1; i <= nrNodes; i++) {
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
node.write("netcmd { repeat 0 20 { randwait 20 sky-alldata | blink | send } }");
YIELD_THEN_WAIT_UNTIL(id == sink);
log.log("Writing collect command\n");
node.write("collect | timestamp | blink | binprint &");

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
  for(i = 1; i <= nrNodes; i++) {
    if(i != sink && count[i] > 10) {
      print_stats();
      log.testFailed(); /* We are done! */
    }
  }
  
  /* Wait until the sink have received at least two messages from
     every node, except the sink, which doesn't send packets. */  
  for(i = 1; i <= nrNodes; i++) {
    if(i != sink) {
      if(count[i] < 2) {
	break;
      }
      if(i == nrNodes) {
	print_stats();
	log.testOK();
      }
    }
  }
}
