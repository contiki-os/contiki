TIMEOUT(10000, log.testFailed());

var failed = false;

while(true) {
    YIELD();

    log.log(time + " " + "node-" + id + " "+ msg + "\n");
    
    if(msg.contains("=check-me=") == false) {
        continue;
    }

    if(msg.contains("FAILED")) {
        failed = true;
    }

    if(msg.contains("DONE")) {
        break;
    }
}
if(failed) {
    log.testFailed();
}
log.testOK();

