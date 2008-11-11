/* Only handle receive messages */
if (!msg.contains('received')) {
  return;
}

/* Count received packets */
result = global.get("recv_" + id);
if (result == null) {
  result = 0;
}
result++;
global.put("recv_" + id, result);
log.log(id + " received " + result + " messages\n");

/* Did all nodes (1 and 2) receive a message? */
for (i = 1; i <= 2; i++) {
  result = global.get("recv_" + i);
  if (result == null) {
    return;
  }
  if (result < 30) {
    return;
  }
}

log.log("Node 1 received " + global.get("recv_1") + " messages\n");
log.log("Node 2 received " + global.get("recv_2") + " messages\n");
log.testOK(); /* Report test success */
