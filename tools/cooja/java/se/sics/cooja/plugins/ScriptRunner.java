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
 * $Id: ScriptRunner.java,v 1.4 2008/09/29 13:03:29 fros4943 Exp $
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
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintStream;
import java.io.StringReader;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.*;

import org.apache.log4j.Logger;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.*;
import se.sics.cooja.dialogs.MessageList;

@ClassDescription("(GUI) Test Script Editor")
@PluginType(PluginType.COOJA_PLUGIN)
public class ScriptRunner extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ScriptRunner.class);

  private JTextArea scriptTextArea;
  private JTextArea logTextArea;
  private GUI gui;
  private LogScriptEngine scriptTester = null;
  private JButton toggleButton;
  private String oldTestName = null;
  private String oldInfo = null;

  private static String exampleScript =
    "/* Script is called once for every node log output. */\n" +
    "/* Input variables: Mote mote, int id, String msg. */\n" +
    "\n" +
    "log.log('MOTE=' + mote + '\\n');\n" +
    "log.log('ID=' + id + '\\n');\n" +
    "log.log('TIME=' + mote.getSimulation().getSimulationTime() + '\\n');\n" +
    "log.log('MSG=' + msg + '\\n');\n" +
    "\n" +
    "log.log('STORED VAR=' + global.get('storedVar') + '\\n');\n" +
    "global.put('storedVar', msg);\n" +
    "\n" +
    "log.log('TEST OK\\n'); /* Report test success */\n" +
    "\n" +
    "/* To increase test run speed, close the simulator when done */\n" +
    "//mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/\n" +
    "\n" +
    "//mote.getSimulation().getGUI().reloadCurrentSimulation(true); /* Reload simulation */\n";

  public ScriptRunner(GUI gui) {
    super("(GUI) Test Script Editor", gui);
    this.gui = gui;

    scriptTextArea = new JTextArea(8,50);
    scriptTextArea.setMargin(new Insets(5,5,5,5));
    scriptTextArea.setEditable(true);
    scriptTextArea.setCursor(null);
    scriptTextArea.setText(exampleScript);

    logTextArea = new JTextArea(8,50);
    logTextArea.setMargin(new Insets(5,5,5,5));
    logTextArea.setEditable(true);
    logTextArea.setCursor(null);

    toggleButton = new JButton("Activate");
    toggleButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        if (toggleButton.getText().equals("Activate")) {
          scriptTester = new LogScriptEngine(ScriptRunner.this.gui, scriptTextArea.getText());
          scriptTester.setScriptLogObserver(new Observer() {
            public void update(Observable obs, Object obj) {
              logTextArea.append((String) obj);
              logTextArea.setCaretPosition(logTextArea.getText().length());
            }
          });
          scriptTester.activateScript();
          toggleButton.setText("Deactivate");
          scriptTextArea.setEnabled(false);

        } else {
          if (scriptTester != null) {
            scriptTester.deactiveScript();
            scriptTester = null;
          }
          toggleButton.setText("Activate");
          scriptTextArea.setEnabled(true);
        }
      }
    });

    JButton importButton = new JButton("Import Contiki test");
    importButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        Runnable doImport = new Runnable() {
          public void run() {
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

    JSplitPane centerPanel = new JSplitPane(
        JSplitPane.VERTICAL_SPLIT,
        new JScrollPane(scriptTextArea),
        new JScrollPane(logTextArea)
    );

    JPanel buttonPanel = new JPanel(new BorderLayout());
    buttonPanel.add(BorderLayout.WEST, importButton);
    buttonPanel.add(BorderLayout.CENTER, toggleButton);
    buttonPanel.add(BorderLayout.EAST, exportButton);

    JPanel southPanel = new JPanel(new BorderLayout());
    southPanel.add(BorderLayout.EAST, buttonPanel);

    getContentPane().add(BorderLayout.CENTER, centerPanel);
    getContentPane().add(BorderLayout.SOUTH, southPanel);

    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  private void importContikiTest() {
    Simulation simulation = ScriptRunner.this.gui.getSimulation();

    /* Load config from test directory */
    final File proposedDir = new File(GUI.getExternalToolsSetting("PATH_CONTIKI") + "/tools/cooja/contiki_tests");
    if (!proposedDir.exists()) {
      logger.fatal("Test directory does not exist: " + proposedDir.getPath());
      return;
    }

    new Thread(new Runnable() {
      public void run() {
        gui.doLoadConfig(false, true, proposedDir);
        Vector<File> history = gui.getFileHistory();

        File cscFile = history.firstElement();
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
          scriptTextArea.setText("");
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

      }
    }).start();

  }

  private void exportAsContikiTest() {

    Simulation simulation = ScriptRunner.this.gui.getSimulation();
    if (simulation == null) {
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          "No simulation loaded. Aborting.", "Error", JOptionPane.ERROR_MESSAGE);
      return;
    }

    /* Confirm test directory */
    File testDir = new File(GUI.getExternalToolsSetting("PATH_CONTIKI") + "/tools/cooja/contiki_tests");
    String s1 = "Ok";
    String s2 = "Cancel";
    Object[] options = { s1, s2 };
    int n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
        "Export current simulation config (.csc) and test script (.js)\n" +
        "to directory '" + testDir.getPath() + "'?",
        "Export Contiki test", JOptionPane.YES_NO_OPTION,
        JOptionPane.QUESTION_MESSAGE, null, options, s1);
    if (n != JOptionPane.YES_OPTION) {
      return;
    }
    if (!testDir.exists()) {
      logger.fatal("Test directory does not exist: " + testDir.getPath());
      return;
    }

    /* Name test to export */
    if (oldTestName == null) {
      oldTestName = "mytest";
    }
    String testName = (String) JOptionPane.showInputDialog(GUI.getTopParentContainer(),
        "Enter test name. No spaces or strange chars allowed.",
        "Test name", JOptionPane.PLAIN_MESSAGE, null, null,
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
    Element root = new Element("simconf");
    Element simulationElement = new Element("simulation");
    simulationElement.addContent(simulation.getConfigXML());
    root.addContent(simulationElement);

    /* Strip plugins */
    Collection<Element> pluginsConfig = ScriptRunner.this.gui.getPluginsConfigXML();
    if (pluginsConfig != null) {
      root.addContent(pluginsConfig);
    }
//    if (pluginsConfig != null) {
//      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
//          "Stripping plugin configuration.\n" +
//          "(Exporting non-GUI plugins not implemented.)",
//          "Plugins detected", JOptionPane.WARNING_MESSAGE);
//    }

    /* Fix simulation delay */
    root.detach();
    String configString = new XMLOutputter().outputString(new Document(root));
    String identifierExtraction = "<delaytime>([^<]*)</delaytime>";
    Matcher matcher = Pattern.compile(identifierExtraction).matcher(configString);
    while (matcher.find()) {
      int delay = Integer.parseInt(matcher.group(1));
      if (delay != 0) {
        JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
            "Simulation delay currently set to " + delay + ".\n" +
            "Changing delay time to 0 in exported test.",
            "Non-zero delay time detected", JOptionPane.WARNING_MESSAGE);
      }
      configString = configString.replace(
          "<delaytime>" + matcher.group(1) + "</delaytime>",
      "<delaytime>0</delaytime>");
    }

    /* Export .csc */
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
          "Optional test info",
          "(OPTIONAL) Enter test description", JOptionPane.PLAIN_MESSAGE, null, null,
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
    n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
        "Run exported test in forked Cooja now?",
        "Run test?", JOptionPane.YES_NO_OPTION,
        JOptionPane.QUESTION_MESSAGE, null, options, s1);
    if (n != JOptionPane.YES_OPTION) {
      return;
    }

    try {
      final Process externalCoojaProcess;
      MessageList testOutput = new MessageList();
      final PrintStream normal = testOutput.getInputStream(MessageList.NORMAL);
      final PrintStream error = testOutput.getInputStream(MessageList.ERROR);

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
      progressDialog.pack();

      progressDialog.getRootPane().setDefaultButton(button);
      progressDialog.setLocationRelativeTo(ScriptRunner.this);
      progressDialog.setVisible(true);

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = input.readLine()) != null) {
              if (normal != null) {
                normal.println(readLine);
              }
            }

          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }

          normal.println("");
          normal.println("");
          normal.println("");

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
              normal.println(line);
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
              if (error != null) {
                error.println(readLine);
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

  public void closePlugin() {
    if (scriptTester != null) {
      scriptTester.deactiveScript();
      scriptTester.setScriptLogObserver(null);
    }
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

}
