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
 * -----------------------------------------------------------------
 *
 * CollectServer
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 3 jul 2008
 */

package se.sics.contiki.collect;
import java.awt.BorderLayout;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.awt.Toolkit;
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
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.DefaultListModel;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.ListCellRenderer;
import javax.swing.SwingUtilities;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.axis.ValueAxis;
import se.sics.contiki.collect.gui.AggregatedTimeChartPanel;
import se.sics.contiki.collect.gui.BarChartPanel;
import se.sics.contiki.collect.gui.MapPanel;
import se.sics.contiki.collect.gui.NodeControl;
import se.sics.contiki.collect.gui.NodeInfoPanel;
import se.sics.contiki.collect.gui.SerialConsole;
import se.sics.contiki.collect.gui.TimeChartPanel;

/**
 *
 */
public class CollectServer implements SerialConnectionListener {

  public static final String WINDOW_TITLE = "Sensor Data Collect with Contiki";
  public static final String STDIN_COMMAND = "<STDIN>";

  public static final String CONFIG_FILE = "collect.conf";
  public static final String SENSORDATA_FILE = "sensordata.log";
  public static final String CONFIG_DATA_FILE = "collect-data.conf";
  public static final String INIT_SCRIPT = "collect-init.script";
  public static final String FIRMWARE_FILE = "collect-view-shell.ihex";

  /* Categories for the tab pane */
  private static final String MAIN = "main";
  private static final String NETWORK = "Network";
  private static final String SENSORS = "Sensors";
  private static final String POWER = "Power";

  private Properties config = new Properties();

  private String configFile;
  private Properties configTable = new Properties();

  private ArrayList<SensorData> sensorDataList = new ArrayList<SensorData>();
  private PrintWriter sensorDataOutput;
  private boolean isSensorLogUsed;

  private Hashtable<String,Node> nodeTable = new Hashtable<String,Node>();
  private Node[] nodeCache;

  private JFrame window;
  private JTabbedPane mainPanel;
  private HashMap<String,JTabbedPane> categoryTable = new HashMap<String,JTabbedPane>();
  private JMenuItem runInitScriptItem;

  private final Visualizer[] visualizers;
  private final MapPanel mapPanel;
  private final SerialConsole serialConsole;
  private final ConnectSerialAction connectSerialAction;
  private final MoteProgramAction moteProgramAction;
  private JFileChooser fileChooser;

  private JList nodeList;
  private DefaultListModel nodeModel;
  private Node[] selectedNodes;

  private SerialConnection serialConnection;
  private boolean hasSerialOpened;
  /* Do not auto send init script at startup */
  private boolean doSendInitAtStartup = false;
  private String initScript;

  private boolean hasStarted = false;
  private boolean doExitOnRequest = true;
  private JMenuItem exitItem;

  private int defaultMaxItemCount = 250;
  private long nodeTimeDelta;

  @SuppressWarnings("serial")
  public CollectServer() {
    loadConfig(config, CONFIG_FILE);

    this.configFile = config.getProperty("config.datafile", CONFIG_DATA_FILE);
    if (this.configFile != null) {
      loadConfig(configTable, this.configFile);
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

    moteProgramAction = new MoteProgramAction("Program Nodes...");
    connectSerialAction = new ConnectSerialAction("Connect to serial");

    nodeModel = new DefaultListModel();
    nodeModel.addElement("<All>");
    nodeList = new JList(nodeModel);
    nodeList.setPrototypeCellValue("888.888");
    nodeList.addListSelectionListener(new ListSelectionListener() {

      @Override
      public void valueChanged(ListSelectionEvent e) {
        if (!e.getValueIsAdjusting() && e.getSource() == nodeList) {
          Node[] selected;
          int iMin = nodeList.getMinSelectionIndex();
          int iMax = nodeList.getMaxSelectionIndex();
          if ((iMin < 0) || (iMax < 0)) {
            selected = null;
          } else if (nodeList.getSelectedIndex() == 0) {
            selected = getNodes();
            if (nodeModel.size() > 1) {
              nodeList.setSelectionInterval(1, nodeModel.size() - 1);
            }
          } else {
            Node[] tmp = new Node[1 + (iMax - iMin)];
            int n = 0;
            if (iMin < 1) {
              iMin = 1;
            }
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
    ListCellRenderer renderer = nodeList.getCellRenderer();
    if (renderer instanceof JLabel) {
      ((JLabel)renderer).setHorizontalAlignment(JLabel.CENTER);
    }
    window.getContentPane().add(new JScrollPane(nodeList), BorderLayout.WEST);

    mainPanel = new JTabbedPane();
    mainPanel.setBackground(nodeList.getBackground());
    mainPanel.setTabLayoutPolicy(JTabbedPane.WRAP_TAB_LAYOUT);
    categoryTable.put(MAIN, mainPanel);

    serialConsole = new SerialConsole(this, MAIN);
    mapPanel = new MapPanel(this, "Sensor Map", MAIN, true);
    String image = getConfig("collect.mapimage");
    if (image != null) {
      mapPanel.setMapBackground(image);
    }
    NodeControl nodeControl = new NodeControl(this, MAIN);

    visualizers = new Visualizer[] {
        nodeControl,
        mapPanel,
        new MapPanel(this, "Network Graph", MAIN, false),
        new BarChartPanel(this, SENSORS, "Average Temperature", "Temperature", "Nodes", "Celsius",
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
        new TimeChartPanel(this, SENSORS, "Temperature", "Temperature", "Time", "Celsius") {
          {
            chart.getXYPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
            setRangeTick(5);
            setRangeMinimumSize(10.0);
            setGlobalRange(true);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getTemperature();
          }
        },
        new TimeChartPanel(this, SENSORS, "Battery Voltage", "Battery Voltage",
			   "Time", "Volt") {
          {
            setRangeTick(1);
	    setRangeMinimumSize(4.0);
	    setGlobalRange(true);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getBatteryVoltage();
          }
        },
        new TimeChartPanel(this, SENSORS, "Battery Indicator", "Battery Indicator",
			   "Time", "Indicator") {
          {
            chart.getXYPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
            setRangeTick(5);
            setRangeMinimumSize(10.0);
            setGlobalRange(true);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getBatteryIndicator();
          }
        },
        new TimeChartPanel(this, SENSORS, "Relative Humidity", "Humidity", "Time", "%") {
          {
            chart.getXYPlot().getRangeAxis().setRange(0.0, 100.0);
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getHumidity();
          }
        },
        new TimeChartPanel(this, SENSORS, "Light 1", "Light 1", "Time", "-") {
          protected double getSensorDataValue(SensorData data) {
            return data.getLight1();
          }
        },
        new TimeChartPanel(this, SENSORS, "Light 2", "Light 2", "Time", "-") {
          protected double getSensorDataValue(SensorData data) {
            return data.getLight2();
          }
        },
        new TimeChartPanel(this, NETWORK, "Neighbors", "Neighbor Count", "Time", "Neighbors") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getValue(SensorData.NUM_NEIGHBORS);
          }
        },
        new TimeChartPanel(this, NETWORK, "Beacon Interval", "Beacon interval", "Time", "Interval (s)") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getValue(SensorData.BEACON_INTERVAL);
          }
        },
        new TimeChartPanel(this, NETWORK, "Network Hops (Over Time)", "Network Hops", "Time", "Hops") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getValue(SensorData.HOPS);
          }
        },
        new BarChartPanel(this, NETWORK, "Network Hops (Per Node)", "Network Hops", "Nodes", "Hops",
            new String[] { "Last Hop", "Average Hops" }, false) {
          {
            chart.getCategoryPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected void addSensorData(SensorData data) {
            dataset.addValue(data.getValue(SensorData.HOPS), categories[0], data.getNode().getName());
            dataset.addValue(data.getNode().getSensorDataAggregator().getAverageValue(SensorData.HOPS), categories[1], data.getNode().getName());
          }
        },
        new TimeChartPanel(this, NETWORK, "Routing Metric (Over Time)", "Routing Metric", "Time", "Routing Metric") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getValue(SensorData.RTMETRIC);
          }
        },
        new AggregatedTimeChartPanel<boolean[]>(this, NETWORK, "Avg Routing Metric (Over Time)", "Time",
                "Average Routing Metric") {
            private int nodeCount;
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          @Override
          protected boolean[] createState(Node node) {
            return new boolean[1];
          }
          @Override
          protected void clearState(Map<Node,boolean[]> map) {
            nodeCount = 0;
            for(boolean[] value : map.values()) {
              value[0] = false;
            }
          }
          @Override
          protected String getTitle(int selectedCount, int dataCount, int duplicateCount) {
            return "Average Routing Metric (" + dataCount + " packets from " + nodeCount + " node"
                + (nodeCount > 1 ? "s" : "") + ')';
          }
          @Override
          protected int getTotalDataValue(int value) {
            // Return average value
            return nodeCount > 0 ? (value / nodeCount) : value;
          }
          @Override
          protected int getSensorDataValue(SensorData data, boolean[] nodeState) {
            if (!nodeState[0]) {
              nodeCount++;
              nodeState[0] = true;
            }
            return data.getValue(SensorData.RTMETRIC);
          }
        },
        new TimeChartPanel(this, NETWORK, "ETX (Over Time)", "ETX to Next Hop", "Time", "ETX") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected double getSensorDataValue(SensorData data) {
            return data.getBestNeighborETX();
          }
        },
        new AggregatedTimeChartPanel<int[]>(this, NETWORK,
            "Next Hop (Over Time)", "Time", "Next Hop Changes") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          @Override
          protected int[] createState(Node node) {
            return new int[1];
          }
          @Override
          protected void clearState(Map<Node,int[]> map) {
            for(int[] value : map.values()) {
              value[0] = 0;
            }
          }
          @Override
          protected int getSensorDataValue(SensorData sd, int[] nodeState) {
            boolean hasBest = nodeState[0] != 0;
            int bestNeighbor = sd.getValue(SensorData.BEST_NEIGHBOR);
            if (bestNeighbor != 0 && bestNeighbor != nodeState[0]) {
              nodeState[0] = bestNeighbor;
              return hasBest ? 1 : 0;
            }
            return 0;
          }
        },
        new TimeChartPanel(this, NETWORK, "Latency", "Latency", "Time", "Seconds") {
          protected double getSensorDataValue(SensorData data) {
            return data.getLatency();
          }
        },
        new AggregatedTimeChartPanel<Node>(this, NETWORK,
            "Received (Over Time)", "Time", "Received Packets") {
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          @Override
          protected String getTitle(int nodeCount, int dataCount, int duplicateCount) {
            return "Received " + dataCount + " packets from " + nodeCount + " node"
                + (nodeCount > 1 ? "s" : "")
                + (duplicateCount > 0 ? (" (" + duplicateCount + " duplicates)") : "");
          }
          @Override
          protected Node createState(Node node) {
            return node;
          }
          @Override
          protected int getSensorDataValue(SensorData sd, Node node) {
            return 1;
          }
        },
        new AggregatedTimeChartPanel<int[]>(this, NETWORK,
            "Lost (Over Time)", "Time", "Estimated Lost Packets") {
          private int totalLost;
          {
            ValueAxis axis = chart.getXYPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
            axis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          @Override
          protected String getTitle(int nodeCount, int dataCount, int duplicateCount) {
            return "Received " + dataCount + " packets from " + nodeCount
                + " node" + (nodeCount > 1 ? "s" : "") + ". Estimated "
                + totalLost + " lost packet" + (totalLost == 1 ? "" : "s")
                + '.';
          }
          @Override
          protected int[] createState(Node node) {
            return new int[1];
          }
          @Override
          protected void clearState(Map<Node,int[]> map) {
            totalLost = 0;
            for(int[] v : map.values()) {
              v[0] = 0;
            }
          }
          @Override
          protected int getSensorDataValue(SensorData sd, int[] nodeState) {
            int lastSeqno = nodeState[0];
            int seqno = sd.getSeqno();
            nodeState[0] = seqno;
            if (seqno > lastSeqno + 1 && lastSeqno != 0) {
              int estimatedLost = seqno - lastSeqno - 1;
              totalLost += estimatedLost;
              return estimatedLost;
            }
            return 0;
          }
        },
        new BarChartPanel(this, NETWORK, "Received (Per Node)", "Received Packets Per Node", "Nodes", "Packets",
            new String[] { "Packets", "Duplicates" }) {
          {
            chart.getCategoryPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected void addSensorData(SensorData data) {
            Node node = data.getNode();
            SensorDataAggregator sda = node.getSensorDataAggregator();
            dataset.addValue(sda.getDataCount(), categories[0], node.getName());
            dataset.addValue(sda.getDuplicateCount(), categories[1], node.getName());
          }
        },
        new BarChartPanel(this, NETWORK, "Received (5 min)", "Received Packets (last 5 min)", "Nodes", "Packets",
            new String[] { "Packets", "Duplicates" }) {
          {
            chart.getCategoryPlot().getRangeAxis().setStandardTickUnits(NumberAxis.createIntegerTickUnits());
          }
          protected void addSensorData(SensorData data) {
            Node node = data.getNode();
            int packetCount = 0;
            int duplicateCount = 0;
            long earliestData = System.currentTimeMillis() - (5 * 60 * 1000);
            for(int index = node.getSensorDataCount() - 1; index >= 0; index--) {
              SensorData sd = node.getSensorData(index);
              if (sd.getNodeTime() < earliestData) {
                break;
              }
              if (sd.isDuplicate()) {
                duplicateCount++;
              } else {
                packetCount++;
              }
            }
            dataset.addValue(packetCount, categories[0], node.getName());
            dataset.addValue(duplicateCount, categories[1], node.getName());
          }
        },
        new BarChartPanel(this, POWER, "Average Power", "Average Power Consumption",
            "Nodes", "Power (mW)",
            new String[] { "LPM", "CPU", "Radio listen", "Radio transmit" }) {
          {
            ValueAxis axis = chart.getCategoryPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
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
        new BarChartPanel(this, POWER, "Radio Duty Cycle", "Average Radio Duty Cycle",
            "Nodes", "Duty Cycle (%)",
            new String[] { "Radio listen", "Radio transmit" }) {
          {
            ValueAxis axis = chart.getCategoryPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
          }
          protected void addSensorData(SensorData data) {
            Node node = data.getNode();
            String nodeName = node.getName();
            SensorDataAggregator aggregator = node.getSensorDataAggregator();
            dataset.addValue(100 * aggregator.getAverageDutyCycle(SensorInfo.TIME_LISTEN),
                             categories[0], nodeName);
            dataset.addValue(100 * aggregator.getAverageDutyCycle(SensorInfo.TIME_TRANSMIT),
                             categories[1], nodeName);
          }
        },
        new BarChartPanel(this, POWER, "Instantaneous Power",
            "Instantaneous Power Consumption", "Nodes", "Power (mW)",
            new String[] { "LPM", "CPU", "Radio listen", "Radio transmit" }) {
          {
            ValueAxis axis = chart.getCategoryPlot().getRangeAxis();
            ((NumberAxis)axis).setAutoRangeIncludesZero(true);
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
        new TimeChartPanel(this, POWER, "Power History", "Historical Power Consumption", "Time", "mW") {
          protected double getSensorDataValue(SensorData data) {
            return data.getAveragePower();
          }
        },
        new NodeInfoPanel(this, MAIN),
        serialConsole
    };
    for (int i = 0, n = visualizers.length; i < n; i++) {
      String category = visualizers[i].getCategory();
      JTabbedPane pane = categoryTable.get(category);
      if (pane == null) {
        pane = new JTabbedPane();
        pane.setBackground(nodeList.getBackground());
        pane.setTabLayoutPolicy(JTabbedPane.WRAP_TAB_LAYOUT);
        categoryTable.put(category, pane);
        mainPanel.add(category, pane);
      }
      pane.add(visualizers[i].getTitle(), visualizers[i].getPanel());
    }
    JTabbedPane pane = categoryTable.get(nodeControl.getCategory());
    if (pane != null) {
      pane.setSelectedComponent(nodeControl.getPanel());
    }
    window.getContentPane().add(mainPanel, BorderLayout.CENTER);

    // Setup menu
    JMenuBar menuBar = new JMenuBar();

    JMenu fileMenu = new JMenu("File");
    fileMenu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(fileMenu);
    fileMenu.add(new JMenuItem(connectSerialAction));
    fileMenu.add(new JMenuItem(moteProgramAction));

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

    JMenuItem item = new JMenuItem("Select Map Background...");
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        if (fileChooser == null) {
          fileChooser = new JFileChooser();
        }
        int reply = fileChooser.showOpenDialog(window);
        if (reply == JFileChooser.APPROVE_OPTION) {
          File file = fileChooser.getSelectedFile();
          String name = file.getAbsolutePath();
          if (!mapPanel.setMapBackground(file.getAbsolutePath())) {
            JOptionPane.showMessageDialog(window, "Failed to set background image", "Error", JOptionPane.ERROR_MESSAGE);
          } else {
            configTable.put("collect.mapimage", name);
            save();
          }
          clearMapItem.setEnabled(mapPanel.getMapBackground() != null);
        }
      }

    });
    fileMenu.add(item);
    fileMenu.add(clearMapItem);

    item = new JMenuItem("Save Settings");
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        save();
        JOptionPane.showMessageDialog(window, "Settings have been saved.");
      }

    });
    fileMenu.add(item);

    fileMenu.addSeparator();
    item = new JMenuItem("Clear Sensor Data...");
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        int reply = isSensorLogUsed
          ? JOptionPane.showConfirmDialog(window, "Also clear the sensor data log file?")
          : JOptionPane.NO_OPTION;
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
    exitItem = new JMenuItem("Exit", KeyEvent.VK_X);
    exitItem.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        exit();
      }

    });
    fileMenu.add(exitItem);

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

    item = new JMenuItem("Set Max Item Count...");
    item.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        int value = getUserInputAsInteger("Specify Max Item Count",
            "Please specify max item count for the time charts.\n" +
            "Charts with more values will aggregate the values into fewer items.",
            defaultMaxItemCount);
        if (value > 0) {
          defaultMaxItemCount = value;
          if (visualizers != null) {
            for(Visualizer v : visualizers) {
              if (v instanceof TimeChartPanel) {
                ((TimeChartPanel)v).setMaxItemCount(defaultMaxItemCount);
              }
            }
          }
        }
      }

    });
    toolsMenu.add(item);

    final JCheckBoxMenuItem baseShapeItem = new JCheckBoxMenuItem("Base Shape Visible");
    baseShapeItem.setSelected(true);
    baseShapeItem.addActionListener(new ActionListener() {

      public void actionPerformed(ActionEvent e) {
        boolean visible = baseShapeItem.getState();
        if (visualizers != null) {
          for(Visualizer v : visualizers) {
            if (v instanceof TimeChartPanel) {
              ((TimeChartPanel)v).setBaseShapeVisible(visible);
            }
          }
        }
      }

    });
    toolsMenu.add(baseShapeItem);

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
  }

  private int getUserInputAsInteger(String title, String message, int defaultValue) {
    String s = (String)JOptionPane.showInputDialog(
      window, message, title, JOptionPane.PLAIN_MESSAGE, null, null, Integer.toString(defaultValue));
    if (s != null) {
      try {
        return Integer.parseInt(s);
      } catch (Exception e) {
        JOptionPane.showMessageDialog(window, "Illegal value", "Error", JOptionPane.ERROR_MESSAGE);
      }
    }
    return -1;
  }

  public void start(SerialConnection connection) {
    if (hasStarted) {
      throw new IllegalStateException("already started");
    }
    hasStarted = true;
    this.serialConnection = connection;
    if (isSensorLogUsed) {
      initSensorData();
    }
    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        window.setVisible(true);
      }
    });
    connectToSerial();
  }

  protected void connectToSerial() {
    if (serialConnection != null && !serialConnection.isOpen()) {
      String comPort = serialConnection.getComPort();
      if (comPort == null && serialConnection.isMultiplePortsSupported()) {
        comPort = MoteFinder.selectComPort(window);
      }
      if (comPort != null || !serialConnection.isMultiplePortsSupported()) {
        serialConnection.open(comPort);
      }
    }
  }

  public void stop() {
    save();
    if (serialConnection != null) {
      serialConnection.close();
    }
    PrintWriter output = this.sensorDataOutput;
    if (output != null) {
      output.close();
    }
    window.setVisible(false);
  }

  public void setUseSensorDataLog(boolean useSensorLog) {
    this.isSensorLogUsed = useSensorLog;
  }

  public void setExitOnRequest(boolean doExit) {
    this.doExitOnRequest = doExit;
    if (exitItem != null) {
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          exitItem.setEnabled(doExitOnRequest);
        }
      });
    }
  }

  private void exit() {
    if (doExitOnRequest) {
      stop();
      System.exit(0);
    } else {
      Toolkit.getDefaultToolkit().beep();
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

  public void setConfig(String property, String value) {
    configTable.setProperty(property, value);
  }

  public void removeConfig(String property) {
    configTable.remove(property);
  }

  public int getDefaultMaxItemCount() {
    return defaultMaxItemCount;
  }

  public Action getMoteProgramAction() {
    return moteProgramAction;
  }

  public Action getConnectSerialAction() {
    return connectSerialAction;
  }

  protected void setSystemMessage(final String message) {
    SwingUtilities.invokeLater(new Runnable() {

      public void run() {
        boolean isOpen = serialConnection != null && serialConnection.isOpen();
        if (message == null) {
          window.setTitle(WINDOW_TITLE);
        } else {
          window.setTitle(WINDOW_TITLE + " (" + message + ')');
        }
        connectSerialAction.putValue(ConnectSerialAction.NAME,
            isOpen ? "Disconnect from serial" : "Connect to serial");
        runInitScriptItem.setEnabled(isOpen
            && serialConnection.isSerialOutputSupported() && hasInitScript());
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

      synchronized (this) {
        nodeCache = null;
      }

      if (notify) {
        final Node newNode = node;
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
              boolean added = false;
              for (int i = 1, n = nodeModel.size(); i < n; i++) {
                int cmp = newNode.compareTo((Node) nodeModel.get(i));
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

   public Node[] getSelectedNodes() {
     return selectedNodes;
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

  private void save() {
    if (configFile != null) {
      configTable.setProperty("collect.bounds", "" + window.getX() + ',' + window.getY() + ',' + window.getWidth() + ',' + window.getHeight());
      if (visualizers != null) {
        for(Visualizer v : visualizers) {
          if (v instanceof Configurable) {
            ((Configurable)v).updateConfig(configTable);
          }
        }
      }
      saveConfig(configTable, configFile);
    }
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
    if (serialConnection != null && serialConnection.isOpen() && serialConnection.isSerialOutputSupported()) {
      serialConsole.addSerialData("SEND: " + data);
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
    serialConsole.addSerialData(line);
  }

  // -------------------------------------------------------------------
  // Node time estimation
  // -------------------------------------------------------------------

  public long getNodeTime() {
    return System.currentTimeMillis() + nodeTimeDelta;
  }

  private void updateNodeTime(SensorData sensorData) {
    this.nodeTimeDelta = sensorData.getNodeTime() - System.currentTimeMillis();
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
    saveSensorData(sensorData);
    if (sensorData.getNode().addSensorData(sensorData)) {
      updateNodeTime(sensorData);
      sensorDataList.add(sensorData);
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
    loadSensorData(SENSORDATA_FILE, true);
  }

  private boolean loadSensorData(String filename, boolean isStrict) {
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
                updateNodeTime(data);
                sensorDataList.add(data);
                handleLinks(data);
              }
            } else if (isStrict) {
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
    this.selectedNodes = null;
    nodeList.clearSelection();
    if (nodeModel.size() > 1) {
      nodeModel.removeRange(1, nodeModel.size() - 1);
    }
    this.nodeTable.clear();
    synchronized (this) {
      this.nodeCache = null;
    }
    if (nodes != null) {
      for(Node node : nodes) {
        node.removeAllSensorData();
      }
    }
    if (visualizers != null) {
      for(Visualizer v : visualizers) {
        v.nodesSelected(null);
        v.clearNodeData();
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

  protected class ConnectSerialAction extends AbstractAction implements Runnable {

    private static final long serialVersionUID = 1L;

    private boolean isRunning;

    public ConnectSerialAction(String name) {
      super(name);
    }

    public void actionPerformed(ActionEvent e) {
      if (!isRunning) {
        isRunning = true;
        new Thread(this, "serial").start();
      }
    }

    public void run() {
      try {
        if (serialConnection != null) {
          if (serialConnection.isOpen()) {
            serialConnection.close();
          } else {
            connectToSerial();
          }
        } else {
          JOptionPane.showMessageDialog(window, "No serial connection configured", "Error", JOptionPane.ERROR_MESSAGE);
        }
      } finally {
        isRunning = false;
      }
    }

  }

  protected class MoteProgramAction extends AbstractAction implements Runnable {

    private static final long serialVersionUID = 1L;

    private boolean isRunning = false;

    public MoteProgramAction(String name) {
      super(name);
    }

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
          JOptionPane.showMessageDialog(window, "Could not find any connected nodes", "Error", JOptionPane.ERROR_MESSAGE);
          return;
        }
        int reply = JOptionPane.showConfirmDialog(window, "Found " + motes.length + " connected nodes.\n"
            + "Do you want to upload the firmware " + FIRMWARE_FILE + '?');
        if (reply == JFileChooser.APPROVE_OPTION) {
          boolean wasOpen = serialConnection != null && serialConnection.isOpen();
          if (serialConnection != null) {
            serialConnection.close();
          }
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
  // SerialConnection Listener
  // -------------------------------------------------------------------

  @Override
  public void serialData(SerialConnection connection, String line) {
    handleIncomingData(System.currentTimeMillis(), line);
  }

  @Override
  public void serialOpened(SerialConnection connection) {
    String connectionName = connection.getConnectionName();
    serialConsole.addSerialData("*** Serial console listening on " + connectionName + " ***");
    hasSerialOpened = true;
    if (connection.isMultiplePortsSupported()) {
      String comPort = connection.getComPort();
      // Remember the last selected serial port
      configTable.put("collect.serialport", comPort);
    }
    setSystemMessage("connected to " + connectionName);

    if (!connection.isSerialOutputSupported()) {
      serialConsole.addSerialData("*** Serial output not supported ***");
    } else if (doSendInitAtStartup) {
      // Send any initial commands
      doSendInitAtStartup = false;

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
      serialConsole.addSerialData("*** Serial connection terminated ***");
      prefix = "Serial connection terminated.\n";
      hasSerialOpened = false;
      setSystemMessage("not connected");
    } else {
      prefix = "Failed to connect to " + connection.getConnectionName() + '\n';
    }
    if (!connection.isClosed()) {
      if (connection.isMultiplePortsSupported()) {
        String options[] = {"Retry", "Search for connected nodes", "Cancel"};
        int value = JOptionPane.showOptionDialog(window,
            prefix + "Do you want to retry or search for connected nodes?",
            "Reconnect to serial port?",
            JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE,
            null, options, options[0]);
        if (value == JOptionPane.CLOSED_OPTION || value == 2) {
//          exit();
        } else {
          String comPort = connection.getComPort();
          if (value == 1) {
            // Select new serial port
            comPort = MoteFinder.selectComPort(window);
            if (comPort == null) {
//              exit();
            }
          }
          // Try to open com port again
          if (comPort != null) {
            connection.open(comPort);
          }
        }
      } else {
//        JOptionPane.showMessageDialog(window,
//            prefix, "Serial Connection Closed", JOptionPane.ERROR_MESSAGE);
      }
    }
  }


  // -------------------------------------------------------------------
  // Main
  // -------------------------------------------------------------------

  public static void main(String[] args) {
    boolean resetSensorLog = false;
    boolean useSensorLog = true;
    boolean useSerialOutput = true;
    String host = null;
    String command = null;
    String logFileToLoad = null;
    String comPort = null;
    int port = -1;
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
        case 'p':
            if (i + 1 < n) {
              port = Integer.parseInt(args[++i]);
            } else {
              usage(arg);
            }
            break;
        case 'r':
          resetSensorLog = true;
          break;
        case 'n':
          useSensorLog = false;
          break;
        case 'i':
          useSerialOutput = false;
          break;
        case 'f':
          command = STDIN_COMMAND;
          if (i + 1 < n && !args[i + 1].startsWith("-")) {
            logFileToLoad = args[++i];
          }
          break;
        case 'h':
          usage(null);
          break;
        default:
          usage(arg);
          break;
        }
      } else if (comPort == null) {
        comPort = arg;
      } else {
        usage(arg);
      }
    }

    CollectServer server = new CollectServer();
    SerialConnection serialConnection;
    if (host != null) {
        if (port <= 0) {
            port = 60001;
        }
        serialConnection = new TCPClientConnection(server, host, port);
    } else if (port > 0) {
      serialConnection = new UDPConnection(server, port);
    } else if (command == null) {
      serialConnection = new SerialDumpConnection(server);
    } else if (command == STDIN_COMMAND) {
      serialConnection = new StdinConnection(server);
    } else {
      serialConnection = new CommandConnection(server, command);
    }
    if (comPort == null) {
      comPort = server.getConfig("collect.serialport");
    }
    if (comPort != null) {
      serialConnection.setComPort(comPort);
    }
    if (!useSerialOutput) {
      serialConnection.setSerialOutputSupported(false);
    }

    server.isSensorLogUsed = useSensorLog;
    if (useSensorLog && resetSensorLog) {
      server.clearSensorDataLog();
    }
    if (logFileToLoad != null) {
      server.loadSensorData(logFileToLoad, false);
    }
    server.start(serialConnection);
  }

  private static void usage(String arg) {
    if (arg != null) {
      System.err.println("Unknown argument '" + arg + '\'');
    }
    System.err.println("Usage: java CollectServer [-n] [-i] [-r] [-f [file]] [-a host:port] [-p port] [-c command] [COMPORT]");
    System.err.println("       -n : Do not read or save sensor data log");
    System.err.println("       -r : Clear any existing sensor data log at startup");
    System.err.println("       -i : Do not allow serial output");
    System.err.println("       -f : Read serial data from standard in");
    System.err.println("       -a : Connect to specified host:port");
    System.err.println("       -p : Read data from specified UDP port");
    System.err.println("       -c : Use specified command for serial data input/output");
    System.err.println("   COMPORT: The serial port to connect to");
    System.exit(arg != null ? 1 : 0);
  }
}
