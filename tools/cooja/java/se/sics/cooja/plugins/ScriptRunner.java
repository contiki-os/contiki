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
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.GraphicsEnvironment;
import java.awt.Insets;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.script.ScriptException;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JDialog;
import javax.swing.JEditorPane;
import javax.swing.JFileChooser;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.filechooser.FileFilter;

import jsyntaxpane.DefaultSyntaxKit;
import jsyntaxpane.actions.DefaultSyntaxAction;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.PluginType;
import se.sics.cooja.Simulation;
import se.sics.cooja.VisPlugin;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.util.StringUtils;

@ClassDescription("Simulation script editor")
@PluginType(PluginType.SIM_PLUGIN)
public class ScriptRunner extends VisPlugin {
  private static final long serialVersionUID = 7614358340336799109L;
  private static Logger logger = Logger.getLogger(ScriptRunner.class);

  static boolean headless;
  {
    headless = GraphicsEnvironment.isHeadless();
    if (!headless) {
      DefaultSyntaxKit.initKit();
    }
  }

  final String[] EXAMPLE_SCRIPTS = new String[] {
      "basic.js", "Various commands",
      "helloworld.js", "Wait for 'Hello, world'",
      "log_all.js", "Just log all printf()'s and timeout",
      "shell.js", "Basic shell interaction",
      "plugins.js", "Interact with surrounding Cooja plugins",
  };

  private Simulation simulation;
  private LogScriptEngine engine;

  private static BufferedWriter logWriter = null; /* For non-GUI tests */

  private JEditorPane codeEditor;
  private JTextArea logTextArea;
  private JSplitPane centerPanel;

  private JSyntaxLinkFile actionLinkFile = null;
  private File linkedFile = null;

  public ScriptRunner(Simulation simulation, GUI gui) {
    super("Simulation script editor", gui, false);
    this.simulation = simulation;
    this.engine = null;

    /* Menus */
    JMenuBar menuBar = new JMenuBar();
    JMenu fileMenu = new JMenu("File");
    JMenu editMenu = new JMenu("Edit");
    JMenu runMenu = new JMenu("Run");

    menuBar.add(fileMenu);
    menuBar.add(editMenu);
    menuBar.add(runMenu);

    this.setJMenuBar(menuBar);

    /* Example scripts */
    final JMenu examplesMenu = new JMenu("Load example script");
    for (int i=0; i < EXAMPLE_SCRIPTS.length; i += 2) {
      final String file = EXAMPLE_SCRIPTS[i];
      JMenuItem exampleItem = new JMenuItem(EXAMPLE_SCRIPTS[i+1]);
      exampleItem.addActionListener(new ActionListener() {
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
      examplesMenu.add(exampleItem);
    }
    fileMenu.add(examplesMenu);

    {
      /* XXX Workaround to configure jsyntaxpane */
      JEditorPane e = new JEditorPane();
      new JScrollPane(e);
      e.setContentType("text/javascript");
      if (e.getEditorKit() instanceof DefaultSyntaxKit) {
        DefaultSyntaxKit kit = (DefaultSyntaxKit) e.getEditorKit();
        kit.setProperty("PopupMenu", "copy-to-clipboard,-,find,find-next,goto-line,-,linkfile");
        kit.setProperty("Action.linkfile", JSyntaxLinkFile.class.getName());
        kit.setProperty("Action.execute-script", "jsyntaxpane.actions.ScriptRunnerAction");
      }
    }

    /* Script area */
    setLayout(new BorderLayout());
    codeEditor = new JEditorPane();

    logTextArea = new JTextArea(12,50);
    logTextArea.setMargin(new Insets(5,5,5,5));
    logTextArea.setEditable(true);
    logTextArea.setCursor(null);

    final JCheckBoxMenuItem activateMenuItem = new JCheckBoxMenuItem("Activate");
    activateMenuItem.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        try {
          setScriptActive(!isActive());
        } catch (Exception e) {
          logger.fatal("Error: " + e.getMessage(), e);
        }
      }
    });
    runMenu.add(activateMenuItem);

    final JMenuItem runTestMenuItem = new JMenuItem("Save simulation and run with script");
    runMenu.add(runTestMenuItem);
    runTestMenuItem.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        exportAndRun();
      }
    });

    doLayout();
    centerPanel = new JSplitPane(
        JSplitPane.VERTICAL_SPLIT,
        new JScrollPane(codeEditor),
        new JScrollPane(logTextArea)
    );

    MenuListener toggleMenuItems = new MenuListener() {
      public void menuSelected(MenuEvent e) {
        activateMenuItem.setSelected(isActive());
        runTestMenuItem.setEnabled(!isActive());
        examplesMenu.setEnabled(!isActive());
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    };
    fileMenu.addMenuListener(toggleMenuItems);
    editMenu.addMenuListener(toggleMenuItems);
    runMenu.addMenuListener(toggleMenuItems);


    codeEditor.setContentType("text/javascript");
    if (codeEditor.getEditorKit() instanceof DefaultSyntaxKit) {
      DefaultSyntaxKit kit = (DefaultSyntaxKit) codeEditor.getEditorKit();
      kit.setProperty("PopupMenu", "copy-to-clipboard,-,find,find-next,goto-line,-,linkfile");
      kit.setProperty("Action.linkfile", JSyntaxLinkFile.class.getName());
      kit.setProperty("Action.execute-script", "jsyntaxpane.actions.ScriptRunnerAction");
    }

    JPopupMenu p = codeEditor.getComponentPopupMenu();
    if (p != null) {
      for (Component c: p.getComponents()) {
        if (!(c instanceof JMenuItem)) {
          continue;
        }
        if (((JMenuItem) c).getAction() == null) {
          continue;
        }
        Action a = ((JMenuItem) c).getAction();
        if (a instanceof JSyntaxLinkFile) {
          actionLinkFile = (JSyntaxLinkFile) a;
          actionLinkFile.setMenuText("Link script to disk file");
          actionLinkFile.putValue("ScriptRunner", this);
        }
      }
    }

    centerPanel.setOneTouchExpandable(true);
    centerPanel.setResizeWeight(0.5);

    JPanel buttonPanel = new JPanel(new BorderLayout());
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

  public void setLinkFile(File source) {
    linkedFile = source;
    if (source == null) {
      updateScript("");

      if (actionLinkFile != null) {
        actionLinkFile.setMenuText("Link script to disk file");
        actionLinkFile.putValue("JavascriptSource", null);
      }

      codeEditor.setEditable(true);
    } else {
      updateScript(linkedFile);
      GUI.setExternalToolsSetting("SCRIPTRUNNER_LAST_SCRIPTFILE", source.getAbsolutePath());

      if (actionLinkFile != null) {
        actionLinkFile.setMenuText("Unlink script: " + source.getName());
        actionLinkFile.putValue("JavascriptSource", source);
      }

      codeEditor.setEditable(false);
    }
    updateTitle();
  }

  public void setScriptActive(boolean active)
  throws Exception {
    if (active) {
      /* setScriptActive(true) */

      /* Free any resources */
      setScriptActive(false);

      /* Reload script from file */
      if (linkedFile != null) {
        String script = StringUtils.loadFromFile(linkedFile);
        if (script == null) {
          logger.fatal("Failed to load script from: " + linkedFile.getAbsolutePath());
        } else {
          updateScript(script);
        }
      }

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
            File logFile = new File("COOJA.testlog");
            if (logFile.exists()) {
              logFile.delete();
            }
            logWriter = new BufferedWriter(new FileWriter(logFile));
            logWriter.write("Random seed: " + simulation.getRandomSeed() + "\n");
            logWriter.flush();
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
        engine.activateScript(codeEditor.getText());

        if (!headless) {
          if (actionLinkFile != null) {
            actionLinkFile.setEnabled(false);
          }
          logTextArea.setText("");
          codeEditor.setEnabled(false);
          updateTitle();
        }

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
      /* setScriptActive(false) */

      if (engine != null) {
        /* Deactivate script */
        engine.deactivateScript();
        engine.setScriptLogObserver(null);
        engine = null;
      }

      if (logWriter != null) {
        try {
          logWriter.write(
              "Test ended at simulation time: " +
              (simulation!=null?simulation.getSimulationTime():"?") + "\n");
          logWriter.flush();
          logWriter.close();
        } catch (IOException e) {
        }
        logWriter = null;
      }

      if (!headless) {
        if (actionLinkFile != null) {
          actionLinkFile.setEnabled(true);
        }
        codeEditor.setEnabled(true);
        updateTitle();
      }
      logger.info("Test script deactivated");
    }
  }

  private void updateTitle() {
    String title = "Simulation script editor ";
    if (linkedFile != null) {
      title += "(" + linkedFile.getName() + ") ";
    }
    if (isActive()) {
      title += "*active*";
    }
    setTitle(title);
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

      File coojaBuild;
      File coojaJAR;
      try {
        coojaBuild = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/build");
        coojaJAR = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/dist/cooja.jar");
        coojaBuild = coojaBuild.getCanonicalFile();
        coojaJAR = coojaJAR.getCanonicalFile();
      } catch (IOException e) {
        coojaBuild = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/build");
        coojaJAR = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), "tools/cooja/dist/cooja.jar");
      }

      if (!coojaJAR.exists()) {
        JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
            "Can't start Cooja, cooja.jar not found:" +
            "\n" + coojaJAR.getAbsolutePath()
            + "\n\nVerify that PATH_CONTIKI is correct in external tools settings.",
            "cooja.jar not found", JOptionPane.ERROR_MESSAGE);
        return;
      }

      final File logFile = new File(coojaBuild, "COOJA.testlog");

      String command[] = {
          "java",
          "-Djava.awt.headless=true",
          "-jar",
          "../dist/cooja.jar",
          "-nogui=" + configFile.getAbsolutePath()
      };

      /* User confirmation */
      String s1 = "Start";
      String s2 = "Cancel";
      int n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
          "Starting Cooja in " + coojaBuild.getPath() + ":\n" +
          " " + command[0] + " " + command[1] + " " + command[2] + " " + command[3] + "\n",
          "Starting Cooja without GUI", JOptionPane.YES_NO_OPTION,
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
              progressDialog.setTitle("Test run completed. Test succeeded.");
              button.setText("Test OK");
            } else {
              progressDialog.setTitle("Test run completed. Test failed.");
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

    codeEditor.setText(script);
    logTextArea.setText("");
  }

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    if (linkedFile != null) {
      element = new Element("scriptfile");
      element.setText(simulation.getGUI().createPortablePath(linkedFile).getPath().replace('\\', '/'));
      config.add(element);
    } else {
      element = new Element("script");
      element.setText(codeEditor.getText());
      config.add(element);
    }

    element = new Element("active");
    element.setText("" + isActive());
    config.add(element);

    return config;
  }

  public boolean isActive() {
    return engine != null;

  }
  public void closePlugin() {
    try {
      setScriptActive(false);
    } catch (Exception e) {
    }
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();
      if ("script".equals(name)) {
        if (!element.getText().isEmpty()) {
          updateScript(element.getText());
        }
      } else if ("scriptfile".equals(name)) {
        File file = simulation.getGUI().restorePortablePath(new File(element.getText().trim()));
        setLinkFile(file);
      } else if ("active".equals(name)) {
        boolean active = Boolean.parseBoolean(element.getText());
        if (GUI.isVisualized()) {
          try {
            setScriptActive(active);
          } catch (Exception e) {
            logger.fatal("Error: " + e.getMessage(), e);
          }
        }
      }
    }

    if (!GUI.isVisualized()) {
      /* Automatically activate script */
      try {
        setScriptActive(true);
      } catch (Exception e) {
        logger.fatal("Error: " + e.getMessage(), e);
      }
    }
    return true;
  }

  private static String loadScript(String file) {
    return StringUtils.loadFromURL(ScriptRunner.class.getResource("/scripts/" + file));
  }

  public static class JSyntaxLinkFile extends DefaultSyntaxAction {
    private static Logger logger = Logger.getLogger(JSyntaxLinkFile.class);

    public JSyntaxLinkFile() {
      super("linkfile");
    }

    public void actionPerformed(ActionEvent e) {
      JMenuItem menuItem = (JMenuItem) e.getSource();
      Action action = menuItem.getAction();
      ScriptRunner scriptRunner = (ScriptRunner) action.getValue("ScriptRunner");
      File currentSource = (File) action.getValue("JavascriptSource");

      if (currentSource != null) {
        scriptRunner.setLinkFile(null);
        return;
      }

      JFileChooser fileChooser = new JFileChooser();
      String suggest = GUI.getExternalToolsSetting("SCRIPTRUNNER_LAST_SCRIPTFILE", null);
      if (suggest != null) {
        fileChooser.setSelectedFile(new File(suggest));
      } else {
        fileChooser.setCurrentDirectory(new java.io.File("."));
      }
      fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
      fileChooser.setDialogTitle("Select script file");
      fileChooser.setFileFilter(new FileFilter() {
        public boolean accept(File file) {
          if (file.isDirectory()) { return true; }
          if (file.getName().endsWith(".js")) {
            return true;
          }
          return false;
        }
        public String getDescription() {
          return "Javascript";
        }
      });
      if (fileChooser.showOpenDialog(scriptRunner) != JFileChooser.APPROVE_OPTION) {
        return;
      }
      scriptRunner.setLinkFile(fileChooser.getSelectedFile());
    }
  }
}
