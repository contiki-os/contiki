<?xml version="1.0" encoding="UTF-8"?>
<simconf>
  <project>../apps/mrm</project>
  <project>../apps/mspsim</project>
  <project>../apps/avrora</project>
  <project>../apps/native_gateway</project>
  <simulation>
    <title>My simulation</title>
    <delaytime>0</delaytime>
    <randomseed>generated</randomseed>
    <motedelay_us>1000000</motedelay_us>
    <radiomedium>
      se.sics.cooja.radiomediums.UDGM
      <transmitting_range>50.0</transmitting_range>
      <interference_range>100.0</interference_range>
      <success_ratio_tx>1.0</success_ratio_tx>
      <success_ratio_rx>1.0</success_ratio_rx>
    </radiomedium>
    <motetype>
      se.sics.cooja.mspmote.SkyMoteType
      <identifier>sky1</identifier>
      <description>Sky Mote Type #1</description>
      <source>../../../examples/sky-shell/sky-checkpoint.c</source>
      <commands>make clean TARGET=sky
make sky-checkpoint.sky TARGET=sky</commands>
      <firmware>../../../examples/sky-shell/sky-checkpoint.sky</firmware>
      <moteinterface>se.sics.cooja.interfaces.Position</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.IPAddress</moteinterface>
      <moteinterface>se.sics.cooja.interfaces.Mote2MoteRelations</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspClock</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.MspMoteID</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyButton</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyFlash</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyByteRadio</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkySerial</moteinterface>
      <moteinterface>se.sics.cooja.mspmote.interfaces.SkyLED</moteinterface>
    </motetype>
    <mote>
      se.sics.cooja.mspmote.SkyMote
      <motetype_identifier>sky1</motetype_identifier>
      <breakpoints />
      <interface_config>
        se.sics.cooja.interfaces.Position
        <x>3.537694077190867</x>
        <y>25.877706916818877</y>
        <z>0.0</z>
      </interface_config>
      <interface_config>
        se.sics.cooja.mspmote.interfaces.MspMoteID
        <id>1</id>
      </interface_config>
    </mote>
  </simulation>
  <plugin>
    se.sics.cooja.plugins.SimControl
    <width>248</width>
    <z>3</z>
    <height>200</height>
    <location_x>0</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.Visualizer
    <plugin_config>
      <skin>Mote IDs</skin>
      <skin>Radio environment (UDGM)</skin>
    </plugin_config>
    <width>246</width>
    <z>2</z>
    <height>210</height>
    <location_x>2</location_x>
    <location_y>199</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.LogListener
    <plugin_config>
      <filter />
    </plugin_config>
    <width>849</width>
    <z>1</z>
    <height>246</height>
    <location_x>0</location_x>
    <location_y>409</location_y>
    <minimized>false</minimized>
  </plugin>
  <plugin>
    se.sics.cooja.plugins.ScriptRunner
    <plugin_config>
      <script>TIMEOUT(360000, log.log("timeout at phase " + phase + ". last message: " + msg + "\n"));
phase=0;

/* Wait until node has booted */
WAIT_UNTIL(msg.startsWith('Starting'));
log.log("Shell started\n");
phase++;

/* 1. BACKGROUND PROCESS - NO CHECKPOINTING */
write(mote, "repeat 10 1 echo bg process &amp;\n");
log.log("Starting background process without checkpointing\n");
expected=10;
while (expected &gt; 0) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('bg process'));
  expected--;
}

/* Make sure background process has exited */
GENERATE_MSG(3000, "continue");
while (!msg.contains('continue')) {
  YIELD();
  if (msg.contains('bg process')) {
    log.log("Too many bg messages at phase: " + phase + "\n");
    log.testFailed(); /* We are done! */
    while (true) YIELD();
  }
}
log.log("Background process without checkpointing done\n\n");
phase++;

/* 2. BACKGROUND PROCESS - CHECKPOINTING EVERY SECOND */
write(mote, "repeat 10 1 echo bg process &amp;\n");
log.log("Starting background process with periodic checkpointing\n");
expected=10;
while (expected &gt; 0) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('bg process'));
  expected--;
  write(mote, "checkpoint file" + expected + "\n");
  log.log("&gt; checkpoint file" + expected + "\n");
}

/* Make sure background process has exited */
GENERATE_MSG(3000, "continue");
while (!msg.contains('continue')) {
  YIELD();
  if (msg.contains('bg process')) {
    log.log("Too many bg messages at phase: " + phase + "\n");
    log.testFailed(); /* We are done! */
    while (true) YIELD();
  }
}
log.log("Background process with periodic checkpointing done\n\n");
phase++;

/* 3. LIST ALL FILES */
write(mote, "ls");
YIELD_THEN_WAIT_UNTIL(msg.contains('file9'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file8'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file7'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file6'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file5'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file4'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file3'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file2'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file1'));
YIELD_THEN_WAIT_UNTIL(msg.contains('file0'));
GENERATE_MSG(1000, "continue");
YIELD_THEN_WAIT_UNTIL(msg.contains('continue'));
log.log("All checkpoints are stored in the filesystem\n\n");
phase++;

/* 4. ROLLBACK TO RESTORE BACKGROUND PROCESS */
write(mote, "rollback file7");
log.log("Rolling back background process at count 7\n");
expected=7;
while (expected &gt; 0) {
  YIELD_THEN_WAIT_UNTIL(msg.contains('bg process'));
  expected--;
}

/* Make sure background process has exited */
GENERATE_MSG(3000, "continue");
while (!msg.contains('continue')) {
  YIELD();
  if (msg.contains('bg process')) {
    log.log("Too many bg messages at phase: " + phase + "\n");
    log.testFailed(); /* We are done! */
    while (true) YIELD();
  }
}
log.log("Background process was rolled back successfully\n\n");
phase++;


log.testOK(); /* We are done! */</script>
      <active>true</active>
    </plugin_config>
    <width>604</width>
    <z>0</z>
    <height>409</height>
    <location_x>246</location_x>
    <location_y>0</location_y>
    <minimized>false</minimized>
  </plugin>
</simconf>

