TIMEOUT(200000, log.log("last message: " + msg + "\n"));

data = 0;
alive = 0;
while(true) {
    YIELD();
    if(msg.startsWith('Data')) {
        data++;
        log.log("Heard " + data + " data messages\n");
    }
    if(msg.startsWith('Alive')) {
        alive++;
        log.log("Heard " + alive + " alive messages\n");
    }
    if(data == 10 && alive == 10) {
        if(msg.startsWith('Data 10') ||
           msg.startsWith('Alive 10')) {
            log.testOK();
        } else {
            log.testError();
        }
    }
}
