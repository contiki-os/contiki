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
 * $Id: MspCodeWatcher.java,v 1.2 2008/02/11 11:50:44 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.*;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.InputStreamReader;
import java.util.*;

import javax.swing.*;
import javax.swing.plaf.basic.BasicComboBoxRenderer;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteType;
import se.sics.mspsim.core.CPUMonitor;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.ui.DebugUI;
import se.sics.mspsim.util.Utils;

@ClassDescription("Msp Code Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
public class MspCodeWatcher extends VisPlugin {
  private static Logger logger = Logger.getLogger(MspCodeWatcher.class);
  private Simulation mySimulation;
  private Observer simObserver;
  private MspMote mspMote;
  private MspMoteType moteType;
  private JButton stepButton;

  private File codeFile = null;
  private int lineNumber = -1;

  private DebugUI instructionsUI;
  private CodeUI codeUI;
  private BreakpointsUI breakpointsUI;

  private Breakpoints breakpoints = null;

  private JLabel filenameLabel;

  private Vector<File> files;
  private JComboBox fileComboBox;

  /**
   * Mini-debugger for MSP Motes.
   * Visualizes instructions, source code and allows a user to manipulate breakpoints.
   *
   * @param mote MSP Mote
   * @param simulationToVisualize Simulation
   * @param gui Simulator
   */
  public MspCodeWatcher(Mote mote, Simulation simulationToVisualize, GUI gui) {
    super("Msp Code Watcher", gui);
    this.mspMote = (MspMote) mote;
    this.moteType = (MspMoteType) mote.getType();
    mySimulation = simulationToVisualize;

    Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo = getFirmwareDebugInfo();
    breakpoints = new Breakpoints(debuggingInfo, mspMote);

    getContentPane().setLayout(new BorderLayout());

    instructionsUI = new DebugUI(this.mspMote.getCPU(), true);
    breakpointsUI = new BreakpointsUI(breakpoints);
    codeUI = new CodeUI(breakpoints);

    JSplitPane leftPanel = new JSplitPane(
        JSplitPane.VERTICAL_SPLIT,
        new JScrollPane(instructionsUI, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED),
        new JScrollPane(breakpointsUI, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED)
    );

    JPanel controlPanel = new JPanel();

    // Extract files found in debugging info
    files = new Vector<File>();
    Enumeration fileEnum = debuggingInfo.keys();
    while (fileEnum.hasMoreElements()) {
      File file = (File) fileEnum.nextElement();

      // Insert file on correct position
      int index = 0;
      for (index=0; index < files.size(); index++) {
        if (file.getName().compareToIgnoreCase(files.get(index).getName()) < 0) {
          break;
        }
      }
      files.add(index, file);
    }
    String[] fileNames = new String[files.size() + 1];
    fileNames[0] = "[view sourcefile]";
    for (int i=0; i < files.size(); i++) {
      fileNames[i+1] = files.get(i).getName();
    }
    fileComboBox = new JComboBox(fileNames);
    fileComboBox.setSelectedIndex(0);
    fileComboBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        sourceFileSelectionChanged();
      }
    });

    fileComboBox.setRenderer(new BasicComboBoxRenderer() {
      public Component getListCellRendererComponent(JList list, Object value,
          int index, boolean isSelected, boolean cellHasFocus) {
        if (isSelected) {
          setBackground(list.getSelectionBackground());
          setForeground(list.getSelectionForeground());
          if (index > 0) {
            list.setToolTipText(files.get(index-1).getPath());
          }
        } else {
          setBackground(list.getBackground());
          setForeground(list.getForeground());
        }
        setFont(list.getFont());
        setText((value == null) ? "" : value.toString());
        return this;
      }
    });

    JPanel topPanel = new JPanel();
    topPanel.setLayout(new BorderLayout());
    topPanel.add(BorderLayout.EAST, fileComboBox);
    topPanel.add(BorderLayout.CENTER, filenameLabel = new JLabel(""));



    // Add single step button
    stepButton = new JButton("Single step");
    stepButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        // TODO Perform single step here
        mspMote.getCPU().step(mspMote.getCPU().cycles+1);
        updateInfo();
      }
    });
    controlPanel.add(stepButton);

    add(BorderLayout.CENTER, new JSplitPane(
        JSplitPane.HORIZONTAL_SPLIT,
        leftPanel,
        new JScrollPane(codeUI)
    ));
    add(BorderLayout.SOUTH, controlPanel);
    add(BorderLayout.NORTH, topPanel);

    // Register as tickobserver
    mySimulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (!mySimulation.isRunning()) {
          stepButton.setEnabled(true);
          updateInfo();
        } else {
          stepButton.setEnabled(false);
        }
      }
    });

    setSize(350, 250);
    pack();

    // Tries to select this plugin
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  private void sourceFileSelectionChanged() {
    int index = fileComboBox.getSelectedIndex();
    if (index == 0) {
      return;
    }

    File selectedFile = files.get(index-1);
    Vector<String> codeData = readTextFile(selectedFile);
    codeUI.displayNewCode(selectedFile, codeData, -1);
    codeFile = null;
  }

  /**
   * Contains currently active breakpoints.
   *
   * @author Fredrik Österlind
   */
  static class Breakpoints {
    private Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo = null;
    private Vector<Breakpoint> breakpoints = new Vector<Breakpoint>();
    private Vector<ActionListener> listeners = new Vector<ActionListener>();
    private Breakpoint lastBreakpoint = null;
    private MspMote mspMote = null;
    private boolean stopOnBreakpointTriggered = true;

    /**
     * @param debuggingInfo Debugging information read from firmware file
     * @param mote MspMote
     */
    public Breakpoints(Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo, MspMote mote) {
      this.debuggingInfo = debuggingInfo;
      this.mspMote = mote;
    }

    /**
     * @param debuggingInfo Debugging information read from firmware file
     * @param mote MspMote
     */
    public Breakpoints(Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo, MspMote mote, boolean stopOnBreakpointTriggered) {
      this(debuggingInfo, mote);
      this.stopOnBreakpointTriggered = stopOnBreakpointTriggered;
    }

    /**
     * Add breakpoint at given address.
     *
     * @param address Executable address
     */
    public void addBreakpoint(Integer address) {
      addBreakpoint((File) null, (Integer) null, address);
    }

    /**
     * Add breakpoint at given address with given meta data.
     *
     * @param codeFile Source code file
     * @param lineNr Source code file line number
     * @param address Executable address
     * @return Added breakpoint
     */
    public Breakpoint addBreakpoint(File codeFile, int lineNr, Integer address) {
      Breakpoint bp = new Breakpoint(codeFile, new Integer(lineNr), address, mspMote);
      breakpoints.add(bp);
      lastBreakpoint = null;
      for (ActionListener listener: listeners) {
        listener.actionPerformed(null);
      }
      return bp;
    }

    /**
     * Remove breakpoint at given address.
     *
     * @param address Executable address
     */
    public void removeBreakpoint(Integer address) {
      Breakpoint breakpointToRemove = null;
      for (Breakpoint breakpoint: breakpoints) {
        if (breakpoint.getExecutableAddress().intValue() == address.intValue()) {
          breakpointToRemove = breakpoint;
          break;
        }
      }
      if (breakpointToRemove == null) {
        return;
      }

      breakpointToRemove.unregisterBreakpoint();
      breakpoints.remove(breakpointToRemove);

      // Notify listeners
      lastBreakpoint = null;
      for (ActionListener listener: listeners) {
        listener.actionPerformed(null);
      }
    }

    /**
     * Checks if a breakpoint exists at given address.
     *
     * @param address Executable address
     * @return True if breakpoint exists, false otherwise
     */
    public boolean breakpointExists(Integer address) {
      if (address == null) {
        return false;
      }

      for (Breakpoint breakpoint: breakpoints) {
        if (breakpoint.getExecutableAddress().intValue() == address.intValue()) {
          return true;
        }
      }
      return false;
    }

    /**
     * @return All breakpoints
     */
    public Vector<Breakpoint> getBreakpoints() {
      return breakpoints;
    }

    /**
     * Adds a breakpoint listener.
     * The listener will be notified when breakpoints are added are removed.
     *
     * @param listener Breakpoint listener
     */
    public void addBreakpointListener(ActionListener listener) {
      listeners.add(listener);
    }

    private void breakpointReached(Breakpoint b) {
      if (stopOnBreakpointTriggered) {
        mspMote.getSimulation().stopSimulation();
        mspMote.stopNextInstruction();
      }

      // Notify listeners
      lastBreakpoint = b;
      for (ActionListener listener: listeners) {
        listener.actionPerformed(null);
      }
    }

    public MspMote getMote() {
      return mspMote;
    }

    public Breakpoint getLastTriggered() {
      return lastBreakpoint;
    }

    /**
     * Breakpoint wrapper class.
     * May contain breakpoint meta data such source code file and line number.
     *
     * @author Fredrik Österlind
     */
    class Breakpoint {
      private CPUMonitor cpuMonitor = null;
      private File codeFile = null;
      private Integer lineNr = null;
      private Integer address = null;
      private MspMote mspMote = null;

      /**
       * Create new uninitialized breakpoint. Used with setConfigXML().
       */
      public Breakpoint(MspMote mote) {
        this.mspMote = mote;
      }

      /**
       * Creates new breakpoint wrapper at given address.
       *
       * @param address Executable address
       * @param mote MSP mote
       */
      public Breakpoint(Integer address, MspMote mote) {
        this.mspMote = mote;

        cpuMonitor = new CPUMonitor() {
          public void cpuAction(int type, int adr, int data) {
            breakpointReached(Breakpoint.this);
          }
        };

        mspMote.getCPU().setBreakPoint(address, cpuMonitor);
        this.address = address;
      }

      /**
       * Creates new breakpoint wrapper at given address with given meta data.
       *
       * @param codeFile Source code file
       * @param lineNr Source code file line number
       * @param address Executable address
       * @param mote MSP mote
       */
      public Breakpoint(File codeFile, Integer lineNr, Integer address, MspMote mote) {
        this(address, mote);
        this.codeFile = codeFile;
        this.lineNr = lineNr;
      }

      /**
       * @return MSP mote
       */
      public MspMote getMote() {
        return mspMote;
      }

      /**
       * @return Source code file
       */
      public File getCodeFile() {
        return codeFile;
      }

      /**
       * @return Source code file line number
       */
      public Integer getLineNumber() {
        return lineNr;
      }

      /**
       * @return Executable address
       */
      public Integer getExecutableAddress() {
        return address;
      }

      private void unregisterBreakpoint() {
        mspMote.getCPU().setBreakPoint(address, null);
      }

      public Collection<Element> getConfigXML() {
        Vector<Element> config = new Vector<Element>();
        Element element;

        element = new Element("address");
        element.setText(address.toString());
        config.add(element);

        if (codeFile != null) {
          element = new Element("codefile");
          element.setText(codeFile.getAbsolutePath());
          config.add(element);
        }

        if (lineNr != null) {
          element = new Element("line");
          element.setText(lineNr.toString());
          config.add(element);
        }

        return config;
      }

      public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
        for (Element element : configXML) {
          if (element.getName().equals("codefile")) {
            codeFile = new File(element.getText());
            if (!codeFile.exists()) {
              return false;
            }
          } else if (element.getName().equals("line")) {
            lineNr = Integer.parseInt(element.getText());
          } else if (element.getName().equals("address")) {
            address = Integer.parseInt(element.getText());
          }
        }

        if (address == null) {
          return false;
        }

        cpuMonitor = new CPUMonitor() {
          public void cpuAction(int type, int adr, int data) {
            breakpointReached(Breakpoint.this);
          }
        };

        mspMote.getCPU().setBreakPoint(address, cpuMonitor);
        return true;
      }
    }

    /**
     * Tries to, using debugging information from firmware file, calculate the executable address of given meta data.
     *
     * @param codeFile Source code file
     * @param lineNr Source code file line number
     * @return Executable address or null if not found
     */
    public Integer getExecutableAddressOf(File codeFile, int lineNr) {
      if (codeFile == null || lineNr < 0) {
        return null;
      }

      // Match file names
      Enumeration fileEnum = debuggingInfo.keys();
      while (fileEnum.hasMoreElements()) {
        File file = (File) fileEnum.nextElement();
        if (file != null && file.getName().equals(codeFile.getName())) {
          /* Found source code file */
          Hashtable<Integer, Integer> lineTable = debuggingInfo.get(file);
          Enumeration lineEnum = lineTable.keys();
          while (lineEnum.hasMoreElements()) {
            Integer line = (Integer) lineEnum.nextElement();
            if (line != null && line.intValue() == lineNr) {
              /* Found line address */
              return lineTable.get(line);
            }
          }
          // TODO Return null here to only allow unique source files
        }
      }
      return null;
    }

    public Collection<Element> getConfigXML() {
      Vector<Element> config = new Vector<Element>();
      Element element;

      for (Breakpoint breakpoint: breakpoints) {
        element = new Element("breakpoint");

        Collection breakpointXML = breakpoint.getConfigXML();
        if (breakpointXML != null) {
          element.addContent(breakpointXML);
          config.add(element);
        }
      }

      return config;
    }

    public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
      for (Element element : configXML) {
        if (element.getName().equals("breakpoint")) {
          Breakpoint breakpoint = new Breakpoint(mspMote);
          boolean ret = breakpoint.setConfigXML(element.getChildren(), visAvailable);
          if (!ret) {
            return false;
          }

          breakpoints.add(breakpoint);
          lastBreakpoint = null;
        }
      }
      return true;
    }

  }

  private void updateInfo() {
    // Update instructions view
    instructionsUI.updateRegs();
    instructionsUI.repaint();

    // Try locate source file
    File oldCodeFile = codeFile;
    updateCurrentSourceCodeFile();

    // If found and not already loaded, load source file
    if (oldCodeFile == null || !oldCodeFile.getPath().equals(codeFile.getPath())) {
      Vector<String> codeData = readTextFile(codeFile);
      codeUI.displayNewCode(codeFile, codeData, lineNumber);
    } else {
      codeUI.displayLine(lineNumber);
    }

    codeUI.repaint();

    if (codeFile != null) {
      filenameLabel.setText(codeFile.getName() + ":" + lineNumber);
    } else {
      filenameLabel.setText("");
    }

    fileComboBox.setSelectedIndex(0);
   }

  public void closePlugin() {
    mySimulation.deleteObserver(simObserver);
    while (breakpoints.getBreakpoints().size() > 0) {
      breakpoints.removeBreakpoint(breakpoints.getBreakpoints().firstElement().getExecutableAddress());
    }

  }

  private void updateCurrentSourceCodeFile() {
    codeFile = null;

    try {
      String[] cmd = new String[]{
          "addr2line",
          "-e",
          moteType.getELFFile().getPath(),
          Utils.hex16(mspMote.getCPU().reg[MSP430.PC])
      };

      Process p = Runtime.getRuntime().exec(cmd);

      BufferedReader input =
        new BufferedReader(
            new InputStreamReader(
                p.getInputStream()));

      String output = input.readLine();
      input.close();

      if (output == null || output.startsWith("?")) {
        return;
      }

      String[] fileInfo = output.split(":");
      if (fileInfo.length != 2) {
        return;
      }

      // TODO Ugly Cygwin-Windows support
      fileInfo[0] = fileInfo[0].replace("/cygdrive/c/", "c:/");

      codeFile = new File(fileInfo[0]);
      lineNumber = Integer.parseInt(fileInfo[1]);

    } catch (Exception e) {
      logger.fatal("Error while calling addr2line: " + e);
      codeFile = null;
      lineNumber = -1;
    }
  }

  private Hashtable<File, Hashtable<Integer, Integer>> getFirmwareDebugInfo() {
    return getFirmwareDebugInfo(moteType.getELFFile());
  }

  /**
   * Tries to read debugging information from firmware file using objdump.
   *
   * @return Hashtable with debugging information
   */
  public static Hashtable<File, Hashtable<Integer, Integer>> getFirmwareDebugInfo(File firmware) {
    Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo =
      new Hashtable<File, Hashtable<Integer, Integer>>();

    Hashtable<Integer, Integer> currentHashtable = null;
    try {
      String[] cmd = new String[]{
          "objdump",
          "-g",
          firmware.getPath()
      };

      Process p = Runtime.getRuntime().exec(cmd);

      BufferedReader input =
        new BufferedReader(
            new InputStreamReader(
                p.getInputStream()));

      String line = null;
      while ((line = input.readLine()) != null) {
        line = line.trim();

        /* Check for new source file */
        if (line.endsWith(":")) {
          String fileName = line.substring(0, line.length()-1);
          fileName = fileName.replace("/cygdrive/c/", "c:/");
          currentHashtable = null;

          // Try path unchanged
          File file = new File(fileName);
          if (currentHashtable == null && file.exists()) {
            currentHashtable = new Hashtable<Integer, Integer>();
            debuggingInfo.put(file, currentHashtable);
          }

          // Try adding firmwarepath
          file = new File(firmware.getParent(), fileName);
          if (currentHashtable == null && file.exists()) {
            currentHashtable = new Hashtable<Integer, Integer>();
            debuggingInfo.put(file, currentHashtable);
          }

          if (currentHashtable == null) { // TODO Currently always true
            logger.warn("Can't locate file: " + fileName);
          }
        }

        /* Check for source file line info */
        if (currentHashtable != null && line.startsWith("/* file ")) {
          String[] lineInfo = line.split(" ");

          String lineNrString = lineInfo[lineInfo.length-4];
          String addressString = lineInfo[lineInfo.length-2];

          int lineNr = Integer.parseInt(lineNrString);
          int address = Integer.parseInt(addressString.substring(2), 16);

          currentHashtable.put(new Integer(lineNr), new Integer(address));
        }
      }

      input.close();

    } catch (Exception e) {
      logger.fatal("Error while calling objdump: " + e);
    }
    return debuggingInfo;
  }

  /**
   * Tries to open and read given text file.
   *
   * @param textFile File
   * @return Line-by-line text in file
   */
  public static Vector<String> readTextFile(File textFile) {
    try {
      BufferedReader in =
        new BufferedReader(
            new FileReader(textFile));

      String line;
      Vector<String> textData = new Vector<String>();
      while ((line = in.readLine()) != null) {
        textData.add(line);
      }

      in.close();

      return textData;

    } catch (Exception e) {
      return null;
    }
  }

  public Collection<Element> getConfigXML() {
    return breakpoints.getConfigXML();
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return breakpoints.setConfigXML(configXML, visAvailable);
  }


}
