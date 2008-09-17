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
 * $Id: ScriptRunner.java,v 1.2 2008/09/17 15:22:39 fros4943 Exp $
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

  public ScriptRunner(GUI gui) {
    super("Script Runner (Log)", gui);
    this.gui = gui;

    scriptTextArea = new JTextArea(8,50);
    scriptTextArea.setMargin(new Insets(5,5,5,5));
    scriptTextArea.setEditable(true);
    scriptTextArea.setCursor(null);
    scriptTextArea.setText(
        "/* Script is called once for every node log output. */\n" +
        "/* Input variables: Mote mote, int id, String msg. */\n" +
        "\n" +
        "log.log('MOTE=' + mote + '\\n');\n" +
        "log.log('ID=' + id + '\\n');\n" +
        "log.log('TIME=' + mote.getSimulation().getSimulationTime() + '\\n');\n" +
        "log.log('MSG=' + msg + '\\n');\n" +
        "\n" +
        "log.log('TEST OK\\n'); /* Report test success */\n" +
        "//mote.getSimulation().getGUI().doQuit(false); /* Quit simulator (to end test run)*/\n" +
        "\n" +
        "//mote.getSimulation().getGUI().reloadCurrentSimulation(true); /* Reload simulation */\n"
    );

    logTextArea = new JTextArea(8,50);
    logTextArea.setMargin(new Insets(5,5,5,5));
    logTextArea.setEditable(false);
    logTextArea.setCursor(null);

    toggleButton = new JButton("Activate");
    toggleButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        if (toggleButton.getText().equals("Activate")) {
          scriptTester = new LogScriptEngine(ScriptRunner.this.gui, scriptTextArea.getText());
          scriptTester.activateScript();
          scriptTester.setScriptLogObserver(new Observer() {
            public void update(Observable obs, Object obj) {
              logTextArea.append((String) obj);
            }
          });
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

    JButton exportButton = new JButton("Export as Contiki test");
    exportButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
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
        String testName = (String) JOptionPane.showInputDialog(GUI.getTopParentContainer(),
            "Enter test name. No spaces or strange chars allowed.",
            "Test name", JOptionPane.PLAIN_MESSAGE, null, null,
            "mytest");
        if (testName == null) {
          return;
        }
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
          JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
              "Stripping plugin configuration.\n" +
              "(Exporting non-GUI plugins not implemented.)",
              "Plugins detected", JOptionPane.WARNING_MESSAGE);
        }

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
          String info = JOptionPane.showInputDialog(GUI.getTopParentContainer(),
              "(OPTIONAL) Enter test description",
              "Optional test info", JOptionPane.QUESTION_MESSAGE);
          if (info != null && !info.equals("")) {
            BufferedWriter writer =
              new BufferedWriter(new OutputStreamWriter(new FileOutputStream(infoFile)));
            writer.write(info);
            writer.close();
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

          JButton button = new JButton("Abort test/Close dialog");
          button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
              if (externalCoojaProcess != null) {
                externalCoojaProcess.destroy();
              }
              if (progressDialog != null && progressDialog.isDisplayable()) {
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
                  if (normal != null && readLine != null) {
                    normal.println(readLine);
                  }
                }

              } catch (IOException e) {
                logger.warn("Error while reading from process");
              }

            /* Parse log file for success info */
              try {
                BufferedReader in = new BufferedReader(new InputStreamReader(
                    new FileInputStream(logFile)));
                boolean testSucceeded = false;
                while (in.ready()) {
                  String line = in.readLine();
                  if (line.contains("TEST OK")) {
                    testSucceeded = true;
                    break;
                  }
                }
                if (testSucceeded) {
                  progressDialog.setTitle("Test run completed. Test succeeded!");
                } else {
                  progressDialog.setTitle("Test run completed. Test failed!");
                }
              } catch (FileNotFoundException e) {
                logger.fatal("File not found: " + e);
              } catch (IOException e) {
                logger.fatal("IO error: " + e);
              }

            }
          }, "read input stream thread");

          Thread readError = new Thread(new Runnable() {
            public void run() {
              String readLine;
              try {
                while ((readLine = err.readLine()) != null) {
                  if (error != null && readLine != null) {
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
    });

    JPanel centerPanel = new JPanel(new BorderLayout());
    centerPanel.add(BorderLayout.CENTER, new JScrollPane(scriptTextArea));
    centerPanel.add(BorderLayout.SOUTH, new JScrollPane(logTextArea));

    JPanel buttonPanel = new JPanel(new BorderLayout());
    buttonPanel.add(BorderLayout.WEST, toggleButton);
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
