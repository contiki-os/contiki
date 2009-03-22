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
 * $Id: ScriptRunner.java,v 1.16 2009/03/22 14:05:19 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.StringReader;
import java.util.*;
import javax.script.ScriptException;
import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;

import org.apache.log4j.Logger;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.*;
import se.sics.cooja.GUI.SimulationCreationException;
import se.sics.cooja.dialogs.MessageList;

@ClassDescription("Contiki Test Editor")
@PluginType(PluginType.COOJA_PLUGIN)
public class ScriptRunner implements Plugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ScriptRunner.class);

  private static final String EXAMPLE_SCRIPT =
    "/*\n" +
    " * Example Contiki test script (JavaScript).\n" +
    " * A Contiki test script acts on mote output, such as via printf()'s.\n" +
    " * The script may operate on the following variables:\n" +
    " *  Mote mote, int id, String msg\n" +
    " */\n" +
    "\n" +
    "/* Make test automatically fail (timeout) after 100 simulated seconds */\n" +
    "//TIMEOUT(100000); /* no action at timeout */\n" +
    "TIMEOUT(100000, log.log(\"last msg: \" + msg + \"\\n\")); /* print last msg at timeout */\n" +
    "\n" +
    "log.log(\"first mote output: '\" + msg + \"'\\n\");\n" +
    "\n" +
    "YIELD(); /* wait for another mote output */\n" +
    "\n" +
    "log.log(\"second mote output: '\" + msg + \"'\\n\");\n" +
    "\n" +
    "log.log(\"waiting for hello world output from mote 1\\n\");\n" +
    "WAIT_UNTIL(id == 1 && msg.equals(\"Hello, world\"));\n" +
    "\n" +
    "log.log(\"ok, reporting success now\\n\");\n" +
    "log.testOK(); /* Report test success and quit */\n" +
    "//log.testFailed(); /* Report test failure and quit */\n";

  private GUI gui;
  private Object coojaTag = null; /* Used by Cooja for book-keeping */

  private LogScriptEngine engine = null;

  private BufferedWriter logWriter = null;

  /* GUI components */
  private JInternalFrame pluginGUI = null;
  private JTextArea scriptTextArea = null;
  private JTextArea lineTextArea = null;

  private JTextArea logTextArea = null;
  private JButton toggleButton = null;
  private String oldTestName = null;
  private String oldInfo = null;

  public ScriptRunner(GUI gui) {
    this.gui = gui;

    if (!GUI.isVisualized()) {
      /* Wait for activateTest(...) */
      return;
    }

    /* Automatically activate test for new simulations */
    gui.addObserver(new Observer() {
      public void update(Observable obs, Object obj) {
        Simulation sim = ScriptRunner.this.gui.getSimulation();
        if (sim == null) {
          setScriptActive(false);
          return;
        }

        setScriptActive(true);
      }
    });

    /* GUI components */
    pluginGUI = new JInternalFrame(
        "Contiki Test Editor",
        true, true, true, true);
    pluginGUI.addInternalFrameListener(new InternalFrameListener() {
      public void internalFrameClosing(InternalFrameEvent e) {
        ScriptRunner.this.gui.removePlugin(ScriptRunner.this, true);
      }
      public void internalFrameClosed(InternalFrameEvent e) { }
      public void internalFrameOpened(InternalFrameEvent e) { }
      public void internalFrameIconified(InternalFrameEvent e) { }
      public void internalFrameDeiconified(InternalFrameEvent e) { }
      public void internalFrameActivated(InternalFrameEvent e) { }
      public void internalFrameDeactivated(InternalFrameEvent e) { }
    }
    );

    lineTextArea = new JTextArea();
    lineTextArea.setEnabled(false);
    lineTextArea.setMargin(new Insets(5,0,5,0));

    scriptTextArea = new JTextArea(12,50);
    scriptTextArea.getDocument().addDocumentListener(new DocumentListener() {
      private int lastLines = -1;

      private void update() {
        int lines = scriptTextArea.getLineCount();
        if (lines == lastLines) {
          return;
        }
        lastLines = lines;

        String txt = "";
        for (int i=1; i < 10; i++) {
          if (i > lines) {
            break;
          }
          txt += "00" + i + "\n";
        }
        for (int i=10; i < 100; i++) {
          if (i > lines) {
            break;
          }
          txt += "0" + i + "\n";
        }
        for (int i=100; i < 1000; i++) {
          if (i > lines) {
            break;
          }
          txt += i + "\n";
        }
        lineTextArea.setText(txt);
      }

      public void changedUpdate(DocumentEvent e) {
        update();
      }
      public void insertUpdate(DocumentEvent e) {
        update();
      }
      public void removeUpdate(DocumentEvent e) {
        update();
      }
    });
    scriptTextArea.setMargin(new Insets(5,0,5,5));
    scriptTextArea.setEditable(true);
    scriptTextArea.setCursor(null);
    scriptTextArea.setTabSize(1);
    scriptTextArea.setText(EXAMPLE_SCRIPT);

    logTextArea = new JTextArea(12,50);
    logTextArea.setMargin(new Insets(5,5,5,5));
    logTextArea.setEditable(true);
    logTextArea.setCursor(null);

    toggleButton = new JButton("Activate");
    toggleButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        if (toggleButton.getText().equals("Activate")) {
          setScriptActive(true);
        } else {
          setScriptActive(false);
        }
      }
    });

    JButton importButton = new JButton("Import Contiki test");
    importButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        Runnable doImport = new Runnable() {
          public void run() {
            setScriptActive(false);
            importContikiTest();
          }
        };
        new Thread(doImport).start();
      }
    });

    JButton exportButton = new JButton("Export as Contiki test");
    exportButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        exportAsContikiTest();
      }
    });


    JPanel scriptArea = new JPanel(new BorderLayout());
    scriptArea.setEnabled(false);
    scriptArea.add(BorderLayout.WEST, lineTextArea);
    scriptArea.add(BorderLayout.CENTER, scriptTextArea);

    JSplitPane centerPanel = new JSplitPane(
        JSplitPane.VERTICAL_SPLIT,
        new JScrollPane(scriptArea),
        new JScrollPane(logTextArea)
    );
    centerPanel.setOneTouchExpandable(true);
    centerPanel.setResizeWeight(0.5);

    JPanel buttonPanel = new JPanel(new BorderLayout());
    buttonPanel.add(BorderLayout.WEST, importButton);
    buttonPanel.add(BorderLayout.CENTER, toggleButton);
    buttonPanel.add(BorderLayout.EAST, exportButton);

    JPanel southPanel = new JPanel(new BorderLayout());
    southPanel.add(BorderLayout.EAST, buttonPanel);

    pluginGUI.getContentPane().add(BorderLayout.CENTER, centerPanel);
    pluginGUI.getContentPane().add(BorderLayout.SOUTH, southPanel);

    pluginGUI.pack();
  }

  private void setScriptActive(boolean active) {
    if (active) {
      setScriptActive(false);

      engine = new LogScriptEngine(ScriptRunner.this.gui);
      engine.setScriptLogObserver(new Observer() {
        public void update(Observable obs, Object obj) {
          logTextArea.append((String) obj);
          logTextArea.setCaretPosition(logTextArea.getText().length());
        }
      });
      try {
        engine.activateScript(scriptTextArea.getText());
      } catch (ScriptException e) {
        e.printStackTrace();
        setScriptActive(false);
      }
      toggleButton.setText("Deactivate");
      logTextArea.setText("");
      scriptTextArea.setEnabled(false);

      logger.info("Test script activated");
    } else {
      if (engine != null) {
        engine.deactivateScript();
        engine = null;

        toggleButton.setText("Activate");
        scriptTextArea.setEnabled(true);
        logger.info("Test script deactivated");
      }
    }
  }

  public JInternalFrame getGUI() {
    return pluginGUI;
  }

  private void importContikiTest() {
    new Thread(new Runnable() {
      public void run() {
        /* Load config from test directory */
        final File proposedDir = new File(GUI.getExternalToolsSetting("PATH_CONTIKI") + "/tools/cooja/contiki_tests");
        if (!proposedDir.exists()) {
          logger.fatal("Test directory does not exist: " + proposedDir.getPath());
          return;
        }

        scriptTextArea.setText("");
        logTextArea.setText("");

        gui.doLoadConfig(false, true, proposedDir);
        if (gui.getSimulation() == null) {
          return;
        }
        File cscFile = gui.currentConfigFile;
        String testName = cscFile.getName().substring(0, cscFile.getName().length()-4);
        File testDir = cscFile.getParentFile();
        File jsFile = new File(testDir, testName + ".js");
        File infoFile = new File(testDir, testName + ".info");

        oldTestName = testName;

        if (!cscFile.exists()) {
          logger.fatal("Can't locate config file: " + cscFile.getAbsolutePath());
          return;
        }

        if (!jsFile.exists()) {
          logger.fatal("Can't locate .js file: " + jsFile.getAbsolutePath());
          return;
        }

        /* Import .js */
        try {
          BufferedReader reader =
            new BufferedReader(new InputStreamReader(new FileInputStream(jsFile)));
          String line;
          while ((line = reader.readLine()) != null) {
            scriptTextArea.append(line + "\n");
          }
          reader.close();
        } catch (Exception ex) {
          ex.printStackTrace();
          return;
        }

        /* Import .info */
        if (infoFile.exists()) {
          try {
            oldInfo = "";
            BufferedReader reader =
              new BufferedReader(new InputStreamReader(new FileInputStream(infoFile)));
            String line;
            while ((line = reader.readLine()) != null) {
              oldInfo +=  line + "\n";
            }
            reader.close();
          } catch (Exception ex) {
            ex.printStackTrace();
            return;
          }
        }

        setScriptActive(true);
      }
    }).start();

  }

  private void exportAsContikiTest() {
    String s1, s2; int n; Object[] options;

    Simulation simulation = ScriptRunner.this.gui.getSimulation();
    if (simulation == null) {
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          "Create a simulation setup to test.", "No simulation to export", JOptionPane.ERROR_MESSAGE);
      return;
    }

    File testDir = new File(GUI.getExternalToolsSetting("PATH_CONTIKI") + "/tools/cooja/contiki_tests");
    if (!testDir.exists()) {
      logger.fatal("Test directory does not exist: " + testDir.getPath());
      return;
    }

    /* Confirm test directory */
    /*s1 = "Ok";
    s2 = "Cancel";
    options = new Object[] { s1, s2 };
    n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
        "The current simulation config (.csc) and test script (.js)\n" +
        "will be stored in directory '" + testDir.getPath() + "'",
        "Export Contiki test", JOptionPane.YES_NO_OPTION,
        JOptionPane.QUESTION_MESSAGE, null, options, s1);
    if (n != JOptionPane.YES_OPTION) {
      return;
    }*/

    /* Name test to export */
    if (oldTestName == null) {
      oldTestName = "mytest";
    }
    String testName = (String) JOptionPane.showInputDialog(GUI.getTopParentContainer(),
        "The test name correponds to the exported test files:\n" +
        "[testname].csc, [testname].js, [testname].info\n",
        "Enter test name", JOptionPane.PLAIN_MESSAGE, null, null,
        oldTestName);
    if (testName == null) {
      return;
    }
    oldTestName = testName;
    if (testName.equals("") || testName.contains(" ")) {
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          "Bad test name: '" + testName + "'", "Bad test name", JOptionPane.ERROR_MESSAGE);
      return;
    }

    File cscFile = new File(testDir, testName + ".csc");
    File jsFile = new File(testDir, testName + ".js");
    File infoFile = new File(testDir, testName + ".info");
    final File logFile = new File(testDir, testName + ".log");

    /* Overwrite existing test */
    if (cscFile.exists() || jsFile.exists() || infoFile.exists()) {
      s1 = "Overwrite";
      s2 = "Cancel";
      options = new Object[] { s1, s2 };
      n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
          "Some output files already exist. Overwrite?",
          "Test already exist", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, options, s1);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }
    }

    if (cscFile.exists()) {
      cscFile.delete();
    }
    if (jsFile.exists()) {
      jsFile.delete();
    }
    if (infoFile.exists()) {
      infoFile.delete();
    }

    /* Get current simulation configuration */
    simulation.getGUI().currentConfigFile = cscFile;
    Element root = new Element("simconf");
    Element simulationElement = new Element("simulation");
    simulationElement.addContent(simulation.getConfigXML());
    root.addContent(simulationElement);

    /* Strip plugins */
    Collection<Element> pluginsConfig = ScriptRunner.this.gui.getPluginsConfigXML();
    if (pluginsConfig != null) {
      root.addContent(pluginsConfig);
    }

    /* Export .csc */
    root.detach();
    String configString = new XMLOutputter().outputString(new Document(root));
    try {
      Element newRoot = new SAXBuilder().build(new StringReader(configString)).getRootElement();
      newRoot.detach();
      Document doc = new Document(newRoot);
      FileOutputStream out = new FileOutputStream(cscFile);
      XMLOutputter outputter = new XMLOutputter();
      outputter.setFormat(Format.getPrettyFormat());
      outputter.output(doc, out);
      out.close();
    } catch (JDOMException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    }

    /* Export .js */
    try {
      BufferedWriter writer =
        new BufferedWriter(new OutputStreamWriter(new FileOutputStream(jsFile)));
      writer.write(scriptTextArea.getText());
      writer.close();
    } catch (Exception ex) {
      ex.printStackTrace();
      return;
    }

    /* Export .info (optional) */
    try {
      if (oldInfo == null) {
        oldInfo = "";
      }
      String info = (String) JOptionPane.showInputDialog(GUI.getTopParentContainer(),
          "This text describes the Contiki test and may contain\n" +
          "information about the simulation setup, radio medium,\n" +
          "node types, Contiki processes etc.\n\n",
          "Enter test description", JOptionPane.PLAIN_MESSAGE, null, null,
          oldInfo);
      if (info != null && !info.equals("")) {
        oldInfo = info;
        BufferedWriter writer =
          new BufferedWriter(new OutputStreamWriter(new FileOutputStream(infoFile)));
        writer.write(info);
        writer.write("\n");
        writer.close();
      } else {
        oldInfo = null;
      }
    } catch (Exception ex) {
      ex.printStackTrace();
      return;
    }

    /* Run exported test (optional) */
    s1 = "Run test";
    s2 = "No";
    options = new Object[] { s1, s2 };

    n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
        "Test files created:\n" +
        (cscFile.exists()?"Config: " + cscFile.getName() + "\n": "") +
        (jsFile.exists()?"Script: " + jsFile.getName() + "\n": "") +
        (infoFile.exists()?"Info: " + infoFile.getName() + "\n": "") +
        "\n" +
        "Run exported test in forked Cooja now?",
        "Run test?", JOptionPane.YES_NO_OPTION,
        JOptionPane.QUESTION_MESSAGE, null, options, s1);
    if (n != JOptionPane.YES_OPTION) {
      return;
    }

    try {
      final Process externalCoojaProcess;
      final MessageList testOutput = new MessageList();

      JPanel progressPanel = new JPanel(new BorderLayout());
      final JDialog progressDialog = new JDialog((Window)GUI.getTopParentContainer(), (String) null);
      progressDialog.setTitle("Running test...");

      String command[] = {
          "java",
          "-jar",
          "../dist/cooja.jar",
          "-nogui",
          "-test=" + testName
      };
      externalCoojaProcess = Runtime.getRuntime().exec(command, null, testDir);
      final BufferedReader input = new BufferedReader(new InputStreamReader(externalCoojaProcess.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(externalCoojaProcess.getErrorStream()));

      final JButton button = new JButton("Abort test");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          externalCoojaProcess.destroy();
          if (progressDialog.isDisplayable()) {
            progressDialog.dispose();
          }
        }
      });

      progressPanel.add(BorderLayout.CENTER, new JScrollPane(testOutput));
      progressPanel.add(BorderLayout.SOUTH, button);
      progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
      progressPanel.setVisible(true);

      progressDialog.getContentPane().add(progressPanel);
      progressDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

      progressDialog.getRootPane().setDefaultButton(button);
      progressDialog.setSize(500, 300);
      progressDialog.setLocationRelativeTo(ScriptRunner.this.pluginGUI);
      progressDialog.setVisible(true);

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = input.readLine()) != null) {
              if (testOutput != null) {
                testOutput.addMessage(readLine, MessageList.NORMAL);
              }
            }

          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }

          testOutput.addMessage("", MessageList.NORMAL);
          testOutput.addMessage("", MessageList.NORMAL);
          testOutput.addMessage("", MessageList.NORMAL);

          /* Parse log file for success info */
          try {
            BufferedReader in = new BufferedReader(new InputStreamReader(
                new FileInputStream(logFile)));
            boolean testSucceeded = false;

            while (in.ready()) {
              String line = in.readLine();
              if (line == null) {
                line = "";
              }
              testOutput.addMessage(line, MessageList.NORMAL);
              if (line.contains("TEST OK")) {
                testSucceeded = true;
               break;
              }
            }
            if (testSucceeded) {
              progressDialog.setTitle("Test run completed. Test succeeded!");
              button.setText("Test OK");
            } else {
              progressDialog.setTitle("Test run completed. Test failed!");
              button.setText("Test failed");
            }
          } catch (FileNotFoundException e) {
            logger.fatal("File not found: " + e);
            progressDialog.setTitle("Test run completed. Test failed! (no logfile)");
            button.setText("Test failed");
          } catch (IOException e) {
            logger.fatal("IO error: " + e);
            progressDialog.setTitle("Test run completed. Test failed! (IO exception)");
            button.setText("Test failed");
          }

        }
      }, "read input stream thread");

      Thread readError = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = err.readLine()) != null) {
              if (testOutput != null) {
                testOutput.addMessage(readLine, MessageList.ERROR);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read input stream thread");

      readInput.start();
      readError.start();

    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public boolean activateTest(File config, File script, File log) {
    try {
      /* Load simulation */
      final Simulation sim = gui.loadSimulationConfig(config, true);
      if (sim == null) {
        gui.doQuit(false);
        return false;
      }
      sim.setDelayTime(0);
      gui.setSimulation(sim);

      /* Load test script */
      BufferedReader in = new BufferedReader(new FileReader(script));
      String line, code = "";
      while ((line = in.readLine()) != null) {
        code += line + "\n";
      }
      in.close();

      /* Prepare test log */
      logWriter = new BufferedWriter(new FileWriter(log));
      logWriter.write("[" + log.getName() + "]\n\n");

      /* Create script engine */
      engine = new LogScriptEngine(gui);
      try {
        engine.activateScript(code);
      } catch (ScriptException e) {
        logger.fatal("Test script error, terminating Cooja.");
        e.printStackTrace();
        System.exit(1);
      }

      engine.setScriptLogObserver(new Observer() {
        public void update(Observable obs, Object obj) {
          try {
            if (logWriter != null) {
              logWriter.write((String) obj);
              logWriter.flush();
            }
          } catch (IOException e) {
            logger.fatal("Error when writing to test log file: " + obj);
          }
        }
      });

      /* Start simulation and leave control to script */
      sim.startSimulation();
    } catch (IOException e) {
      logger.fatal("Error when running script: " + e);
      System.exit(1);
      return false;
    } catch (UnsatisfiedLinkError e) {
      logger.fatal("Error when running script: " + e);
      System.exit(1);
      return false;
    } catch (SimulationCreationException e) {
      logger.fatal("Error when running script: " + e);
      System.exit(1);
      return false;
    }

    return true;
  }

  public void closePlugin() {
    if (engine != null) {
      engine.deactivateScript();
      engine.setScriptLogObserver(null);
      engine = null;
    }
    if (logWriter != null) {
      try {
        logWriter.close();
      } catch (IOException e) {
      }
      logWriter = null;
    }
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

  public void tagWithObject(Object tag) {
    this.coojaTag = tag;
  }

  public Object getTag() {
    return coojaTag;
  }

}
