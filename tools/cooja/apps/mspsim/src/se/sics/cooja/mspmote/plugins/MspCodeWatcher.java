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
 * $Id: MspCodeWatcher.java,v 1.18 2009/06/15 09:44:42 fros4943 Exp $
 */

package se.sics.cooja.mspmote.plugins;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.*;
import java.io.File;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.*;

import javax.swing.*;
import javax.swing.plaf.basic.BasicComboBoxRenderer;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteType;
import se.sics.cooja.util.StringUtils;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.ui.DebugUI;
import se.sics.mspsim.util.DebugInfo;

@ClassDescription("Msp Code Watcher")
@PluginType(PluginType.MOTE_PLUGIN)
public class MspCodeWatcher extends VisPlugin {
  private static Logger logger = Logger.getLogger(MspCodeWatcher.class);
  private Simulation simulation;
  private Observer simObserver;
  private MspMote mspMote;

  private File currentCodeFile = null;
  private int currentLineNumber = -1;

  private JSplitPane leftSplitPane, rightSplitPane;
  private DebugUI assCodeUI;
  private CodeUI sourceCodeUI;
  private BreakpointsUI breakpointsUI;

  private MspBreakpointContainer breakpoints = null;

  private JComboBox fileComboBox;
  private final File[] sourceFiles;
  
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
    simulation = simulationToVisualize;

    getContentPane().setLayout(new BorderLayout());

    /* Breakpoints */
    breakpoints = mspMote.getBreakpointsContainer();
    
    /* Create source file list */
    sourceFiles = getSourceFiles(mspMote);
    String[] sourceFilesHeader = new String[sourceFiles.length + 1];
    sourceFilesHeader[0] = "[view sourcefile]";
    for (int i=0; i < sourceFiles.length; i++) {
      sourceFilesHeader[i+1] = sourceFiles[i].getName();
    }
    fileComboBox = new JComboBox(sourceFilesHeader);
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
            list.setToolTipText(sourceFiles[index-1].getPath());
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

    
    /* Browse code control (north) */
    JButton currentFileButton = new JButton(currentFileAction);

    Box browseBox = Box.createHorizontalBox();
    browseBox.add(Box.createHorizontalStrut(10));
    browseBox.add(new JLabel("Program counter: "));
    browseBox.add(currentFileButton);
    browseBox.add(Box.createHorizontalGlue());
    browseBox.add(new JLabel("Browse: "));
    browseBox.add(fileComboBox);
    browseBox.add(Box.createHorizontalStrut(10));

    
    /* Execution control panel (south) */
    JPanel controlPanel = new JPanel();
    JButton button = new JButton(stepAction);
    stepAction.putValue(Action.NAME, "Step instruction");
    controlPanel.add(button);
    button = new JButton(untilAction);
    untilAction.putValue(Action.NAME, "Until function return");
    /*controlPanel.add(button);*/

    
    /* Main components: assembler and C code + breakpoints (center) */
    assCodeUI = new DebugUI(this.mspMote.getCPU(), true);
    breakpointsUI = new BreakpointsUI(breakpoints, this);
    sourceCodeUI = new CodeUI(breakpoints);
    leftSplitPane = new JSplitPane(
        JSplitPane.HORIZONTAL_SPLIT,
        new JScrollPane(assCodeUI),
        new JScrollPane(breakpointsUI)
    );
    leftSplitPane.setOneTouchExpandable(true);
    leftSplitPane.setDividerLocation(0.0);
    rightSplitPane = new JSplitPane(
        JSplitPane.HORIZONTAL_SPLIT,
        leftSplitPane,
        new JScrollPane(sourceCodeUI)
        );
    rightSplitPane.setOneTouchExpandable(true);
    rightSplitPane.setDividerLocation(0.0);

    add(BorderLayout.NORTH, browseBox);
    add(BorderLayout.CENTER, rightSplitPane);
    add(BorderLayout.SOUTH, controlPanel);

    
    /* Observe when simulation starts/stops */
    simulation.addObserver(simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (!simulation.isRunning()) {
          stepAction.setEnabled(true);
          untilAction.setEnabled(true);
          updateInfo();
        } else {
          stepAction.setEnabled(false);
          untilAction.setEnabled(false);
        }
      }
    });

    setSize(750, 500);
    updateInfo();
  }

  public void displaySourceFile(File file, int line) {
    if (file != null &&
        sourceCodeUI.displayedFile != null &&
        file.compareTo(sourceCodeUI.displayedFile) == 0) {
      sourceCodeUI.displayLine(line);
      return;
    }
    logger.info("Reading source file: " + file);
    String[] codeData = readTextFile(file);
    if (codeData == null) {
      return;
    }
    sourceCodeUI.displayNewCode(file, codeData, line);
  }

  private void sourceFileSelectionChanged() {
    int index = fileComboBox.getSelectedIndex();
    if (index == 0) {
      return;
    }

    File selectedFile = sourceFiles[index-1];
    displaySourceFile(selectedFile, -1);
  }

  private void updateInfo() {
    // Update instructions view
    assCodeUI.updateRegs();
    assCodeUI.repaint();

    // Try locate source file
    updateCurrentSourceCodeFile();
    if (currentCodeFile == null) {
      currentFileAction.setEnabled(false);
      currentFileAction.putValue(Action.NAME, "[unknown]");
      return;
    }

    displaySourceFile(currentCodeFile, currentLineNumber);

    currentFileAction.setEnabled(true);
    currentFileAction.putValue(Action.NAME, currentCodeFile.getName() + ":" + currentLineNumber);

    fileComboBox.setSelectedIndex(0);
  }

  public void closePlugin() {
    simulation.deleteObserver(simObserver);

//    if (breakpoints.getBreakpoints().length == 0) {
//      return;
//    }
//    
//    if (GUI.isVisualized()) {
//      String s1 = "Remove";
//      String s2 = "Keep";
//      Object[] options = { s1, s2 };
//      int n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
//          "Mote has " + breakpoints.getBreakpoints().length + " breakpoints." +
//          "\nShould they be removed?",
//          "Remove mote breakpoints?", JOptionPane.YES_NO_OPTION,
//          JOptionPane.QUESTION_MESSAGE, null, options, s1);
//      if (n != JOptionPane.YES_OPTION) {
//        return;
//      }
//    }
//    
//    /* Remove all mote breakpoints */
//    while (breakpoints.getBreakpoints().length > 0) {
//      breakpoints.removeBreakpoint(breakpoints.getBreakpoints()[0].getExecutableAddress());
//    }
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
      currentCodeFile = currentCodeFile.getCanonicalFile();
      
      currentLineNumber = debugInfo.getLine();
    } catch (Exception e) {
      logger.fatal("Exception: " + e);
      currentCodeFile = null;
      currentLineNumber = -1;
    }
  }

  private static File[] getSourceFiles(MspMote mote) {
    String[] sourceFiles = mote.getELF().getDebug().getSourceFiles();
    File contikiSource = mote.getType().getContikiSourceFile();
    try {
      contikiSource = contikiSource.getCanonicalFile();
    } catch (IOException e1) {
    }
    
    /* Verify that files exist */
    Vector<File> existing = new Vector<File>();
    for (String sourceFile: sourceFiles) {
      /* Nasty Cygwin-Windows fix */
      if (sourceFile.contains("/cygdrive/")) {
        int index = sourceFile.indexOf("/cygdrive/");
        char driveCharacter = sourceFile.charAt(index+10);
        sourceFile = sourceFile.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
      }

      File file = new File(sourceFile);
      try {
        file = file.getCanonicalFile();
      } catch (IOException e1) {
      }
      if (!GUI.isVisualizedInApplet()) {
        if (file.exists() && file.isFile()) {
          existing.add(file);
        } else {
          /*logger.warn("Can't locate source file, skipping: " + file.getPath());*/
        }
      } else {
        /* Accept all files without existence check */
        existing.add(file);
      }
    }

    /* Sort alphabetically */
    Vector<File> sorted = new Vector<File>();
    for (File file: existing) {
      int index = 0;
      for (index=0; index < sorted.size(); index++) {
        if (file.getName().compareToIgnoreCase(sorted.get(index).getName()) < 0) {
          break;
        }
      }
      sorted.add(index, file);
    }
    
    /* Add Contiki source first */
    if (contikiSource != null && contikiSource.exists()) {
      sorted.add(0, contikiSource);
    }

    File[] sortedArr = sorted.toArray(new File[0]);
    return sortedArr;
  }

  /**
   * Tries to open and read given text file.
   *
   * @param file File
   * @return Line-by-line text in file
   */
  public static String[] readTextFile(File file) {
    if (GUI.isVisualizedInApplet()) {
      /* Download from web server instead */
      String path = file.getPath();

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
        String data = StringUtils.loadFromURL(url);
        return data!=null?data.split("\n"):null;
      } catch (MalformedURLException e) {
        logger.warn("Failure to read source code: " + e);
        return null;
      } catch (IOException e) {
        logger.warn("Failure to read source code: " + e);
        return null;
      }
    }

    String data = StringUtils.loadFromFile(file);
    return data!=null?data.split("\n"):null;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;
    
    element = new Element("split_1");
    element.addContent("" + leftSplitPane.getDividerLocation());
    config.add(element);
    
    element = new Element("split_2");
    element.addContent("" + rightSplitPane.getDividerLocation());
    config.add(element);
    
    return config;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("split_1")) {
        leftSplitPane.setDividerLocation(Integer.parseInt(element.getText()));
      } else if (element.getName().equals("split_2")) {
        rightSplitPane.setDividerLocation(Integer.parseInt(element.getText()));
      }
    }
    return true;
  }

  private AbstractAction currentFileAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      if (currentCodeFile == null) {
        return;
      }
      displaySourceFile(currentCodeFile, currentLineNumber);
    }
  };

  private AbstractAction stepAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      try {
        mspMote.getCPU().step(mspMote.getCPU().cycles+1);
      } catch (EmulationException ex) {
        logger.fatal("Error: ", ex);
      }
      updateInfo();
    }
  };

  private AbstractAction untilAction = new AbstractAction() {
    public void actionPerformed(ActionEvent e) {
      /* XXX TODO Implement me */
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

      try {
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
      } catch (EmulationException e1) {
        logger.fatal("Error: ", e1);
      }

      logger.fatal("Function '" + functionName + "' did not return within " + max + " instructions");
      updateInfo();
    }
  };

}
