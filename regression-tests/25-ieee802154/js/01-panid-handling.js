TIMEOUT(10000, log.testFailed());

while(true) {
    YIELD();

    log.log(time + " " + id + " "+ msg + "\n");
    
    if(msg.contains("=check-me=") == false) {
        continue;
    }

    if(msg.contains("FAILED")) {
        log.testFailed();
    }

    if(msg.contains("DONE")) {
        log.testOK();
        break;
    }
    
}
