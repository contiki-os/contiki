TIMEOUT(60000);

fileOK = null;
gcOK = null;

while (fileOK == null || gcOK == null) {
  YIELD();

  if (msg.startsWith('Coffee file test: 0')) {
    fileOK = true;
  } else if (msg.startsWith('Coffee file test')) {
    fileOK = false;
  } else if (msg.startsWith('Coffee garbage collection test: 0')) {
    gcOK = true;
  } else if (msg.startsWith('Coffee garbage collection test')) {
    gcOK = false;
  }

}

if (gcOK == false) {
  log.log("coffee garbage collection failed\n");
}
if (fileOK == false) {
  log.log("coffee file test failed\n");
}
if (!fileOK || !gcOK) {
  log.testFailed();
} else {
  log.testOK();
}
