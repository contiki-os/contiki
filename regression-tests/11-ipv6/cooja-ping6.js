TIMEOUT(300000, log.testFailed());

var motes = [];
var ready = 0;
var csma_ping_sender = null;
var csma_ping_receiver;
var csma_ieee802154e_2012_ping_sender = null;
var csma_ieee802154e_2012_ping_receiver;
var tsch_ping_sender;
var tsch_ping_receiver
var rpl_ping_sender;
var rpl_ping_receiver;
var have_tsch_coord = false;
var have_rpl_root = false;

var number_of_motes = sim.getMotes().length;

function collect_results(str) {
    /* Collect results */
    var checks = 0;
    var success = [];
    var failure = [];

    log.log("=== collect result for " + str + " ===\n");
    
    while (checks < (number_of_motes / 2)) {
        YIELD();
        log.log(time + " " + id + " "+ msg + "\n");
        /* check ping6 result */
        if (msg.contains("ping6")) {
            if (msg.contains("OK")) {
                success.push(id);
            }
        }

        if (msg.contains("post-ping")) {
            if ((msg.contains("00:00:00:00:00:00:00:00") ||
                 msg.contains("null")) &&
                success.indexOf(id) >= 0) {
                success.splice(success.indexOf(id), 1);
            }
            checks++;
        }
    }

    /* print results */
    if(success.indexOf(csma_ping_sender.id) >= 0) {
        log.log("CSMA:                       OK\n");
    } else {
        log.log("CSMA:                       NG\n");
    }

    if(success.indexOf(csma_ieee802154e_2012_ping_sender.id) >= 0) {
        log.log("CSMA + IEEE 802.15.4e-2012: OK\n");
    } else {
        log.log("CSMA + IEEE 802.15.4e-2012: NG\n");
    }

    if(success.indexOf(tsch_ping_sender.id) >= 0) {
        log.log("TSCH:                       OK\n");
    } else {
        log.log("TSCH:                       NG\n");
    }

    if(success.indexOf(rpl_ping_sender.id) >= 0) {
        log.log("RPL:                        OK\n");
    } else {
        log.log("RPL:                        NG\n");
    }

    
    /* finish */
    if (success.length != (number_of_motes / 2)) {
        log.testFailed();
    }
}

/* wait for all nodes getting ready */
while (ready < number_of_motes) {
    YIELD();
    log.log(time + " ID:" + id + " "+ msg + "\n");

    if (msg.contains("TSCH") && msg.contains("node")) {
        var m = { obj: sim.getMoteWithID(id), id: id };
        if (have_tsch_coord == false) {
            tsch_ping_receiver = m;
            sim.getMoteWithID(id).getInterfaces().getButton().clickButton();
            have_tsch_coord = true;
        } else {
            tsch_ping_sender = m;
        }
        motes.push(m);
    } else if (msg.contains("CSMA") && msg.contains("node")) {
        var m = { obj: sim.getMoteWithID(id), id: id };
        if (msg.contains("IEEE 802.15.4e-2012")) {
            if(csma_ieee802154e_2012_ping_sender == null) {
                csma_ieee802154e_2012_ping_sender = m;
            } else {
                csma_ieee802154e_2012_ping_receiver = m;
            }
        } else if (msg.contains("RPL")) {
            if (have_rpl_root == false) {
                rpl_ping_receiver = m;
                sim.getMoteWithID(id).getInterfaces().getButton().clickButton();
                have_rpl_root = true;
            } else {
                rpl_ping_sender = m;
            }
        } else {
            /* CSMA without RPL nor IEEE 802.15.4e-2012 */
            if (csma_ping_sender == null) {
                csma_ping_sender = m;
            } else {
                csma_ping_receiver = m;
            }
        }
        motes.push(m);
    }
    if (msg.contains("ready for test")) {
        ready++;
    }
}

/* incorporate motes' IPv6 addresses */
for (var i = 0; i < number_of_motes; i++) {
    motes[i].link_local = motes[i].obj.getInterfaces().getIPAddress().getLocalIP();
    motes[i].global = motes[i].obj.getInterfaces().getIPAddress().getIP(0); /* XXX: expect a global IPv6 address and not null*/
    motes[i].send_ping = function(m, type) {
        var dest;
        var cmd_string;
        if (type === "link local") {
            dest = m.link_local;
        } else {
            dest = m.global;
        }
        cmd_string = "ping " + dest;
        write(this.obj, cmd_string);
        log.log(time + " ID:" + this.obj.getID() + " " + cmd_string + "\n");        
    };
}

/* order to send ping */
csma_ping_sender.send_ping(csma_ping_receiver, "link local");
csma_ieee802154e_2012_ping_sender.send_ping(csma_ieee802154e_2012_ping_receiver, "link local");
tsch_ping_sender.send_ping(tsch_ping_receiver, "link local");
rpl_ping_sender.send_ping(rpl_ping_receiver, "link local");

collect_results("ping for link local address");

log.testOK();
