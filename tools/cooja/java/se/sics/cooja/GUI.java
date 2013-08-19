/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

package se.sics.cooja;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dialog.ModalityType;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.GridLayout;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.beans.PropertyVetoException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.StringReader;
import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.net.URL;
import java.net.URLClassLoader;
import java.security.AccessControlException;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.List;
import java.util.Observable;
import java.util.Observer;
import java.util.Properties;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.DefaultDesktopManager;
import javax.swing.InputMap;
import javax.swing.JApplet;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JComponent;
import javax.swing.JDesktopPane;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JInternalFrame;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JTabbedPane;
import javax.swing.JTextPane;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.ToolTipManager;
import javax.swing.UIManager;
import javax.swing.UIManager.LookAndFeelInfo;
import javax.swing.UnsupportedLookAndFeelException;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.filechooser.FileFilter;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.VisPlugin.PluginRequiresVisualizationException;
import se.sics.cooja.contikimote.ContikiMoteType;
import se.sics.cooja.dialogs.AddMoteDialog;
import se.sics.cooja.dialogs.BufferSettings;
import se.sics.cooja.dialogs.ConfigurationWizard;
import se.sics.cooja.dialogs.CreateSimDialog;
import se.sics.cooja.dialogs.ExternalToolsDialog;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.dialogs.ProjectDirectoriesDialog;
import se.sics.cooja.plugins.MoteTypeInformation;
import se.sics.cooja.plugins.ScriptRunner;
import se.sics.cooja.plugins.SimControl;
import se.sics.cooja.plugins.SimInformation;
import se.sics.cooja.util.ExecuteJAR;

/**
 * Main file of COOJA Simulator. Typically contains a visualizer for the
 * simulator, but can also be started without visualizer.
 *
 * This class loads external Java classes (in extension directories), and handles the
 * COOJA plugins as well as the configuration system. If provides a number of
 * help methods for the rest of the COOJA system, and is the starting point for
 * loading and saving simulation configs.
 *
 * @author Fredrik Osterlind
 */
public class GUI extends Observable {
  private static JFrame frame = null;
  private static JApplet applet = null;
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(GUI.class);

  /**
   * External tools configuration.
   */
  public static final String EXTERNAL_TOOLS_SETTINGS_FILENAME = "/external_tools.config";

  /**
   * External tools default Win32 settings filename.
   */
  public static final String EXTERNAL_TOOLS_WIN32_SETTINGS_FILENAME = "/external_tools_win32.config";

  /**
   * External tools default Mac OS X settings filename.
   */
  public static final String EXTERNAL_TOOLS_MACOSX_SETTINGS_FILENAME = "/external_tools_macosx.config";

  /**
   * External tools default FreeBSD settings filename.
   */
  public static final String EXTERNAL_TOOLS_FREEBSD_SETTINGS_FILENAME = "/external_tools_freebsd.config";

  /**
   * External tools default Linux/Unix settings filename.
   */
  public static final String EXTERNAL_TOOLS_LINUX_SETTINGS_FILENAME = "/external_tools_linux.config";

  /**
   * External tools default Linux/Unix settings filename for 64 bit architectures.
   * Tested on Intel 64-bit Gentoo Linux.
   */
  public static final String EXTERNAL_TOOLS_LINUX_64_SETTINGS_FILENAME = "/external_tools_linux_64.config";

  /**
   * External tools user settings filename.
   */
  public static final String EXTERNAL_TOOLS_USER_SETTINGS_FILENAME = ".cooja.user.properties";
  public static File externalToolsUserSettingsFile;
  private static boolean externalToolsUserSettingsFileReadOnly = false;

  private static String specifiedContikiPath = null;

  /**
   * Logger settings filename.
   */
  public static final String LOG_CONFIG_FILE = "log4j_config.xml";

  /**
   * Default extension configuration filename.
   */
  public static String PROJECT_DEFAULT_CONFIG_FILENAME = null;

  /**
   * User extension configuration filename.
   */
  public static final String PROJECT_CONFIG_FILENAME = "cooja.config";

  /**
   * File filter only showing saved simulations files (*.csc).
   */
  public static final FileFilter SAVED_SIMULATIONS_FILES = new FileFilter() {
    public boolean accept(File file) {
      if (file.isDirectory()) {
        return true;
      }

      if (file.getName().endsWith(".csc")) {
        return true;
      }
      if (file.getName().endsWith(".csc.gz")) {
        return true;
      }

      return false;
    }
    public String getDescription() {
      return "Cooja simulation (.csc, .csc.gz)";
    }
    public String toString() {
      return ".csc";
    }
  };

  // External tools setting names
  public static Properties defaultExternalToolsSettings;
  public static Properties currentExternalToolsSettings;

  private static final String externalToolsSettingNames[] = new String[] {
    "PATH_CONTIKI", "PATH_COOJA_CORE_RELATIVE","PATH_COOJA","PATH_APPS",
    "PATH_APPSEARCH",

    "PATH_MAKE",
    "PATH_SHELL",
    "PATH_C_COMPILER", "COMPILER_ARGS",
    "PATH_LINKER", "LINK_COMMAND_1", "LINK_COMMAND_2",
    "PATH_AR", "AR_COMMAND_1", "AR_COMMAND_2",
    "PATH_OBJDUMP", "OBJDUMP_ARGS",
    "PATH_OBJCOPY",
    "PATH_JAVAC",

    "CONTIKI_STANDARD_PROCESSES",

    "CMD_GREP_PROCESSES", "REGEXP_PARSE_PROCESSES",
    "CMD_GREP_INTERFACES", "REGEXP_PARSE_INTERFACES",
    "CMD_GREP_SENSORS", "REGEXP_PARSE_SENSORS",

    "DEFAULT_PROJECTDIRS",
    "CORECOMM_TEMPLATE_FILENAME",

    "PARSE_WITH_COMMAND",

    "MAPFILE_DATA_START", "MAPFILE_DATA_SIZE",
    "MAPFILE_BSS_START", "MAPFILE_BSS_SIZE",
    "MAPFILE_COMMON_START", "MAPFILE_COMMON_SIZE",
    "MAPFILE_VAR_NAME",
    "MAPFILE_VAR_ADDRESS_1", "MAPFILE_VAR_ADDRESS_2",
    "MAPFILE_VAR_SIZE_1", "MAPFILE_VAR_SIZE_2",

    "PARSE_COMMAND",
    "COMMAND_VAR_NAME_ADDRESS",
    "COMMAND_DATA_START", "COMMAND_DATA_END",
    "COMMAND_BSS_START", "COMMAND_BSS_END",
    "COMMAND_COMMON_START", "COMMAND_COMMON_END",

    "HIDE_WARNINGS"
  };

  private static final int FRAME_NEW_OFFSET = 30;

  private static final int FRAME_STANDARD_WIDTH = 150;

  private static final int FRAME_STANDARD_HEIGHT = 300;

  private static final String WINDOW_TITLE = "Cooja: The Contiki Network Simulator";

  private GUI myGUI;

  private Simulation mySimulation;

  protected GUIEventHandler guiEventHandler = new GUIEventHandler();

  private JMenu menuMoteTypeClasses, menuMoteTypes;

  private JMenu menuOpenSimulation;
  private boolean hasFileHistoryChanged;

  private Vector<Class<? extends Plugin>> menuMotePluginClasses = new Vector<Class<? extends Plugin>>();

  private JDesktopPane myDesktopPane;

  private Vector<Plugin> startedPlugins = new Vector<Plugin>();

  private ArrayList<GUIAction> guiActions = new ArrayList<GUIAction>();

  // Platform configuration variables
  // Maintained via method reparseProjectConfig()
  private ProjectConfig projectConfig;

  private ArrayList<COOJAProject> currentProjects = new ArrayList<COOJAProject>();

  public ClassLoader projectDirClassLoader;

  private Vector<Class<? extends MoteType>> moteTypeClasses = new Vector<Class<? extends MoteType>>();

  private Vector<Class<? extends Plugin>> pluginClasses = new Vector<Class<? extends Plugin>>();

  private Vector<Class<? extends RadioMedium>> radioMediumClasses = new Vector<Class<? extends RadioMedium>>();

  private Vector<Class<? extends Positioner>> positionerClasses = new Vector<Class<? extends Positioner>>();

  private class HighlightObservable extends Observable {
    private void setChangedAndNotify(Mote mote) {
      setChanged();
      notifyObservers(mote);
    }
  }
  private HighlightObservable moteHighlightObservable = new HighlightObservable();

  private class MoteRelationsObservable extends Observable {
    private void setChangedAndNotify() {
      setChanged();
      notifyObservers();
    }
  }
  private MoteRelationsObservable moteRelationObservable = new MoteRelationsObservable();

  private JTextPane quickHelpTextPane;
  private JScrollPane quickHelpScroll;
  private Properties quickHelpProperties = null; /* quickhelp.txt */

  /**
   * Mote relation (directed).
   */
  public static class MoteRelation {
    public Mote source;
    public Mote dest;
    public Color color;
    public MoteRelation(Mote source, Mote dest, Color color) {
      this.source = source;
      this.dest = dest;
      this.color = color;
    }
  }
  private ArrayList<MoteRelation> moteRelations = new ArrayList<MoteRelation>();

  /**
   * Creates a new COOJA Simulator GUI.
   *
   * @param desktop Desktop pane
   */
  public GUI(JDesktopPane desktop) {
    myGUI = this;
    mySimulation = null;
    myDesktopPane = desktop;

    /* Help panel */
    quickHelpTextPane = new JTextPane();
    quickHelpTextPane.setContentType("text/html");
    quickHelpTextPane.setEditable(false);
    quickHelpTextPane.setVisible(false);
    quickHelpScroll = new JScrollPane(quickHelpTextPane, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
    quickHelpScroll.setPreferredSize(new Dimension(200, 0));
    quickHelpScroll.setBorder(BorderFactory.createCompoundBorder(
        BorderFactory.createLineBorder(Color.GRAY),
        BorderFactory.createEmptyBorder(0, 3, 0, 0)
    ));
    quickHelpScroll.setVisible(false);
    loadQuickHelp("GETTING_STARTED");

    // Load default and overwrite with user settings (if any)
    loadExternalToolsDefaultSettings();
    loadExternalToolsUserSettings();

    final boolean showQuickhelp = getExternalToolsSetting("SHOW_QUICKHELP", "true").equalsIgnoreCase("true");
    if (showQuickhelp) {
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          JCheckBoxMenuItem checkBox = ((JCheckBoxMenuItem)showQuickHelpAction.getValue("checkbox"));
          if (checkBox == null) {
            return;
          }
          if (checkBox.isSelected()) {
            return;
          }
          checkBox.doClick();
        }
      });
    }

    /* Debugging - Break on repaints outside EDT */
    /*RepaintManager.setCurrentManager(new RepaintManager() {
      public void addDirtyRegion(JComponent comp, int a, int b, int c, int d) {
        if(!java.awt.EventQueue.isDispatchThread()) {
          throw new RuntimeException("Repainting outside EDT");
        }
        super.addDirtyRegion(comp, a, b, c, d);
      }
    });*/

    // Register default extension directories
    String defaultProjectDirs = getExternalToolsSetting("DEFAULT_PROJECTDIRS", null);
    if (defaultProjectDirs != null && defaultProjectDirs.length() > 0) {
      String[] arr = defaultProjectDirs.split(";");
      for (String p : arr) {
        File projectDir = restorePortablePath(new File(p));
        currentProjects.add(new COOJAProject(projectDir));
      }
    }
    
    //Scan for projects
    String searchProjectDirs = getExternalToolsSetting("PATH_APPSEARCH", null);
    if (searchProjectDirs != null && searchProjectDirs.length() > 0) {
      String[] arr = searchProjectDirs.split(";");
      for (String d : arr) {
    	  File searchDir = restorePortablePath(new File(d));
    	  File[] projects = COOJAProject.sarchProjects(searchDir, 3);
    	  if(projects == null) continue;
    	  for(File p : projects){
    		  currentProjects.add(new COOJAProject(p));
    	  }
      }
    }

    /* Parse current extension configuration */
    try {
      reparseProjectConfig();
    } catch (ParseProjectsException e) {
      logger.fatal("Error when loading extensions: " + e.getMessage(), e);
      if (isVisualized()) {
      	JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
      			"All Cooja extensions could not load.\n\n" +
      			"To manage Cooja extensions:\n" +
      			"Menu->Settings->Cooja extensions",
      			"Reconfigure Cooja extensions", JOptionPane.INFORMATION_MESSAGE);
      	showErrorDialog(getTopParentContainer(), "Cooja extensions load error", e, false);
      }
    }

    // Start all standard GUI plugins
    for (Class<? extends Plugin> pluginClass : pluginClasses) {
      int pluginType = pluginClass.getAnnotation(PluginType.class).value();
      if (pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        tryStartPlugin(pluginClass, this, null, null);
      }
    }
  }


  /**
   * Add mote highlight observer.
   *
   * @see #deleteMoteHighlightObserver(Observer)
   * @param newObserver
   *          New observer
   */
  public void addMoteHighlightObserver(Observer newObserver) {
    moteHighlightObservable.addObserver(newObserver);
  }

  /**
   * Delete mote highlight observer.
   *
   * @see #addMoteHighlightObserver(Observer)
   * @param observer
   *          Observer to delete
   */
  public void deleteMoteHighlightObserver(Observer observer) {
    moteHighlightObservable.deleteObserver(observer);
  }

  /**
   * @return True if simulator is visualized
   */
  public static boolean isVisualized() {
    return isVisualizedInFrame() || isVisualizedInApplet();
  }

  public static Container getTopParentContainer() {
    if (isVisualizedInFrame()) {
      return frame;
    }

    if (isVisualizedInApplet()) {
      /* Find parent frame for applet */
      Container container = applet;
      while((container = container.getParent()) != null){
        if (container instanceof Frame) {
          return container;
        }
        if (container instanceof Dialog) {
          return container;
        }
        if (container instanceof Window) {
          return container;
        }
      }

      logger.fatal("Returning null top owner container");
    }

    return null;
  }

  public static boolean isVisualizedInFrame() {
    return frame != null;
  }

  public static URL getAppletCodeBase() {
    return applet.getCodeBase();
  }

  public static boolean isVisualizedInApplet() {
    return applet != null;
  }

  /**
   * Tries to create/remove simulator visualizer.
   *
   * @param visualized Visualized
   */
  public void setVisualizedInFrame(boolean visualized) {
    if (visualized) {
      if (!isVisualizedInFrame()) {
        configureFrame(myGUI, false);
      }
    } else {
      if (frame != null) {
        frame.setVisible(false);
        frame.dispose();
        frame = null;
      }
    }
  }

  public File getLastOpenedFile() {
    // Fetch current history
    String[] historyArray = getExternalToolsSetting("SIMCFG_HISTORY", "").split(";");
    return historyArray.length > 0 ? new File(historyArray[0]) : null;
  }

  public File[] getFileHistory() {
    // Fetch current history
    String[] historyArray = getExternalToolsSetting("SIMCFG_HISTORY", "").split(";");
    File[] history = new File[historyArray.length];
    for (int i = 0; i < historyArray.length; i++) {
      history[i] = new File(historyArray[i]);
    }
    return history;
  }

  public void addToFileHistory(File file) {
    // Fetch current history
    String[] history = getExternalToolsSetting("SIMCFG_HISTORY", "").split(";");
    String newFile = file.getAbsolutePath();
    if (history.length > 0 && history[0].equals(newFile)) {
      // File already added
      return;
    }
    // Create new history
    StringBuilder newHistory = new StringBuilder();
    newHistory.append(newFile);
    for (int i = 0, count = 1; i < history.length && count < 10; i++) {
      String historyFile = history[i];
      if (newFile.equals(historyFile) || historyFile.length() == 0) {
        // File already added or empty file name
      } else {
        newHistory.append(';').append(historyFile);
        count++;
      }
    }
    setExternalToolsSetting("SIMCFG_HISTORY", newHistory.toString());
    saveExternalToolsUserSettings();
    hasFileHistoryChanged = true;
  }

  private void updateOpenHistoryMenuItems() {
    if (isVisualizedInApplet()) {
      return;
    }
    if (!hasFileHistoryChanged) {
      return;
    }
    hasFileHistoryChanged = false;

    File[] openFilesHistory = getFileHistory();
    updateOpenHistoryMenuItems(openFilesHistory);
  }

  private void populateMenuWithHistory(JMenu menu, final boolean quick, File[] openFilesHistory) {
    JMenuItem lastItem;
    int index = 0;
    for (File file: openFilesHistory) {
      if (index < 10) {
        char mnemonic = (char) ('0' + (++index % 10));
        lastItem = new JMenuItem(mnemonic + " " + file.getName());
        lastItem.setMnemonic(mnemonic);
      } else {
        lastItem = new JMenuItem(file.getName());
      }
      final File f = file;
      lastItem.addActionListener(new ActionListener() {
  			public void actionPerformed(ActionEvent e) {
  				doLoadConfigAsync(true, quick, f);
  			}
      });
      lastItem.putClientProperty("file", file);
      lastItem.setToolTipText(file.getAbsolutePath());
      menu.add(lastItem);
    }
  }

  private void doLoadConfigAsync(final boolean ask, final boolean quick, final File file) {
    new Thread(new Runnable() {
      public void run() {
        myGUI.doLoadConfig(ask, quick, file);
      }
    }).start();
  }
  private void updateOpenHistoryMenuItems(File[] openFilesHistory) {
  	menuOpenSimulation.removeAll();

    /* Reconfigure submenu */
    JMenu reconfigureMenu = new JMenu("Open and Reconfigure");
    JMenuItem browseItem2 = new JMenuItem("Browse...");
    browseItem2.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doLoadConfigAsync(true, false, null);
			}
    });
    reconfigureMenu.add(browseItem2);
    reconfigureMenu.add(new JSeparator());
    populateMenuWithHistory(reconfigureMenu, false, openFilesHistory);

    /* Open menu */
    JMenuItem browseItem = new JMenuItem("Browse...");
    browseItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				doLoadConfigAsync(true, true, null);
			}
    });
    menuOpenSimulation.add(browseItem);
    menuOpenSimulation.add(new JSeparator());
    menuOpenSimulation.add(reconfigureMenu);
    menuOpenSimulation.add(new JSeparator());
    populateMenuWithHistory(menuOpenSimulation, true, openFilesHistory);
  }

  /**
   * Enables/disables menues and menu items depending on whether a simulation is loaded etc.
   */
  private void updateGUIComponentState() {
    if (!isVisualized()) {
      return;
    }

    /* Update action state */
    for (GUIAction a: guiActions) {
      a.setEnabled(a.shouldBeEnabled());
    }

    /* Mote and mote type menues */
    if (menuMoteTypeClasses != null) {
      menuMoteTypeClasses.setEnabled(getSimulation() != null);
    }
    if (menuMoteTypes != null) {
      menuMoteTypes.setEnabled(getSimulation() != null);
    }
  }

  private JMenuBar createMenuBar() {

    JMenuItem menuItem;

    /* Prepare GUI actions */
    guiActions.add(newSimulationAction);
    guiActions.add(closeSimulationAction);
    guiActions.add(reloadSimulationAction);
    guiActions.add(reloadRandomSimulationAction);
    guiActions.add(saveSimulationAction);
    /*    guiActions.add(closePluginsAction);*/
    guiActions.add(exportExecutableJARAction);
    guiActions.add(exitCoojaAction);
    guiActions.add(startStopSimulationAction);
    guiActions.add(removeAllMotesAction);
    guiActions.add(showBufferSettingsAction);

    /* Menus */
    JMenuBar menuBar = new JMenuBar();
    JMenu fileMenu = new JMenu("File");
    JMenu simulationMenu = new JMenu("Simulation");
    JMenu motesMenu = new JMenu("Motes");
    final JMenu toolsMenu = new JMenu("Tools");
    JMenu settingsMenu = new JMenu("Settings");
    JMenu helpMenu = new JMenu("Help");

    menuBar.add(fileMenu);
    menuBar.add(simulationMenu);
    menuBar.add(motesMenu);
    menuBar.add(toolsMenu);
    menuBar.add(settingsMenu);
    menuBar.add(helpMenu);

    fileMenu.setMnemonic(KeyEvent.VK_F);
    simulationMenu.setMnemonic(KeyEvent.VK_S);
    motesMenu.setMnemonic(KeyEvent.VK_M);
    toolsMenu.setMnemonic(KeyEvent.VK_T);
    helpMenu.setMnemonic(KeyEvent.VK_H);

    /* File menu */
    fileMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateGUIComponentState();
        updateOpenHistoryMenuItems();
      }
      public void menuDeselected(MenuEvent e) {
      }

      public void menuCanceled(MenuEvent e) {
      }
    });

    fileMenu.add(new JMenuItem(newSimulationAction));

    menuOpenSimulation = new JMenu("Open simulation");
    menuOpenSimulation.setMnemonic(KeyEvent.VK_O);
    fileMenu.add(menuOpenSimulation);
    if (isVisualizedInApplet()) {
      menuOpenSimulation.setEnabled(false);
      menuOpenSimulation.setToolTipText("Not available in applet version");
    }

    fileMenu.add(new JMenuItem(closeSimulationAction));

    hasFileHistoryChanged = true;

    fileMenu.add(new JMenuItem(saveSimulationAction));

    fileMenu.add(new JMenuItem(exportExecutableJARAction));

    /*    menu.addSeparator();*/

    /*    menu.add(new JMenuItem(closePluginsAction));*/

    fileMenu.addSeparator();

    fileMenu.add(new JMenuItem(exitCoojaAction));

    /* Simulation menu */
    simulationMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateGUIComponentState();
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });

    simulationMenu.add(new JMenuItem(startStopSimulationAction));

    JMenuItem reloadSimulationMenuItem = new JMenu("Reload simulation");
    reloadSimulationMenuItem.add(new JMenuItem(reloadSimulationAction));
    reloadSimulationMenuItem.add(new JMenuItem(reloadRandomSimulationAction));
    simulationMenu.add(reloadSimulationMenuItem);

    GUIAction guiAction = new StartPluginGUIAction("Control panel...");
    menuItem = new JMenuItem(guiAction);
    guiActions.add(guiAction);
    menuItem.setMnemonic(KeyEvent.VK_C);
    menuItem.putClientProperty("class", SimControl.class);
    simulationMenu.add(menuItem);

    guiAction = new StartPluginGUIAction("Simulation...");
    menuItem = new JMenuItem(guiAction);
    guiActions.add(guiAction);
    menuItem.setMnemonic(KeyEvent.VK_I);
    menuItem.putClientProperty("class", SimInformation.class);
    simulationMenu.add(menuItem);

    // Mote type menu
    motesMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateGUIComponentState();
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });

    // Mote type classes sub menu
    menuMoteTypeClasses = new JMenu("Create new mote type");
    menuMoteTypeClasses.setMnemonic(KeyEvent.VK_C);
    menuMoteTypeClasses.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        // Clear menu
        menuMoteTypeClasses.removeAll();

        // Recreate menu items
        JMenuItem menuItem;

        for (Class<? extends MoteType> moteTypeClass : moteTypeClasses) {
          /* Sort mote types according to abstraction level */
          String abstractionLevelDescription = GUI.getAbstractionLevelDescriptionOf(moteTypeClass);
          if(abstractionLevelDescription == null) {
            abstractionLevelDescription = "[unknown cross-level]";
          }

          /* Check if abstraction description already exists */
          JSeparator abstractionLevelSeparator = null;
          for (Component component: menuMoteTypeClasses.getMenuComponents()) {
            if (component == null || !(component instanceof JSeparator)) {
              continue;
            }
            JSeparator existing = (JSeparator) component;
            if (abstractionLevelDescription.equals(existing.getToolTipText())) {
              abstractionLevelSeparator = existing;
              break;
            }
          }
          if (abstractionLevelSeparator == null) {
            abstractionLevelSeparator = new JSeparator();
            abstractionLevelSeparator.setToolTipText(abstractionLevelDescription);
            menuMoteTypeClasses.add(abstractionLevelSeparator);
          }

          String description = GUI.getDescriptionOf(moteTypeClass);
          menuItem = new JMenuItem(description + "...");
          menuItem.setActionCommand("create mote type");
          menuItem.putClientProperty("class", moteTypeClass);
        /*  menuItem.setToolTipText(abstractionLevelDescription);*/
          menuItem.addActionListener(guiEventHandler);
          if (isVisualizedInApplet() && moteTypeClass.equals(ContikiMoteType.class)) {
            menuItem.setEnabled(false);
            menuItem.setToolTipText("Not available in applet version");
          }

          /* Add new item directly after cross level separator */
          for (int i=0; i < menuMoteTypeClasses.getMenuComponentCount(); i++) {
            if (menuMoteTypeClasses.getMenuComponent(i) == abstractionLevelSeparator) {
              menuMoteTypeClasses.add(menuItem, i+1);
              break;
            }
          }
        }
      }

      public void menuDeselected(MenuEvent e) {
      }

      public void menuCanceled(MenuEvent e) {
      }
    });




    // Mote menu
    motesMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateGUIComponentState();
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });


    // Mote types sub menu
    menuMoteTypes = new JMenu("Add motes");
    menuMoteTypes.setMnemonic(KeyEvent.VK_A);
    menuMoteTypes.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        // Clear menu
        menuMoteTypes.removeAll();



        if (mySimulation != null) {

          // Recreate menu items
          JMenuItem menuItem;

          for (MoteType moteType : mySimulation.getMoteTypes()) {
            menuItem = new JMenuItem(moteType.getDescription());
            menuItem.setActionCommand("add motes");
            menuItem.setToolTipText(getDescriptionOf(moteType.getClass()));
            menuItem.putClientProperty("motetype", moteType);
            menuItem.addActionListener(guiEventHandler);
            menuMoteTypes.add(menuItem);
          }

          if(mySimulation.getMoteTypes().length > 0) {
            menuMoteTypes.add(new JSeparator());
          }
        }


        menuMoteTypes.add(menuMoteTypeClasses);
      }

      public void menuDeselected(MenuEvent e) {
      }

      public void menuCanceled(MenuEvent e) {
      }
    });
    motesMenu.add(menuMoteTypes);

    guiAction = new StartPluginGUIAction("Mote types...");
    menuItem = new JMenuItem(guiAction);
    guiActions.add(guiAction);
    menuItem.putClientProperty("class", MoteTypeInformation.class);

    motesMenu.add(menuItem);

    motesMenu.add(new JMenuItem(removeAllMotesAction));

    /* Tools menu */
    toolsMenu.addMenuListener(new MenuListener() {
      private ActionListener menuItemListener = new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          Object pluginClass = ((JMenuItem)e.getSource()).getClientProperty("class");
          Object mote = ((JMenuItem)e.getSource()).getClientProperty("mote");
          tryStartPlugin((Class<? extends Plugin>) pluginClass, myGUI, getSimulation(), (Mote)mote);
        }
      };
      private JMenuItem createMenuItem(Class<? extends Plugin> newPluginClass, int pluginType) {
        String description = getDescriptionOf(newPluginClass);
        JMenuItem menuItem = new JMenuItem(description + "...");
        menuItem.putClientProperty("class", newPluginClass);
        menuItem.addActionListener(menuItemListener);

        String tooltip = "<html><pre>";
        if (pluginType == PluginType.COOJA_PLUGIN || pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
          tooltip += "Cooja plugin: ";
        } else if (pluginType == PluginType.SIM_PLUGIN || pluginType == PluginType.SIM_STANDARD_PLUGIN) {
          tooltip += "Simulation plugin: ";
          if (getSimulation() == null) {
            menuItem.setEnabled(false);
          }
        } else if (pluginType == PluginType.MOTE_PLUGIN) {
          tooltip += "Mote plugin: ";
        }
        tooltip += description + " (" + newPluginClass.getName() + ")";

        /* Check if simulation plugin depends on any particular radio medium */
        if (pluginType == PluginType.SIM_PLUGIN || pluginType == PluginType.SIM_STANDARD_PLUGIN) {
          if (newPluginClass.getAnnotation(SupportedArguments.class) != null) {
            boolean active = false;
            Class<? extends RadioMedium>[] radioMediums = newPluginClass.getAnnotation(SupportedArguments.class).radioMediums();
            for (Class<? extends Object> o: radioMediums) {
              if (o.isAssignableFrom(getSimulation().getRadioMedium().getClass())) {
                active = true;
                break;
              }
            }
            if (!active) {
              menuItem.setVisible(false);
            }
          }
        }

        /* Check if plugin was imported by a extension directory */
        File project =
          getProjectConfig().getUserProjectDefining(GUI.class, "PLUGINS", newPluginClass.getName());
        if (project != null) {
          tooltip += "\nLoaded by extension: " + project.getPath();
        }

        tooltip += "</html>";
        /*menuItem.setToolTipText(tooltip);*/
        return menuItem;
      }

      public void menuSelected(MenuEvent e) {
        /* Populate tools menu */
        toolsMenu.removeAll();

        /* Cooja plugins */
        boolean hasCoojaPlugins = false;
        for (Class<? extends Plugin> pluginClass: pluginClasses) {
          int pluginType = pluginClass.getAnnotation(PluginType.class).value();
          if (pluginType != PluginType.COOJA_PLUGIN && pluginType != PluginType.COOJA_STANDARD_PLUGIN) {
            continue;
          }
          toolsMenu.add(createMenuItem(pluginClass, pluginType));
          hasCoojaPlugins = true;
        }

        /* Simulation plugins */
        boolean hasSimPlugins = false;
        for (Class<? extends Plugin> pluginClass: pluginClasses) {
          if (pluginClass.equals(SimControl.class)) {
            continue; /* ignore */
          }
          if (pluginClass.equals(SimInformation.class)) {
            continue; /* ignore */
          }
          if (pluginClass.equals(MoteTypeInformation.class)) {
            continue; /* ignore */
          }

          int pluginType = pluginClass.getAnnotation(PluginType.class).value();
          if (pluginType != PluginType.SIM_PLUGIN && pluginType != PluginType.SIM_STANDARD_PLUGIN) {
            continue;
          }

          if (hasCoojaPlugins) {
            hasCoojaPlugins = false;
            toolsMenu.addSeparator();
          }

          toolsMenu.add(createMenuItem(pluginClass, pluginType));
          hasSimPlugins = true;
        }

        for (Class<? extends Plugin> pluginClass: pluginClasses) {
          int pluginType = pluginClass.getAnnotation(PluginType.class).value();
          if (pluginType != PluginType.MOTE_PLUGIN) {
            continue;
          }

          if (hasSimPlugins) {
            hasSimPlugins = false;
            toolsMenu.addSeparator();
          }

          toolsMenu.add(createMotePluginsSubmenu(pluginClass));
        }
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });

    // Settings menu
    settingsMenu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateGUIComponentState();
      }
      public void menuDeselected(MenuEvent e) {
      }
      public void menuCanceled(MenuEvent e) {
      }
    });

    menuItem = new JMenuItem("External tools paths...");
    menuItem.setActionCommand("edit paths");
    menuItem.addActionListener(guiEventHandler);
    settingsMenu.add(menuItem);
    if (isVisualizedInApplet()) {
      menuItem.setEnabled(false);
      menuItem.setToolTipText("Not available in applet version");
    }

    menuItem = new JMenuItem("Cooja extensions...");
    menuItem.setActionCommand("manage extensions");
    menuItem.addActionListener(guiEventHandler);
    settingsMenu.add(menuItem);
    if (isVisualizedInApplet()) {
      menuItem.setEnabled(false);
      menuItem.setToolTipText("Not available in applet version");
    }

    menuItem = new JMenuItem("Cooja mote configuration wizard...");
    menuItem.setActionCommand("configuration wizard");
    menuItem.addActionListener(guiEventHandler);
    settingsMenu.add(menuItem);
    if (isVisualizedInApplet()) {
      menuItem.setEnabled(false);
      menuItem.setToolTipText("Not available in applet version");
    }

    settingsMenu.add(new JMenuItem(showBufferSettingsAction));

    /* Help */
    helpMenu.add(new JMenuItem(showGettingStartedAction));
    helpMenu.add(new JMenuItem(showKeyboardShortcutsAction));
    JCheckBoxMenuItem checkBox = new JCheckBoxMenuItem(showQuickHelpAction);
    showQuickHelpAction.putValue("checkbox", checkBox);
    helpMenu.add(checkBox);

    helpMenu.addSeparator();

    menuItem = new JMenuItem("Java version: "
        + System.getProperty("java.version") + " ("
        + System.getProperty("java.vendor") + ")");
    menuItem.setEnabled(false);
    helpMenu.add(menuItem);
    menuItem = new JMenuItem("System \"os.arch\": "
        + System.getProperty("os.arch"));
    menuItem.setEnabled(false);
    helpMenu.add(menuItem);
    menuItem = new JMenuItem("System \"sun.arch.data.model\": "
        + System.getProperty("sun.arch.data.model"));
    menuItem.setEnabled(false);
    helpMenu.add(menuItem);

    return menuBar;
  }

  private static void configureFrame(final GUI gui, boolean createSimDialog) {

    if (frame == null) {
      frame = new JFrame(WINDOW_TITLE);
    }
    frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    /* Menu bar */
    frame.setJMenuBar(gui.createMenuBar());

    /* Scrollable desktop */
    JComponent desktop = gui.getDesktopPane();
    desktop.setOpaque(true);

    JScrollPane scroll = new JScrollPane(desktop);
    scroll.setBorder(null);

    JPanel container = new JPanel(new BorderLayout());
    container.add(BorderLayout.CENTER, scroll);
    container.add(BorderLayout.EAST, gui.quickHelpScroll);
    frame.setContentPane(container);

    frame.setSize(700, 700);
    frame.setLocationRelativeTo(null);
    frame.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        gui.doQuit(true);
      }
    });
    frame.addComponentListener(new ComponentAdapter() {
      public void componentResized(ComponentEvent e) {
        updateDesktopSize(gui.getDesktopPane());
      }
    });

    /* Restore frame size and position */
    int framePosX = Integer.parseInt(getExternalToolsSetting("FRAME_POS_X", "0"));
    int framePosY = Integer.parseInt(getExternalToolsSetting("FRAME_POS_Y", "0"));
    int frameWidth = Integer.parseInt(getExternalToolsSetting("FRAME_WIDTH", "0"));
    int frameHeight = Integer.parseInt(getExternalToolsSetting("FRAME_HEIGHT", "0"));
    String frameScreen = getExternalToolsSetting("FRAME_SCREEN", "");

    /* Restore position to the same graphics device */
    GraphicsDevice device = null;
    GraphicsDevice all[] = GraphicsEnvironment.getLocalGraphicsEnvironment().getScreenDevices();
    for (GraphicsDevice gd : all) {
      if (gd.getIDstring().equals(frameScreen)) {
        device = gd;
      }
    }

    /* Check if frame should be maximized */
    if (device != null) {
      if (frameWidth == Integer.MAX_VALUE && frameHeight == Integer.MAX_VALUE) {
        frame.setLocation(device.getDefaultConfiguration().getBounds().getLocation());
        frame.setExtendedState(JFrame.MAXIMIZED_BOTH);
      } else if (frameWidth > 0 && frameHeight > 0) {

        /* Sanity-check: will Cooja be visible on screen? */
        boolean intersects =
          device.getDefaultConfiguration().getBounds().intersects(
              new Rectangle(framePosX, framePosY, frameWidth, frameHeight));

        if (intersects) {
          frame.setLocation(framePosX, framePosY);
          frame.setSize(frameWidth, frameHeight);
        }

      }
    }

    frame.setVisible(true);

    if (createSimDialog) {
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          gui.doCreateSimulation(true);
        }
      });
    }
  }

  private static void configureApplet(final GUI gui, boolean createSimDialog) {
    applet = CoojaApplet.applet;

    // Add menu bar
    JMenuBar menuBar = gui.createMenuBar();
    applet.setJMenuBar(menuBar);

    JComponent newContentPane = gui.getDesktopPane();
    newContentPane.setOpaque(true);
    applet.setContentPane(newContentPane);
    applet.setSize(700, 700);

    if (createSimDialog) {
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          gui.doCreateSimulation(true);
        }
      });
    }
  }

  /**
   * @return Current desktop pane (simulator visualizer)
   */
  public JDesktopPane getDesktopPane() {
    return myDesktopPane;
  }

  public static void setLookAndFeel() {

    JFrame.setDefaultLookAndFeelDecorated(true);
    JDialog.setDefaultLookAndFeelDecorated(true);

    ToolTipManager.sharedInstance().setDismissDelay(60000);

    /* Nimbus */
    try {
      String osName = System.getProperty("os.name").toLowerCase();
      if (osName.startsWith("linux")) {
        try {
          for (LookAndFeelInfo info : UIManager.getInstalledLookAndFeels()) {
            if ("Nimbus".equals(info.getName())) {
                UIManager.setLookAndFeel(info.getClassName());
                break;
            }
          }

        } catch (UnsupportedLookAndFeelException e) {
          UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
        }
      } else {
        UIManager.setLookAndFeel("com.sun.java.swing.plaf.nimbus.NimbusLookAndFeel");
      }
      return;
    } catch (Exception e) {
    }

    /* System */
    try {
      UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      return;
    } catch (Exception e) {
    }
  }

  private static void updateDesktopSize(final JDesktopPane desktop) {
    if (desktop == null || !desktop.isVisible() || desktop.getParent() == null) {
      return;
    }

    Rectangle rect = desktop.getVisibleRect();
    Dimension pref = new Dimension(rect.width - 1, rect.height - 1);
    for (JInternalFrame frame : desktop.getAllFrames()) {
      if (pref.width < frame.getX() + frame.getWidth() - 20) {
        pref.width = frame.getX() + frame.getWidth();
      }
      if (pref.height < frame.getY() + frame.getHeight() - 20) {
        pref.height = frame.getY() + frame.getHeight();
      }
    }
    desktop.setPreferredSize(pref);
    desktop.revalidate();
  }

  private static JDesktopPane createDesktopPane() {
    final JDesktopPane desktop = new JDesktopPane() {
			private static final long serialVersionUID = -8272040875621119329L;
			public void setBounds(int x, int y, int w, int h) {
        super.setBounds(x, y, w, h);
        updateDesktopSize(this);
      }
      public void remove(Component c) {
        super.remove(c);
        updateDesktopSize(this);
      }
      public Component add(Component comp) {
        Component c = super.add(comp);
        updateDesktopSize(this);
        return c;
      }
    };
    desktop.setDesktopManager(new DefaultDesktopManager() {
			private static final long serialVersionUID = -5987404936292377152L;
			public void endResizingFrame(JComponent f) {
        super.endResizingFrame(f);
        updateDesktopSize(desktop);
      }
      public void endDraggingFrame(JComponent f) {
        super.endDraggingFrame(f);
        updateDesktopSize(desktop);
      }
    });
    desktop.setDragMode(JDesktopPane.OUTLINE_DRAG_MODE);
    return desktop;
  }

  public static Simulation quickStartSimulationConfig(File config, boolean vis) {
    logger.info("> Starting Cooja");
    JDesktopPane desktop = createDesktopPane();
    if (vis) {
      frame = new JFrame(WINDOW_TITLE);
    }
    GUI gui = new GUI(desktop);
    if (vis) {
      configureFrame(gui, false);
    }

    if (vis) {
      gui.doLoadConfig(false, true, config);
      return gui.getSimulation();
    } else {
      try {
        Simulation newSim = gui.loadSimulationConfig(config, true);
        if (newSim == null) {
          return null;
        }
        gui.setSimulation(newSim, false);
        return newSim;
      } catch (Exception e) {
        logger.fatal("Exception when loading simulation: ", e);
        return null;
      }
    }
  }

  /**
   * Allows user to create a simulation with a single mote type.
   *
   * @param source Contiki application file name
   * @return True if simulation was created
   */
  private static boolean quickStartSimulation(String source) {
    logger.info("> Starting Cooja");
    JDesktopPane desktop = createDesktopPane();
    frame = new JFrame(WINDOW_TITLE);
    GUI gui = new GUI(desktop);
    configureFrame(gui, false);

    logger.info("> Creating simulation");
    Simulation simulation = new Simulation(gui);
    simulation.setTitle("Quickstarted simulation: " + source);
    boolean simOK = CreateSimDialog.showDialog(GUI.getTopParentContainer(), simulation);
    if (!simOK) {
      logger.fatal("No simulation, aborting quickstart");
      System.exit(1);
    }
    gui.setSimulation(simulation, true);

    logger.info("> Creating mote type");
    ContikiMoteType moteType = new ContikiMoteType();
    moteType.setContikiSourceFile(new File(source));
    moteType.setDescription("Cooja mote type (" + source + ")");

    try {
      boolean compileOK = moteType.configureAndInit(GUI.getTopParentContainer(), simulation, true);
      if (!compileOK) {
        logger.fatal("Mote type initialization failed, aborting quickstart");
        return false;
      }
    } catch (MoteTypeCreationException e1) {
      logger.fatal("Mote type initialization failed, aborting quickstart");
      return false;
    }
    simulation.addMoteType(moteType);

    logger.info("> Adding motes");
    gui.doAddMotes(moteType);
    return true;
  }

  //// PROJECT CONFIG AND EXTENDABLE PARTS METHODS ////

  /**
   * Register new mote type class.
   *
   * @param moteTypeClass
   *          Class to register
   */
  public void registerMoteType(Class<? extends MoteType> moteTypeClass) {
    moteTypeClasses.add(moteTypeClass);
  }

  /**
   * Unregister all mote type classes.
   */
  public void unregisterMoteTypes() {
    moteTypeClasses.clear();
  }

  /**
   * @return All registered mote type classes
   */
  public Vector<Class<? extends MoteType>> getRegisteredMoteTypes() {
    return moteTypeClasses;
  }

  /**
   * Register new positioner class.
   *
   * @param positionerClass
   *          Class to register
   * @return True if class was registered
   */
  public boolean registerPositioner(Class<? extends Positioner> positionerClass) {
    // Check that interval constructor exists
    try {
      positionerClass
      .getConstructor(new Class[] { int.class, double.class, double.class,
          double.class, double.class, double.class, double.class });
    } catch (Exception e) {
      logger.fatal("No interval constructor found of positioner: "
          + positionerClass);
      return false;
    }

    positionerClasses.add(positionerClass);
    return true;
  }

  /**
   * Unregister all positioner classes.
   */
  public void unregisterPositioners() {
    positionerClasses.clear();
  }

  /**
   * @return All registered positioner classes
   */
  public Vector<Class<? extends Positioner>> getRegisteredPositioners() {
    return positionerClasses;
  }

  /**
   * Register new radio medium class.
   *
   * @param radioMediumClass
   *          Class to register
   * @return True if class was registered
   */
  public boolean registerRadioMedium(
      Class<? extends RadioMedium> radioMediumClass) {
    // Check that simulation constructor exists
    try {
      radioMediumClass.getConstructor(new Class[] { Simulation.class });
    } catch (Exception e) {
      logger.fatal("No simulation constructor found of radio medium: "
          + radioMediumClass);
      return false;
    }

    radioMediumClasses.add(radioMediumClass);
    return true;
  }

  /**
   * Unregister all radio medium classes.
   */
  public void unregisterRadioMediums() {
    radioMediumClasses.clear();
  }

  /**
   * @return All registered radio medium classes
   */
  public Vector<Class<? extends RadioMedium>> getRegisteredRadioMediums() {
    return radioMediumClasses;
  }

  /**
   * Builds new extension configuration using current extension directories settings.
   * Reregisters mote types, plugins, positioners and radio
   * mediums. This method may still return true even if all classes could not be
   * registered, but always returns false if all extension directory configuration
   * files were not parsed correctly.
   */
  public void reparseProjectConfig() throws ParseProjectsException {
    if (PROJECT_DEFAULT_CONFIG_FILENAME == null) {
      if (isVisualizedInApplet()) {
        PROJECT_DEFAULT_CONFIG_FILENAME = "/cooja_applet.config";
      } else {
        PROJECT_DEFAULT_CONFIG_FILENAME = "/cooja_default.config";
      }
    }

    /* Remove current dependencies */
    unregisterMoteTypes();
    unregisterPlugins();
    unregisterPositioners();
    unregisterRadioMediums();
    projectDirClassLoader = null;

    /* Build cooja configuration */
    try {
      projectConfig = new ProjectConfig(true);
    } catch (FileNotFoundException e) {
      logger.fatal("Could not find default extension config file: " + PROJECT_DEFAULT_CONFIG_FILENAME);
      throw (ParseProjectsException) new ParseProjectsException(
          "Could not find default extension config file: " + PROJECT_DEFAULT_CONFIG_FILENAME).initCause(e);
    } catch (IOException e) {
      logger.fatal("Error when reading default extension config file: " + PROJECT_DEFAULT_CONFIG_FILENAME);
      throw (ParseProjectsException) new ParseProjectsException(
          "Error when reading default extension config file: " + PROJECT_DEFAULT_CONFIG_FILENAME).initCause(e);
    }
    if (!isVisualizedInApplet()) {
      for (COOJAProject project: currentProjects) {
        try {
          projectConfig.appendProjectDir(project.dir);
        } catch (FileNotFoundException e) {
          throw (ParseProjectsException) new ParseProjectsException(
              "Error when loading extension: " + e.getMessage()).initCause(e);
        } catch (IOException e) {
          throw (ParseProjectsException) new ParseProjectsException(
              "Error when reading extension config: " + e.getMessage()).initCause(e);
        }
      }

      /* Create extension class loader */
      try {
        projectDirClassLoader = createClassLoader(currentProjects);
      } catch (ClassLoaderCreationException e) {
        throw (ParseProjectsException) new ParseProjectsException(
        "Error when creating class loader").initCause(e);
      }
    }

    // Register mote types
    String[] moteTypeClassNames = projectConfig.getStringArrayValue(GUI.class,
    "MOTETYPES");
    if (moteTypeClassNames != null) {
      for (String moteTypeClassName : moteTypeClassNames) {
        if (moteTypeClassName.trim().isEmpty()) {
          continue;
        }
        Class<? extends MoteType> moteTypeClass = tryLoadClass(this,
            MoteType.class, moteTypeClassName);

        if (moteTypeClass != null) {
          registerMoteType(moteTypeClass);
          // logger.info("Loaded mote type class: " + moteTypeClassName);
        } else {
          logger.warn("Could not load mote type class: " + moteTypeClassName);
        }
      }
    }

    // Register plugins
    registerPlugin(SimControl.class);
    registerPlugin(SimInformation.class);
    registerPlugin(MoteTypeInformation.class);
    String[] pluginClassNames = projectConfig.getStringArrayValue(GUI.class,
    "PLUGINS");
    if (pluginClassNames != null) {
      for (String pluginClassName : pluginClassNames) {
        Class<? extends Plugin> pluginClass = tryLoadClass(this, Plugin.class,
            pluginClassName);

        if (pluginClass != null) {
          registerPlugin(pluginClass);
          // logger.info("Loaded plugin class: " + pluginClassName);
        } else {
          logger.warn("Could not load plugin class: " + pluginClassName);
        }
      }
    }

    // Register positioners
    String[] positionerClassNames = projectConfig.getStringArrayValue(
        GUI.class, "POSITIONERS");
    if (positionerClassNames != null) {
      for (String positionerClassName : positionerClassNames) {
        Class<? extends Positioner> positionerClass = tryLoadClass(this,
            Positioner.class, positionerClassName);

        if (positionerClass != null) {
          registerPositioner(positionerClass);
          // logger.info("Loaded positioner class: " + positionerClassName);
        } else {
          logger
          .warn("Could not load positioner class: " + positionerClassName);
        }
      }
    }

    // Register radio mediums
    String[] radioMediumsClassNames = projectConfig.getStringArrayValue(
        GUI.class, "RADIOMEDIUMS");
    if (radioMediumsClassNames != null) {
      for (String radioMediumClassName : radioMediumsClassNames) {
        Class<? extends RadioMedium> radioMediumClass = tryLoadClass(this,
            RadioMedium.class, radioMediumClassName);

        if (radioMediumClass != null) {
          registerRadioMedium(radioMediumClass);
          // logger.info("Loaded radio medium class: " + radioMediumClassName);
        } else {
          logger.warn("Could not load radio medium class: "
              + radioMediumClassName);
        }
      }
    }

  }

  /**
   * Returns the current extension configuration common to the entire simulator.
   *
   * @return Current extension configuration
   */
  public ProjectConfig getProjectConfig() {
    return projectConfig;
  }

  /**
   * Returns the current extension directories common to the entire simulator.
   *
   * @return Current extension directories.
   */
  public COOJAProject[] getProjects() {
    return currentProjects.toArray(new COOJAProject[0]);
  }

  // // PLUGIN METHODS ////

  /**
   * Show a started plugin in working area.
   *
   * @param plugin Plugin
   */
  public void showPlugin(final Plugin plugin) {
    new RunnableInEDT<Boolean>() {
      public Boolean work() {
        JInternalFrame pluginFrame = plugin.getGUI();
        if (pluginFrame == null) {
          logger.fatal("Failed trying to show plugin without visualizer.");
          return false;
        }

        int nrFrames = myDesktopPane.getAllFrames().length;
        myDesktopPane.add(pluginFrame);

        /* Set size if not already specified by plugin */
        if (pluginFrame.getWidth() <= 0 || pluginFrame.getHeight() <= 0) {
          pluginFrame.setSize(FRAME_STANDARD_WIDTH, FRAME_STANDARD_HEIGHT);
        }

        /* Set location if not already visible */
        if (pluginFrame.getLocation().x <= 0 && pluginFrame.getLocation().y <= 0) {
          pluginFrame.setLocation(
              nrFrames * FRAME_NEW_OFFSET,
              nrFrames * FRAME_NEW_OFFSET);
        }

        pluginFrame.setVisible(true);

        /* Select plugin */
        try {
          for (JInternalFrame existingPlugin : myDesktopPane.getAllFrames()) {
            existingPlugin.setSelected(false);
          }
          pluginFrame.setSelected(true);
        } catch (Exception e) { }
        myDesktopPane.moveToFront(pluginFrame);

        return true;
      }
    }.invokeAndWait();
  }

  /**
   * Close all mote plugins for given mote.
   *
   * @param mote Mote
   */
  public void closeMotePlugins(Mote mote) {
    for (Plugin p: startedPlugins.toArray(new Plugin[0])) {
      if (!(p instanceof MotePlugin)) {
        continue;
      }

      Mote pluginMote = ((MotePlugin)p).getMote();
      if (pluginMote == mote) {
        removePlugin(p, false);
      }
    }
  }

  /**
   * Remove a plugin from working area.
   *
   * @param plugin
   *          Plugin to remove
   * @param askUser
   *          If plugin is the last one, ask user if we should remove current
   *          simulation also?
   */
  public void removePlugin(final Plugin plugin, final boolean askUser) {
    new RunnableInEDT<Boolean>() {
      public Boolean work() {
        /* Free resources */
        plugin.closePlugin();
        startedPlugins.remove(plugin);
        updateGUIComponentState();

        /* Dispose visualized components */
        if (plugin.getGUI() != null) {
          plugin.getGUI().dispose();
        }

        /* (OPTIONAL) Remove simulation if all plugins are closed */
        if (getSimulation() != null && askUser && startedPlugins.isEmpty()) {
          doRemoveSimulation(true);
        }

        return true;
      }
    }.invokeAndWait();
  }

  /**
   * Same as the {@link #startPlugin(Class, GUI, Simulation, Mote)} method,
   * but does not throw exceptions. If COOJA is visualised, an error dialog
   * is shown if plugin could not be started.
   *
   * @see #startPlugin(Class, GUI, Simulation, Mote)
   * @param pluginClass Plugin class
   * @param argGUI Plugin GUI argument
   * @param argSimulation Plugin simulation argument
   * @param argMote Plugin mote argument
   * @return Started plugin
   */
  private Plugin tryStartPlugin(final Class<? extends Plugin> pluginClass,
      final GUI argGUI, final Simulation argSimulation, final Mote argMote, boolean activate) {
    try {
      return startPlugin(pluginClass, argGUI, argSimulation, argMote, activate);
    } catch (PluginConstructionException ex) {
      if (GUI.isVisualized()) {
        GUI.showErrorDialog(GUI.getTopParentContainer(), "Error when starting plugin", ex, false);
      } else {
        /* If the plugin requires visualization, inform user */
        Throwable cause = ex;
        do {
          if (cause instanceof PluginRequiresVisualizationException) {
            logger.info("Visualized plugin was not started: " + pluginClass);
            return null;
          }
        } while (cause != null && (cause=cause.getCause()) != null);

        logger.fatal("Error when starting plugin", ex);
      }
    }
    return null;
  }

  public Plugin tryStartPlugin(final Class<? extends Plugin> pluginClass,
      final GUI argGUI, final Simulation argSimulation, final Mote argMote) {
    return tryStartPlugin(pluginClass, argGUI, argSimulation, argMote, true);
  }

  public Plugin startPlugin(final Class<? extends Plugin> pluginClass,
      final GUI argGUI, final Simulation argSimulation, final Mote argMote)
  throws PluginConstructionException
  {
    return startPlugin(pluginClass, argGUI, argSimulation, argMote, true);
  }

  /**
   * Starts given plugin. If visualized, the plugin is also shown.
   *
   * @see PluginType
   * @param pluginClass Plugin class
   * @param argGUI Plugin GUI argument
   * @param argSimulation Plugin simulation argument
   * @param argMote Plugin mote argument
   * @return Started plugin
   * @throws PluginConstructionException At errors
   */
  private Plugin startPlugin(final Class<? extends Plugin> pluginClass,
      final GUI argGUI, final Simulation argSimulation, final Mote argMote, boolean activate)
  throws PluginConstructionException
  {

    // Check that plugin class is registered
    if (!pluginClasses.contains(pluginClass)) {
      throw new PluginConstructionException("Tool class not registered: " + pluginClass);
    }

    // Construct plugin depending on plugin type
    int pluginType = pluginClass.getAnnotation(PluginType.class).value();
    Plugin plugin;

    try {
      if (pluginType == PluginType.MOTE_PLUGIN) {
        if (argGUI == null) {
          throw new PluginConstructionException("No GUI argument for mote plugin");
        }
        if (argSimulation == null) {
          throw new PluginConstructionException("No simulation argument for mote plugin");
        }
        if (argMote == null) {
          throw new PluginConstructionException("No mote argument for mote plugin");
        }

        plugin =
          pluginClass.getConstructor(new Class[] { Mote.class, Simulation.class, GUI.class })
          .newInstance(argMote, argSimulation, argGUI);

      } else if (pluginType == PluginType.SIM_PLUGIN
          || pluginType == PluginType.SIM_STANDARD_PLUGIN) {
        if (argGUI == null) {
          throw new PluginConstructionException("No GUI argument for simulation plugin");
        }
        if (argSimulation == null) {
          throw new PluginConstructionException("No simulation argument for simulation plugin");
        }

        plugin =
          pluginClass.getConstructor(new Class[] { Simulation.class, GUI.class })
          .newInstance(argSimulation, argGUI);

      } else if (pluginType == PluginType.COOJA_PLUGIN
          || pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        if (argGUI == null) {
          throw new PluginConstructionException("No GUI argument for GUI plugin");
        }

        plugin =
          pluginClass.getConstructor(new Class[] { GUI.class })
          .newInstance(argGUI);

      } else {
        throw new PluginConstructionException("Bad plugin type: " + pluginType);
      }
    } catch (PluginRequiresVisualizationException e) {
      PluginConstructionException ex = new PluginConstructionException("Tool class requires visualization: " + pluginClass.getName());
      ex.initCause(e);
      throw ex;
    } catch (Exception e) {
      PluginConstructionException ex = new PluginConstructionException("Construction error for tool of class: " + pluginClass.getName());
      ex.initCause(e);
      throw ex;
    }

    if (activate) {
      plugin.startPlugin();
    }

    // Add to active plugins list
    startedPlugins.add(plugin);
    updateGUIComponentState();

    // Show plugin if visualizer type
    if (activate && plugin.getGUI() != null) {
      myGUI.showPlugin(plugin);
    }

    return plugin;
  }

  /**
   * Unregister a plugin class. Removes any plugin menu items links as well.
   *
   * @param pluginClass Plugin class
   */
  public void unregisterPlugin(Class<? extends Plugin> pluginClass) {
    pluginClasses.remove(pluginClass);
    menuMotePluginClasses.remove(pluginClass);
  }

  /**
   * Register a plugin to be included in the GUI.
   *
   * @param pluginClass New plugin to register
   * @return True if this plugin was registered ok, false otherwise
   */
  public boolean registerPlugin(final Class<? extends Plugin> pluginClass) {

    /* Check plugin type */
    final int pluginType;
    if (pluginClass.isAnnotationPresent(PluginType.class)) {
      pluginType = pluginClass.getAnnotation(PluginType.class).value();
    } else {
      logger.fatal("Could not register plugin, no plugin type found: " + pluginClass);
      return false;
    }

    /* Check plugin constructor */
    try {
      if (pluginType == PluginType.COOJA_PLUGIN || pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        pluginClass.getConstructor(new Class[] { GUI.class });
      } else if (pluginType == PluginType.SIM_PLUGIN || pluginType == PluginType.SIM_STANDARD_PLUGIN) {
        pluginClass.getConstructor(new Class[] { Simulation.class, GUI.class });
      } else if (pluginType == PluginType.MOTE_PLUGIN) {
        pluginClass.getConstructor(new Class[] { Mote.class, Simulation.class, GUI.class });
        menuMotePluginClasses.add(pluginClass);
      } else {
        logger.fatal("Could not register plugin, bad plugin type: " + pluginType);
        return false;
      }
      pluginClasses.add(pluginClass);
    } catch (NoClassDefFoundError e) {
      logger.fatal("No plugin class: " + pluginClass + ": " + e.getMessage());
      return false;
    } catch (NoSuchMethodException e) {
      logger.fatal("No plugin class constructor: " + pluginClass + ": " + e.getMessage());
      return false;
    }
    return true;
  }

  /**
   * Unregister all plugin classes
   */
  public void unregisterPlugins() {
    menuMotePluginClasses.clear();
    pluginClasses.clear();
  }

  /**
   * Returns started plugin that ends with given class name, if any.
   *
   * @param classname Class name
   * @return Plugin instance
   */
  public Plugin getPlugin(String classname) {
    for (Plugin p: startedPlugins) {
      if (p.getClass().getName().endsWith(classname)) {
        return p;
      }
    }
    return null;
  }

  /**
   * Returns started plugin with given class name, if any.
   *
   * @param classname Class name
   * @return Plugin instance
   * @deprecated
   */
  @Deprecated
  public Plugin getStartedPlugin(String classname) {
    return getPlugin(classname);
  }

  public Plugin[] getStartedPlugins() {
    return startedPlugins.toArray(new Plugin[0]);
  }

  private boolean isMotePluginCompatible(Class<? extends Plugin> motePluginClass, Mote mote) {
    if (motePluginClass.getAnnotation(SupportedArguments.class) == null) {
      return true;
    }

    /* Check mote interfaces */
    boolean moteInterfacesOK = true;
    Class<? extends MoteInterface>[] moteInterfaces =
      motePluginClass.getAnnotation(SupportedArguments.class).moteInterfaces();
    StringBuilder moteTypeInterfacesError = new StringBuilder();
    moteTypeInterfacesError.append(
        "The plugin:\n" +
        getDescriptionOf(motePluginClass) +
        "\nrequires the following mote interfaces:\n"
    );
    for (Class<? extends MoteInterface> requiredMoteInterface: moteInterfaces) {
      moteTypeInterfacesError.append(getDescriptionOf(requiredMoteInterface) + "\n");
      if (mote.getInterfaces().getInterfaceOfType(requiredMoteInterface) == null) {
        moteInterfacesOK = false;
      }
    }

    /* Check mote type */
    boolean moteTypeOK = false;
    Class<? extends Mote>[] motes =
      motePluginClass.getAnnotation(SupportedArguments.class).motes();
    StringBuilder moteTypeError = new StringBuilder();
    moteTypeError.append(
        "The plugin:\n" +
        getDescriptionOf(motePluginClass) +
        "\ndoes not support motes of type:\n" +
        getDescriptionOf(mote) +
        "\n\nIt only supports motes of types:\n"
    );
    for (Class<? extends Mote> supportedMote: motes) {
      moteTypeError.append(getDescriptionOf(supportedMote) + "\n");
      if (supportedMote.isAssignableFrom(mote.getClass())) {
        moteTypeOK = true;
      }
    }

    /*if (!moteInterfacesOK) {
      menuItem.setToolTipText(
          "<html><pre>" + moteTypeInterfacesError + "</html>"
      );
    }
    if (!moteTypeOK) {
      menuItem.setToolTipText(
          "<html><pre>" + moteTypeError + "</html>"
      );
    }*/

    return moteInterfacesOK && moteTypeOK;
  }

  public JMenu createMotePluginsSubmenu(Class<? extends Plugin> pluginClass) {
    JMenu menu = new JMenu(getDescriptionOf(pluginClass));
    if (getSimulation() == null || getSimulation().getMotesCount() == 0) {
      menu.setEnabled(false);
      return menu;
    }

    ActionListener menuItemListener = new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        Object pluginClass = ((JMenuItem)e.getSource()).getClientProperty("class");
        Object mote = ((JMenuItem)e.getSource()).getClientProperty("mote");
        tryStartPlugin((Class<? extends Plugin>) pluginClass, myGUI, getSimulation(), (Mote)mote);
      }
    };

    final int MAX_PER_ROW = 30;
    final int MAX_COLUMNS = 5;

    int added = 0;
    for (Mote mote: getSimulation().getMotes()) {
      if (!isMotePluginCompatible(pluginClass, mote)) {
        continue;
      }

      JMenuItem menuItem = new JMenuItem(mote.toString() + "...");
      menuItem.putClientProperty("class", pluginClass);
      menuItem.putClientProperty("mote", mote);
      menuItem.addActionListener(menuItemListener);

      menu.add(menuItem);
      added++;

      if (added == MAX_PER_ROW) {
        menu.getPopupMenu().setLayout(new GridLayout(MAX_PER_ROW, MAX_COLUMNS));
      }
      if (added >= MAX_PER_ROW*MAX_COLUMNS) {
        break;
      }
    }
    if (added == 0) {
      menu.setEnabled(false);
    }

    return menu;
  }

  /**
   * Return a mote plugins submenu for given mote.
   *
   * @param mote Mote
   * @return Mote plugins menu
   */
  public JMenu createMotePluginsSubmenu(Mote mote) {
    JMenu menuMotePlugins = new JMenu("Mote tools for " + mote);

    for (Class<? extends Plugin> motePluginClass: menuMotePluginClasses) {
      if (!isMotePluginCompatible(motePluginClass, mote)) {
        continue;
      }

      GUIAction guiAction = new StartPluginGUIAction(getDescriptionOf(motePluginClass) + "...");
      JMenuItem menuItem = new JMenuItem(guiAction);
      menuItem.putClientProperty("class", motePluginClass);
      menuItem.putClientProperty("mote", mote);

      menuMotePlugins.add(menuItem);
    }
    return menuMotePlugins;
  }

  // // GUI CONTROL METHODS ////

  /**
   * @return Current simulation
   */
  public Simulation getSimulation() {
    return mySimulation;
  }

  public void setSimulation(Simulation sim, boolean startPlugins) {
    if (sim != null) {
      doRemoveSimulation(false);
    }
    mySimulation = sim;
    updateGUIComponentState();

    // Set frame title
    if (frame != null) {
      frame.setTitle(sim.getTitle() + " - " + WINDOW_TITLE);
    }

    // Open standard plugins (if none opened already)
    if (startPlugins) {
      for (Class<? extends Plugin> pluginClass : pluginClasses) {
        int pluginType = pluginClass.getAnnotation(PluginType.class).value();
        if (pluginType == PluginType.SIM_STANDARD_PLUGIN) {
          tryStartPlugin(pluginClass, this, sim, null);
        }
      }
    }

    setChanged();
    notifyObservers();
  }

  /**
   * Creates a new mote type of the given mote type class.
   * This may include displaying a dialog for user configurations.
   *
   * If mote type is created successfully, the add motes dialog will appear.
   *
   * @param moteTypeClass Mote type class
   */
  public void doCreateMoteType(Class<? extends MoteType> moteTypeClass) {
    doCreateMoteType(moteTypeClass, true);
  }

  /**
   * Creates a new mote type of the given mote type class.
   * This may include displaying a dialog for user configurations.
   *
   * @param moteTypeClass Mote type class
   * @param addMotes Show add motes dialog after successfully adding mote type
   */
  public void doCreateMoteType(Class<? extends MoteType> moteTypeClass, boolean addMotes) {
    if (mySimulation == null) {
      logger.fatal("Can't create mote type (no simulation)");
      return;
    }
    mySimulation.stopSimulation();

    // Create mote type
    MoteType newMoteType = null;
    try {
      newMoteType = moteTypeClass.newInstance();
      if (!newMoteType.configureAndInit(GUI.getTopParentContainer(), mySimulation, isVisualized())) {
        return;
      }
      mySimulation.addMoteType(newMoteType);
    } catch (Exception e) {
      logger.fatal("Exception when creating mote type", e);
      if (isVisualized()) {
        showErrorDialog(getTopParentContainer(), "Mote type creation error", e, false);
      }
      return;
    }

    /* Allow user to immediately add motes */
    if (addMotes) {
      doAddMotes(newMoteType);
    }
  }

  /**
   * Remove current simulation
   *
   * @param askForConfirmation
   *          Should we ask for confirmation if a simulation is already active?
   * @return True if no simulation exists when method returns
   */
  public boolean doRemoveSimulation(boolean askForConfirmation) {

    if (mySimulation == null) {
      return true;
    }

    if (askForConfirmation) {
      boolean ok = new RunnableInEDT<Boolean>() {
        public Boolean work() {
          String s1 = "Remove";
          String s2 = "Cancel";
          Object[] options = { s1, s2 };
          int n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
              "You have an active simulation.\nDo you want to remove it?",
              "Remove current simulation?", JOptionPane.YES_NO_OPTION,
              JOptionPane.QUESTION_MESSAGE, null, options, s2);
          if (n != JOptionPane.YES_OPTION) {
            return false;
          }
          return true;
        }
      }.invokeAndWait();

      if (!ok) {
        return false;
      }
    }

    // Close all started non-GUI plugins
    for (Object startedPlugin : startedPlugins.toArray()) {
      int pluginType = startedPlugin.getClass().getAnnotation(PluginType.class).value();
      if (pluginType != PluginType.COOJA_PLUGIN
          && pluginType != PluginType.COOJA_STANDARD_PLUGIN) {
        removePlugin((Plugin) startedPlugin, false);
      }
    }

    // Delete simulation
    mySimulation.deleteObservers();
    mySimulation.stopSimulation();
    mySimulation.removed();

    /* Clear current mote relations */
    MoteRelation relations[] = getMoteRelations();
    for (MoteRelation r: relations) {
      removeMoteRelation(r.source, r.dest);
    }

    mySimulation = null;
    updateGUIComponentState();

    // Reset frame title
    if (isVisualizedInFrame()) {
      frame.setTitle(WINDOW_TITLE);
    }

    setChanged();
    notifyObservers();

    return true;
  }

  /**
   * Load a simulation configuration file from disk
   *
   * @param askForConfirmation Ask for confirmation before removing any current simulation
   * @param quick Quick-load simulation
   * @param configFile Configuration file to load, if null a dialog will appear
   */
  public void doLoadConfig(boolean askForConfirmation, final boolean quick, File configFile) {
    if (isVisualizedInApplet()) {
      return;
    }

    /* Warn about memory usage */
    if (warnMemory()) {
      return;
    }

    /* Remove current simulation */
    if (!doRemoveSimulation(true)) {
      return;
    }

    /* Use provided configuration, or open File Chooser */
    if (configFile != null && !configFile.isDirectory()) {
      if (!configFile.exists() || !configFile.canRead()) {
        logger.fatal("No read access to file: " + configFile.getAbsolutePath());
        /* File does not exist, open dialog */
        doLoadConfig(askForConfirmation, quick, null);
        return;
      }
    } else {
      final File suggestedFile = configFile;
      configFile = new RunnableInEDT<File>() {
        public File work() {
          JFileChooser fc = new JFileChooser();

          fc.setFileFilter(GUI.SAVED_SIMULATIONS_FILES);

          if (suggestedFile != null && suggestedFile.isDirectory()) {
            fc.setCurrentDirectory(suggestedFile);
          } else {
            /* Suggest file using file history */
            File suggestedFile = getLastOpenedFile();
            if (suggestedFile != null) {
              fc.setSelectedFile(suggestedFile);
            }
          }

          int returnVal = fc.showOpenDialog(GUI.getTopParentContainer());
          if (returnVal != JFileChooser.APPROVE_OPTION) {
            logger.info("Load command cancelled by user...");
            return null;
          }

          File file = fc.getSelectedFile();

          if (!file.exists()) {
            /* Try default file extension */
            file = new File(file.getParent(), file.getName() + SAVED_SIMULATIONS_FILES);
          }

          if (!file.exists() || !file.canRead()) {
            logger.fatal("No read access to file");
            return null;
          }

          return file;
        }
      }.invokeAndWait();

      if (configFile == null) {
        return;
      }
    }

    addToFileHistory(configFile);

    final JDialog progressDialog;
    final String progressTitle = configFile == null
    ? "Loading" : ("Loading " + configFile.getAbsolutePath());

    if (quick) {
      final Thread loadThread = Thread.currentThread();

      progressDialog = new RunnableInEDT<JDialog>() {
        public JDialog work() {
          final JDialog progressDialog = new JDialog((Window) GUI.getTopParentContainer(), progressTitle, ModalityType.APPLICATION_MODAL);

          JPanel progressPanel = new JPanel(new BorderLayout());
          JProgressBar progressBar;
          JButton button;

          progressBar = new JProgressBar(0, 100);
          progressBar.setValue(0);
          progressBar.setIndeterminate(true);

          PROGRESS_BAR = progressBar; /* Allow various parts of COOJA to show messages */

          button = new JButton("Abort");
          button.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
              if (loadThread.isAlive()) {
                loadThread.interrupt();
                doRemoveSimulation(false);
              }
            }
          });

          progressPanel.add(BorderLayout.CENTER, progressBar);
          progressPanel.add(BorderLayout.SOUTH, button);
          progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));

          progressPanel.setVisible(true);

          progressDialog.getContentPane().add(progressPanel);
          progressDialog.setSize(400, 200);

          progressDialog.getRootPane().setDefaultButton(button);
          progressDialog.setLocationRelativeTo(GUI.getTopParentContainer());
          progressDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);

          java.awt.EventQueue.invokeLater(new Runnable() {
            public void run() {
              progressDialog.setVisible(true);
            }
          });

          return progressDialog;
        }
      }.invokeAndWait();
    } else {
      progressDialog = null;
    }

    // Load simulation in this thread, while showing progress monitor
    final File fileToLoad = configFile;
    Simulation newSim = null;
    boolean shouldRetry = false;
    do {
      try {
        shouldRetry = false;
        myGUI.doRemoveSimulation(false);
        PROGRESS_WARNINGS.clear();
        newSim = loadSimulationConfig(fileToLoad, quick);
        myGUI.setSimulation(newSim, false);

        /* Optionally show compilation warnings */
        boolean hideWarn = Boolean.parseBoolean(
            GUI.getExternalToolsSetting("HIDE_WARNINGS", "false")
        );
        if (quick && !hideWarn && !PROGRESS_WARNINGS.isEmpty()) {
          showWarningsDialog(frame, PROGRESS_WARNINGS.toArray(new String[0]));
        }
        PROGRESS_WARNINGS.clear();

      } catch (UnsatisfiedLinkError e) {
        shouldRetry = showErrorDialog(GUI.getTopParentContainer(), "Simulation load error", e, true);
      } catch (SimulationCreationException e) {
        shouldRetry = showErrorDialog(GUI.getTopParentContainer(), "Simulation load error", e, true);
      }
    } while (shouldRetry);

    if (progressDialog != null && progressDialog.isDisplayable()) {
      progressDialog.dispose();
    }
    return;
  }

  /**
   * Reload currently configured simulation.
   * Reloading a simulation may include recompiling Contiki.
   *
   * @param autoStart Start executing simulation when loaded
   * @param randomSeed Simulation's next random seed
   */
  public void reloadCurrentSimulation(final boolean autoStart, final long randomSeed) {
    if (getSimulation() == null) {
      logger.fatal("No simulation to reload");
      return;
    }

    /* Warn about memory usage */
    if (warnMemory()) {
      return;
    }

    final JDialog progressDialog = new JDialog(frame, "Reloading", true);
    final Thread loadThread = new Thread(new Runnable() {
      public void run() {

        /* Get current simulation configuration */
        Element root = new Element("simconf");
        Element simulationElement = new Element("simulation");

        simulationElement.addContent(getSimulation().getConfigXML());
        root.addContent(simulationElement);
        Collection<Element> pluginsConfig = getPluginsConfigXML();
        if (pluginsConfig != null) {
          root.addContent(pluginsConfig);
        }

        /* Remove current simulation, and load config */
        boolean shouldRetry = false;
        do {
          try {
            shouldRetry = false;
            myGUI.doRemoveSimulation(false);
            PROGRESS_WARNINGS.clear();
            Simulation newSim = loadSimulationConfig(root, true, new Long(randomSeed));
            myGUI.setSimulation(newSim, false);

            if (autoStart) {
              newSim.startSimulation();
            }

            /* Optionally show compilation warnings */
            boolean hideWarn = Boolean.parseBoolean(
                GUI.getExternalToolsSetting("HIDE_WARNINGS", "false")
            );
            if (!hideWarn && !PROGRESS_WARNINGS.isEmpty()) {
              showWarningsDialog(frame, PROGRESS_WARNINGS.toArray(new String[0]));
            }
            PROGRESS_WARNINGS.clear();

          } catch (UnsatisfiedLinkError e) {
            shouldRetry = showErrorDialog(frame, "Simulation reload error", e, true);

            myGUI.doRemoveSimulation(false);
          } catch (SimulationCreationException e) {
            shouldRetry = showErrorDialog(frame, "Simulation reload error", e, true);

            myGUI.doRemoveSimulation(false);
          }
        } while (shouldRetry);

        if (progressDialog.isDisplayable()) {
          progressDialog.dispose();
        }
      }
    });

    // Display progress dialog while reloading
    JProgressBar progressBar = new JProgressBar(0, 100);
    progressBar.setValue(0);
    progressBar.setIndeterminate(true);

    PROGRESS_BAR = progressBar; /* Allow various parts of COOJA to show messages */

    JButton button = new JButton("Abort");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (loadThread.isAlive()) {
          loadThread.interrupt();
          doRemoveSimulation(false);
        }
      }
    });

    JPanel progressPanel = new JPanel(new BorderLayout());
    progressPanel.add(BorderLayout.CENTER, progressBar);
    progressPanel.add(BorderLayout.SOUTH, button);
    progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));

    progressPanel.setVisible(true);

    progressDialog.getContentPane().add(progressPanel);
    progressDialog.setSize(400, 200);

    progressDialog.getRootPane().setDefaultButton(button);
    progressDialog.setLocationRelativeTo(frame);
    progressDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);

    loadThread.start();
    progressDialog.setVisible(true);
  }

  private boolean warnMemory() {
    long max = Runtime.getRuntime().maxMemory();
    long used  = Runtime.getRuntime().totalMemory() - Runtime.getRuntime().freeMemory();
    double memRatio = (double) used / (double) max;
    if (memRatio < 0.8) {
      return false;
    }

    DecimalFormat format = new DecimalFormat("0.000");
    logger.warn("Memory usage is getting critical. Reboot Cooja to avoid out of memory error. Current memory usage is " + format.format(100*memRatio) + "%.");
    if (isVisualized()) {
      int n = JOptionPane.showOptionDialog(
          GUI.getTopParentContainer(),
          "Reboot Cooja to avoid out of memory error.\n" +
          "Current memory usage is " + format.format(100*memRatio) + "%.",
          "Out of memory warning",
          JOptionPane.YES_NO_OPTION,
          JOptionPane.WARNING_MESSAGE, null,
          new String[] { "Continue", "Abort"}, "Abort");
      if (n != JOptionPane.YES_OPTION) {
        return true;
      }
    }

    return false;
  }

  /**
   * Reload currently configured simulation.
   * Reloading a simulation may include recompiling Contiki.
   * The same random seed is used.
   *
   * @see #reloadCurrentSimulation(boolean, long)
   * @param autoStart Start executing simulation when loaded
   */
  public void reloadCurrentSimulation(boolean autoStart) {
    reloadCurrentSimulation(autoStart, getSimulation().getRandomSeed());
  }

  /**
   * Save current simulation configuration to disk
   *
   * @param askForConfirmation
   *          Ask for confirmation before overwriting file
   */
  public File doSaveConfig(boolean askForConfirmation) {
    if (isVisualizedInApplet()) {
      return null;
    }

    if (mySimulation == null) {
      return null;
    }

    mySimulation.stopSimulation();

    JFileChooser fc = new JFileChooser();
    fc.setFileFilter(GUI.SAVED_SIMULATIONS_FILES);

    // Suggest file using history
    File suggestedFile = getLastOpenedFile();
    if (suggestedFile != null) {
      fc.setSelectedFile(suggestedFile);
    }

    int returnVal = fc.showSaveDialog(myDesktopPane);
    if (returnVal == JFileChooser.APPROVE_OPTION) {
      File saveFile = fc.getSelectedFile();
      if (!fc.accept(saveFile)) {
        saveFile = new File(saveFile.getParent(), saveFile.getName() + SAVED_SIMULATIONS_FILES);
      }
      if (saveFile.exists()) {
        if (askForConfirmation) {
          String s1 = "Overwrite";
          String s2 = "Cancel";
          Object[] options = { s1, s2 };
          int n = JOptionPane.showOptionDialog(
              GUI.getTopParentContainer(),
              "A file with the same name already exists.\nDo you want to remove it?",
              "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
              JOptionPane.QUESTION_MESSAGE, null, options, s1);
          if (n != JOptionPane.YES_OPTION) {
            return null;
          }
        }
      }
      if (!saveFile.exists() || saveFile.canWrite()) {
        saveSimulationConfig(saveFile);
        addToFileHistory(saveFile);
        return saveFile;
      } else {
      	JOptionPane.showMessageDialog(
      			getTopParentContainer(), "No write access to " + saveFile, "Save failed",
      			JOptionPane.ERROR_MESSAGE);
        logger.fatal("No write access to file: " + saveFile.getAbsolutePath());
      }
    } else {
      logger.info("Save command cancelled by user...");
    }
    return null;
  }

  /**
   * Add new mote to current simulation
   */
  public void doAddMotes(MoteType moteType) {
    if (mySimulation != null) {
      mySimulation.stopSimulation();

      Vector<Mote> newMotes = AddMoteDialog.showDialog(getTopParentContainer(), mySimulation,
          moteType);
      if (newMotes != null) {
        for (Mote newMote : newMotes) {
          mySimulation.addMote(newMote);
        }
      }
      updateGUIComponentState();

    } else {
      logger.warn("No simulation active");
    }
  }

  /**
   * Create a new simulation
   *
   * @param askForConfirmation
   *          Should we ask for confirmation if a simulation is already active?
   */
  public void doCreateSimulation(boolean askForConfirmation) {
    /* Remove current simulation */
    if (!doRemoveSimulation(askForConfirmation)) {
      return;
    }

    // Create new simulation
    Simulation newSim = new Simulation(this);
    boolean createdOK = CreateSimDialog.showDialog(GUI.getTopParentContainer(), newSim);
    if (createdOK) {
      myGUI.setSimulation(newSim, true);
    }
  }

  /**
   * Quit program
   *
   * @param askForConfirmation Should we ask for confirmation before quitting?
   */
  public void doQuit(boolean askForConfirmation) {
    doQuit(askForConfirmation, 0);
  }
  
  public void doQuit(boolean askForConfirmation, int exitCode) {
    if (isVisualizedInApplet()) {
      return;
    }

    if (askForConfirmation) {
      if (getSimulation() != null) {
        /* Save? */
        String s1 = "Yes";
        String s2 = "No";
        String s3 = "Cancel";
        Object[] options = { s1, s2, s3 };
        int n = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
            "Do you want to save the current simulation?",
            WINDOW_TITLE, JOptionPane.YES_NO_CANCEL_OPTION,
            JOptionPane.WARNING_MESSAGE, null, options, s1);
        if (n == JOptionPane.YES_OPTION) {
          if (myGUI.doSaveConfig(true) == null) {
            return;
          }
        } else if (n == JOptionPane.CANCEL_OPTION) {
          return;
        } else if (n != JOptionPane.NO_OPTION) {
          return;
        }
      }
    }

    if (getSimulation() != null) {
      doRemoveSimulation(false);
    }

    // Clean up resources
    Object[] plugins = startedPlugins.toArray();
    for (Object plugin : plugins) {
      removePlugin((Plugin) plugin, false);
    }

    /* Store frame size and position */
    if (isVisualizedInFrame()) {
      setExternalToolsSetting("FRAME_SCREEN", frame.getGraphicsConfiguration().getDevice().getIDstring());
      setExternalToolsSetting("FRAME_POS_X", "" + frame.getLocationOnScreen().x);
      setExternalToolsSetting("FRAME_POS_Y", "" + frame.getLocationOnScreen().y);

      if (frame.getExtendedState() == JFrame.MAXIMIZED_BOTH) {
        setExternalToolsSetting("FRAME_WIDTH", "" + Integer.MAX_VALUE);
        setExternalToolsSetting("FRAME_HEIGHT", "" + Integer.MAX_VALUE);
      } else {
        setExternalToolsSetting("FRAME_WIDTH", "" + frame.getWidth());
        setExternalToolsSetting("FRAME_HEIGHT", "" + frame.getHeight());
      }
    }
    saveExternalToolsUserSettings();

    System.exit(exitCode);
  }

  // // EXTERNAL TOOLS SETTINGS METHODS ////

  /**
   * @return Number of external tools settings
   */
  public static int getExternalToolsSettingsCount() {
    return externalToolsSettingNames.length;
  }

  /**
   * Get name of external tools setting at given index.
   *
   * @param index
   *          Setting index
   * @return Name
   */
  public static String getExternalToolsSettingName(int index) {
    return externalToolsSettingNames[index];
  }

  /**
   * @param name
   *          Name of setting
   * @return Value
   */
  public static String getExternalToolsSetting(String name) {
    return getExternalToolsSetting(name, null);
  }

  /**
   * @param name
   *          Name of setting
   * @param defaultValue
   *          Default value
   * @return Value
   */
  public static String getExternalToolsSetting(String name, String defaultValue) {
    if (specifiedContikiPath != null && "PATH_CONTIKI".equals(name)) {
      return specifiedContikiPath;
    }
    return currentExternalToolsSettings.getProperty(name, defaultValue);
  }

  /**
   * @param name
   *          Name of setting
   * @param defaultValue
   *          Default value
   * @return Value
   */
  public static String getExternalToolsDefaultSetting(String name, String defaultValue) {
    return defaultExternalToolsSettings.getProperty(name, defaultValue);
  }

  /**
   * @param name
   *          Name of setting
   * @param newVal
   *          New value
   */
  public static void setExternalToolsSetting(String name, String newVal) {
    currentExternalToolsSettings.setProperty(name, newVal);
  }

  /**
   * Load external tools settings from default file.
   */
  public static void loadExternalToolsDefaultSettings() {
    String osName = System.getProperty("os.name").toLowerCase();
    String osArch = System.getProperty("os.arch").toLowerCase();

    String filename = null;
    if (osName.startsWith("win")) {
      filename = GUI.EXTERNAL_TOOLS_WIN32_SETTINGS_FILENAME;
    } else if (osName.startsWith("mac os x")) {
      filename = GUI.EXTERNAL_TOOLS_MACOSX_SETTINGS_FILENAME;
    } else if (osName.startsWith("freebsd")) {
      filename = GUI.EXTERNAL_TOOLS_FREEBSD_SETTINGS_FILENAME;
    } else if (osName.startsWith("linux")) {
      filename = GUI.EXTERNAL_TOOLS_LINUX_SETTINGS_FILENAME;
      if (osArch.startsWith("amd64")) {
        filename = GUI.EXTERNAL_TOOLS_LINUX_64_SETTINGS_FILENAME;
      }
    } else {
      logger.warn("Unknown system: " + osName);
      logger.warn("Using default linux external tools configuration");
      filename = GUI.EXTERNAL_TOOLS_LINUX_SETTINGS_FILENAME;
    }

    try {
      InputStream in = GUI.class.getResourceAsStream(EXTERNAL_TOOLS_SETTINGS_FILENAME);
      if (in == null) {
        throw new FileNotFoundException(filename + " not found");
      }
      Properties settings = new Properties();
      settings.load(in);
      in.close();

      in = GUI.class.getResourceAsStream(filename);
      if (in == null) {
        throw new FileNotFoundException(filename + " not found");
      }
      settings.load(in);
      in.close();

      currentExternalToolsSettings = settings;
      defaultExternalToolsSettings = (Properties) currentExternalToolsSettings.clone();
      logger.info("External tools default settings: " + filename);
    } catch (IOException e) {
      logger.warn("Error when reading external tools settings from " + filename, e);
    } finally {
      if (currentExternalToolsSettings == null) {
        defaultExternalToolsSettings = new Properties();
        currentExternalToolsSettings = new Properties();
      }
    }
  }

  /**
   * Load user values from external properties file
   */
  public static void loadExternalToolsUserSettings() {
    if (externalToolsUserSettingsFile == null) {
      return;
    }

    try {
      FileInputStream in = new FileInputStream(externalToolsUserSettingsFile);
      Properties settings = new Properties();
      settings.load(in);
      in.close();

      Enumeration<Object> en = settings.keys();
      while (en.hasMoreElements()) {
        String key = (String) en.nextElement();
        setExternalToolsSetting(key, settings.getProperty(key));
      }
      logger.info("External tools user settings: " + externalToolsUserSettingsFile);
    } catch (FileNotFoundException e) {
      logger.warn("Error when reading user settings from: " + externalToolsUserSettingsFile);
    } catch (IOException e) {
      logger.warn("Error when reading user settings from: " + externalToolsUserSettingsFile);
    }
  }

  /**
   * Save external tools user settings to file.
   */
  public static void saveExternalToolsUserSettings() {
    if (isVisualizedInApplet()) {
      return;
    }

    if (externalToolsUserSettingsFileReadOnly) {
      return;
    }

    try {
      FileOutputStream out = new FileOutputStream(externalToolsUserSettingsFile);

      Properties differingSettings = new Properties();
      Enumeration keyEnum = currentExternalToolsSettings.keys();
      while (keyEnum.hasMoreElements()) {
        String key = (String) keyEnum.nextElement();
        String defaultSetting = getExternalToolsDefaultSetting(key, "");
        String currentSetting = currentExternalToolsSettings.getProperty(key, "");
        if (!defaultSetting.equals(currentSetting)) {
          differingSettings.setProperty(key, currentSetting);
        }
      }

      differingSettings.store(out, "Cooja External Tools (User specific)");
      out.close();
    } catch (FileNotFoundException ex) {
      // Could not open settings file for writing, aborting
      logger.warn("Could not save external tools user settings to "
          + externalToolsUserSettingsFile + ", aborting");
    } catch (IOException ex) {
      // Could not open settings file for writing, aborting
      logger.warn("Error while saving external tools user settings to "
          + externalToolsUserSettingsFile + ", aborting");
    }
  }

  // // GUI EVENT HANDLER ////

  private class GUIEventHandler implements ActionListener {
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("create mote type")) {
        myGUI.doCreateMoteType((Class<? extends MoteType>) ((JMenuItem) e
            .getSource()).getClientProperty("class"));
      } else if (e.getActionCommand().equals("add motes")) {
        myGUI.doAddMotes((MoteType) ((JMenuItem) e.getSource())
            .getClientProperty("motetype"));
      } else if (e.getActionCommand().equals("edit paths")) {
        ExternalToolsDialog.showDialog(GUI.getTopParentContainer());
      } else if (e.getActionCommand().equals("manage extensions")) {
        COOJAProject[] newProjects = ProjectDirectoriesDialog.showDialog(
            GUI.getTopParentContainer(),
            GUI.this,
            getProjects()
        );
        if (newProjects != null) {
        	currentProjects.clear();
        	for (COOJAProject p: newProjects) {
            currentProjects.add(p);
        	}
          try {
            reparseProjectConfig();
          } catch (ParseProjectsException ex) {
            logger.fatal("Error when loading extensions: " + ex.getMessage(), ex);
            if (isVisualized()) {
            	JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
            			"All Cooja extensions could not load.\n\n" +
            			"To manage Cooja extensions:\n" +
            			"Menu->Settings->Cooja extensions",
            			"Reconfigure Cooja extensions", JOptionPane.INFORMATION_MESSAGE);
            }
            showErrorDialog(getTopParentContainer(), "Cooja extensions load error", ex, false);
          }
        }
      } else if (e.getActionCommand().equals("configuration wizard")) {
        ConfigurationWizard.startWizard(GUI.getTopParentContainer(), GUI.this);
      } else {
        logger.warn("Unhandled action: " + e.getActionCommand());
      }
    }
  }

  // // VARIOUS HELP METHODS ////

  /**
   * Help method that tries to load and initialize a class with given name.
   *
   * @param <N> Class extending given class type
   * @param classType Class type
   * @param className Class name
   * @return Class extending given class type or null if not found
   */
  public <N extends Object> Class<? extends N> tryLoadClass(
      Object callingObject, Class<N> classType, String className) {

    if (callingObject != null) {
      try {
        return callingObject.getClass().getClassLoader().loadClass(className).asSubclass(classType);
      } catch (ClassNotFoundException e) {
      } catch (UnsupportedClassVersionError e) {
      }
    }

    try {
      return Class.forName(className).asSubclass(classType);
    } catch (ClassNotFoundException e) {
    } catch (UnsupportedClassVersionError e) {
    }

    if (!isVisualizedInApplet()) {
      try {
        if (projectDirClassLoader != null) {
          return projectDirClassLoader.loadClass(className).asSubclass(
              classType);
        }
      } catch (NoClassDefFoundError e) {
      } catch (ClassNotFoundException e) {
      } catch (UnsupportedClassVersionError e) {
      }
    }

    return null;
  }

  private ClassLoader createClassLoader(Collection<COOJAProject> projects)
  throws ClassLoaderCreationException {
    return createClassLoader(ClassLoader.getSystemClassLoader(), projects);
  }

  public static File findJarFile(File projectDir, String jarfile) {
    File fp = new File(jarfile);
    if (!fp.exists()) {
      fp = new File(projectDir, jarfile);
    }
    if (!fp.exists()) {
      fp = new File(projectDir, "java/" + jarfile);
    }
    if (!fp.exists()) {
      fp = new File(projectDir, "java/lib/" + jarfile);
    }
    if (!fp.exists()) {
      fp = new File(projectDir, "lib/" + jarfile);
    }
    return fp.exists() ? fp : null;
  }

  private ClassLoader createClassLoader(ClassLoader parent, Collection<COOJAProject> projects)
  throws ClassLoaderCreationException {
    if (projects == null || projects.isEmpty()) {
      return parent;
    }

    /* Create class loader from JARs */
    ArrayList<URL> urls = new ArrayList<URL>();
    for (COOJAProject project: projects) {
    	File projectDir = project.dir;
      try {
        urls.add((new File(projectDir, "java")).toURI().toURL());

        // Read configuration to check if any JAR files should be loaded
        ProjectConfig projectConfig = new ProjectConfig(false);
        projectConfig.appendProjectDir(projectDir);
        String[] projectJarFiles = projectConfig.getStringArrayValue(
            GUI.class, "JARFILES");
        if (projectJarFiles != null && projectJarFiles.length > 0) {
          for (String jarfile : projectJarFiles) {
            File jarpath = findJarFile(projectDir, jarfile);
            if (jarpath == null) {
              throw new FileNotFoundException(jarfile);
            }
            urls.add(jarpath.toURI().toURL());
          }
        }

      } catch (Exception e) {
        logger.fatal("Error when trying to read JAR-file in " + projectDir
            + ": " + e);
        throw (ClassLoaderCreationException) new ClassLoaderCreationException(
            "Error when trying to read JAR-file in " + projectDir).initCause(e);
      }
    }

    URL[] urlsArray = urls.toArray(new URL[urls.size()]);
    /* TODO Load from webserver if applet */
    return new URLClassLoader(urlsArray, parent);
  }

  /**
   * Help method that returns the description for given object. This method
   * reads from the object's class annotations if existing. Otherwise it returns
   * the simple class name of object's class.
   *
   * @param object
   *          Object
   * @return Description
   */
  public static String getDescriptionOf(Object object) {
    return getDescriptionOf(object.getClass());
  }

  /**
   * Help method that returns the description for given class. This method reads
   * from class annotations if existing. Otherwise it returns the simple class
   * name.
   *
   * @param clazz
   *          Class
   * @return Description
   */
  public static String getDescriptionOf(Class<? extends Object> clazz) {
    if (clazz.isAnnotationPresent(ClassDescription.class)) {
      return clazz.getAnnotation(ClassDescription.class).value();
    }
    return clazz.getSimpleName();
  }

  /**
   * Help method that returns the abstraction level description for given mote type class.
   *
   * @param clazz
   *          Class
   * @return Description
   */
  public static String getAbstractionLevelDescriptionOf(Class<? extends MoteType> clazz) {
    if (clazz.isAnnotationPresent(AbstractionLevelDescription.class)) {
      return clazz.getAnnotation(AbstractionLevelDescription.class).value();
    }
    return null;
  }

  /**
   * Load configurations and create a GUI.
   *
   * @param args
   *          null
   */
  public static void main(String[] args) {
    String logConfigFile = null;
    for (String element : args) {
      if (element.startsWith("-log4j=")) {
        String arg = element.substring("-log4j=".length());
        logConfigFile = arg;
      }
    }

    try {
      // Configure logger
      if (logConfigFile != null) {
        if (new File(logConfigFile).exists()) {
          DOMConfigurator.configure(logConfigFile);
        } else {
          System.err.println("Failed to open " + logConfigFile);
          System.exit(1);
        }
      } else if (new File(LOG_CONFIG_FILE).exists()) {
        DOMConfigurator.configure(LOG_CONFIG_FILE);
      } else {
        // Used when starting from jar
        DOMConfigurator.configure(GUI.class.getResource("/" + LOG_CONFIG_FILE));
      }

      externalToolsUserSettingsFile = new File(System.getProperty("user.home"), EXTERNAL_TOOLS_USER_SETTINGS_FILENAME);
    } catch (AccessControlException e) {
      BasicConfigurator.configure();
      externalToolsUserSettingsFile = null;
    }

    /* Look and Feel: Nimbus */
    setLookAndFeel();

    /* Warn at no JAVA_HOME */
    String javaHome = System.getenv().get("JAVA_HOME");
    if (javaHome == null || javaHome.equals("")) {
      logger.warn("JAVA_HOME environment variable not set, Cooja motes may not compile");
    }

    // Parse general command arguments
    for (String element : args) {
      if (element.startsWith("-contiki=")) {
        String arg = element.substring("-contiki=".length());
        GUI.specifiedContikiPath = arg;
      }

      if (element.startsWith("-external_tools_config=")) {
        String arg = element.substring("-external_tools_config=".length());
        File specifiedExternalToolsConfigFile = new File(arg);
        if (!specifiedExternalToolsConfigFile.exists()) {
          logger.fatal("Specified external tools configuration not found: " + specifiedExternalToolsConfigFile);
          specifiedExternalToolsConfigFile = null;
          System.exit(1);
        } else {
          GUI.externalToolsUserSettingsFile = specifiedExternalToolsConfigFile;
          GUI.externalToolsUserSettingsFileReadOnly = true;
        }
      }
    }

    // Check if simulator should be quick-started
    if (args.length > 0 && args[0].startsWith("-quickstart=")) {
      String contikiApp = args[0].substring("-quickstart=".length());

      /* Cygwin fix */
      if (contikiApp.startsWith("/cygdrive/")) {
        char driveCharacter = contikiApp.charAt("/cygdrive/".length());
        contikiApp = contikiApp.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
      }

      boolean ok = false;
      if (contikiApp.endsWith(".csc")) {

        ok = quickStartSimulationConfig(new File(contikiApp), true) != null;

      } else {
        if (contikiApp.endsWith(".cooja")) {
          contikiApp = contikiApp.substring(0, contikiApp.length() - ".cooja".length());
        }
        if (!contikiApp.endsWith(".c")) {
          contikiApp += ".c";
        }

        ok = quickStartSimulation(contikiApp);
      }

      if (!ok) {
        System.exit(1);
      }

    } else if (args.length > 0 && args[0].startsWith("-nogui=")) {

      /* Load simulation */
      String config = args[0].substring("-nogui=".length());
      File configFile = new File(config);
      Simulation sim = quickStartSimulationConfig(configFile, false);
      if (sim == null) {
        System.exit(1);
      }
      GUI gui = sim.getGUI();

      /* Make sure at least one test editor is controlling the simulation */
      boolean hasEditor = false;
      for (Plugin startedPlugin : gui.startedPlugins) {
        if (startedPlugin instanceof ScriptRunner) {
          hasEditor = true;
          break;
        }
      }

      /* Backwards compatibility:
       * simulation has no test editor, but has external (old style) test script.
       * We will manually start a test editor from here. */
      if (!hasEditor) {
        File scriptFile = new File(config.substring(0, config.length()-4) + ".js");
        if (scriptFile.exists()) {
          logger.info("Detected old simulation test, starting test editor manually from: " + scriptFile);
          ScriptRunner plugin = (ScriptRunner) gui.tryStartPlugin(ScriptRunner.class, gui, sim, null);
          if (plugin == null) {
            System.exit(1);
          }
          plugin.updateScript(scriptFile);
          try {
            plugin.setScriptActive(true);
          } catch (Exception e) {
            logger.fatal("Error: " + e.getMessage(), e);
            System.exit(1);
          }
        } else {
          logger.fatal("No test editor controlling simulation, aborting");
          System.exit(1);
        }
      }
      sim.setSpeedLimit(null);
      sim.startSimulation();
      
    } else if (args.length > 0 && args[0].startsWith("-applet")) {

      String tmpWebPath=null, tmpBuildPath=null, tmpEsbFirmware=null, tmpSkyFirmware=null;
      for (int i = 1; i < args.length; i++) {
        if (args[i].startsWith("-web=")) {
          tmpWebPath = args[i].substring("-web=".length());
        } else if (args[i].startsWith("-sky_firmware=")) {
          tmpSkyFirmware = args[i].substring("-sky_firmware=".length());
        } else if (args[i].startsWith("-esb_firmware=")) {
          tmpEsbFirmware = args[i].substring("-esb_firmware=".length());
        } else if (args[i].startsWith("-build=")) {
          tmpBuildPath = args[i].substring("-build=".length());
        }
      }

      // Applet start-up
      final String webPath = tmpWebPath, buildPath = tmpBuildPath;
      final String skyFirmware = tmpSkyFirmware, esbFirmware = tmpEsbFirmware;
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          JDesktopPane desktop = createDesktopPane();

          applet = CoojaApplet.applet;
          GUI gui = new GUI(desktop);

          GUI.setExternalToolsSetting("PATH_CONTIKI_BUILD", buildPath);
          GUI.setExternalToolsSetting("PATH_CONTIKI_WEB", webPath);

          GUI.setExternalToolsSetting("SKY_FIRMWARE", skyFirmware);
          GUI.setExternalToolsSetting("ESB_FIRMWARE", esbFirmware);

          configureApplet(gui, false);
        }
      });

    } else {

      // Frame start-up
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          JDesktopPane desktop = createDesktopPane();
          frame = new JFrame(WINDOW_TITLE);
          GUI gui = new GUI(desktop);
          configureFrame(gui, false);
        }
      });

    }
  }

  /**
   * Loads a simulation configuration from given file.
   *
   * When loading Contiki mote types, the libraries must be recompiled. User may
   * change mote type settings at this point.
   *
   * @see #saveSimulationConfig(File)
   * @param file
   *          File to read
   * @return New simulation or null if recompiling failed or aborted
   * @throws UnsatisfiedLinkError
   *           If associated libraries could not be loaded
   */
  public Simulation loadSimulationConfig(File file, boolean quick)
  throws UnsatisfiedLinkError, SimulationCreationException {
    this.currentConfigFile = file; /* Used to generate config relative paths */
    try {
      this.currentConfigFile = this.currentConfigFile.getCanonicalFile();
    } catch (IOException e) {
    }

    try {
      SAXBuilder builder = new SAXBuilder();
    	InputStream in = new FileInputStream(file);
      if (file.getName().endsWith(".gz")) {
      	in = new GZIPInputStream(in);
      }
      Document doc = builder.build(in);
      Element root = doc.getRootElement();
      in.close();

      return loadSimulationConfig(root, quick, null);
    } catch (JDOMException e) {
      throw (SimulationCreationException) new SimulationCreationException("Config not wellformed").initCause(e);
    } catch (IOException e) {
      throw (SimulationCreationException) new SimulationCreationException("Load simulation error").initCause(e);
    }
  }

  public Simulation loadSimulationConfig(Element root, boolean quick, Long manualRandomSeed)
  throws SimulationCreationException {
    Simulation newSim = null;

    try {
      // Check that config file version is correct
      if (!root.getName().equals("simconf")) {
        logger.fatal("Not a valid Cooja simulation config.");
        return null;
      }

      /* Verify extension directories */
      boolean projectsOk = verifyProjects(root.getChildren(), !quick);

      /* GENERATE UNIQUE MOTE TYPE IDENTIFIERS */
      root.detach();
      String configString = new XMLOutputter().outputString(new Document(root));

      /* Locate Contiki mote types in config */
      Properties moteTypeIDMappings = new Properties();
      String identifierExtraction = ContikiMoteType.class.getName() + "[\\s\\n]*<identifier>([^<]*)</identifier>";
      Matcher matcher = Pattern.compile(identifierExtraction).matcher(configString);
      while (matcher.find()) {
        moteTypeIDMappings.setProperty(matcher.group(1), "");
      }

      /* Create old to new identifier mappings */
      Enumeration<Object> existingIdentifiers = moteTypeIDMappings.keys();
      while (existingIdentifiers.hasMoreElements()) {
        String existingIdentifier = (String) existingIdentifiers.nextElement();
        MoteType[] existingMoteTypes = null;
        if (mySimulation != null) {
          existingMoteTypes = mySimulation.getMoteTypes();
        }
        ArrayList<Object> reserved = new ArrayList<Object>();
        reserved.addAll(moteTypeIDMappings.keySet());
        reserved.addAll(moteTypeIDMappings.values());
        String newID = ContikiMoteType.generateUniqueMoteTypeID(existingMoteTypes, reserved);
        moteTypeIDMappings.setProperty(existingIdentifier, newID);
      }

      /* Create new config */
      existingIdentifiers = moteTypeIDMappings.keys();
      while (existingIdentifiers.hasMoreElements()) {
        String existingIdentifier = (String) existingIdentifiers.nextElement();
        configString = configString.replaceAll(
            "<identifier>" + existingIdentifier + "</identifier>",
            "<identifier>" + moteTypeIDMappings.get(existingIdentifier) + "</identifier>");
        configString = configString.replaceAll(
            "<motetype_identifier>" + existingIdentifier + "</motetype_identifier>",
            "<motetype_identifier>" + moteTypeIDMappings.get(existingIdentifier) + "</motetype_identifier>");
      }

      /* Replace existing config */
      root = new SAXBuilder().build(new StringReader(configString)).getRootElement();

      // Create new simulation from config
      for (Object element : root.getChildren()) {
        if (((Element) element).getName().equals("simulation")) {
          Collection<Element> config = ((Element) element).getChildren();
          newSim = new Simulation(this);
          System.gc();
          boolean createdOK = newSim.setConfigXML(config, !quick, manualRandomSeed);
          if (!createdOK) {
            logger.info("Simulation not loaded");
            return null;
          }
        }
      }

      // Restart plugins from config
      setPluginsConfigXML(root.getChildren(), newSim, !quick);

    } catch (JDOMException e) {
      throw (SimulationCreationException) new SimulationCreationException(
          "Configuration file not wellformed: " + e.getMessage()).initCause(e);
    } catch (IOException e) {
      throw (SimulationCreationException) new SimulationCreationException(
          "No access to configuration file: " + e.getMessage()).initCause(e);
    } catch (MoteTypeCreationException e) {
      throw (SimulationCreationException) new SimulationCreationException(
          "Mote type creation error: " + e.getMessage()).initCause(e);
    } catch (Exception e) {
      throw (SimulationCreationException) new SimulationCreationException(
          "Unknown error: " + e.getMessage()).initCause(e);
    }

    return newSim;
  }

  /**
   * Saves current simulation configuration to given file and notifies
   * observers.
   *
   * @see #loadSimulationConfig(File, boolean)
   * @param file
   *          File to write
   */
   public void saveSimulationConfig(File file) {
    this.currentConfigFile = file; /* Used to generate config relative paths */
    try {
      this.currentConfigFile = this.currentConfigFile.getCanonicalFile();
    } catch (IOException e) {
    }

    try {
      // Create and write to document
      Document doc = new Document(extractSimulationConfig());
      OutputStream out = new FileOutputStream(file);

      if (file.getName().endsWith(".gz")) {
      	out = new GZIPOutputStream(out);
      }

      XMLOutputter outputter = new XMLOutputter();
      outputter.setFormat(Format.getPrettyFormat());
      outputter.output(doc, out);
      out.close();

      logger.info("Saved to file: " + file.getAbsolutePath());
    } catch (Exception e) {
      logger.warn("Exception while saving simulation config: " + e);
      e.printStackTrace();
    }
  }

  public Element extractSimulationConfig() {
    // Create simulation config
    Element root = new Element("simconf");

    /* Store extension directories meta data */
    for (COOJAProject project: currentProjects) {
      Element projectElement = new Element("project");
      projectElement.addContent(createPortablePath(project.dir).getPath().replaceAll("\\\\", "/"));
      projectElement.setAttribute("EXPORT", "discard");
      root.addContent(projectElement);
    }

    Element simulationElement = new Element("simulation");
    simulationElement.addContent(mySimulation.getConfigXML());
    root.addContent(simulationElement);

    // Create started plugins config
    Collection<Element> pluginsConfig = getPluginsConfigXML();
    if (pluginsConfig != null) {
      root.addContent(pluginsConfig);
    }

    return root;
  }

  /**
   * Returns started plugins config.
   *
   * @return Config or null
   */
  public Collection<Element> getPluginsConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element pluginElement, pluginSubElement;

    /* Loop over all plugins */
    for (Plugin startedPlugin : startedPlugins) {
      int pluginType = startedPlugin.getClass().getAnnotation(PluginType.class).value();

      // Ignore GUI plugins
      if (pluginType == PluginType.COOJA_PLUGIN
          || pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        continue;
      }

      pluginElement = new Element("plugin");
      pluginElement.setText(startedPlugin.getClass().getName());

      // Create mote argument config (if mote plugin)
      if (pluginType == PluginType.MOTE_PLUGIN) {
        pluginSubElement = new Element("mote_arg");
        Mote taggedMote = ((MotePlugin) startedPlugin).getMote();
        for (int moteNr = 0; moteNr < mySimulation.getMotesCount(); moteNr++) {
          if (mySimulation.getMote(moteNr) == taggedMote) {
            pluginSubElement.setText(Integer.toString(moteNr));
            pluginElement.addContent(pluginSubElement);
            break;
          }
        }
      }

      // Create plugin specific configuration
      Collection<Element> pluginXML = startedPlugin.getConfigXML();
      if (pluginXML != null) {
        pluginSubElement = new Element("plugin_config");
        pluginSubElement.addContent(pluginXML);
        pluginElement.addContent(pluginSubElement);
      }

      // If plugin is visualizer plugin, create visualization arguments
      if (startedPlugin.getGUI() != null) {
        JInternalFrame pluginFrame = startedPlugin.getGUI();

        pluginSubElement = new Element("width");
        pluginSubElement.setText("" + pluginFrame.getSize().width);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("z");
        pluginSubElement.setText("" + getDesktopPane().getComponentZOrder(pluginFrame));
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("height");
        pluginSubElement.setText("" + pluginFrame.getSize().height);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("location_x");
        pluginSubElement.setText("" + pluginFrame.getLocation().x);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("location_y");
        pluginSubElement.setText("" + pluginFrame.getLocation().y);
        pluginElement.addContent(pluginSubElement);

        if (pluginFrame.isIcon()) {
          pluginSubElement = new Element("minimized");
          pluginSubElement.setText("" + true);
          pluginElement.addContent(pluginSubElement);
        }
      }

      config.add(pluginElement);
    }

    return config;
  }

  public boolean verifyProjects(Collection<Element> configXML, boolean visAvailable) {
    boolean allOk = true;

    /* Match current extensions against extensions in simulation config */
    for (final Element pluginElement : configXML.toArray(new Element[0])) {
      if (pluginElement.getName().equals("project")) {
        File projectFile = restorePortablePath(new File(pluginElement.getText()));
        try {
          projectFile = projectFile.getCanonicalFile();
        } catch (IOException e) {
        }

        boolean found = false;
        for (COOJAProject currentProject: currentProjects) {
          if (projectFile.getPath().replaceAll("\\\\", "/").
              equals(currentProject.dir.getPath().replaceAll("\\\\", "/"))) {
            found = true;
            break;
          }
        }

        if (!found) {
          logger.warn("Loaded simulation may depend on not found  extension: '" + projectFile + "'");
          allOk = false;
        }
      }
    }

    return allOk;
  }


  /**
   * Starts plugins with arguments in given config.
   *
   * @param configXML
   *          Config XML elements
   * @param simulation
   *          Simulation on which to start plugins
   * @return True if all plugins started, false otherwise
   */
  public boolean setPluginsConfigXML(Collection<Element> configXML,
      Simulation simulation, boolean visAvailable) {

    for (final Element pluginElement : configXML.toArray(new Element[0])) {
      if (pluginElement.getName().equals("plugin")) {

        // Read plugin class
        String pluginClassName = pluginElement.getText().trim();

        /* Backwards compatibility: old visualizers were replaced */
        if (pluginClassName.equals("se.sics.cooja.plugins.VisUDGM") ||
            pluginClassName.equals("se.sics.cooja.plugins.VisBattery") ||
            pluginClassName.equals("se.sics.cooja.plugins.VisTraffic") ||
            pluginClassName.equals("se.sics.cooja.plugins.VisState") ||
            pluginClassName.equals("se.sics.cooja.plugins.VisUDGM")) {
          logger.warn("Old simulation config detected: visualizers have been remade");
          pluginClassName = "se.sics.cooja.plugins.Visualizer";
        }

        Class<? extends Plugin> pluginClass =
          tryLoadClass(this, Plugin.class, pluginClassName);
        if (pluginClass == null) {
          logger.fatal("Could not load plugin class: " + pluginClassName);
          return false;
        }

        // Parse plugin mote argument (if any)
        Mote mote = null;
        for (Element pluginSubElement : (List<Element>) pluginElement.getChildren()) {
          if (pluginSubElement.getName().equals("mote_arg")) {
            int moteNr = Integer.parseInt(pluginSubElement.getText());
            if (moteNr >= 0 && moteNr < simulation.getMotesCount()) {
              mote = simulation.getMote(moteNr);
            }
          }
        }

        /* Start plugin */
        final Plugin startedPlugin = tryStartPlugin(pluginClass, this, simulation, mote, false);
        if (startedPlugin == null) {
          continue;
        }

        /* Apply plugin specific configuration */
        for (Element pluginSubElement : (List<Element>) pluginElement.getChildren()) {
          if (pluginSubElement.getName().equals("plugin_config")) {
            startedPlugin.setConfigXML(pluginSubElement.getChildren(), visAvailable);
          }
        }

        /* Activate plugin */
        startedPlugin.startPlugin();

        /* If Cooja not visualized, ignore window configuration */
        if (startedPlugin.getGUI() == null) {
          continue;
        }

        // If plugin is visualizer plugin, parse visualization arguments
        new RunnableInEDT<Boolean>() {
          public Boolean work() {
            Dimension size = new Dimension(100, 100);
            Point location = new Point(100, 100);

            for (Element pluginSubElement : (List<Element>) pluginElement.getChildren()) {
              if (pluginSubElement.getName().equals("width")) {
                size.width = Integer.parseInt(pluginSubElement.getText());
                startedPlugin.getGUI().setSize(size);
              } else if (pluginSubElement.getName().equals("height")) {
                size.height = Integer.parseInt(pluginSubElement.getText());
                startedPlugin.getGUI().setSize(size);
              } else if (pluginSubElement.getName().equals("z")) {
                int zOrder = Integer.parseInt(pluginSubElement.getText());
                startedPlugin.getGUI().putClientProperty("zorder", zOrder);
              } else if (pluginSubElement.getName().equals("location_x")) {
                location.x = Integer.parseInt(pluginSubElement.getText());
                startedPlugin.getGUI().setLocation(location);
              } else if (pluginSubElement.getName().equals("location_y")) {
                location.y = Integer.parseInt(pluginSubElement.getText());
                startedPlugin.getGUI().setLocation(location);
              } else if (pluginSubElement.getName().equals("minimized")) {
                boolean minimized = Boolean.parseBoolean(pluginSubElement.getText());
                final JInternalFrame pluginGUI = startedPlugin.getGUI();
                if (minimized && pluginGUI != null) {
                  SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                      try {
                        pluginGUI.setIcon(true);
                      } catch (PropertyVetoException e) {
                      }
                    };
                  });
                }
              }
            }

            showPlugin(startedPlugin);
            return true;
          }
        }.invokeAndWait();

      }
    }

    /* Z order visualized plugins */
    try {
    	for (int z=0; z < getDesktopPane().getAllFrames().length; z++) {
        for (JInternalFrame plugin : getDesktopPane().getAllFrames()) {
          if (plugin.getClientProperty("zorder") == null) {
          	continue;
          }
          int zOrder = ((Integer) plugin.getClientProperty("zorder")).intValue();
          if (zOrder != z) {
          	continue;
          }
          getDesktopPane().setComponentZOrder(plugin, zOrder);
          if (z == 0) {
            plugin.setSelected(true);
          }
          plugin.putClientProperty("zorder", null);
          break;
        }
        getDesktopPane().repaint();
    	}
    } catch (Exception e) { }

    return true;
  }

  public class ParseProjectsException extends Exception {
		private static final long serialVersionUID = 1508168026300714850L;
		public ParseProjectsException(String message) {
      super(message);
    }
  }

  public class ClassLoaderCreationException extends Exception {
		private static final long serialVersionUID = 1578001681266277774L;
		public ClassLoaderCreationException(String message) {
      super(message);
    }
  }

  public class SimulationCreationException extends Exception {
		private static final long serialVersionUID = -2414899187405770448L;
		public SimulationCreationException(String message) {
      super(message);
    }
  }

  public class PluginConstructionException extends Exception {
		private static final long serialVersionUID = 8004171223353676751L;
		public PluginConstructionException(String message) {
      super(message);
    }
  }

  /**
   * A simple error dialog with compilation output and stack trace.
   *
   * @param parentComponent
   *          Parent component
   * @param title
   *          Title of error window
   * @param exception
   *          Exception causing window to be shown
   * @param retryAvailable
   *          If true, a retry option is presented
   * @return Retry failed operation
   */
  public static boolean showErrorDialog(final Component parentComponent,
      final String title, final Throwable exception, final boolean retryAvailable) {

    return new RunnableInEDT<Boolean>() {
      public Boolean work() {
        JTabbedPane tabbedPane = new JTabbedPane();
        final JDialog errorDialog;
        if (parentComponent instanceof Dialog) {
          errorDialog = new JDialog((Dialog) parentComponent, title, true);
        } else if (parentComponent instanceof Frame) {
          errorDialog = new JDialog((Frame) parentComponent, title, true);
        } else {
          errorDialog = new JDialog((Frame) null, title);
        }
        Box buttonBox = Box.createHorizontalBox();

        if (exception != null) {
          /* Contiki error */
          if (exception instanceof ContikiError) {
            String contikiError = ((ContikiError) exception).getContikiError();
            MessageList list = new MessageList();
            for (String l: contikiError.split("\n")) {
              list.addMessage(l);
            }
            list.addPopupMenuItem(null, true);
            tabbedPane.addTab("Contiki error", new JScrollPane(list));
          }

          /* Compilation output */
          MessageList compilationOutput = null;
          if (exception instanceof MoteTypeCreationException
              && ((MoteTypeCreationException) exception).hasCompilationOutput()) {
            compilationOutput = ((MoteTypeCreationException) exception).getCompilationOutput();
          } else if (exception.getCause() != null
              && exception.getCause() instanceof MoteTypeCreationException
              && ((MoteTypeCreationException) exception.getCause()).hasCompilationOutput()) {
            compilationOutput = ((MoteTypeCreationException) exception.getCause()).getCompilationOutput();
          }
          if (compilationOutput != null) {
            compilationOutput.addPopupMenuItem(null, true);
            tabbedPane.addTab("Compilation output", new JScrollPane(compilationOutput));
          }

          /* Stack trace */
          MessageList stackTrace = new MessageList();
          PrintStream printStream = stackTrace.getInputStream(MessageList.NORMAL);
          exception.printStackTrace(printStream);
          stackTrace.addPopupMenuItem(null, true);
          tabbedPane.addTab("Java stack trace", new JScrollPane(stackTrace));

          /* Exception message */
          buttonBox.add(Box.createHorizontalStrut(10));
          buttonBox.add(new JLabel(exception.getMessage()));
          buttonBox.add(Box.createHorizontalStrut(10));
        }

        buttonBox.add(Box.createHorizontalGlue());

        if (retryAvailable) {
          Action retryAction = new AbstractAction() {
						private static final long serialVersionUID = 2370456199250998435L;
						public void actionPerformed(ActionEvent e) {
              errorDialog.setTitle("-RETRY-");
              errorDialog.dispose();
            }
          };
          JButton retryButton = new JButton(retryAction);
          retryButton.setText("Retry Ctrl+R");
          buttonBox.add(retryButton);

          InputMap inputMap = errorDialog.getRootPane().getInputMap(
              JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
          inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_R, KeyEvent.CTRL_DOWN_MASK, false), "retry");
          errorDialog.getRootPane().getActionMap().put("retry", retryAction);
        }

        AbstractAction closeAction = new AbstractAction(){
					private static final long serialVersionUID = 6225539435993362733L;
					public void actionPerformed(ActionEvent e) {
            errorDialog.dispose();
          }
        };

        JButton closeButton = new JButton(closeAction);
        closeButton.setText("Close");
        buttonBox.add(closeButton);

        InputMap inputMap = errorDialog.getRootPane().getInputMap(
            JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0, false), "close");
        errorDialog.getRootPane().getActionMap().put("close", closeAction);


        errorDialog.getRootPane().setDefaultButton(closeButton);
        errorDialog.getContentPane().add(BorderLayout.CENTER, tabbedPane);
        errorDialog.getContentPane().add(BorderLayout.SOUTH, buttonBox);
        errorDialog.setSize(700, 500);
        errorDialog.setLocationRelativeTo(parentComponent);
        errorDialog.setVisible(true); /* BLOCKS */

        if (errorDialog.getTitle().equals("-RETRY-")) {
          return true;
        }
        return false;

      }
    }.invokeAndWait();

  }

  private static void showWarningsDialog(final Frame parent, final String[] warnings) {
    new RunnableInEDT<Boolean>() {
      public Boolean work() {
        final JDialog dialog = new JDialog(parent, "Compilation warnings", false);
        Box buttonBox = Box.createHorizontalBox();

        /* Warnings message list */
        MessageList compilationOutput = new MessageList();
        for (String w: warnings) {
          compilationOutput.addMessage(w, MessageList.ERROR);
        }
        compilationOutput.addPopupMenuItem(null, true);

        /* Checkbox */
        buttonBox.add(Box.createHorizontalGlue());
        JCheckBox hideButton = new JCheckBox("Hide compilation warnings", false);
        hideButton.addActionListener(new ActionListener() {
          public void actionPerformed(ActionEvent e) {
            GUI.setExternalToolsSetting("HIDE_WARNINGS",
                "" + ((JCheckBox)e.getSource()).isSelected());
          };
        });
        buttonBox.add(Box.createHorizontalStrut(10));
        buttonBox.add(hideButton);

        /* Close on escape */
        AbstractAction closeAction = new AbstractAction(){
					private static final long serialVersionUID = 2646163984382201634L;
					public void actionPerformed(ActionEvent e) {
            dialog.dispose();
          }
        };
        InputMap inputMap = dialog.getRootPane().getInputMap(
            JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
        inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0, false), "close");
        dialog.getRootPane().getActionMap().put("close", closeAction);

        /* Layout */
        dialog.getContentPane().add(BorderLayout.CENTER, new JScrollPane(compilationOutput));
        dialog.getContentPane().add(BorderLayout.SOUTH, buttonBox);
        dialog.setSize(700, 500);
        dialog.setLocationRelativeTo(parent);
        dialog.setVisible(true);
        return true;
      }
    }.invokeAndWait();
  }

  /**
   * Runs work method in event dispatcher thread.
   * Worker method returns a value.
   *
   * @author Fredrik Osterlind
   */
  public static abstract class RunnableInEDT<T> {
    private T val;

    /**
     * Work method to be implemented.
     *
     * @return Return value
     */
    public abstract T work();

    /**
     * Runs worker method in event dispatcher thread.
     *
     * @see #work()
     * @return Worker method return value
     */
    public T invokeAndWait() {
      if(java.awt.EventQueue.isDispatchThread()) {
        return RunnableInEDT.this.work();
      }

      try {
        java.awt.EventQueue.invokeAndWait(new Runnable() {
          public void run() {
            val = RunnableInEDT.this.work();
          }
        });
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (InvocationTargetException e) {
        e.printStackTrace();
      }

      return val;
    }
  }

  /**
   * This method can be used by various different modules in the simulator to
   * indicate for example that a mote has been selected. All mote highlight
   * listeners will be notified. An example application of mote highlightinh is
   * a simulator visualizer that highlights the mote.
   *
   * @see #addMoteHighlightObserver(Observer)
   * @param m
   *          Mote to highlight
   */
  public void signalMoteHighlight(Mote m) {
    moteHighlightObservable.setChangedAndNotify(m);
  }

  /**
   * Adds directed relation between given motes.
   *
   * @param source Source mote
   * @param dest Destination mote
   */
  public void addMoteRelation(Mote source, Mote dest) {
    addMoteRelation(source, dest, null);
  }

  /**
   * Adds directed relation between given motes.
   *
   * @param source Source mote
   * @param dest Destination mote
   * @param color The color to use when visualizing the mote relation
   */
  public void addMoteRelation(Mote source, Mote dest, Color color) {
    if (source == null || dest == null) {
      return;
    }
    removeMoteRelation(source, dest); /* Unique relations */
    moteRelations.add(new MoteRelation(source, dest, color));
    moteRelationObservable.setChangedAndNotify();
  }

  /**
   * Removes the relations between given motes.
   *
   * @param source Source mote
   * @param dest Destination mote
   */
  public void removeMoteRelation(Mote source, Mote dest) {
    if (source == null || dest == null) {
      return;
    }
    MoteRelation[] arr = getMoteRelations();
    for (MoteRelation r: arr) {
      if (r.source == source && r.dest == dest) {
        moteRelations.remove(r);
        /* Relations are unique */
        moteRelationObservable.setChangedAndNotify();
        break;
      }
    }
  }

  /**
   * @return All current mote relations.
   *
   * @see #addMoteRelationsObserver(Observer)
   */
  public MoteRelation[] getMoteRelations() {
    return moteRelations.toArray(new MoteRelation[moteRelations.size()]);
  }

  /**
   * Adds mote relation observer.
   * Typically used by visualizer plugins.
   *
   * @param newObserver Observer
   */
  public void addMoteRelationsObserver(Observer newObserver) {
    moteRelationObservable.addObserver(newObserver);
  }

  /**
   * Removes mote relation observer.
   * Typically used by visualizer plugins.
   *
   * @param observer Observer
   */
  public void deleteMoteRelationsObserver(Observer observer) {
    moteRelationObservable.deleteObserver(observer);
  }

  /**
   * Tries to convert given file to be "portable".
   * The portable path is either relative to Contiki, or to the configuration (.csc) file.
   *
   * If this method fails, it returns the original file.
   *
   * @param file Original file
   * @return Portable file, or original file is conversion failed
   */
  public File createPortablePath(File file) {
    return createPortablePath(file, true);
  }

  public File createPortablePath(File file, boolean allowConfigRelativePaths) {
    File portable = null;

    portable = createContikiRelativePath(file);
    if (portable != null) {
      /*logger.info("Generated Contiki relative path '" + file.getPath() + "' to '" + portable.getPath() + "'");*/
      return portable;
    }

    if (allowConfigRelativePaths) {
      portable = createConfigRelativePath(file);
      if (portable != null) {
        /*logger.info("Generated config relative path '" + file.getPath() + "' to '" + portable.getPath() + "'");*/
        return portable;
      }
    }

    logger.warn("Path is not portable: '" + file.getPath());
    return file;
  }

  /**
   * Tries to restore a previously "portable" file to be "absolute".
   * If the given file already exists, no conversion is performed.
   *
   * @see #createPortablePath(File)
   * @param file Portable file
   * @return Absolute file
   */
  public File restorePortablePath(File file) {
    if (file == null || file.exists()) {
      /* No conversion possible/needed */
      return file;
    }

    File absolute = null;
    absolute = restoreContikiRelativePath(file);
    if (absolute != null) {
      /*logger.info("Restored Contiki relative path '" + file.getPath() + "' to '" + absolute.getPath() + "'");*/
      return absolute;
    }

    absolute = restoreConfigRelativePath(file);
    if (absolute != null) {
      /*logger.info("Restored config relative path '" + file.getPath() + "' to '" + absolute.getPath() + "'");*/
      return absolute;
    }

    /*logger.info("Portable path was not restored: '" + file.getPath());*/
    return file;
  }

  private final static String[][] PATH_IDENTIFIER = {
	  {"[CONTIKI_DIR]","PATH_CONTIKI",""},
	  {"[COOJA_DIR]","PATH_COOJA","/tools/cooja"},
	  {"[APPS_DIR]","PATH_APPS","/tools/cooja/apps"}
  };
  
  private File createContikiRelativePath(File file) {
    try {
    	int elem = PATH_IDENTIFIER.length;
    	File path[] = new File [elem];
    	String canonicals[] = new String[elem];
    	int match = -1;
    	int mlength = 0;
    	String fileCanonical = file.getCanonicalPath();
      
    	//No so nice, but goes along with GUI.getExternalToolsSetting
    	String defp = GUI.getExternalToolsSetting("PATH_CONTIKI", null);
    	
    	
		for(int i = 0; i < elem; i++){
			path[i] = new File(GUI.getExternalToolsSetting(PATH_IDENTIFIER[i][1], defp + PATH_IDENTIFIER[i][2]));			
			canonicals[i] = path[i].getCanonicalPath();
			if (fileCanonical.startsWith(canonicals[i])){
				if(mlength < canonicals[i].length()){
					mlength = canonicals[i].length();
					match = i;
				}
 
	    	}
		}
      
	    if(match == -1) return null;


	    /* Replace Contiki's canonical path with Contiki identifier */
        String portablePath = fileCanonical.replaceFirst(
          java.util.regex.Matcher.quoteReplacement(canonicals[match]), 
          java.util.regex.Matcher.quoteReplacement(PATH_IDENTIFIER[match][0]));
        File portable = new File(portablePath);
      
        /* Verify conversion */
        File verify = restoreContikiRelativePath(portable);
        if (verify == null || !verify.exists()) {
        	/* Error: did file even exist pre-conversion? */
        	return null;
        }

        return portable;
    } catch (IOException e1) {
      /*logger.warn("Error when converting to Contiki relative path: " + e1.getMessage());*/
      return null;
    }
  }
  
  
  private File restoreContikiRelativePath(File portable) {
  	int elem = PATH_IDENTIFIER.length;
  	File path = null;
	String canonical = null;
	
    try {
    	    	
    	String portablePath = portable.getPath();
    	
        int i = 0;
        //logger.info("PPATH: " + portablePath);
        
    	for(; i < elem; i++){
    		if (portablePath.startsWith(PATH_IDENTIFIER[i][0])) break;
    		
    	}
    	
    	
    	if(i == elem) return null;
    	//logger.info("Found: " + PATH_IDENTIFIER[i][0]);
    	
    	//No so nice, but goes along with GUI.getExternalToolsSetting
    	String defp = GUI.getExternalToolsSetting("PATH_CONTIKI", null);
    	path = new File(GUI.getExternalToolsSetting(PATH_IDENTIFIER[i][1], defp + PATH_IDENTIFIER[i][2]));
    	
    	//logger.info("Config: " + PATH_IDENTIFIER[i][1] + ", " + defp + PATH_IDENTIFIER[i][2] + " = " + path.toString());
		canonical = path.getCanonicalPath();
    	
		
    	File absolute = new File(portablePath.replace(PATH_IDENTIFIER[i][0], canonical));
		if(!absolute.exists()){
			logger.warn("Replaced " + portable  + " with " + absolute.toString() + " (default: "+ defp + PATH_IDENTIFIER[i][2] +"), but could not find it. This does not have to be an error, as the file might be created later.");
		}
    	     
      
    	return absolute;
    } catch (IOException e) {
    	return null;
    }
  }

  private final static String PATH_CONFIG_IDENTIFIER = "[CONFIG_DIR]";
  public File currentConfigFile = null; /* Used to generate config relative paths */
  private File createConfigRelativePath(File file) {
    String id = PATH_CONFIG_IDENTIFIER;
    if (currentConfigFile == null) {
      return null;
    }
    try {
      File configPath = currentConfigFile.getParentFile();
      if (configPath == null) {
        /* File is in current directory */
        configPath = new File("");
      }
      String configCanonical = configPath.getCanonicalPath();

      String fileCanonical = file.getCanonicalPath();
      if (!fileCanonical.startsWith(configCanonical)) {
        /* SPECIAL CASE: Allow one parent directory */
        File parent = new File(configCanonical).getParentFile();
        if (parent != null) {
          configCanonical = parent.getCanonicalPath();
          id += "/..";
        }
      }
      if (!fileCanonical.startsWith(configCanonical)) {
        /* SPECIAL CASE: Allow two parent directories */
        File parent = new File(configCanonical).getParentFile();
        if (parent != null) {
          configCanonical = parent.getCanonicalPath();
          id += "/..";
        }
      }
      if (!fileCanonical.startsWith(configCanonical)) {
        /* SPECIAL CASE: Allow three parent directories */
        File parent = new File(configCanonical).getParentFile();
        if (parent != null) {
          configCanonical = parent.getCanonicalPath();
          id += "/..";
        }
      }
      if (!fileCanonical.startsWith(configCanonical)) {
        /* File is not in a config subdirectory */
        /*logger.info("File is not in a config subdirectory: " + file.getAbsolutePath());*/
        return null;
      }

      /* Replace config's canonical path with config identifier */
      String portablePath = fileCanonical.replaceFirst(
          java.util.regex.Matcher.quoteReplacement(configCanonical),
          java.util.regex.Matcher.quoteReplacement(id));
      File portable = new File(portablePath);

      /* Verify conversion */
      File verify = restoreConfigRelativePath(portable);
      if (verify == null || !verify.exists()) {
        /* Error: did file even exist pre-conversion? */
        return null;
      }

      return portable;
    } catch (IOException e1) {
      /*logger.warn("Error when converting to config relative path: " + e1.getMessage());*/
      return null;
    }
  }
  private File restoreConfigRelativePath(File portable) {
    return restoreConfigRelativePath(currentConfigFile, portable);
  }
  public static File restoreConfigRelativePath(File configFile, File portable) {
    if (configFile == null) {
      return null;
    }
    File configPath = configFile.getParentFile();
    if (configPath == null) {
        /* File is in current directory */
        configPath = new File("");
    }
    String portablePath = portable.getPath();
    if (!portablePath.startsWith(PATH_CONFIG_IDENTIFIER)) {
      return null;
    }
    File absolute = new File(portablePath.replace(PATH_CONFIG_IDENTIFIER, configPath.getAbsolutePath()));
    return absolute;
  }

  private static JProgressBar PROGRESS_BAR = null;
  private static ArrayList<String> PROGRESS_WARNINGS = new ArrayList<String>();
  public static void setProgressMessage(String msg) {
    setProgressMessage(msg, MessageList.NORMAL);
  }
  public static void setProgressMessage(String msg, int type) {
    if (PROGRESS_BAR != null && PROGRESS_BAR.isShowing()) {
      PROGRESS_BAR.setString(msg);
      PROGRESS_BAR.setStringPainted(true);
    }
    if (type != MessageList.NORMAL) {
      PROGRESS_WARNINGS.add(msg);
    }
  }

  /**
   * Load quick help for given object or identifier. Note that this method does not
   * show the quick help pane.
   *
   * @param obj If string: help identifier. Else, the class name of the argument
   * is used as help identifier.
   */
  public void loadQuickHelp(final Object obj) {
    if (obj == null) {
      return;
    }

    String key;
    if (obj instanceof String) {
      key = (String) obj;
    } else {
      key = obj.getClass().getName();
    }

    String help = null;
    if (obj instanceof HasQuickHelp) {
      help = ((HasQuickHelp) obj).getQuickHelp();
    } else {
      if (quickHelpProperties == null) {
        /* Load quickhelp.txt */
        try {
          quickHelpProperties = new Properties();
          quickHelpProperties.load(new FileReader("quickhelp.txt"));
        } catch (Exception e) {
          quickHelpProperties = null;
          help = "<html><b>Failed to read quickhelp.txt:</b><p>" + e.getMessage() + "</html>";
        }
      }

      if (quickHelpProperties != null) {
        help = quickHelpProperties.getProperty(key);
      }
    }

    if (help != null) {
      quickHelpTextPane.setText("<html>" + help + "</html>");
    } else {
      quickHelpTextPane.setText(
          "<html><b>" + getDescriptionOf(obj) +"</b>" +
          "<p>No help available</html>");
    }
    quickHelpTextPane.setCaretPosition(0);
  }

  /* GUI actions */
  abstract class GUIAction extends AbstractAction {
		private static final long serialVersionUID = 6946179457635198477L;
		public GUIAction(String name) {
      super(name);
    }
    public GUIAction(String name, int nmenomic) {
      this(name);
      putValue(Action.MNEMONIC_KEY, nmenomic);
    }
    public GUIAction(String name, KeyStroke accelerator) {
      this(name);
      putValue(Action.ACCELERATOR_KEY, accelerator);
    }
    public GUIAction(String name, int nmenomic, KeyStroke accelerator) {
      this(name, nmenomic);
      putValue(Action.ACCELERATOR_KEY, accelerator);
    }
    public abstract boolean shouldBeEnabled();
  }
  GUIAction newSimulationAction = new GUIAction("New simulation...", KeyEvent.VK_N, KeyStroke.getKeyStroke(KeyEvent.VK_N, ActionEvent.CTRL_MASK)) {
		private static final long serialVersionUID = 5053703908505299911L;
		public void actionPerformed(ActionEvent e) {
      myGUI.doCreateSimulation(true);
    }
    public boolean shouldBeEnabled() {
      return true;
    }
  };
  GUIAction closeSimulationAction = new GUIAction("Close simulation", KeyEvent.VK_C) {
		private static final long serialVersionUID = -4783032948880161189L;
		public void actionPerformed(ActionEvent e) {
      myGUI.doRemoveSimulation(true);
    }
    public boolean shouldBeEnabled() {
      return getSimulation() != null;
    }
  };
  GUIAction reloadSimulationAction = new GUIAction("Reload with same random seed", KeyEvent.VK_K, KeyStroke.getKeyStroke(KeyEvent.VK_R, ActionEvent.CTRL_MASK)) {
		private static final long serialVersionUID = 66579555555421977L;
		public void actionPerformed(ActionEvent e) {
      if (getSimulation() == null) {
        /* Reload last opened simulation */
        final File file = getLastOpenedFile();
        new Thread(new Runnable() {
          public void run() {
            myGUI.doLoadConfig(true, true, file);
          }
        }).start();
        return;
      }

      /* Reload current simulation */
      long seed = getSimulation().getRandomSeed();
      reloadCurrentSimulation(getSimulation().isRunning(), seed);
    }
    public boolean shouldBeEnabled() {
      return true;
    }
  };
  GUIAction reloadRandomSimulationAction = new GUIAction("Reload with new random seed", KeyEvent.VK_N, KeyStroke.getKeyStroke(KeyEvent.VK_R, ActionEvent.CTRL_MASK | ActionEvent.SHIFT_MASK)) {
		private static final long serialVersionUID = -4494402222740250203L;
		public void actionPerformed(ActionEvent e) {
      /* Replace seed before reloading */
      if (getSimulation() != null) {
        getSimulation().setRandomSeed(getSimulation().getRandomSeed()+1);
        reloadSimulationAction.actionPerformed(null);
      }
    }
    public boolean shouldBeEnabled() {
      return getSimulation() != null;
    }
  };
  GUIAction saveSimulationAction = new GUIAction("Save simulation as...", KeyEvent.VK_S) {
		private static final long serialVersionUID = 1132582220401954286L;
		public void actionPerformed(ActionEvent e) {
      myGUI.doSaveConfig(true);
    }
    public boolean shouldBeEnabled() {
      if (isVisualizedInApplet()) {
        return false;
      }
      return getSimulation() != null;
    }
  };
    /*  GUIAction closePluginsAction = new GUIAction("Close all plugins") {
		private static final long serialVersionUID = -37575622808266989L;
		public void actionPerformed(ActionEvent e) {
      Object[] plugins = startedPlugins.toArray();
      for (Object plugin : plugins) {
        removePlugin((Plugin) plugin, false);
      }
    }
    public boolean shouldBeEnabled() {
      return !startedPlugins.isEmpty();
    }
    };*/
  GUIAction exportExecutableJARAction = new GUIAction("Export simulation...") {
		private static final long serialVersionUID = -203601967460630049L;
		public void actionPerformed(ActionEvent e) {
      getSimulation().stopSimulation();

      /* Info message */
      String[] options = new String[] { "OK", "Cancel" };
      int n = JOptionPane.showOptionDialog(
          GUI.getTopParentContainer(),
          "This function attempts to build an executable Cooja JAR from the current simulation.\n" +
          "The JAR will contain all simulation dependencies, including extension JAR files and mote firmware files.\n" +
          "\nExecutable simulations can be used to run already prepared simulations on several computers.\n" +
          "\nThis is an experimental feature.",
          "Export simulation to executable JAR", JOptionPane.OK_CANCEL_OPTION,
          JOptionPane.INFORMATION_MESSAGE, null, options, options[0]);
      if (n != JOptionPane.OK_OPTION) {
        return;
      }

      /* Select output file */
      JFileChooser fc = new JFileChooser();
      FileFilter jarFilter = new FileFilter() {
        public boolean accept(File file) {
          if (file.isDirectory()) {
            return true;
          }
          if (file.getName().endsWith(".jar")) {
            return true;
          }
          return false;
        }
        public String getDescription() {
          return "Java archive";
        }
        public String toString() {
          return ".jar";
        }
      };
      fc.setFileFilter(jarFilter);
      File suggest = new File(getExternalToolsSetting("EXECUTE_JAR_LAST", "cooja_simulation.jar"));
      fc.setSelectedFile(suggest);
      int returnVal = fc.showSaveDialog(GUI.getTopParentContainer());
      if (returnVal != JFileChooser.APPROVE_OPTION) {
        return;
      }
      File outputFile = fc.getSelectedFile();
      if (outputFile.exists()) {
        options = new String[] { "Overwrite", "Cancel" };
        n = JOptionPane.showOptionDialog(
            GUI.getTopParentContainer(),
            "A file with the same name already exists.\nDo you want to remove it?",
            "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, options[0]);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
        outputFile.delete();
      }

      final File finalOutputFile = outputFile;
      setExternalToolsSetting("EXECUTE_JAR_LAST", outputFile.getPath());
      new Thread() {
        public void run() {
          try {
            ExecuteJAR.buildExecutableJAR(GUI.this, finalOutputFile);
          } catch (RuntimeException ex) {
            JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
                ex.getMessage(),
                "Error", JOptionPane.ERROR_MESSAGE);
          }
        }
      }.start();
    }
    public boolean shouldBeEnabled() {
      return getSimulation() != null;
    }
  };
  GUIAction exitCoojaAction = new GUIAction("Exit", 'x') {
		private static final long serialVersionUID = 7523822251658687665L;
		public void actionPerformed(ActionEvent e) {
      myGUI.doQuit(true);
    }
    public boolean shouldBeEnabled() {
      if (isVisualizedInApplet()) {
        return false;
      }
      return true;
    }
  };
  GUIAction startStopSimulationAction = new GUIAction("Start simulation", KeyStroke.getKeyStroke(KeyEvent.VK_S, ActionEvent.CTRL_MASK)) {
		private static final long serialVersionUID = 6750107157493939710L;
		public void actionPerformed(ActionEvent e) {
      /* Start/Stop current simulation */
      Simulation s = getSimulation();
      if (s == null) {
        return;
      }
      if (s.isRunning()) {
        s.stopSimulation();
      } else {
        s.startSimulation();
      }
    }
    public void setEnabled(boolean newValue) {
      if (getSimulation() == null) {
        putValue(NAME, "Start simulation");
      } else if (getSimulation().isRunning()) {
        putValue(NAME, "Pause simulation");
      } else {
        putValue(NAME, "Start simulation");
      }
      super.setEnabled(newValue);
    }
    public boolean shouldBeEnabled() {
      return getSimulation() != null && getSimulation().isRunnable();
    }
  };
  class StartPluginGUIAction extends GUIAction {
               private static final long serialVersionUID = 7368495576372376196L;
               public StartPluginGUIAction(String name) {
      super(name);
    }
    public void actionPerformed(final ActionEvent e) {
      new Thread(new Runnable() {
        public void run() {
          Class<Plugin> pluginClass =
            (Class<Plugin>) ((JMenuItem) e.getSource()).getClientProperty("class");
          Mote mote = (Mote) ((JMenuItem) e.getSource()).getClientProperty("mote");
          tryStartPlugin(pluginClass, myGUI, mySimulation, mote);
        }
      }).start();
    }
    public boolean shouldBeEnabled() {
      return getSimulation() != null;
    }
  }

  GUIAction removeAllMotesAction = new GUIAction("Remove all motes") {
		private static final long serialVersionUID = 4709776747913364419L;
		public void actionPerformed(ActionEvent e) {
      Simulation s = getSimulation();
      if (s.isRunning()) {
        s.stopSimulation();
      }

      while (s.getMotesCount() > 0) {
        s.removeMote(getSimulation().getMote(0));
      }
    }
    public boolean shouldBeEnabled() {
      Simulation s = getSimulation();
      return s != null && s.getMotesCount() > 0;
    }
  };
  GUIAction showQuickHelpAction = new GUIAction("Quick help", KeyStroke.getKeyStroke(KeyEvent.VK_F1, 0)) {
		private static final long serialVersionUID = 3151729036597971681L;
		public void actionPerformed(ActionEvent e) {
      if (!(e.getSource() instanceof JCheckBoxMenuItem)) {
        return;
      }
      boolean show = ((JCheckBoxMenuItem) e.getSource()).isSelected();
      quickHelpTextPane.setVisible(show);
      quickHelpScroll.setVisible(show);
      setExternalToolsSetting("SHOW_QUICKHELP", new Boolean(show).toString());
      ((JPanel)frame.getContentPane()).revalidate();
      updateDesktopSize(getDesktopPane());
    }

    public boolean shouldBeEnabled() {
      return true;
    }
  };
  GUIAction showGettingStartedAction = new GUIAction("Getting started") {
    private static final long serialVersionUID = 2382848024856978524L;
    public void actionPerformed(ActionEvent e) {
      loadQuickHelp("GETTING_STARTED");
      JCheckBoxMenuItem checkBox = ((JCheckBoxMenuItem)showQuickHelpAction.getValue("checkbox"));
      if (checkBox == null) {
        return;
      }
      if (checkBox.isSelected()) {
        return;
      }
      checkBox.doClick();
    }

    public boolean shouldBeEnabled() {
      return true;
    }
  };
  GUIAction showKeyboardShortcutsAction = new GUIAction("Keyboard shortcuts") {
		private static final long serialVersionUID = 2382848024856978524L;
		public void actionPerformed(ActionEvent e) {
      loadQuickHelp("KEYBOARD_SHORTCUTS");
      JCheckBoxMenuItem checkBox = ((JCheckBoxMenuItem)showQuickHelpAction.getValue("checkbox"));
      if (checkBox == null) {
        return;
      }
      if (checkBox.isSelected()) {
        return;
      }
      checkBox.doClick();
    }

    public boolean shouldBeEnabled() {
      return true;
    }
  };
  GUIAction showBufferSettingsAction = new GUIAction("Buffer sizes...") {
		private static final long serialVersionUID = 7018661735211901837L;
		public void actionPerformed(ActionEvent e) {
      if (mySimulation == null) {
        return;
      }
      BufferSettings.showDialog(myDesktopPane, mySimulation);
    }
    public boolean shouldBeEnabled() {
      return mySimulation != null;
    }
  };

}

