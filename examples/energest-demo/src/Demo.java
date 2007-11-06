/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * This file is part of the Contiki operating system.
 *
 * $Id: Demo.java,v 1.3 2007/11/06 14:46:14 adamdunkels Exp $
 */

/**
 * \file
 *         Java program showing energy estimates from a Contiki app
 * \author
 *         Fredrik Österlind <fros@sics.se>
 */

import java.awt.BorderLayout;
import java.awt.Font;
import java.awt.GraphicsEnvironment;
import java.awt.GridLayout;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Vector;
import javax.swing.ImageIcon;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.Timer;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.data.general.DefaultPieDataset;
import org.jfree.data.time.Second;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;

public class Demo extends JPanel {
  public static final boolean REPLAY_TEMP_DATA = false;

  public static final int TOTAL_HEIGHT = 300;
  public static final int TOTAL_WIDTH = 900;

  public static final String SERIALDUMP_WINDOWS = "./tools/sky/serialdump-windows.exe";
  public static final String SERIALDUMP_LINUX = "./tools/sky/serialdump-linux";

  public static final int HISTORY_MAX_SECONDS = 120;

  public static final int NODE_HEIGHT = 300;
  public static final int NODE_WIDTH = 450;

  public static final int NUMBER_NODES = 7;
  public static final int NODE_IDS[] = {42, 43, 44, 45, 46, 47, 48};
  public static final String CATEGORIES[] = {"LPM", "CPU", "Radio listen", "Radio transmit"};

  public static final int PARSE_NR_COMPONENTS = 8;
  public static final int PARSE_POS_SINK_ID = 0;
  public static final int PARSE_POS_SICS_ID = 1;
  public static final int PARSE_POS_COUNT = 2;
  public static final int PARSE_POS_SOURCE_ID = 3;
  public static final int PARSE_POS_TIME_CPU = 4;
  public static final int PARSE_POS_TIME_LPM = 5;
  public static final int PARSE_POS_TIME_TRANSMIT = 6;
  public static final int PARSE_POS_TIME_LISTEN = 7;
  public static final String PARSE_SICS_ID = "SICS";

  public static final double TICKS_PER_SECOND = 4096; /* TODO Convert from TimerB ticks to seconds */
  public static final double UPDATE_PERIOD = 1; /* TODO Set update period (1 second?) */

  /*  CC2420 has 8.5 (-25dBm), 9.9 (-15dBm), 11 (-10dBm), 14 (-5dBm) and 17.4 (0dBm) */
  public static final int CHARTS_MAX_MILLIWATTS = 70;
  public static final double VOLTAGE = 3;
  public static final double POWER_CPU = 1.800 * VOLTAGE; /* mW */
  public static final double POWER_LPM = 0.0545 * VOLTAGE; /* mW */
  public static final double POWER_TRANSMIT = 17.7 * VOLTAGE; /* mW */
  public static final double POWER_LISTEN = 20.0 * VOLTAGE; /* mW */

  public static final int MA_HISTORY_LENGTH = 40;

  private Process serialDumpProcess;

  private Vector<Double> historyLPM = new Vector<Double>();
  private Vector<Double> historyCPU = new Vector<Double>();
  private Vector<Double> historyListen = new Vector<Double>();
  private Vector<Double> historyTransmit = new Vector<Double>();

  private int trackedNodeIndex = 0; /* Currently tracked node index */

  private String comPort;
  private JFrame frame;

  private TimeSeries nodeHistorySerie;
  private JFreeChart nodeHistoryChart;
  private JLabel nodeHistoryLabel;

  private JFreeChart relativeChart;
  private JLabel relativeLabel;
  private DefaultPieDataset relativeDataset;

  private JFreeChart totalChart;
  private DefaultCategoryDataset totalDataset;
  private JLabel totalLabel;
  private int categoryOrder = 0;

  public Demo(String comPort) {
    this.comPort = comPort;

    System.out.println("Demo application listening on COM port: " + comPort);

    /* Make sure we have nice window decorations */
    JFrame.setDefaultLookAndFeelDecorated(true);
    JDialog.setDefaultLookAndFeelDecorated(true);
    Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment()
        .getMaximumWindowBounds();

    /* Create and set up the window */
    frame = new JFrame("Contiki Energy Estimation Demo (ACM SenSys 2007)");
    if (maxSize != null) {
      frame.setMaximizedBounds(maxSize);
    }
    frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    frame.addWindowListener(new WindowListener() {
      public void windowDeactivated(WindowEvent e) {
      }

      public void windowIconified(WindowEvent e) {
      }

      public void windowDeiconified(WindowEvent e) {
      }

      public void windowOpened(WindowEvent e) {
      }

      public void windowClosed(WindowEvent e) {
      }

      public void windowActivated(WindowEvent e) {
      }

      public void windowClosing(WindowEvent e) {
        /* TODO Clean up resources */
        if (serialDumpProcess != null) {
          try {
          serialDumpProcess.destroy();
          } catch (Exception ex) {
            System.err.println("Serialdump process exception: " + ex.getMessage());
          }
        }
        System.exit(0);
      }
    });

    frame.setContentPane(this);

    /* Create charts */
    createAllCharts();

    /* Add charts */
    this.setLayout(new BorderLayout());
    JPanel contentPanel = new JPanel(new GridLayout(2, 1));
    add(contentPanel);

    JPanel upperPanel = new JPanel(new GridLayout());
    totalLabel.setAlignmentX(JPanel.CENTER_ALIGNMENT);
    upperPanel.add(totalLabel);
    contentPanel.add(upperPanel);

    JPanel lowerPanel = new JPanel(new GridLayout(1, 2));
    relativeLabel.setAlignmentX(JPanel.CENTER_ALIGNMENT);
    lowerPanel.add(relativeLabel);
    nodeHistoryLabel.setAlignmentX(JPanel.CENTER_ALIGNMENT);
    lowerPanel.add(nodeHistoryLabel);
    contentPanel.add(lowerPanel);

    JLabel advertisementLabel = new JLabel("Contiki Sensor Node Energy Estimation",
					   JLabel.CENTER);
    advertisementLabel.setFont(new Font("Sans-serif", Font.BOLD, 40));
    JLabel urlLabel = new JLabel("http://www.sics.se/contiki/",
					   JLabel.CENTER);
    urlLabel.setFont(new Font("Monospace", Font.BOLD, 36));

    add(advertisementLabel, BorderLayout.NORTH);
    add(urlLabel, BorderLayout.SOUTH);
    
    /* Display the window */
    frame.pack();
    frame.setLocationRelativeTo(null);
    frame.setVisible(true);

    if (!REPLAY_TEMP_DATA) {
      connectToCOMPort();
    }
    Timer updateTimer = new Timer(500, null);
    updateTimer.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
// 		parseIncomingLine(TEMP_NODE_DATA[TEMP_COUNTER]);
// 		TEMP_COUNTER = (TEMP_COUNTER + 1) % TEMP_NODE_DATA.length;
		try {
		    updateCharts();
		} catch(Exception eeeee) {}
	    }
	});
    updateTimer.start();

    if (REPLAY_TEMP_DATA) {
	//      Timer updateTimer = new Timer(1000, null);
      updateTimer.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          parseIncomingLine(TEMP_NODE_DATA[TEMP_COUNTER]);
          TEMP_COUNTER = (TEMP_COUNTER + 1) % TEMP_NODE_DATA.length;
          updateCharts();
        }
      });
      updateTimer.start();
    }
  }

  public void connectToCOMPort() {
    /* Connect to COM using external serialdump application */
    String osName = System.getProperty("os.name").toLowerCase();
    String fullCommand;
    if (osName.startsWith("win")) {
      fullCommand = SERIALDUMP_WINDOWS + " " + "-b115200" + " " + comPort;
    } else {
      fullCommand = SERIALDUMP_LINUX + " " + "-b115200" + " " + comPort;
    }

    try {
      String[] cmd = fullCommand.split(" ");

      serialDumpProcess = Runtime.getRuntime().exec(cmd);
      final BufferedReader input = new BufferedReader(new InputStreamReader(serialDumpProcess.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(serialDumpProcess.getErrorStream()));

      /* Start thread listening on stdout */
      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              parseIncomingLine(line);
            }
            input.close();
            System.out.println("Serialdump process shut down, exiting");
            System.exit(1);
          } catch (IOException e) {
            System.err.println("Exception when reading from serialdump");
            e.printStackTrace();
            System.exit(1);
          }
        }
      }, "read input stream thread");

      /* Start thread listening on stderr */
      Thread readError = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = err.readLine()) != null) {
              System.err.println("Serialdump error stream> " + line);
            }
            err.close();
          } catch (IOException e) {
            System.err.println("Exception when reading from serialdump");
            e.printStackTrace();
            System.exit(1);
          }
        }
      }, "read error stream thread");

      readInput.start();
      readError.start();

    } catch (Exception e) {
      System.err.println("Exception when executing '" + fullCommand + "'");
      System.err.println("Exiting demo application");
      e.printStackTrace();
      System.exit(1);
    }
  }

  public void createAllCharts() {
    BufferedImage image;

    /* Create total power history chart for tracked node */
    nodeHistoryLabel = new JLabel();
    createHistoryChartForNode(nodeHistoryLabel, trackedNodeIndex);

    /* Create moving average relative power distribution chart */
    relativeDataset = new DefaultPieDataset();
    for (String category: CATEGORIES) {
      relativeDataset.setValue(category, 0);
    }
    relativeChart = ChartFactory.createPieChart("Moving Average: Relative power distribution", relativeDataset, false, false, false);
    image = relativeChart.createBufferedImage(NODE_WIDTH,NODE_HEIGHT);
    relativeLabel = new JLabel();
    relativeLabel.setIcon(new ImageIcon(image));

    /* Create chart with power of all nodes */
    totalDataset = new DefaultCategoryDataset();
    for (int i=0; i < NUMBER_NODES; i++) {
      for (String category: CATEGORIES) {
        totalDataset.addValue(0, category, getNodeNameFromIndex(i));
      }
    }
    totalChart = ChartFactory.createStackedBarChart(null, null, "Power (mW)", totalDataset, PlotOrientation.VERTICAL, true, true, true);
    ValueAxis rangeAxis = totalChart.getCategoryPlot().getRangeAxis();
    //    rangeAxis.setRange(0, CHARTS_MAX_MILLIWATTS);

    image = totalChart.createBufferedImage(TOTAL_WIDTH,TOTAL_HEIGHT);
    totalLabel = new JLabel();
    totalLabel.setIcon(new ImageIcon(image));

    MouseListener categoryChangeListener = new MouseListener() {
      public void mouseClicked(MouseEvent e) {
        System.out.println("Toggling category order");

        categoryOrder++;
        totalDataset.clear();
        for (int i=0; i < NUMBER_NODES; i++) {
          for (int j=0; j < CATEGORIES.length; j++) {
            totalDataset.addValue(0, CATEGORIES[(j + categoryOrder) % CATEGORIES.length], getNodeNameFromIndex(i));
          }
        }
        totalChart = ChartFactory.createStackedBarChart(null, null, "Power (mW)", totalDataset, PlotOrientation.VERTICAL, true, true, true);
        ValueAxis rangeAxis = totalChart.getCategoryPlot().getRangeAxis();
//         rangeAxis.setRange(0, CHARTS_MAX_MILLIWATTS);

        relativeDataset.clear();
        for (int i=0; i < NUMBER_NODES; i++) {
          for (int j=0; j < CATEGORIES.length; j++) {
            relativeDataset.setValue(CATEGORIES[(j + categoryOrder) % CATEGORIES.length], 0.00001);
          }
        }
        relativeChart = ChartFactory.createPieChart("Moving Average: Relative power distribution", relativeDataset, false, false, false);

	updateCharts();
      }
      public void mousePressed(MouseEvent e) {
      }
      public void mouseReleased(MouseEvent e) {
      }
      public void mouseEntered(MouseEvent e) {
      }
      public void mouseExited(MouseEvent e) {
      }
    };
    totalLabel.addMouseListener(categoryChangeListener);
    relativeLabel.addMouseListener(categoryChangeListener);

    MouseListener toggleTrackedListener = new MouseListener() {
      public void mouseClicked(MouseEvent e) {
        trackedNodeIndex = (trackedNodeIndex + 1) % NODE_IDS.length;
        System.out.println("Tracking " + getNodeNameFromIndex(trackedNodeIndex));

        createHistoryChartForNode(nodeHistoryLabel, trackedNodeIndex);

        updateCharts();
      }
      public void mousePressed(MouseEvent e) {
      }
      public void mouseReleased(MouseEvent e) {
      }
      public void mouseEntered(MouseEvent e) {
      }
      public void mouseExited(MouseEvent e) {
      }
    };
    nodeHistoryLabel.addMouseListener(toggleTrackedListener);
  }

  public void createHistoryChartForNode(JLabel label, int index) {
    BufferedImage image;

    /* Create history */
    nodeHistorySerie = new TimeSeries("", Second.class);
    nodeHistorySerie.removeAgedItems(true);
    nodeHistorySerie.setMaximumItemCount(HISTORY_MAX_SECONDS);
    TimeSeriesCollection historyData = new TimeSeriesCollection(nodeHistorySerie);
    nodeHistoryChart = ChartFactory.createTimeSeriesChart(getNodeNameFromIndex(index) + ": Total power (mW)", null, null, historyData, false, false, false);
    ValueAxis rangeAxis = nodeHistoryChart.getXYPlot().getRangeAxis();
    rangeAxis.setRange(0, CHARTS_MAX_MILLIWATTS);
    image = nodeHistoryChart.createBufferedImage(NODE_WIDTH,NODE_HEIGHT);
    label.setIcon(new ImageIcon(image));
  }

  public void addHistoryPower(double newPower) {
    if (nodeHistorySerie != null) {
      nodeHistorySerie.addOrUpdate(new Second(), newPower);
    }
  }

  public void updateTotalPower(String category, String nodeName, double newValue) {
    if (totalDataset != null) {
      totalDataset.addValue(newValue, category, nodeName);
    }
  }

  public void updateTotalPowers(String nodeName, double lpm, double cpu, double listen, double transmit) {
    updateTotalPower(CATEGORIES[0], nodeName, lpm);
    updateTotalPower(CATEGORIES[1], nodeName, cpu);
    updateTotalPower(CATEGORIES[2], nodeName, listen);
    updateTotalPower(CATEGORIES[3], nodeName, transmit);
  }

  public void setRelativePower(String category, double newVal) {
    if (relativeDataset != null) {
      relativeDataset.setValue(category, newVal);
    }
  }

  public void setRelativePowers(double lpm, double cpu, double listen, double transmit) {
    setRelativePower(CATEGORIES[0], lpm);
    setRelativePower(CATEGORIES[1], cpu);
    setRelativePower(CATEGORIES[2], listen);
    setRelativePower(CATEGORIES[3], transmit);
  }

  public void updateMARelativePowers(double lpm, double cpu, double listen, double transmit) {
    /* Add new values */
    historyLPM.add(lpm);
    historyCPU.add(cpu);
    historyListen.add(listen);
    historyTransmit.add(transmit);

    /* Remove old values (if any) */
    if (historyLPM.size() > MA_HISTORY_LENGTH) {
      historyLPM.remove(0);
    }
    if (historyCPU.size() > MA_HISTORY_LENGTH) {
      historyCPU.remove(0);
    }
    if (historyListen.size() > MA_HISTORY_LENGTH) {
      historyListen.remove(0);
    }
    if (historyTransmit.size() > MA_HISTORY_LENGTH) {
      historyTransmit.remove(0);
    }

    /* Calculate average */
    double lpmMA = 0;
    for (double power: historyLPM) {
      lpmMA += power;
    }
    lpmMA /= historyLPM.size();

    double cpuMA = 0;
    for (double power: historyCPU) {
      cpuMA += power;
    }
    cpuMA /= historyCPU.size();

    double transmitMA = 0;
    for (double power: historyTransmit) {
      transmitMA += power;
    }
    transmitMA /= historyTransmit.size();

    double listenMA = 0;
    for (double power: historyListen) {
      listenMA += power;
    }
    listenMA /= historyListen.size();

    setRelativePowers(lpmMA, cpuMA, listenMA, transmitMA);
  }


  public void parseIncomingLine(String line) {
    if (line == null) {
      System.err.println("Parsing null line");
      return;
    }

    /* Split line into components */
    String[] components = line.split(" ");
    if (components.length != PARSE_NR_COMPONENTS) {
      System.err.println("Parsing wrong components count (" + components.length + "): '" + line + "'");
      return;
    }

    /* Parse source and components times */
    int sinkNodeID=-1, sourceNodeID=-1, timeCPU=-1, timeLPM=-1, timeTransmit=-1, timeListen=-1;
    try {
      sinkNodeID = Integer.parseInt(components[PARSE_POS_SINK_ID]);
      if (!components[PARSE_POS_SICS_ID].equals(PARSE_SICS_ID)) {
        throw new Exception("Parsing non-demo data: '" + line + "'");
      }
      /*Integer.parseInt(components[PARSE_POS_COUNT]);*/
      sourceNodeID = Integer.parseInt(components[PARSE_POS_SOURCE_ID]);
      timeCPU = Integer.parseInt(components[PARSE_POS_TIME_CPU]);
      timeLPM = Integer.parseInt(components[PARSE_POS_TIME_LPM]);
      timeTransmit = Integer.parseInt(components[PARSE_POS_TIME_TRANSMIT]);
      /* TODO Too big transmit time? */
      timeListen = Integer.parseInt(components[PARSE_POS_TIME_LISTEN]);
    } catch (Exception e) {
      System.err.println(e.getMessage());
      return;
    }

    /* Validate parsed values */
    String nodeName = getNodeNameFromId(sourceNodeID);
    if (nodeName == null) {
      System.err.println("No registered node with ID " + sourceNodeID + ": '" + line + "'");
      return;
    }
    if (timeCPU < 0) {
      System.err.println("Parsed negative CPU time (" + timeCPU + "): '" + line + "'");
      return;
    }
    if (timeLPM < 0) {
      System.err.println("Parsed negative LPM time (" + timeLPM + "): '" + line + "'");
      return;
    }
    if (timeTransmit < 0) {
      System.err.println("Parsed negative transmit time (" + timeTransmit + "): '" + line + "'");
      return;
    }
    if (timeListen < 0) {
      System.err.println("Parsed negative listen time (" + timeListen + "): '" + line + "'");
      return;
    }

    /* Calculate component specific powers using parsed times */
    double powerCPU = (timeCPU / TICKS_PER_SECOND) * POWER_CPU / UPDATE_PERIOD;
    double powerLPM = (timeLPM  / TICKS_PER_SECOND)* POWER_LPM / UPDATE_PERIOD;
    double powerTransmit = (timeTransmit / TICKS_PER_SECOND)* POWER_TRANSMIT / UPDATE_PERIOD;
    double powerListen = (timeListen / TICKS_PER_SECOND) * POWER_LISTEN / UPDATE_PERIOD;

    /* Update node history */
    if (getNodeNameFromId(sourceNodeID).equals(getNodeNameFromIndex(trackedNodeIndex))) {
      System.out.println("Parsed data from tracked " + nodeName);
      addHistoryPower(powerCPU + powerLPM + powerTransmit + powerListen);
    } else {
      System.out.println("Parsed data from " + nodeName);
    }

    updateMARelativePowers(powerLPM, powerCPU, powerListen, powerTransmit);

    updateTotalPowers(nodeName, powerLPM, powerCPU, powerListen, powerTransmit);

    //    updateCharts();
  }


  public void updateCharts() {
    BufferedImage image;

    /* Recreate all label icons */
    /* TODO Only update changed charts: i */
    if (relativeLabel != null) {
      image = relativeChart.createBufferedImage(NODE_WIDTH,NODE_HEIGHT);
      relativeLabel.setIcon(new ImageIcon(image));
    }
    if (nodeHistoryLabel != null) {
      image = nodeHistoryChart.createBufferedImage(NODE_WIDTH,NODE_HEIGHT);
      nodeHistoryLabel.setIcon(new ImageIcon(image));
    }
    if (totalLabel != null) {
      image = totalChart.createBufferedImage(TOTAL_WIDTH,TOTAL_HEIGHT);
      totalLabel.setIcon(new ImageIcon(image));
    }

    repaint();
  }


  public String getNodeNameFromIndex(int index) {
    return "Node " + NODE_IDS[index];
  }

  public static String getNodeNameFromId(int id) {
    boolean exists = false;
    for (int existingID: NODE_IDS) {
      if (existingID == id) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      System.err.println("Node " + id + " is not registered!");
      return null;
    }
    return "Node " + id;
  }


  public static void main(final String[] args) {
    if (args.length != 1) {
      System.err.println("Usage: java Demo COMPORT [TRACK_NODE_ID]");
      return;
    }

    final String comPort = args[0];

    javax.swing.SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        new Demo(comPort);
      }
    });
  }

  /* TEMPORARY DATA */
  int TEMP_COUNTER = 0;
  String[] TEMP_NODE_DATA = {
    "33 S 1 33 3 0 4093 0",
    "33 R 1 34 44 0 4052 0",
    "33 S 2 33 77 0 4019 0",
    "33 S 3 33 39 0 4057 0",
    "33 R 2 34 39 0 4057 0",
    "33 S 4 33 78 0 4018 0",
    "33 R 3 34 80 0 4016 0",
    "33 S 5 33 83 0 4013 0",
    "33 R 4 34 80 0 4015 0",
    "33 S 6 33 90 0 4006 0",
    "33 S 7 33 40 0 4056 0",
    "33 R 5 34 87 0 4009 0",
    "33 S 8 33 80 0 4016 0",
    "33 R 6 34 130 0 3965 0",
    "33 S 9 33 81 0 4015 0",
    "33 S 10 33 39 0 4057 0",
    "33 R 7 34 87 0 4009 0",
    "33 S 11 33 89 0 4007 0",
    "33 R 8 34 94 0 4002 0",
    "33 S 12 33 89 0 4007 0",
    "33 R 9 34 47 0 4049 0",
    "33 S 13 33 83 0 4013 0",
    "33 R 10 34 82 0 4014 0",
    "33 S 14 33 90 0 4006 0",
    "33 R 11 34 94 0 3999 0",
    "33 S 15 33 92 0 4004 0",
    "33 S 16 33 46 0 4050 0",
    "33 R 12 34 132 0 3964 0",
    "33 R 13 34 46 0 4050 0",
    "33 S 17 33 140 0 3956 0",
    "33 R 14 34 86 0 4009 0",
    "33 S 18 33 85 0 4010 0",
    "33 R 15 34 94 0 4002 0",
    "33 S 19 33 83 0 4013 0",
    "33 S 20 33 41 0 4055 0",
    "33 R 16 34 41 0 4055 0",
    "33 S 21 33 86 0 4009 0",
    "33 R 17 34 86 0 4010 0",
    "33 R 18 34 84 0 4012 0",
    "33 S 22 33 127 0 3969 0",
    "33 R 19 34 84 0 4012 0",
    "33 S 23 33 85 0 4011 0",
    "33 S 24 33 44 0 4052 0",
    "33 R 20 34 41 0 4055 0",
    "33 S 25 33 86 0 4010 0",
    "33 S 26 33 47 0 4048 0",
    "33 R 21 34 94 0 4000 0",
    "33 S 27 33 89 0 4004 0"
  };
}
