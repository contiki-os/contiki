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
    for(i = 1; i <= nrNodes; i++) {
      if(count[i] > 10) {
	print_stats();
	log.testFailed(); /* We are done! */
      }
    }
    
    /* Wait until the sink have received at least two messages from every node */
    for(i = 1; i <= nrNodes; i++) {
      if(count[i] < 2) {
	break;
      }
      if(i == nrNodes) {
	print_stats();
	log.testOK();
      }
    }
    print_stats();
  }
}
