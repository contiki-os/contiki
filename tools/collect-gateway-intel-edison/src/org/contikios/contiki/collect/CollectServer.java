/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * Copyright (c) 2015, Singapore University of Technology and Design
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * -----------------------------------------------------------------
 *
 * CollectServer
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 *
 * Update by: Ngo Van Mao
 * Date: 1/10/2015
 * Email: vanmao_ngo@sutd.edu.sg
 */

package org.contikios.contiki.collect;

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;

import java.util.Scanner;
/**
 *
 */
public class CollectServer implements SerialConnectionListener {

  public static final String STDIN_COMMAND = "<STDIN>";

  public static final String CONFIG_FILE = "collect.conf";
  public static final String SENSORDATA_FILE = "sensordata.log";
  public static final String CONFIG_DATA_FILE = "collect-data.conf";
  public static final String INIT_SCRIPT = "collect-init.script";
  public static final String FIRMWARE_FILE = "collect-view-shell.ihex";
  public static final String COMPORT = "/dev/ttyUSB0";
  public static final String RIMESTACK = "RIME";

  private Properties config = new Properties();

  private String configFile;
  private Properties configTable = new Properties();

  private ArrayList<SensorData> sensorDataList = new ArrayList<SensorData>();
  private PrintWriter sensorDataOutput;
  private boolean isSensorLogUsed;

  private Hashtable<String,Node> nodeTable = new Hashtable<String,Node>();
  private Node[] nodeCache;
  private SerialConnection serialConnection;
  private SerialConnection tcpClientConnection;
  private boolean hasSerialOpened;
  /* Do not auto send init script at startup */
  private boolean doSendInitAtStartup = false;
  private String initScript;

  private boolean hasStarted = false;
  private boolean doExitOnRequest = true;


  @SuppressWarnings("serial")
  public CollectServer() {
    loadConfig(config, CONFIG_FILE);

    this.configFile = config.getProperty("config.datafile", CONFIG_DATA_FILE);
    if (this.configFile != null) {
      loadConfig(configTable, this.configFile);
    }
    this.initScript = config.getProperty("init.script", INIT_SCRIPT);

    for(Object key: configTable.keySet()) {
      String property = key.toString();
      if (!property.startsWith("collect")) {
        getNode(property, true);
      }
    }
  }


  public void start(SerialConnection connection, String serverAddr, int serverPort,
		            String stack, String comPort,
		     	    int interval, int random, int reports, int rexmits) {
    if (hasStarted) {
      throw new IllegalStateException("already started");
    }
    hasStarted = true;
    this.serialConnection = connection;
    System.out.println("==================Start Collector Sensor data=================");
    connectToSerial(comPort);

    if (serialConnection != null && serialConnection.isOpen()) {

    	System.out.println("Connect this SBAN to sever");
    	tcpClientConnection = new TCPClientReport(this, serverAddr, serverPort);
    	System.out.println("Start new thread runTCPClient to " + serverAddr + " at port:" + serverPort);
        tcpClientConnection.open(null);
        System.out.println(tcpClientConnection.getConnectionName());

    	if (stack.equals(RIMESTACK)) {
        	// Wait a short time before running the init script
            sleep(3000);
            System.out.println("Start runInitScript()");
            runInitScript();

            sleep(5000);
            System.out.println("starting collect");
            sendMultipleCommands("Start Collect", "~K", "killall",
                               "mac 0", SET_TIME_COMMAND,
                               "collect | timestamp | binprint &");

            // send command by netcmd to sensor nodes
            sleep(10000);
            System.out.println("Starting send netcmd");
            sendCommand("netcmd { repeat " + reports + " " + interval
                  + " { randwait " + random + " collect-view-data | send " + rexmits + " } }");
            System.out.println("After sending command");
            //handleInputCommand();
            //sendDataToServer("=========SystemTime=========" + System.currentTimeMillis());
    	} else { //RPL stack
    		System.out.println("OK for RPL receiving");
    	}
    } else {
        System.out.println("No serial port connection. No connected node");
    }
  }


  private final static String SET_TIME_COMMAND = "time %TIME% | null";

  private void  sendMultipleCommands(final String... command) {
    System.out.println("Start sending multiple commands");
    for(int i = 0, n = command.length; i < n; i++) {
      if (i > 0) {
        try {
          // Do not send multiple commands too fast
          Thread.sleep(1000);
        } catch (InterruptedException e1) {
        }
      }
      String cmd = command[i];
      if (cmd == SET_TIME_COMMAND) {
        cmd = "time " + (System.currentTimeMillis() / 1000) + " | null";
      }
      if (!sendCommand(cmd)) {
        break;
      }
    }

  }

  protected boolean sendCommand(String command) {
    if (sendToNode(command)) {
      System.out.println("Sent command '" + command + "'");
      return true;
    }
    System.out.println("Failed to send command. No serial connection.");
    return false;
  }


  protected void connectToSerial(String comPort) {
    if (serialConnection != null && !serialConnection.isOpen()) {
      serialConnection.open(comPort);
    }
  }

  public void stop() {
    if (serialConnection != null) {
      serialConnection.close();
    }
    if (tcpClientConnection != null) {
      tcpClientConnection.close();
    }
    PrintWriter output = this.sensorDataOutput;
    if (output != null) {
      output.close();
    }
  }

  public void setUseSensorDataLog(boolean useSensorLog) {
    this.isSensorLogUsed = useSensorLog;
  }

  private void exit() {
    if (doExitOnRequest) {
      stop();
      System.exit(0);
    } else {
      System.err.println("Cannot exit. What the heck?");
    }
  }

  private void sleep(long delay) {
    try {
      Thread.sleep(delay);
    } catch (InterruptedException e1) {
      // Ignore
    }
  }

  protected boolean hasInitScript() {
    return initScript != null && new File(initScript).canRead();
  }

  protected void runInitScript() {
    if (initScript != null) {
      runScript(initScript);
    }
  }

  protected void runScript(final String scriptFileName) {
    new Thread("scripter") {
      public void run() {
        try {
          BufferedReader in = new BufferedReader(new FileReader(scriptFileName));
          String line;
          while ((line = in.readLine()) != null) {
            if (line.length() == 0 || line.charAt(0) == '#') {
              // Ignore empty lines and comments
            } else if (line.startsWith("echo ")) {
              line = line.substring(5).trim();
              if (line.indexOf('%') >= 0) {
                line = line.replace("%TIME%", "" + (System.currentTimeMillis() / 1000));
              }
              sendToNode(line);
            } else if (line.startsWith("sleep ")) {
              long delay = Integer.parseInt(line.substring(6).trim());
              Thread.sleep(delay * 1000);
            } else {
              System.err.println("Unknown script command: " + line);
              break;
            }
          }
          in.close();
        } catch (Exception e) {
          System.err.println("Failed to run script: " + scriptFileName);
          e.printStackTrace();
        }
      }
    }.start();
  }

  public String getConfig(String property) {
    return getConfig(property, null);
  }

  public String getConfig(String property, String defaultValue) {
    return configTable.getProperty(property, config.getProperty(property, defaultValue));
  }


  /* -------------------------------------------------------------------
   * Node Handling
   * -------------------------------------------------------------------*/

  public synchronized Node[] getNodes() {
    if (nodeCache == null) {
      Node[] tmp = nodeTable.values().toArray(new Node[nodeTable.size()]);
      Arrays.sort(tmp);
      nodeCache = tmp;
    }
    return nodeCache;
  }

  // This method cannot be deleted. Why not I don't know?
  // Exception in thread "read input stream thread" java.lang.NoSuchMethodError:
  // org.contikios.contiki.collect.CollectServer.addNode(Ljava/lang/String;)Lorg/contikios/contiki/collect/Node;
  public Node addNode(String nodeID) {
    return getNode(nodeID, true);
  }

   private Node getNode(final String nodeID, boolean notify) {
     Node node = nodeTable.get(nodeID);
     if (node == null) {
       node = new Node(nodeID);
       nodeTable.put(nodeID, node);

       synchronized (this) {
         nodeCache = null;
       }

       if (notify) {
         final Node newNode = node;
       }
     }
     return node;
   }


  // -------------------------------------------------------------------
  // Node location handling
  // -------------------------------------------------------------------

  private boolean loadConfig(Properties properties, String configFile) {
    try {
      BufferedInputStream input =
        new BufferedInputStream(new FileInputStream(configFile));
      try {
        properties.load(input);
      } finally {
        input.close();
      }
      return true;
    } catch (FileNotFoundException e) {
      // No configuration file exists.
    } catch (IOException e) {
      System.err.println("Failed to read configuration file: " + configFile);
      e.printStackTrace();
    }
    return false;
  }

  private void saveConfig(Properties properties, String configFile) {
    try {
      File fp = new File(configFile);
      if (fp.exists()) {
        File targetFp = new File(configFile + ".bak");
        if (targetFp.exists()) {
          targetFp.delete();
        }
        fp.renameTo(targetFp);
      }
      FileOutputStream output = new FileOutputStream(configFile);
      try {
        properties.store(output, "Configuration for Collect");
      } finally {
        output.close();
      }
    } catch (IOException e) {
      System.err.println("failed to save configuration to " + configFile);
      e.printStackTrace();
    }
  }

  // ------------------------------------------------------------------
  // Handle Input Command
  // Function:
  // Input:
  // Output:
  // ------------------------------------------------------------------
  protected void handleInputCommand() {
    new Thread("handle input command") {
      public void run() {
        try {
	  Scanner in = new Scanner(System.in);
	  String line;
	  System.out.println("Started new thread handleInputCommand");
	  while ((line = in.nextLine()) != null) {
	    System.out.println("input command is: " + line);

	    if (line.equals("retry")) {
	      System.out.println("Retry to connect serial port");
              if (!serialConnection.isClosed()) {
          	System.out.println("The first connection to serial is failed. Will try another automatically!");
                serialConnection.open(COMPORT);
              }
	    }
      	    if (line.equals("script")) {
      	      System.out.println("Before running script");
      	      runInitScript();
      	      System.out.println("After running script");
      	    }
	    if (line.equals("send")) {
	      System.out.println("Before sending command");
              // send command by netcmd to sensor nodes
              System.out.println("Start send netcmd");
              int interval = 60;
              int random = 60;
              int reports = 0; // 0-report forever
              int rexmits = 31; // 0-31

              sendCommand("netcmd { repeat " + reports + " " + interval
                      + " { randwait " + random + " collect-view-data | send " + rexmits + " } }");
	      System.out.println("After sending command");
	    }
	    if (line.equals("start")) {
	      System.out.println("Before starting collect");
	      sendMultipleCommands("Start Collect", "~K", "killall",
		                   "mac 0", SET_TIME_COMMAND,
                                   "collect | timestamp | binprint &");
	      System.out.println("After starting collect");
	    }
	    if(line.equals("exit")) {
	      exit();
	    }

	  }
	} catch (Exception e) {
	  System.err.println("Failed to handle input command");
	  e.printStackTrace();
	}
      }
    }.start();
  }

  // -------------------------------------------------------------------
  // Serial communication
  // -------------------------------------------------------------------

  public boolean sendToNode(String data) {
    if (serialConnection != null && serialConnection.isOpen() && serialConnection.isSerialOutputSupported()) {
      serialConnection.writeSerialData(data);
      return true;
    }
    return false;
  }

  public void handleIncomingData(long systemTime, String line) {
    if (line.length() == 0 || line.charAt(0) == '#') {
      // Ignore empty lines, comments, and annotations.
      return;
    }
    SensorData sensorData = SensorData.parseSensorData(this, line, systemTime);
    if (sensorData != null) {
      // Sensor data received
      handleSensorData(sensorData);
      return;
    }
    System.out.println("SERIAL: " + line);
  }

  // -------------------------------------------------------------------
  // SensorData handling
  // Function:
  //    + Save collected data to a local file
  //    + Send these data to the server for further processing
  // -------------------------------------------------------------------

  private void handleSensorData(final SensorData sensorData) {
    System.out.println("SENSOR DATA: " + sensorData);
    saveSensorData(sensorData);
    sendDataToServer(sensorData.toString());
  }

  private void sendDataToServer(String data) {
    tcpClientConnection.writeSerialData(data.toString());
  }

  private void saveSensorData(SensorData data) {
    PrintWriter output = this.sensorDataOutput;
    if (output == null && isSensorLogUsed) {
      try {
        output = sensorDataOutput = new PrintWriter(new FileWriter(SENSORDATA_FILE, true));
      } catch (IOException e) {
        System.err.println("Failed to add sensor data to log '" + SENSORDATA_FILE + '\'');
        e.printStackTrace();
      }
    }
    if (output != null) {
      output.println(data.toString());
      output.flush();
    }
  }

  private void clearSensorData() {
    sensorDataList.clear();
    Node[] nodes = getNodes();
    this.nodeTable.clear();
    synchronized (this) {
      this.nodeCache = null;
    }
    if (nodes != null) {
      for(Node node : nodes) {
        node.removeAllSensorData();
      }
    }
    // Remove any saved node positions
    for(String key: configTable.keySet().toArray(new String[0])) {
      String property = key.toString();
      if (!property.startsWith("collect")) {
        configTable.remove(property);
      }
    }
  }

  private void clearSensorDataLog() {
    PrintWriter output = this.sensorDataOutput;
    if (output != null) {
      output.close();
    }
    // Remove the sensor data log
    new File(SENSORDATA_FILE).delete();
    this.sensorDataOutput = null;
  }


  // -------------------------------------------------------------------
  // SerialConnection Listener
  // -------------------------------------------------------------------

  @Override
  public void serialData(SerialConnection connection, String line) {
    handleIncomingData(System.currentTimeMillis(), line);
  }

  @Override
  public void serialOpened(SerialConnection connection) {
    String connectionName = connection.getConnectionName();
    System.out.println("*** Serial console listening on " + connectionName + " ***");
    hasSerialOpened = true;
    System.out.println("connected to " + connectionName);

    if (!connection.isSerialOutputSupported()) {
        System.out.println("*** Serial output not supported ***");
    } else if (doSendInitAtStartup) {
      // Send any initial commands
      doSendInitAtStartup = false;
      System.out.println("Mao serialOpened 2...");

      if (hasInitScript()) {
        // Wait a short time before running the init script
        sleep(3000);

        runInitScript();
      }
    }
  }

  @Override
  public void serialClosed(SerialConnection connection) {
    String prefix;
    if (hasSerialOpened) {
      prefix = "Serial connection terminated.\n";
      hasSerialOpened = false;
      //setSystemMessage("not connected");
      System.out.println("not connected");
    } else {
      prefix = "Failed to connect to " + connection.getConnectionName() + '\n';
    }
  }


  // -------------------------------------------------------------------
  // Main
  // -------------------------------------------------------------------

  public static void main(String[] args) {
    boolean resetSensorLog = false;
    boolean useSensorLog = true;
    String host = "localhost";
    String command = null;
    String comPort = COMPORT;
    String stack = RIMESTACK;
    int port = 6789;

    // parameters for shell command
    int interval = 60;
    int random = 60;
    int reports = 0; // 0-report forever
    int rexmits = 31; // 0-31

    for(int i = 0, n = args.length; i < n; i++) {
      String arg = args[i];
      if (arg.length() == 2 && arg.charAt(0) == '-') {
        switch (arg.charAt(1)) {
        case 'a':
            if (i + 1 < n) {
                host = args[++i];
                int pIndex = host.indexOf(':');
                if (pIndex > 0) {
                    port = Integer.parseInt(host.substring(pIndex + 1));
                    host = host.substring(0, pIndex);
                }
              } else {
                usage(arg);
              }
              break;
        case 'c':
          if (i + 1 < n) {
            command = args[++i];
          } else {
            usage(arg);
          }
          break;
        case 'd':
            if (i + 1 < n) {
            	comPort = args[++i];
            } else {
            	usage(arg);
            }
            System.out.println("Mao Com port " + comPort);
            break;
        case 'h':
            usage(null);
            break;
        case 'i':
          if (i + 1 < n) {
            interval = Integer.parseInt(args[++i]);
          } else {
            usage(arg);
          }
          break;
        case 'k':
        	if (i + 1 < n) {
        		random = Integer.parseInt(args[++i]);
            } else {
               usage(arg);
            }
        	break;
        case 'l':
        	if (i + 1 < n) {
        		reports = Integer.parseInt(args[++i]);
            } else {
               usage(arg);
            }
        	break;
        case 'm':
        	if (i + 1 < n) {
        		rexmits = Integer.parseInt(args[++i]);
            } else {
               usage(arg);
            }
        	break;
        case 'n':
            useSensorLog = false;
            break;

        case 'p':
            if (i + 1 < n) {
              port = Integer.parseInt(args[++i]);
            } else {
              usage(arg);
            }
            break;
        case 's':
        	if (i+1 < n) {
        		stack = args[++i];
        	} else {
        		usage(arg);
        	}
        case 'r':
            resetSensorLog = true;
            break;
        default:
          usage(arg);
          break;
        }
      } else {
        usage(arg);
      }
    }

    CollectServer server = new CollectServer();
    SerialConnection serialConnection;

    if (command == null) {
      serialConnection = new SerialDumpConnection(server);
    } else if (command == STDIN_COMMAND) {
      serialConnection = new StdinConnection(server);
    } else {
      serialConnection = new CommandConnection(server, command);
    }

    server.isSensorLogUsed = useSensorLog;
    if (useSensorLog && resetSensorLog) {
      server.clearSensorDataLog();
    }
    server.start(serialConnection, host, port, stack, comPort, interval, random, reports, rexmits);
  }

  private static void usage(String arg) {
    if (arg != null) {
      System.err.println("Unknown argument '" + arg + '\'');
    }
    System.err.println("Usage: java CollectServer [-a host:port] [-c command] [-d COMPORT] [-i] [-k] [-l] [-m] [-n] [-r] [-s stack] ");
    System.err.println("       -n : Do not read or save sensor data log");
    System.err.println("       -r : Clear any existing sensor data log at startup");
    System.err.println("       -i : interval value in seconds");
    System.err.println("       -k : random value in seconds");
    System.err.println("       -l : reports value in seconds. reports = 0 means infinite.");
    System.err.println("       -m : rexmits value in seconds");
    System.err.println("       -a : Connect to specified host:port. Default value is localhost:6789.");
    System.err.println("       -p : Read data from specified UDP port");
    System.err.println("       -c : Use specified command for serial data input/output");
    System.err.println("       -d : The serial port to connect to, default is /dev/ttyUSB0");
    System.err.println("       -s : Network Stack will be used. stack is either RIME or RPL. RIME is default value.\n");
    System.err.println("Example for RIME netstack:");
    System.err.println("java -jar collect-view.jar -a 10.11.1.208:6789 -i 60 -k 60 -l 0 -m 31 \n");
    System.err.println("Start SBAN and report sensor data to server at 10.11.1.208:6789.");
    System.err.println("Sensor data will be collected randomly with interval 60 seconds, radom 60 seconds, retransmit 31 and nonstop.\n");
    System.err.println("Example for RPL netstack:");
    System.err.println("java -jar collect-view.jar -a 10.11.1.208:6789 -s RPL -d /dev/ttyUSB1\n");
    System.err.println("Start SBAN gateway in RPL stack at comport=/dev/ttyUSB1.");
    System.err.println("And transmit collected data to the server 10.11.1.208:6789.\n");

    System.exit(arg != null ? 1 : 0);
  }
}