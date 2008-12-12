if (msg.startsWith('Coffee file test: 0')) {
  global.put("coffee_file_test", true);
} else if (msg.startsWith('Coffee file test')) {
  global.put("coffee_file_test", false);
} else if (msg.startsWith('Coffee garbage collection test: 0')) {
  global.put("coffee_gc_test", true);
} else if (msg.startsWith('Coffee garbage collection test')) {
  global.put("coffee_gc_test", false);
}

if (global.get("coffee_file_test") == null) {
  //log.log("file test not finished yet\n");
  return;
}
if (global.get("coffee_gc_test") == null) {
  //log.log("gc test not finished yet\n");
  return;
}

if (global.get("coffee_gc_test") == false) {
  log.log("coffee garbage collection failed\n");
  log.testFailed();
}

if (global.get("coffee_file_test") == false) {
  log.log("coffee file test failed\n");
  log.testFailed();
}

log.testOK();
