<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project EXPORT="discard">[APPS_DIR]/mrm</project>
  <project EXPORT="discard">[APPS_DIR]/mspsim</project>
  <project EXPORT="discard">[APPS_DIR]/avrora</project>
  <project EXPORT="discard">[APPS_DIR]/serial_socket</project>
  <project EXPORT="discard">[APPS_DIR]/collect-view</project>
  <project EXPORT="discard">[APPS_DIR]/powertracker</project>
  <project EXPORT="discard">[APPS_DIR]/mobility</project>
  <project EXPORT="discard">[APPS_DIR]/node-generator</project>
  <project EXPORT="discard">[APPS_DIR]/radiologger-headless</project>
  <simulation>
    <title>SCRIPT_SIMULATION_NAME</title>
    <randomseed>SCRIPT_SEED</randomseed>
    <motedelay_us>100000</motedelay_us>
    <radiomedium>
      org.contikios.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>70.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>SCRIPT_RX_RATIO</success_ratio_rx>
    </radiomedium>
    <events>
      <logoutput>40000</logoutput>
    </events>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sink Node</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/rpl-mobility/udp-server.c</source>
      <commands EXPORT="discard">make udp-server-custom.sky TARGET=sky WITH_COMPOWER=1 SEED=SCRIPT_SEED RPL_CONF_FIXED_DIO=SCRIPT_RPL_CONF_FIXED_DIO RPL_CONF_DIO_ON_INCONSISTENCY=SCRIPT_RPL_CONF_DIO_ON_INCONSISTENCY RPL_CONF_PROBE_ON_NEW_NEIGHBOR=SCRIPT_RPL_CONF_PROBE_ON_NEW_NEIGHBOR RPL_CONF_REVERSE_TRICKLE=SCRIPT_RPL_CONF_REVERSE_TRICKLE PROJ_DIR=SCRIPT_SIMULATION_NAME</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/rpl-mobility/SCRIPT_SIMULATION_NAME/udp-server-custom.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky2</identifier>
      <description>Static Node</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/rpl-mobility/udp-client.c</source>
      <commands EXPORT="discard">make udp-client-static.sky TARGET=sky WITH_COMPOWER=1 SEED=SCRIPT_SEED PERIOD=SCRIPT_PERIOD RPL_CONF_FIXED_DIO=SCRIPT_RPL_CONF_FIXED_DIO RPL_CONF_DIO_ON_INCONSISTENCY=SCRIPT_RPL_CONF_DIO_ON_INCONSISTENCY RPL_CONF_DIO_DAO_ON_NEW_PARENT=SCRIPT_RPL_CONF_DIO_DAO_ON_NEW_PARENT RPL_CONF_PROBE_ON_NEW_NEIGHBOR=SCRIPT_RPL_CONF_PROBE_ON_NEW_NEIGHBOR RPL_CONF_REVERSE_TRICKLE=SCRIPT_RPL_CONF_REVERSE_TRICKLE PROJ_DIR=SCRIPT_SIMULATION_NAME </commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/rpl-mobility/SCRIPT_SIMULATION_NAME/udp-client-static.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
    <motetype>
      org.contikios.cooja.mspmote.SkyMoteType
      <identifier>sky3</identifier>
      <description>Mobile Node</description>
      <source EXPORT="discard">[CONTIKI_DIR]/examples/rpl-mobility/udp-client.c</source>
      <commands EXPORT="discard">make udp-client-mobile.sky TARGET=sky WITH_COMPOWER=1 SEED=SCRIPT_SEED PERIOD=SCRIPT_PERIOD RPL_CONF_FIXED_DIO=SCRIPT_RPL_CONF_FIXED_DIO RPL_CONF_DIO_ON_INCONSISTENCY=SCRIPT_RPL_CONF_DIO_ON_INCONSISTENCY RPL_CONF_DIO_DAO_ON_NEW_PARENT=SCRIPT_RPL_CONF_DIO_DAO_ON_NEW_PARENT RPL_CONF_PROBE_ON_NEW_NEIGHBOR=SCRIPT_RPL_CONF_PROBE_ON_NEW_NEIGHBOR RPL_CONF_REVERSE_TRICKLE=SCRIPT_RPL_CONF_REVERSE_TRICKLE RPL_CONF_MOBILE=1 PROJ_DIR=SCRIPT_SIMULATION_NAME</commands>
      <firmware EXPORT="copy">[CONTIKI_DIR]/examples/rpl-mobility/SCRIPT_SIMULATION_NAME/udp-client-mobile.sky</firmware>
      <moteinterface>org.contikios.cooja.interfaces.Position</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.RimeAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>org.contikios.cooja.interfaces.MoteAttributes</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyCoffeeFilesystem</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.Msp802154Radio</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspSerial</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyLED</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.MspDebugOutput</moteinterface>
      <moteinterface>org.contikios.cooja.mspmote.interfaces.SkyTemperature</moteinterface>
    </motetype>
  </simulation>
  <plugin>
    pt.inescid.cnm.nodegenerator.NodeGenerator
    <plugin_config>
      <radius_min>SCRIPT_RADIUS_MIN</radius_min>
      <radius_max>SCRIPT_RADIUS_MAX</radius_max>
      <radius_increment>SCRIPT_RADIUS_INCREMENT</radius_increment>
      <angle_min>SCRIPT_ANGLE_MIN</angle_min>
      <angle_multiplier>SCRIPT_ANGLE_MULTIPLIER</angle_multiplier>
      <random_offset>SCRIPT_RANDOM_OFFSET</random_offset>
      <mote_type_ratio>SCRIPT_MOBILITY_RATIO</mote_type_ratio>
      <generation_seed>SCRIPT_SEED</generation_seed>
      <mote_type_2>sky3</mote_type_2>
    </plugin_config>
  </plugin>
  <plugin>
    be.cetic.cooja.plugins.RadioLoggerHeadless
    <plugin_config>
      <pcap_file>[CONTIKI_DIR]/examples/rpl-mobility/SCRIPT_SIMULATION_NAME/mob.pcap</pcap_file>
    </plugin_config>
  </plugin>
  <plugin>
    org.contikios.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>
var folder=".";
var packetsReceived= new Array();
var packetsSent = new Array();
var packetsTime = new Object();
var serverID = 1;
var nodeCount = sim.getMotesCount();
var totalPRR = 0;
var flushCycle = 10000;

var packetsSentFile = new java.io.FileWriter(folder + "/log_sent.log");
var packetsRecvFile = new java.io.FileWriter(folder + "/log_recv.log");
var allPacketsFile = new java.io.FileWriter(folder + "/log_allp.log");
var energyFile = new java.io.FileWriter(folder + "/log_energy.log");
var finalFile = new java.io.FileWriter(folder + "/final.log",true);

for(i = 0; i &lt;= nodeCount; i++) {
        packetsReceived[i] = 0;
        packetsSent[i] = 0;
}

function endCalculations(){
  var totalReceived = 0;
  var totalSent = 0;
  for(i = serverID + 1; i &lt;= nodeCount; i++) {
          totalReceived += packetsReceived[i];
          totalSent += packetsSent[i];
  }
  totalPRR = totalReceived / totalSent;
  var str="";
  var totalJitter=0;
  var totalAVG=0;
  var totalCNT=0;
  Object.keys(packetsTime).map(function(el){
    var cnt = 0;
    var avg = Object.keys(packetsTime[el]).reduce(function(prev,curr){
      var pkt = packetsTime[el][curr];
      if(!pkt.recv) return prev;
      var diff = pkt.recv - pkt.sent;
      return (cnt*prev + diff)/++cnt;
    },0);
    var jitter = Object.keys(packetsTime[el]).reduce(function(prev,curr){
      var pkt = packetsTime[el][curr];
      if(!pkt.recv) return prev;
      var diff = pkt.recv - pkt.sent;
      return (cnt*prev + Math.abs(diff-avg))/++cnt;
    },0);
    str += "Sender: " + el + "\tSuccess: " + cnt + "\tLatency: " + avg/1000000 + "\tJitter:"+jitter/1000000+"\n";
    totalAVG+= avg;
    totalJitter+=jitter;
    totalCNT+=1;
  });
  log.log(str);

  totalAVG = totalAVG / totalCNT;

  totalJitter = totalJitter / totalCNT;
  str = "PRR:\t" + totalPRR + "\n";
  str += "Delay:\t" + totalAVG/1000000 + "\n";
  str += "Jitter:\t" + totalJitter/1000000 + "\n";
  finalFile.write(str);
  finalFile.flush();
};

function closeFiles(){
  packetsSentFile.close();
  packetsRecvFile.close();
  energyFile.close();
  allPacketsFile.close();
  finalFile.close();
}

function flushFiles(){
  packetsSentFile.flush();
  packetsRecvFile.flush();
  energyFile.flush();
  allPacketsFile.flush();
  finalFile.flush();
}

TIMEOUT(SCRIPT_TIMEOUT)

var iteration = 0;

while(1) {
    try{
      YIELD();
    }catch(e){
      endCalculations();
      closeFiles();
      log.testOK();
    }
    if(sim.getSimulationTimeMillis() > SCRIPT_TIMEOUT - 10000){
      endCalculations();
      closeFiles();
      log.testOK();
    }

    iteration+=1;
    if(iteration%flushCycle == 0) flushFiles();

    if(msg.startsWith("{")) {
      var pinfo = eval("(" + msg + ")");
      pinfo.node = id;
      allPacketsFile.write(JSON.stringify(pinfo) + "\n");
      allPacketsFile.write(msg + "\n");
      allPacketsFile.flush();
      if(pinfo.type == 'Data' &amp;&amp; pinfo.action == 'Recv'){
        senderID = pinfo.object.sender;
        packetID = pinfo.object.message.id;
        if(packetsTime[senderID][packetID].recv) continue;
        packetsReceived[senderID]++;
        packetsTime[senderID][packetID].recv = sim.getSimulationTime();
        diff = packetsTime[senderID][packetID].recv - packetsTime[senderID][packetID].sent;
        packetsRecvFile.write(sim.getSimulationTime()+"\t"+senderID+"\t"+packetID+"\t"+id+"\t"+diff+"\n");
      }else if(pinfo.type == 'Data' &amp;&amp; pinfo.action == 'Send'){
        senderID = id;
        packetID = pinfo.object.id;
        packetsTime[senderID] = packetsTime[senderID] || new Object();
        packetsTime[senderID][packetID] = {sent: sim.getSimulationTime()};
        packetsSentFile.write(sim.getSimulationTime()+"\t"+senderID+"\t"+packetID+"\n");
        packetsSent[senderID]++;
      }
    }else if(msg.startsWith("#P")){
      var msgArray = msg.split(' ');
      if(msgArray[2] == "P"){
        energyFile.write(
            id + " " + msgArray[1] + " " + msgArray[4] +  " " + msgArray[5] + " " + msgArray[6] + " " + msgArray[7] +
            " " + msgArray[8] + " " + msgArray[9] + " " + msgArray[10] + "\n");
      }
    }
 }
      </script>
      <active>true</active>
    </plugin_config>
    <width>600</width>
    <z>0</z>
    <height>700</height>
    <location_x>665</location_x>
    <location_y>6</location_y>
  </plugin>
</simconf>
