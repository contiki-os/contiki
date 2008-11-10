/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: CollectServer.java,v 1.9 2008/11/10 21:14:20 adamdunkels Exp $
 *
 * -----------------------------------------------------------------
 *
 * CollectServer
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 * Updated : $Date: 2008/11/10 21:14:20 $
 *           $Revision: 1.9 $
 */

package se.sics.contiki.collect;
import java.awt.BorderLayout;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
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
import java.util.Properties;
import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.SwingUtilities;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.ValueAxis;
import se.sics.contiki.collect.gui.BarChartPanel;
import se.sics.contiki.collect.gui.MapPanel;
import se.sics.contiki.collect.gui.SerialConsole;
import se.sics.contiki.collect.gui.TimeChartPanel;

/**
 *
 */
public class CollectServer {

  public static final String WINDOW_TITLE = "Sensor Data Collect with Contiki";

  public static final String CONFIG_FILE = "collect.conf";
  public static final String SENSORDATA_FILE = "sensordata.log";
  public static final String CONFIG_DATA_FILE = "collect-data.conf";
  public static final String INIT_SCRIPT = "collect-init.script";
  public static final String FIRMWARE_FILE = "sky-shell.ihex";

  private Properties config = new Properties();

  private String configFile;
  private Properties configTable = new Properties();

  private ArrayList<SensorData> sensorDataList = new ArrayList<SensorData>();
  private PrintWriter sensorDataOutput;

  private Hashtable<String,Node> nodeTable = new Hashtable<String,Node>();
  private Node[] nodeCache;

  private JFrame window;
  private JTabbedPane mainPanel;
  private JMenuItem serialItem;
  private JMenuItem runInitScriptItem;

  private Visualizer[] visualizers;
  private MapPanel mapPanel;
  private SerialConsole serialConsole;
  private JFileChooser fileChooser;

  private JList nodeList;
  private DefaultListModel nodeModel;
  private Node[] selectedNodes;

  private SerialConnection serialConnection;
  private String initScript;

  @SuppressWarnings("serial")
  public CollectServer(String comPort) {
    loadConfig(config, CONFIG_FILE);

    this.configFile = config.getProperty("config.datafile", CONFIG_DATA_FILE);
    if (this.configFile != null) {
      loadConfig(configTable, this.configFile);
    }
    if (comPort == null) {
      comPort = configTable.getProperty("collect.serialport");
    }
    this.initScript = config.getProperty("init.script", INIT_SCRIPT);

    /* Make sure we have nice window decorations */
//    JFrame.setDefaultLookAndFeelDecorated(true);
//    JDialog.setDefaultLookAndFeelDecorated(true);
    Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment()
        .getMaximumWindowBounds();

    /* Create and set up the window */
    window = new JFrame(WINDOW_TITLE + " (not connected)");
    window.setLocationByPlatform(true);
    if (maxSize != null) {
      window.setMaximizedBounds(maxSize);
    }
    window.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    window.addWindowListener(new WindowAdapter() {

      public void windowClosing(WindowEvent e) {
        exit();
      }
    });

    nodeModel = new DefaultListModel();
    nodeList = new JList(nodeModel);
    nodeList.setPrototypeCellValue("Node 88888");
    nodeList.addListSelectionListener(new ListSelectionListener() {

      @Override
      public void valueChanged(ListSelectionEvent e) {
        if (!e.getValueIsAdjusting() && e.getSource() == nodeList) {
          Node[] selected;
          int iMin = nodeList.getMinSelectionIndex();
          int iMax = nodeList.getMaxSelectionIndex();
          if ((iMin < 0) || (iMax < 0)) {
            selected = null;
          } else {
            Node[] tmp = new Node[1 + (iMax - iMin)];
            int n = 0;
            for(int i = iMin; i <= iMax; i++) {
              if (nodeList.isSelectedIndex(i)) {
                tmp[n++] = (Node) nodeModel.getElementAt(i);
              }
            }
            if (n != tmp.length) {
              Node[] t = new Node[n];
              System.arraycopy(tmp, 0, t, 0, n);
              tmp = t;
            }
            selected = tmp;
          }
          selectNodes(selected, false);
        }

      }});
    nodeList.setBorder(BorderFactory.createTitledBorder("Nodes"));
    window.getContentPane().add(new JScrollPane(nodeList), BorderLayout.WEST);

    mainPanel = new JTabbedPane();
    mainPanel.setBackground(nodeList.getBackground());
    mainPanel.setTabLayoutPolicy(JTabbedPane.WRAP_TAB_LAYOUT);

    serialConsole = new SerialConsole(this);
    mapPanel = new MapPanel(this);
    String image = getConfig("collect.mapimage");
    if (image != null) {
      mapPanel.setMapBackground(image);
    }
    final int defaultMaxItemCount = 250;
    visualizers = new Visualizer[] {
        mapPanel,
        new BarChartPanel(this, "Average Power", "Average Power Consumption", null, "Power (mW)",
            new String[] { "LPM", "CPU", "Radio listen", "Radio transmit" }) {
          {
            ValueAxis axis = chart.getCategoryPlot().getRangeAxis();
            axis.setLowerBound(0.0);
            axis.setUpperBound(75.0);
          }
          protected void addSensorData(SensorData data) {
            Node node = data.getNode();
            String nodeName = node.getName();
            SensorDataAggregator aggregator = node.getSensorDataAggregator();
            dataset.addValue(aggregator.getLPMPower(), categories[0], nodeName);
            dataset.addValue(aggregator.getCPUPower(), categories[1], nodeName);
            dataset.addValue(aggregator.getListenPower(), categories[2], nodeName);
            dataset.addValue(aggregator.getTransmitPower(), categories[3], nodeName);
          }
        },
        new BarChartPanel(this, "Instantaneous Power", "Instantaneous Power Consumption", null, "Power (mW)",
            new String[] { "LPM", "CPU", "Radio listen", "Radio transmit" }) {
          {
            ValueAxis axis = chart.getCategoryPlot().getRangeAxis();
            axis.setLowerBound(0.0);
            axis.setUpperBound(75.0);
          }
          protected void addSensorData(SensorData data) {
            Node node = data.getNode();
            String nodeName = node.getName();
            dataset.addValue(data.getLPMPower(), categories[0], nodeName);
            dataset.addValue(data.getCPUPower(), categories[1], nodeName);
            dataset.addValue(data.getListenPower(), categories[2], nodeName);
            dataset.addValue(data.getTransmitPower(), categories[3], nodeName);
          }
        },
        new TimeChartPanel(this, "Power History", "Historical Power Consumption", "Time", "mW") {
          {
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getAveragePower();
          }
        },
        new BarChartPanel(this, "Average Temperature", "Temperature", null, "Celsius",
            new String[] { "Celsius" }) {
          {
            chart.getCategoryPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected void addSensorData(SensorData data) {
            Node node = data.getNode();
            String nodeName = node.getName();
            SensorDataAggregator aggregator = node.getSensorDataAggregator();
            dataset.addValue(aggregator.getAverageTemperature(), categories[0], nodeName);
          }
        },
        new TimeChartPanel(this, "Temperature", "Temperature", "Time", "Celsius") {
          {
            chart.getXYPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
            setRangeTick(5);
            setRangeMinimumSize(10.0);
            setGlobalRange(true);
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getTemperature();
          }
        },
        new TimeChartPanel(this, "Battery Voltage", "Battery Voltage",
			   "Time", "Volt") {
          {
            setRangeTick(1);
	    setRangeMinimumSize(4.0);
	    setGlobalRange(true);
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getBatteryVoltage();
          }
        },
        new TimeChartPanel(this, "Battery Indicator", "Battery Indicator",
			   "Time", "Indicator") {
          {
            chart.getXYPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
            setRangeTick(5);
            setRangeMinimumSize(10.0);
            setGlobalRange(true);
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getBatteryIndicator();
          }
        },
        new TimeChartPanel(this, "Relative Humidity", "Humidity", "Time", "%") {
          {
            setMaxItemCount(defaultMaxItemCount);
            chart.getXYPlot().getRangeAxis().setRange(0.0, 100.0);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getHumidity();
          }
        },
        new TimeChartPanel(this, "Light 1", "Light 1", "Time", "-") {
          {
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getLight1();
          }
        },
        new TimeChartPanel(this, "Light 2", "Light 2", "Time", "-") {
          {
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getLight2();
          }
        },
        new TimeChartPanel(this, "Network Hops", "Network Hops", "Time", "Hops") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            axis.setLowerBound(0.0);
	    axis.setUpperBound(4.0);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getValue(SensorData.HOPS);
          }
        },
        new BarChartPanel(this, "Network Hops", "Network Hops", null, "Hops",
            new String[] { "Hops" }) {
          {
            chart.getCategoryPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected void addSensorData(SensorData data) {
            dataset.addValue(data.getValue(SensorData.HOPS), categories[0], data.getNode().getName());
          }
        },
        new TimeChartPanel(this, "Latency", "Latency", "Time", "Seconds") {
          {
            setMaxItemCount(defaultMaxItemCount);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getLatency();
          }
        },
        serialConsole
    };
    for (int i = 0, n = visualizers.length; i < n; i++) {
      mainPanel.add(visualizers[i].getTitle(), visualizers[i].getPanel());
    }
    window.getContentPane().add(mainPanel, BorderLayout.CENTER);

    // Setup menu
    JMenuBar menuBar = new JMenuBar();

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(fileMenu);
    serialItem = new JMenuItem("Connect to serial");
    serialItem.addActionListener(new SerialItemHandler());
    fileMenu.add(serialItem);
    JMenuItem item = new JMenuItem("Program Sky nodes...");
    item.addActionListener(new ProgramItemHandler());
    fileMenu.add(item);

    fileMenu.addSeparator();
    final JMenuItem clearMapItem = new JMenuItem("Remove Map Background");
    clearMapItem.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        mapPanel.setMapBackground(null);
        clearMapItem.setEnabled(false);
        configTable.remove("collect.mapimage");
      }

    });
    clearMapItem.setEnabled(mapPanel.getMapBackground() != null);

    item = new JMenuItem("Select Map Background...");
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        if (fileChooser == null) {
          fileChooser = new JFileChooser();
          int reply = fileChooser.showOpenDialog(window);
          if (reply == JFileChooser.APPROVE_OPTION) {
            File file = fileChooser.getSelectedFile();
            String name = file.getAbsolutePath();
            configTable.put("collect.mapimage", name);
            if (!mapPanel.setMapBackground(file.getAbsolutePath())) {
              JOptionPane.showMessageDialog(window, "Failed to set background image", "Error", JOptionPane.ERROR_MESSAGE);
            }
            clearMapItem.setEnabled(mapPanel.getMapBackground() != null);
            saveConfig(configTable, configFile);
          }
        }
      }

    });
    fileMenu.add(item);
    fileMenu.add(clearMapItem);

    fileMenu.addSeparator();
    item = new JMenuItem("Clear Sensor Data...");
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        int reply = JOptionPane.showConfirmDialog(window, "Also clear the sensor data log file?");
        if (reply == JOptionPane.YES_OPTION) {
          // Clear data from both memory and sensor log file
          clearSensorDataLog();
          clearSensorData();
        } else if (reply == JOptionPane.NO_OPTION) {
          // Only clear data from memory
          clearSensorData();
        }
      }

    });
    fileMenu.add(item);

    fileMenu.addSeparator();
    item = new JMenuItem("Exit", KeyEvent.VK_X);
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        exit();
      }

    });
    fileMenu.add(item);

    JMenu toolsMenu = new JMenu("Tools");
    toolsMenu.setMnemonic(KeyEvent.VK_T);
    menuBar.add(toolsMenu);

    runInitScriptItem = new JMenuItem("Run Init Script");
    runInitScriptItem.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        mainPanel.setSelectedComponent(serialConsole.getPanel());
        if (serialConnection != null && serialConnection.isOpen()) {
          runInitScript();
        } else {
          JOptionPane.showMessageDialog(mainPanel, "No serial port connection", "No connected node", JOptionPane.ERROR_MESSAGE);
        }
      }

    });
    runInitScriptItem.setEnabled(false);
    toolsMenu.add(runInitScriptItem);
    toolsMenu.addSeparator();

    final JCheckBoxMenuItem scrollItem = new JCheckBoxMenuItem("Scroll Layout");
    scrollItem.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        if (scrollItem.getState()) {
          mainPanel.setTabLayoutPolicy(JTabbedPane.SCROLL_TAB_LAYOUT);
        } else {
          mainPanel.setTabLayoutPolicy(JTabbedPane.WRAP_TAB_LAYOUT);
        }
      }

    });
    toolsMenu.add(scrollItem);

    window.setJMenuBar(menuBar);
    window.pack();

    String bounds = configTable.getProperty("collect.bounds");
    if (bounds != null) {
      String[] b = bounds.split(",");
      if (b.length == 4) {
        window.setBounds(Integer.parseInt(b[0]), Integer.parseInt(b[1]),
            Integer.parseInt(b[2]), Integer.parseInt(b[3]));
      }
    }

    for(Object key: configTable.keySet()) {
      String property = key.toString();
      if (!property.startsWith("collect")) {
        getNode(property, true);
      }
    }
    initSensorData();

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        window.setVisible(true);
      }
    });

    serialConnection = new SerialConnection() {

      private boolean hasOpened;
      private boolean hasSentInit;

      @Override
      protected void serialOpened() {
        serialConsole.addSerialData("*** Serial console listening on port: " + getComPort() + " ***");
        hasOpened = true;
        // Remember the last selected serial port
        configTable.put("collect.serialport", getComPort());
        setSystemMessage("connected to " + getComPort());

        // Send any initial commands
        if (!hasSentInit) {
          hasSentInit = true;

          if (hasInitScript()) {
            // Wait a short time before running the init script
            sleep(3000);

            runInitScript();
          }
        }
      }

      @Override
      protected void serialClosed() {
        String comPort = getComPort();
        String prefix;
        if (hasOpened) {
          serialConsole.addSerialData("*** Serial connection terminated ***");
          prefix = "Serial connection terminated.\n";
          hasOpened = false;
          setSystemMessage("not connected");
        } else {
          prefix = "Failed to connect to " + getComPort() + '\n';
        }
        if (!isClosed) {
          String options[] = {"Retry", "Search for connected nodes", "Cancel"};
          int value = JOptionPane.showOptionDialog(window,
              prefix + "Do you want to retry or search for connected nodes?",
              "Reconnect to serial port?",
              JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
              null, options, options[0]);
          if (value == JOptionPane.CLOSED_OPTION || value == 2) {
//          exit();
          } else {
            if (value == 1) {
              // Select new serial port
              comPort = MoteFinder.selectComPort(window);
              if (comPort == null) {
//              exit();
              }
            }
            // Try to open com port again
            if (comPort != null) {
              open(comPort);
            }
          }
        }
      }

      @Override
      protected void serialData(String line) {
        parseIncomingLine(System.currentTimeMillis(), line);
      }

    };
    if (comPort != null) {
      serialConnection.setComPort(comPort);
    }
    connectToSerial();
  }

  protected void connectToSerial() {
    if (!serialConnection.isOpen()) {
      String comPort = serialConnection.getComPort();
      if (comPort == null) {
        comPort = MoteFinder.selectComPort(window);
      }
      if (comPort != null) {
        serialConnection.open(comPort);
      }
    }
  }

  private void exit() {
    /* TODO Clean up resources */
    if (configFile != null) {
      configTable.setProperty("collect.bounds", "" + window.getX() + ',' + window.getY() + ',' + window.getWidth() + ',' + window.getHeight());
      saveConfig(configTable, configFile);
    }
    if (serialConnection != null) {
      serialConnection.close();
    }
    PrintWriter output = this.sensorDataOutput;
    if (output != null) {
      output.close();
    }
    System.exit(0);
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

  protected void setSystemMessage(final String message) {
    SwingUtilities.invokeLater(new Runnable() {

      public void run() {
        boolean isOpen = serialConnection.isOpen();
        if (message == null) {
          window.setTitle(WINDOW_TITLE);
        } else {
          window.setTitle(WINDOW_TITLE + " (" + message + ')');
        }
        serialItem.setText(isOpen ? "Disconnect from serial" : "Connect to serial");
        runInitScriptItem.setEnabled(isOpen && hasInitScript());
      }

    });
  }

  // -------------------------------------------------------------------
  // Node Handling
  // -------------------------------------------------------------------

  public synchronized Node[] getNodes() {
    if (nodeCache == null) {
      Node[] tmp = nodeTable.values().toArray(new Node[nodeTable.size()]);
      Arrays.sort(tmp);
      nodeCache = tmp;
    }
    return nodeCache;
  }

  public Node addNode(String nodeID) {
    return getNode(nodeID, true);
  }

  private Node getNode(final String nodeID, boolean notify) {
    Node node = nodeTable.get(nodeID);
    if (node == null) {
      node = new Node(nodeID);
      nodeTable.put(nodeID, node);
      updateNodeLocation(node);

      synchronized (this) {
        nodeCache = null;
      }

      if (notify) {
        final Node newNode = node;
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              // Insert the node sorted by name
              String nodeName = newNode.getName();
              boolean added = false;
              for (int i = 0, n = nodeModel.size(); i < n; i++) {
                int cmp = nodeName.compareTo(((Node) nodeModel.get(i)).getName());
                if (cmp < 0) {
                  nodeModel.insertElementAt(newNode, i);
                  added = true;
                  break;
                } else if (cmp == 0) {
                  // node already added
                  added = true;
                  break;
                }
              }
              if (!added) {
                nodeModel.addElement(newNode);
              }
              if (visualizers != null) {
                for (int i = 0, n = visualizers.length; i < n; i++) {
                  visualizers[i].nodeAdded(newNode);
                }
              }
            }
          });
      }
    }
    return node;
  }

   public void selectNodes(Node[] nodes) {
     selectNodes(nodes, true);
   }

   private void selectNodes(Node[] nodes, boolean updateList) {
    if (nodes != selectedNodes) {
      selectedNodes = nodes;
      if (updateList) {
        nodeList.clearSelection();
        if (selectedNodes != null) {
          for (int i = 0, n = selectedNodes.length; i < n; i++) {
            int index = nodeModel.indexOf(selectedNodes[i]);
            if (index >= 0) {
              nodeList.addSelectionInterval(index, index);
            }
          }
        }
      }
      if (visualizers != null) {
        for (int i = 0, n = visualizers.length; i < n; i++) {
          visualizers[i].nodesSelected(nodes);
        }
      }
    }
  }


  // -------------------------------------------------------------------
  // Node location handling
  // -------------------------------------------------------------------

  public boolean updateNodeLocation(Node node) {
    String id = node.getID();
    if (node.hasLocation()) {
      String location = "" + node.getX() + ',' + node.getY();
      if (!location.equals(configTable.get(id))) {
        configTable.put(id, location);
      }
      return false;
    }

    String location = configTable.getProperty(id);
    if (location != null) {
      try {
        String[] pos = location.split(",");
        node.setLocation(Integer.parseInt(pos[0].trim()),
            Integer.parseInt(pos[1].trim()));
        return true;
      } catch (Exception e) {
        System.err.println("could not parse node location: " + location);
        e.printStackTrace();
      }
    }
    return false;
  }

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


  // -------------------------------------------------------------------
  // Serial communication
  // -------------------------------------------------------------------

  public boolean sendToNode(String data) {
    if (serialConnection != null && serialConnection.isOpen()) {
      serialConsole.addSerialData("SEND: " + data);
      serialConnection.writeSerialData(data);
      return true;
    }
    return false;
  }

  protected void parseIncomingLine(long systemTime, String line) {
    SensorData sensorData = SensorData.parseSensorData(this, line, systemTime);
    if (sensorData != null) {
      // Sensor data received
      handleSensorData(sensorData);
      return;
    }
    System.out.println("SERIAL: " + line);
    serialConsole.addSerialData(line);
  }


  // -------------------------------------------------------------------
  // SensorData handling
  // -------------------------------------------------------------------

  public int getSensorDataCount() {
    return sensorDataList.size();
  }

  public SensorData getSensorData(int i) {
    return sensorDataList.get(i);
  }

  private void handleSensorData(final SensorData sensorData) {
    System.out.println("SENSOR DATA: " + sensorData);
    if (sensorData.getNode().addSensorData(sensorData)) {
      sensorDataList.add(sensorData);
      saveSensorData(sensorData);
      handleLinks(sensorData);
      if (visualizers != null) {
        SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            for (int i = 0, n = visualizers.length; i < n; i++) {
              visualizers[i].nodeDataReceived(sensorData);
            }
          }
        });
      }
    }
  }

  private void handleLinks(SensorData sensorData) {
    String nodeID = sensorData.getBestNeighborID();
    if (nodeID != null) {
      Node neighbor = addNode(nodeID);
      Node source = sensorData.getNode();
      Link link = source.getLink(neighbor);
      link.setETX(sensorData.getBestNeighborETX());
      link.setLastActive(sensorData.getNodeTime());
    }
  }

  private void initSensorData() {
    loadSensorData(SENSORDATA_FILE);
  }

  private boolean loadSensorData(String filename) {
    File fp = new File(filename);
    if (fp.exists() && fp.canRead()) {
      BufferedReader in = null;
      try {
        in = new BufferedReader(new FileReader(fp));
        String line;
        int no = 0;
        while ((line = in.readLine()) != null) {
          no++;
          if (line.length() == 0 || line.charAt(0) == '#') {
            // Ignore empty lines and comments
          } else {
            SensorData data = SensorData.parseSensorData(this, line);
            if (data != null) {
              if (data.getNode().addSensorData(data)) {
                sensorDataList.add(data);
                handleLinks(data);
              }
            } else {
              // TODO exit here?
              System.err.println("Failed to parse sensor data from log line " + no + ": " + line);
            }
          }
        }
        in.close();
      } catch (IOException e) {
        System.err.println("Failed to read sensor data log from " + fp.getAbsolutePath());
        e.printStackTrace();
        return false;
      }
    }
    return true;
  }

  private void saveSensorData(SensorData data) {
    PrintWriter output = this.sensorDataOutput;
    if (output == null) {
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
    if (nodes != null) {
      for(Node node : nodes) {
        node.removeAllSensorData();
      }
    }
    if (visualizers != null) {
      for(Visualizer v : visualizers) {
        v.clearNodeData();
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

  protected class SerialItemHandler implements ActionListener, Runnable {

    private boolean isRunning;

    public void actionPerformed(ActionEvent e) {
      if (!isRunning) {
        isRunning = true;
        new Thread(this, "serial").start();
      }
    }

    public void run() {
      try {
        if (serialConnection != null && serialConnection.isOpen()) {
          serialConnection.close();
        } else {
          connectToSerial();
        }
      } finally {
        isRunning = false;
      }
    }

  }

  protected class ProgramItemHandler implements ActionListener, Runnable {

    private boolean isRunning = false;
    public void actionPerformed(ActionEvent e) {
      if (!isRunning) {
        isRunning = true;
        new Thread(this, "program thread").start();
      }
    }

    @Override
    public void run() {
      try {
        MoteProgrammer mp = new MoteProgrammer();
        mp.setParentComponent(window);
        mp.setFirmwareFile(FIRMWARE_FILE);
        mp.searchForMotes();
        String[] motes = mp.getMotes();
        if (motes == null || motes.length == 0) {
          JOptionPane.showMessageDialog(window, "Could not find any connected Sky nodes", "Error", JOptionPane.ERROR_MESSAGE);
          return;
        }
        int reply = JOptionPane.showConfirmDialog(window, "Found " + motes.length + " connected Sky nodes.\n"
            + "Do you want to upload the firmware " + FIRMWARE_FILE + '?');
        if (reply == JFileChooser.APPROVE_OPTION) {
          boolean wasOpen = serialConnection.isOpen();
          serialConnection.close();
          if (wasOpen) {
            Thread.sleep(1000);
          }
          mp.programMotes();
          mp.waitForProcess();
          if (wasOpen) {
            connectToSerial();
          }
        }
      } catch (Exception e) {
        e.printStackTrace();
        JOptionPane.showMessageDialog(window, "Programming failed: " + e, "Error", JOptionPane.ERROR_MESSAGE);
      } finally {
        isRunning = false;
      }
    }

  }

  // -------------------------------------------------------------------
  // Main
  // -------------------------------------------------------------------

  public static void main(String[] args) {
    String comPort = null;
    if (args.length > 0) {
      if (args.length > 1 || args[0].startsWith("-h")) {
        System.err.println("Usage: java CollectServer COMPORT");
        System.exit(1);
      }
      comPort = args[0];
    }
    new CollectServer(comPort);
  }

}
