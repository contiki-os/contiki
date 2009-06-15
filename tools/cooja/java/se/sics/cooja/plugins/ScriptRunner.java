/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: ScriptRunner.java,v 1.20 2009/06/15 15:41:32 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.*;

import javax.script.ScriptException;
import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.plugins.ScriptParser.ScriptSyntaxErrorException;
import se.sics.cooja.util.StringUtils;

@ClassDescription("Contiki Test Editor")
@PluginType(PluginType.SIM_PLUGIN)
public class ScriptRunner extends VisPlugin {
  private static Logger logger = Logger.getLogger(ScriptRunner.class);

  final String[] EXAMPLE_SCRIPTS = new String[] {
      "basic.js", "Basic example script",
      "helloworld.js", "Wait for 'Hello, world'",
  };

  private Simulation simulation;

  private LogScriptEngine engine = null;

  private File coojaBuild;
  private File coojaJAR;
  private final File logFile;
  private static BufferedWriter logWriter = null; /* For non-GUI tests */

  private JTextArea scriptTextArea = null;

  private JTextArea logTextArea = null;
  private JButton toggleButton = null;
  private JButton examplesButton = null;

  private int scriptFirstLinesNumber;

  public ScriptRunner(Simulation simulation, GUI gui) {
    super("Contiki Test Editor", gui, false);
    this.simulation = simulation;

    try {
      coojaBuild = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/build");
      coojaJAR = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/dist/cooja.jar");
      coojaBuild = coojaBuild.getCanonicalFile();
      coojaJAR = coojaJAR.getCanonicalFile();
    } catch (IOException e) {
      coojaBuild = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/build");
      coojaJAR = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/dist/cooja.jar");
    }
    logFile = new File(coojaBuild, "COOJA.testlog");
    
    final JTextArea lineTextArea = new JTextArea();
    lineTextArea.setEnabled(false);
    lineTextArea.setMargin(new Insets(5,0,5,0));

    try {
      scriptFirstLinesNumber = new ScriptParser("").getJSCode().split("\n").length + 2;
    } catch (ScriptSyntaxErrorException e1) {
      scriptFirstLinesNumber = 1;
    }

    /* Examples popup menu */
    final JPopupMenu popupMenu = new JPopupMenu();
    JMenuItem moteItem;
    moteItem = new JMenuItem("Example script to import:");
    moteItem.setEnabled(false);
    popupMenu.add(moteItem);
    popupMenu.addSeparator();

    for (int i=0; i < EXAMPLE_SCRIPTS.length; i += 2) {
      final String file = EXAMPLE_SCRIPTS[i];
      moteItem = new JMenuItem(EXAMPLE_SCRIPTS[i+1]);
      moteItem.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          String script = loadScript(file);
          if (script == null) {
            JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
                "Could not load example script: scripts/" + file,
                "Could not load script", JOptionPane.ERROR_MESSAGE);
            return;
          }
          updateScript(script);
        }
      });
      popupMenu.add(moteItem);
    }

    examplesButton = new JButton("Example scripts");
    examplesButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        popupMenu.show(examplesButton, 0, examplesButton.getHeight());
      }
    });

    /* Script area */
    scriptTextArea = new JTextArea(12,50);
    scriptTextArea.getDocument().addDocumentListener(new DocumentListener() {
      private int lastLines = -1;

      private void update() {
        int lines = scriptTextArea.getLineCount() + scriptFirstLinesNumber;
        if (lines == lastLines) {
          return;
        }
        lastLines = lines;

        String txt = "";
        for (int i=scriptFirstLinesNumber; i < 10; i++) {
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

    JButton runTestButton = new JButton("Run without GUI");
    runTestButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        exportAndRun();
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
    buttonPanel.add(BorderLayout.CENTER, toggleButton);
    buttonPanel.add(BorderLayout.WEST, examplesButton);  
    buttonPanel.add(BorderLayout.EAST, runTestButton);

    JPanel southPanel = new JPanel(new BorderLayout());
    southPanel.add(BorderLayout.EAST, buttonPanel);

    getContentPane().add(BorderLayout.CENTER, centerPanel);
    getContentPane().add(BorderLayout.SOUTH, southPanel);

    setSize(600, 700);
    Dimension maxSize = gui.getDesktopPane().getSize();
    if (getWidth() > maxSize.getWidth()) {
      setSize((int)maxSize.getWidth(), getHeight());
    }
    if (getHeight() > maxSize.getHeight()) {
      setSize(getWidth(), (int)maxSize.getHeight());
    }

    /* Set default script */
    String script = loadScript(EXAMPLE_SCRIPTS[0]);
    if (script != null) {
      updateScript(script);
    }
  }

  public void setScriptActive(boolean active) {
    if (active) {
      /* Free any resources */
      setScriptActive(false);

      /* Create new engine */
      engine = new LogScriptEngine(simulation);
      if (GUI.isVisualized()) {
        /* Attach visualized log observer */
        engine.setScriptLogObserver(new Observer() {
          public void update(Observable obs, Object obj) {
            logTextArea.append((String) obj);
            logTextArea.setCaretPosition(logTextArea.getText().length());
          }
        });
      } else {
        try {
          /* Continously write test output to file */
          if (logWriter == null) {
            /* Warning: static variable, used by all active test editor plugins */
            if (logFile.exists()) {
              logFile.delete();
            }
            logWriter = new BufferedWriter(new FileWriter(logFile));
          }
          engine.setScriptLogObserver(new Observer() {
            public void update(Observable obs, Object obj) {
              try {
                if (logWriter != null) {
                  logWriter.write((String) obj);
                  logWriter.flush();
                } else {
                  logger.fatal("No log writer: " + obj);
                }
              } catch (IOException e) {
                logger.fatal("Error when writing to test log file: " + obj, e);
              }
            }
          });
        } catch (Exception e) {
          logger.fatal("Create log writer error: ", e);
          setScriptActive(false);
        }
      }

      /* Activate engine */
      try {
        engine.activateScript(scriptTextArea.getText());

        toggleButton.setText("Deactivate");
        examplesButton.setEnabled(false);
        logTextArea.setText("");
        scriptTextArea.setEnabled(false);
        setTitle("Contiki Test Editor (ACTIVE)");

        logger.info("Test script activated");

      } catch (ScriptException e) {
        logger.fatal("Test script error: ", e);
        setScriptActive(false);
        if (GUI.isVisualized()) {
          GUI.showErrorDialog(GUI.getTopParentContainer(),
              "Script error", e, false);
        }
      } catch (RuntimeException e) {
        logger.fatal("Test script error: ", e);
        setScriptActive(false);
      }

    } else {
      if (engine == null) {
        return;
      }

      /* Deactivate script */
      engine.deactivateScript();
      engine.setScriptLogObserver(null);
      engine = null;

      if (logWriter != null) {
        try {
          logWriter.close();
        } catch (IOException e) {
        }
        logWriter = null;
      }

      toggleButton.setText("Activate");
      examplesButton.setEnabled(true);
      scriptTextArea.setEnabled(true);
      logger.info("Test script deactivated");
      setTitle("Contiki Test Editor");

      /* Automatically exit COOJA */
      if (!GUI.isVisualized()) {
        System.exit(1);
      }
    }
  }

  private void exportAndRun() {
    /* Save simulation config */
    File configFile = simulation.getGUI().doSaveConfig(true);
    if (configFile == null) {
      return;
    }

    /* Start test in external process */
    try {
      JPanel progressPanel = new JPanel(new BorderLayout());
      final JDialog progressDialog = new JDialog((Window)GUI.getTopParentContainer(), (String) null);
      progressDialog.setTitle("Running test...");

      if (!coojaJAR.exists()) {
        JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
            "Can't start COOJA, cooja.jar not found:" +
            "\n" + coojaJAR.getAbsolutePath()
            + "\n\nVerify that PATH_CONTIKI is correct in external tools settings.",
            "cooja.jar not found", JOptionPane.ERROR_MESSAGE);
        return;
      }

      String command[] = {
          "java",
          "-jar",
          "../dist/cooja.jar",
          "-nogui=" + configFile.getAbsolutePath()
      };

      /* User confirmation */
      String s1 = "Start";
      String s2 = "Cancel";
      int n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
          "Starting COOJA in " + coojaBuild.getPath() + ":\n" + 
          " " + command[0] + " " + command[1] + " " + command[2] + " " + command[3] + "\n",
          "Starting COOJA without GUI", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, new Object[] { s1, s2 }, s1);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }

      /* Start process */
      final Process process = Runtime.getRuntime().exec(command, null, coojaBuild);
      final BufferedReader input = new BufferedReader(new InputStreamReader(process.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(process.getErrorStream()));

      /* GUI components */
      final MessageList testOutput = new MessageList();
      final AbstractAction abort = new AbstractAction() {
        public void actionPerformed(ActionEvent e) {
          process.destroy();
          if (progressDialog.isDisplayable()) {
            progressDialog.dispose();
          }
        }
      };
      abort.putValue(AbstractAction.NAME, "Abort test");
      final JButton button = new JButton(abort);

      progressPanel.add(BorderLayout.CENTER, new JScrollPane(testOutput));
      progressPanel.add(BorderLayout.SOUTH, button);
      progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
      progressPanel.setVisible(true);

      progressDialog.getContentPane().add(progressPanel);
      progressDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);

      progressDialog.getRootPane().setDefaultButton(button);
      progressDialog.setSize(800, 300);
      progressDialog.setLocationRelativeTo(ScriptRunner.this);
      progressDialog.addWindowListener(new WindowAdapter() {
        public void windowClosed(WindowEvent e) {
          abort.actionPerformed(null);
        }
      });
      progressDialog.setVisible(true);

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = input.readLine()) != null) {
              testOutput.addMessage(line, MessageList.NORMAL);
            }
          } catch (IOException e) {
          }

          testOutput.addMessage("", MessageList.NORMAL);
          testOutput.addMessage("", MessageList.NORMAL);
          testOutput.addMessage("", MessageList.NORMAL);

          /* Parse log file, check if test succeeded  */
          try {
            String log = StringUtils.loadFromFile(logFile);
            if (log == null) {
              throw new FileNotFoundException(logFile.getPath());
            }
            String[] lines = log.split("\n");
            boolean testSucceeded = false;
            for (String l: lines) {
              if (l == null) {
                line = "";
              }
              testOutput.addMessage(l, MessageList.NORMAL);
              if (l.contains("TEST OK")) {
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
            logger.fatal("No test output : " + logFile);
            progressDialog.setTitle("Test run completed. Test failed! (no logfile)");
            button.setText("Test failed");
          }

        }
      });

      Thread readError = new Thread(new Runnable() {
        public void run() {
          String line;
          try {
            while ((line = err.readLine()) != null) {
              testOutput.addMessage(line, MessageList.ERROR);
            }
          } catch (IOException e) {
          }
        }
      });

      readInput.start();
      readError.start();

    } catch (IOException e) {
      e.printStackTrace();
    }
  }

  public boolean updateScript(File scriptFile) {
    String script = StringUtils.loadFromFile(scriptFile);
    if (script == null) {
      return false;
    }
    updateScript(script);
    return true;
  }

  private void updateScript(String script) {
    if (script == null) {
      return;
    }

    scriptTextArea.setText(script);
    logTextArea.setText("");
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    element = new Element("script");
    element.setText(scriptTextArea.getText());
    config.add(element);

    element = new Element("active");
    element.setText("" + (engine != null));
    config.add(element);

    return config;
  }

  public void closePlugin() {
    setScriptActive(false);
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();
      if ("script".equals(name)) {
        if (!element.getText().isEmpty()) {
          updateScript(element.getText());
        }
      } else if ("active".equals(name)) {
        boolean active = Boolean.parseBoolean(element.getText());
        if (GUI.isVisualized()) {
          setScriptActive(active);
        }
      }
    }

    if (!GUI.isVisualized()) {
      /* Automatically activate script */
      setScriptActive(true);
      simulation.setDelayTime(0);
      simulation.startSimulation();
    }
    return true;
  }

  private static String loadScript(String file) {
    return StringUtils.loadFromURL(ScriptRunner.class.getResource("/scripts/" + file));
  }

}
