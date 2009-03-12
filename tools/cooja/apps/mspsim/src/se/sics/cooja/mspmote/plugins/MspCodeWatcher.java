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
 * $Id: MspCodeWatcher.java,v 1.15 2009/03/12 15:12:23 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.*;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
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
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.ui.DebugUI;
import se.sics.mspsim.util.DebugInfo;

@ClassDescription("Msp Code Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
public class MspCodeWatcher extends VisPlugin {
  private static Logger logger = Logger.getLogger(MspCodeWatcher.class);
  private Simulation mySimulation;
  private Observer simObserver;
  private MspMote mspMote;
  private MspMoteType moteType;
  private JButton stepButton;

  private File currentCodeFile = null;
  private int currentLineNumber = -1;

  private DebugUI instructionsUI;
  private CodeUI codeUI;
  private BreakpointsUI breakpointsUI;

  private Breakpoints breakpoints = null;

  private JButton currentFileButton;

  private Vector<File> sourceFilesAlpha;
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
    breakpoints.addBreakpointListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        updateInfo();
      }
    });

    getContentPane().setLayout(new BorderLayout());

    instructionsUI = new DebugUI(this.mspMote.getCPU(), true);
    breakpointsUI = new BreakpointsUI(breakpoints, this);
    codeUI = new CodeUI(breakpoints);

    JSplitPane rightPanel = new JSplitPane(
        JSplitPane.VERTICAL_SPLIT,
        new JScrollPane(instructionsUI, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED),
        new JScrollPane(breakpointsUI, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED)
    );
    rightPanel.setOneTouchExpandable(true);
    rightPanel.setResizeWeight(0.2);

    JPanel controlPanel = new JPanel();

    /* Create source file list */
    File[] sourceFiles = getAllSourceFileNames();
    sourceFilesAlpha = new Vector<File>();
    for (File file: sourceFiles) {
      /* Insert files alphabetically */
      int index = 0;
      for (index=0; index < sourceFilesAlpha.size(); index++) {
        if (file.getName().compareToIgnoreCase(sourceFilesAlpha.get(index).getName()) < 0) {
          break;
        }
      }
      sourceFilesAlpha.add(index, file);
    }
    String[] sourceFilesAlphaArray = new String[sourceFilesAlpha.size() + 1];
    sourceFilesAlphaArray[0] = "[view sourcefile]";
    for (int i=0; i < sourceFilesAlpha.size(); i++) {
      sourceFilesAlphaArray[i+1] = sourceFilesAlpha.get(i).getName();
    }
    fileComboBox = new JComboBox(sourceFilesAlphaArray);
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
            list.setToolTipText(sourceFilesAlpha.get(index-1).getPath());
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

    currentFileButton = new JButton("[unknown]");
    currentFileButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (currentCodeFile == null) {
          return;
        }

        displaySourceFile(currentCodeFile, currentLineNumber);
      }
    });

    JPanel topPanel = new JPanel();
    topPanel.setLayout(new BorderLayout());
    topPanel.add(BorderLayout.EAST, fileComboBox);

    JPanel currentFilePanel = new JPanel();
    currentFilePanel.add(BorderLayout.WEST, new JLabel("current file:"));
    currentFilePanel.add(BorderLayout.WEST, currentFileButton);
    topPanel.add(BorderLayout.WEST, currentFilePanel);


    /* Instruction button */
    stepButton = new JButton("Single instruction");
    stepButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        // TODO Perform single step here
        mspMote.getCPU().step(mspMote.getCPU().cycles+1);
        updateInfo();
      }
    });
    controlPanel.add(stepButton);

    /* Return button */
    stepButton = new JButton("Until function returns");
    stepButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        /* XXX Experimental */
        final int max = 10000;
        int count=0;
        int pc, instruction;
        MSP430 cpu = mspMote.getCPU();
        int depth = 0;

        /* Extract function name */
        DebugInfo debugInfo = mspMote.getELF().getDebugInfo(mspMote.getCPU().reg[MSP430.PC]);
        if (debugInfo == null || debugInfo.getFunction() == null) {
          logger.fatal("Unknown function");
          return;
        }
        String functionName = debugInfo.getFunction().split(":")[0];
        logger.info("Function: '" + functionName + "'");

        pc = cpu.readRegister(MSP430Core.PC);
        instruction = cpu.memory[pc] + (cpu.memory[pc + 1] << 8);
        if (instruction == MSP430.RETURN) {
          logger.fatal("Already at return instruction");
          return;
        }

        while (count++ < max) {
          cpu.step(mspMote.getCPU().cycles+1);
          pc = cpu.readRegister(MSP430Core.PC);
          instruction = cpu.memory[pc] + (cpu.memory[pc + 1] << 8);
          if ((instruction & 0xff80) == MSP430.CALL) {
            depth++;
          } else if (instruction == MSP430.RETURN) {
            depth--;
            if (depth < 0) {
              updateInfo();
              return;
            }

          }
        }

        logger.fatal("Function '" + functionName + "' did not return within " + max + " instructions");
        updateInfo();
      }
    });
    controlPanel.add(stepButton);

    JSplitPane splitPane = new JSplitPane(
        JSplitPane.HORIZONTAL_SPLIT,
        new JScrollPane(codeUI),
        rightPanel);
    splitPane.setOneTouchExpandable(true);
    splitPane.setResizeWeight(0.8);

    add(BorderLayout.CENTER, splitPane);

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

    setSize(750, 500);
    updateInfo();

    // Tries to select this plugin
    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  private File lastReadTextFile = null;
  public void displaySourceFile(File file, int line) {

    if (lastReadTextFile != null && file.compareTo(lastReadTextFile) == 0) {
      codeUI.displayLine(line);
      return;
    }

    logger.info("Reading source file " + file);
    Vector<String> codeData = readTextFile(file);
    lastReadTextFile = file;
    codeUI.displayNewCode(file, codeData, line);
  }

  private void sourceFileSelectionChanged() {
    int index = fileComboBox.getSelectedIndex();
    if (index == 0) {
      return;
    }

    File selectedFile = sourceFilesAlpha.get(index-1);
    displaySourceFile(selectedFile, -1);
  }

  /**
   * Contains currently active breakpoints.
   *
   * @author Fredrik Osterlind
   */
  class Breakpoints {
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
     * @author Fredrik Osterlind
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
          File file = mySimulation.getGUI().createPortablePath(codeFile);
          element.setText(file.getPath().replaceAll("\\\\", "/"));
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
            File file = new File(element.getText());
            if (!file.exists()) {
              file = mySimulation.getGUI().restorePortablePath(file);
            }
            codeFile = file;
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
    updateCurrentSourceCodeFile();
    if (currentCodeFile == null) {
      return;
    }
    displaySourceFile(currentCodeFile, currentLineNumber);

    codeUI.repaint();

    if (currentCodeFile != null) {
      currentFileButton.setText(currentCodeFile.getName() + ":" + currentLineNumber);
    } else {
      currentFileButton.setText("[unknown]");
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
    currentCodeFile = null;

    try {
      DebugInfo debugInfo = mspMote.getELF().getDebugInfo(mspMote.getCPU().reg[MSP430.PC]);

      if (debugInfo == null) {
        return;
      }

      /* Nasty Cygwin-Windows fix */
      String path = debugInfo.getPath();

      if (path == null) {
        return;
      }

      if (path.contains("/cygdrive/")) {
        int index = path.indexOf("/cygdrive/");
        char driveCharacter = path.charAt(index+10);

        path = path.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
      }

      currentCodeFile = new File(path, debugInfo.getFile());
      currentLineNumber = debugInfo.getLine();

    } catch (Exception e) {
      logger.fatal("Exception: " + e);
      currentCodeFile = null;
      currentLineNumber = -1;
    }
  }

  private File[] getAllSourceFileNames() {
    String[] sourceFiles = mspMote.getELF().getDebug().getSourceFiles();
    Vector<File> files = new Vector<File>();

    for (String sourceFile: sourceFiles) {
      /* Nasty Cygwin-Windows fix */
      if (sourceFile.contains("/cygdrive/")) {
        int index = sourceFile.indexOf("/cygdrive/");
        char driveCharacter = sourceFile.charAt(index+10);
        sourceFile = sourceFile.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
      }

      File file = new File(sourceFile);
      if (!GUI.isVisualizedInApplet()) {
        if (!file.exists() || !file.isFile()) {
          logger.warn("Can't locate source file, skipping: " + file.getPath());
        } else {
          files.add(file);
        }
      } else {
        /* Accept all files without existence check */
        files.add(file);
      }
    }

    File[] filesArray = new File[files.size()];
    for (int i=0; i < filesArray.length; i++) {
      filesArray[i] = files.get(i);
    }

    return filesArray;
  }

  private Hashtable<File, Hashtable<Integer, Integer>> getFirmwareDebugInfo() {

    /* Fetch all executable addresses */
    ArrayList<Integer> addresses = mspMote.getELF().getDebug().getExecutableAddresses();

    Hashtable<File, Hashtable<Integer, Integer>> fileToLineHash =
      new Hashtable<File, Hashtable<Integer, Integer>>();

    for (int address: addresses) {
      DebugInfo info = mspMote.getELF().getDebugInfo(address);

      if (info != null && info.getPath() != null && info.getFile() != null && info.getLine() >= 0) {

        /* Nasty Cygwin-Windows fix */
        String path = info.getPath();
        if (path.contains("/cygdrive/")) {
          int index = path.indexOf("/cygdrive/");
          char driveCharacter = path.charAt(index+10);

          path = path.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
        }

        File file = new File(path, info.getFile());

        Hashtable<Integer, Integer> lineToAddrHash = fileToLineHash.get(file);
        if (lineToAddrHash == null) {
          lineToAddrHash = new Hashtable<Integer, Integer>();
          fileToLineHash.put(file, lineToAddrHash);
        }

        lineToAddrHash.put(info.getLine(), address);
      }
    }

    return fileToLineHash;
  }

  /**
   * Tries to open and read given text file.
   *
   * @param textFile File
   * @return Line-by-line text in file
   */
  public static Vector<String> readTextFile(URL textFile) throws IOException {
    URLConnection urlConnection = textFile.openConnection();
    urlConnection.setDoInput(true);
    urlConnection.setUseCaches(false);

    Vector<String> data = new Vector<String>();
    BufferedReader reader = new BufferedReader(new InputStreamReader(urlConnection.getInputStream()));
    String line;
    while ((line = reader.readLine()) != null) {
      data.add(line);
    }

    return data;
  }

  /**
   * Tries to open and read given text file.
   *
   * @param textFile File
   * @return Line-by-line text in file
   */
  public static Vector<String> readTextFile(File textFile) {
    if (GUI.isVisualizedInApplet()) {
      /* Download from web server instead */
      String path = textFile.getPath();

      /* Extract Contiki build path */
      String contikiBuildPath = GUI.getExternalToolsSetting("PATH_CONTIKI_BUILD");
      String contikiWebPath = GUI.getExternalToolsSetting("PATH_CONTIKI_WEB");

      if (!path.startsWith(contikiBuildPath)) {
        return null;
      }

      try {
        /* Replace Contiki parent path with web server code base */
        path = contikiWebPath + '/' + path.substring(contikiBuildPath.length());
        path = path.replace('\\', '/');
        URL url = new URL(GUI.getAppletCodeBase(), path);
        return readTextFile(url);
      } catch (MalformedURLException e) {
        logger.warn("Failure to read source code: " + e);
        return null;
      } catch (IOException e) {
        logger.warn("Failure to read source code: " + e);
        return null;
      }
    }

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
