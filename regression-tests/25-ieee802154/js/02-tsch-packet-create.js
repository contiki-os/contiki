TIMEOUT(10000, log.testFailed());

num_of_motes = sim.getMotesCount();

while(true) {
    YIELD();

    log.log(time + " node-" + id + ": "+ msg + "\n");
    
    if(msg.contains("=check-me=") == false) {
        continue;
    }

    if(msg.contains("FAILED")) {
        log.testFailed();
        break;
    }

    if(msg.contains("DONE")) {
        num_of_motes -= 1;
        if(num_of_motes == 0) {
            log.testOK();
            break;
        }
    }
    
}
