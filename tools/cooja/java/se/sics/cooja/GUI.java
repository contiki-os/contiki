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
 * $Id: GUI.java,v 1.45 2007/05/11 10:55:07 fros4943 Exp $
 */

package se.sics.cooja;

import java.awt.*;
import java.awt.Dialog.ModalityType;
import java.awt.event.*;
import java.beans.PropertyVetoException;
import java.io.*;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.*;
import java.util.List;
import javax.swing.*;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.filechooser.FileFilter;
import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.contikimote.*;
import se.sics.cooja.dialogs.*;
import se.sics.cooja.plugins.*;

/**
 * Main file of COOJA Simulator. Typically contains a visualizer for the
 * simulator, but can also be started without visualizer.
 * 
 * This class loads external Java classes (in project directories), and handles the
 * COOJA plugins as well as the configuration system. If provides a number of
 * help methods for the rest of the COOJA system, and is the starting point for
 * loading and saving simulation configs.
 * 
 * @author Fredrik Osterlind
 */
public class GUI {

  /**
   * External tools default Win32 settings filename.
   */
  public static final String EXTERNAL_TOOLS_WIN32_SETTINGS_FILENAME = "/external_tools_win32.config";

  /**
   * External tools default Linux/Unix settings filename.
   */
  public static final String EXTERNAL_TOOLS_LINUX_SETTINGS_FILENAME = "/external_tools_linux.config";

  /**
   * External tools user settings filename.
   */
  public static final String EXTERNAL_TOOLS_USER_SETTINGS_FILENAME = ".cooja.user.properties";
  private static File externalToolsUserSettingsFile = 
    new File(System.getProperty("user.home"), EXTERNAL_TOOLS_USER_SETTINGS_FILENAME);

  private static String specifiedContikiPath = null;
  
  /**
   * Logger settings filename.
   */
  public static final String LOG_CONFIG_FILE = "log4j_config.xml";

  /**
   * Default project configuration filename.
   */
  public static final String PROJECT_DEFAULT_CONFIG_FILENAME = "/cooja_default.config";

  /**
   * User project configuration filename.
   */
  public static final String PROJECT_CONFIG_FILENAME = "cooja.config";

  /**
   * File filter only showing saved simulations files (*.csc).
   */
  public static final FileFilter SAVED_SIMULATIONS_FILES = new FileFilter() {
    public boolean accept(File file) {
      if (file.isDirectory())
        return true;

      if (file.getName().endsWith(".csc"))
        return true;

      return false;
    }

    public String getDescription() {
      return "COOJA Configuration files";
    }

    public String toString() {
      return ".csc";
    }
  };

  /**
   * Main frame for current GUI. Null when COOJA is run without visualizer!
   */
  public static JFrame frame;

  private static final long serialVersionUID = 1L;

  private static Logger logger = Logger.getLogger(GUI.class);

  // External tools setting names
  private static Properties defaultExternalToolsSettings;
  private static Properties currentExternalToolsSettings;

  private static final String externalToolsSettingNames[] = new String[] {
      "PATH_CONTIKI", "PATH_COOJA_CORE_RELATIVE", "PATH_MAKE", "PATH_SHELL",
      "PATH_C_COMPILER", "COMPILER_ARGS", "PATH_LINKER", "PATH_NM", "NM_ARGS",
      "PATH_OBJDUMP", "OBJDUMP_ARGS", "LINKER_ARGS_1", "LINKER_ARGS_2",
      "CONTIKI_STANDARD_PROCESSES", "CMD_GREP_PROCESSES",
      "REGEXP_PARSE_PROCESSES", "CMD_GREP_INTERFACES",
      "REGEXP_PARSE_INTERFACES", "CMD_GREP_SENSORS", "REGEXP_PARSE_SENSORS",
      "CONTIKI_MAIN_TEMPLATE_FILENAME", "DEFAULT_PROJECTDIRS",
      "CORECOMM_TEMPLATE_FILENAME", "PATH_JAVAC"};

  private static final int FRAME_NEW_OFFSET = 30;

  private static final int FRAME_STANDARD_WIDTH = 150;

  private static final int FRAME_STANDARD_HEIGHT = 300;

  private GUI myGUI;

  private Simulation mySimulation;

  protected GUIEventHandler guiEventHandler = new GUIEventHandler();

  private JMenu menuPlugins, menuMoteTypeClasses, menuMoteTypes;

  private JMenu menuOpenSimulation, menuConfOpenSimulation;

  private Vector<Class<? extends Plugin>> menuMotePluginClasses;
  
  private JDesktopPane myDesktopPane;

  private Vector<Plugin> startedPlugins = new Vector<Plugin>();

  // Platform configuration variables
  // Maintained via method reparseProjectConfig()
  private ProjectConfig projectConfig;

  private Vector<File> currentProjectDirs = new Vector<File>();

  private ClassLoader projectDirClassLoader;

  private Vector<Class<? extends MoteType>> moteTypeClasses = new Vector<Class<? extends MoteType>>();

  private Vector<Class<? extends Plugin>> pluginClasses = new Vector<Class<? extends Plugin>>();

  private Vector<Class<? extends Plugin>> pluginClassesTemporary = new Vector<Class<? extends Plugin>>();

  private Vector<Class<? extends RadioMedium>> radioMediumClasses = new Vector<Class<? extends RadioMedium>>();

  private Vector<Class<? extends IPDistributor>> ipDistributorClasses = new Vector<Class<? extends IPDistributor>>();

  private Vector<Class<? extends Positioner>> positionerClasses = new Vector<Class<? extends Positioner>>();

  /**
   * Creates a new COOJA Simulator GUI.
   * 
   * @param desktop Desktop pane
   * @param createSimDialog If true, a create simulation dialog will be displayed at startup
   */
  public GUI(JDesktopPane desktop, boolean createSimDialog) {
    myGUI = this;
    mySimulation = null;
    myDesktopPane = desktop;

    // Load default and overwrite with user settings (if any)
    loadExternalToolsDefaultSettings();
    loadExternalToolsUserSettings();
    if (specifiedContikiPath != null) {
      setExternalToolsSetting("PATH_CONTIKI", specifiedContikiPath);
    }
    
    
    // Add menu bar
    if (frame != null) {
      frame.setJMenuBar(createMenuBar());
    }

    // Register default project directories
    String defaultProjectDirs = getExternalToolsSetting(
        "DEFAULT_PROJECTDIRS", null);
    if (defaultProjectDirs != null) {
      String[] defaultProjectDirsArr = defaultProjectDirs.split(";");
      if (defaultProjectDirsArr.length > 0) {
        for (String defaultProjectDir : defaultProjectDirsArr) {
          File projectDir = new File(defaultProjectDir);
          if (projectDir.exists() && projectDir.isDirectory()) {
            currentProjectDirs.add(projectDir);
          }
        }
      }
    }

    // Load extendable parts (using current project config)
    reparseProjectConfig();

    // Start all standard GUI plugins
    for (Class<? extends Plugin> visPluginClass : pluginClasses) {
      int pluginType = visPluginClass.getAnnotation(PluginType.class).value();
      if (pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        startPlugin(visPluginClass, this, null, null);
      }
    }

    if (createSimDialog && frame != null) {
      SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          myGUI.doCreateSimulation(true);
        }
      });
    }
  }

  /**
   * @return True if simulator is visualized
   */
  public boolean isVisualized() {
    return frame != null;
  }

  private Vector<File> getFileHistory() {
    Vector<File> history = new Vector<File>();

    // Fetch current history
    String[] historyArray = getExternalToolsSetting("SIMCFG_HISTORY", "").split(";");

    for (String file: historyArray) {
      history.add(new File(file));
    }
    
    return history;
  }

  private void addToFileHistory(File file) {
    // Fetch current history
    String[] history = getExternalToolsSetting("SIMCFG_HISTORY", "").split(";");
    
    // Create new history
    String[] newHistory = null;
    if (history == null || history.length <= 1 && history[0].equals("")) {
      newHistory = new String[1];
    } else {
      newHistory = new String[Math.min(5, history.length+1)];
      System.arraycopy(history, 0, newHistory, 1, newHistory.length-1);
    }
    newHistory[0] = file.getAbsolutePath();

    // Abort if file added is equal to last file
    if (history.length >= 1 && 
        file.getAbsolutePath().equals(new File(history[0]).getAbsolutePath())) {
      return;
    }
    
    String newHistoryConfig = null;
    for (String path: newHistory) {
      if (newHistoryConfig == null)
        newHistoryConfig = path;
      else
        newHistoryConfig += ";" + path;
    }

    setExternalToolsSetting("SIMCFG_HISTORY", newHistoryConfig);
    saveExternalToolsUserSettings();
  }
  
  private void updateOpenHistoryMenuItems() {
    menuConfOpenSimulation.removeAll();
    
    JMenuItem browseItem = new JMenuItem("Browse...");
    browseItem.setActionCommand("confopen sim");
    browseItem.addActionListener(guiEventHandler);
    menuConfOpenSimulation.add(browseItem);
    menuConfOpenSimulation.add(new JSeparator());
    Vector<File> openFilesHistory = getFileHistory();

    for (File file: openFilesHistory) {
      JMenuItem lastItem = new JMenuItem(file.getName());
      lastItem.setActionCommand("confopen last sim");
      lastItem.putClientProperty("file", file);
      lastItem.setToolTipText(file.getAbsolutePath());
      lastItem.addActionListener(guiEventHandler);
      menuConfOpenSimulation.add(lastItem);
    }

    menuOpenSimulation.removeAll();
    
    browseItem = new JMenuItem("Browse...");
    browseItem.setActionCommand("open sim");
    browseItem.addActionListener(guiEventHandler);
    menuOpenSimulation.add(browseItem);
    menuOpenSimulation.add(new JSeparator());
    
    for (File file: openFilesHistory) {
      JMenuItem lastItem = new JMenuItem(file.getName());
      lastItem.setActionCommand("open last sim");
      lastItem.putClientProperty("file", file);
      lastItem.setToolTipText(file.getAbsolutePath());
      lastItem.addActionListener(guiEventHandler);
      menuOpenSimulation.add(lastItem);
    }
  }

  private JMenuBar createMenuBar() {
    JMenuBar menuBar = new JMenuBar();
    JMenu menu;
    JMenuItem menuItem;

    // File menu
    menu = new JMenu("File");
    menu.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        updateOpenHistoryMenuItems();
      }
      public void menuDeselected(MenuEvent e) {
      }

      public void menuCanceled(MenuEvent e) {
      }
    });
    menu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(menu);

    menuItem = new JMenuItem("New simulation");
    menuItem.setMnemonic(KeyEvent.VK_N);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,
        ActionEvent.CTRL_MASK));
    menuItem.setActionCommand("new sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menuItem = new JMenuItem("Reload simulation");
    menuItem.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        reloadCurrentSimulation();
      }
    });
    menu.add(menuItem);
    
    menuItem = new JMenuItem("Close simulation");
    menuItem.setMnemonic(KeyEvent.VK_C);
    menuItem.setActionCommand("close sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);
    
    menuOpenSimulation = new JMenu("Open simulation");
    menuOpenSimulation.setMnemonic(KeyEvent.VK_O);
    menu.add(menuOpenSimulation);

    menuConfOpenSimulation = new JMenu("Open & Reconfigure simulation");
    menuConfOpenSimulation.setMnemonic(KeyEvent.VK_R);
    menu.add(menuConfOpenSimulation);

    menuItem = new JMenuItem("Save simulation");
    menuItem.setMnemonic(KeyEvent.VK_S);
    menuItem.setActionCommand("save sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menu.addSeparator();

    menuItem = new JMenuItem("Close all plugins");
    menuItem.setActionCommand("close plugins");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menu.addSeparator();

    menuItem = new JMenuItem("Exit");
    menuItem.setMnemonic(KeyEvent.VK_X);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X,
        ActionEvent.CTRL_MASK));
    menuItem.setActionCommand("quit");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    // Simulation menu
    menu = new JMenu("Simulation");
    menu.setMnemonic(KeyEvent.VK_S);
    menuBar.add(menu);

    menuItem = new JMenuItem("Open Control");
    menuItem.setMnemonic(KeyEvent.VK_C);
    menuItem.setActionCommand("start plugin");
    menuItem.putClientProperty("class", SimControl.class);
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menuItem = new JMenuItem("Information");
    menuItem.setMnemonic(KeyEvent.VK_I);
    menuItem.setActionCommand("start plugin");
    menuItem.putClientProperty("class", SimInformation.class);
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    // Mote type menu
    menu = new JMenu("Mote Types");
    menu.setMnemonic(KeyEvent.VK_T);
    menuBar.add(menu);

    // Mote type classes sub menu
    menuMoteTypeClasses = new JMenu("Create mote type");
    menuMoteTypeClasses.setMnemonic(KeyEvent.VK_C);
    menuMoteTypeClasses.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        // Clear menu
        menuMoteTypeClasses.removeAll();

        // Recreate menu items
        JMenuItem menuItem;

        for (Class<? extends MoteType> moteTypeClass : moteTypeClasses) {
          String description = GUI.getDescriptionOf(moteTypeClass);
          menuItem = new JMenuItem(description);
          menuItem.setActionCommand("create mote type");
          menuItem.putClientProperty("class", moteTypeClass);
          menuItem.addActionListener(guiEventHandler);
          menuMoteTypeClasses.add(menuItem);
        }
      }

      public void menuDeselected(MenuEvent e) {
      }

      public void menuCanceled(MenuEvent e) {
      }
    });
    menu.add(menuMoteTypeClasses);

    menuItem = new JMenuItem("Information");
    menuItem.setActionCommand("start plugin");
    menuItem.putClientProperty("class", MoteTypeInformation.class);
    menuItem.addActionListener(guiEventHandler);

    menu.add(menuItem);

    // Mote menu
    menu = new JMenu("Motes");
    menu.setMnemonic(KeyEvent.VK_M);
    menuBar.add(menu);

    // Mote types sub menu
    menuMoteTypes = new JMenu("Add motes of type");
    menuMoteTypes.setMnemonic(KeyEvent.VK_A);
    menuMoteTypes.addMenuListener(new MenuListener() {
      public void menuSelected(MenuEvent e) {
        // Clear menu
        menuMoteTypes.removeAll();

        if (mySimulation == null) {
          return;
        }

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
      }

      public void menuDeselected(MenuEvent e) {
      }

      public void menuCanceled(MenuEvent e) {
      }
    });
    menu.add(menuMoteTypes);

    menuItem = new JMenuItem("Remove all motes");
    menuItem.setActionCommand("remove all motes");
    menuItem.addActionListener(guiEventHandler);

    menu.add(menuItem);

    // Plugins menu
    menuPlugins = new JMenu("Plugins");
    menuPlugins.setMnemonic(KeyEvent.VK_P);
    menuBar.add(menuPlugins);

    // Settings menu
    menu = new JMenu("Settings");
    menuBar.add(menu);

    menuItem = new JMenuItem("External tools paths");
    menuItem.setActionCommand("edit paths");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menuItem = new JMenuItem("Manage project directories");
    menuItem.setActionCommand("manage projects");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    // Mote plugins popup menu (not available via menu bar)
    menuMotePluginClasses = new Vector<Class<? extends Plugin>>();
    return menuBar;
  }

  private static void createAndShowGUI() {

    // Make sure we have nice window decorations.
    JFrame.setDefaultLookAndFeelDecorated(true);
    JDialog.setDefaultLookAndFeelDecorated(true);

    // Create and set up the window.
    frame = new JFrame("COOJA Simulator");
    frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    // Create and set up the content pane.
    JDesktopPane desktop = new JDesktopPane();
    desktop.setDragMode(JDesktopPane.OUTLINE_DRAG_MODE);

    GUI gui = new GUI(desktop, true);
    JComponent newContentPane = gui.getDesktopPane();
    newContentPane.setOpaque(true);
    frame.setContentPane(newContentPane);

    frame.setSize(700, 700);
    frame.setLocationRelativeTo(null);
    frame.addWindowListener(gui.guiEventHandler);

    // Restore last frame size and position
    if (frame != null) {
      int framePosX = Integer.parseInt(getExternalToolsSetting("FRAME_POS_X", "-1"));
      int framePosY = Integer.parseInt(getExternalToolsSetting("FRAME_POS_Y", "-1"));
      int frameWidth = Integer.parseInt(getExternalToolsSetting("FRAME_WIDTH", "-1"));
      int frameHeight = Integer.parseInt(getExternalToolsSetting("FRAME_HEIGHT", "-1"));
      if (framePosX >= 0 && framePosY >= 0 && frameWidth > 0 && frameHeight > 0) {
        frame.setLocation(framePosX, framePosY);
        frame.setSize(frameWidth, frameHeight);
      }
    }

    // Display the window.
    frame.setVisible(true);
  }

  /**
   * @return Current desktop pane (simulator visualizer)
   */
  public JDesktopPane getDesktopPane() {
    return myDesktopPane;
  }

  /**
   * Quick-starts a simulation using given parameters. TODO Experimental code
   * 
   * @param moteTypeID
   *          Mote type ID (if null "mtype1" will be used)
   * @param projectDirs
   *          GUI project directories
   * @param sensors
   *          Contiki sensors (if null sensors will be scanned for)
   * @param coreInterfaces
   *          COOJA core interfaces (if null interfaces will be scanned for)
   * @param userProcesses
   *          Contiki user processes (if null processes all in given main file
   *          will be added)
   * @param addAutostartProcesses
   *          Should autostart processes automatically be added?
   * @param numberOfNodes
   *          Number of nodes to add
   * @param areaSideLength
   *          Side of node positioning square
   * @param delayTime
   *          Initial delay time
   * @param simulationStartinge
   *          Simulation automatically started?
   * @param filename
   *          Main Contiki process file
   * @param contikiPath
   *          Contiki path
   * @return True if simulation was quickstarted correctly
   */
  private static boolean quickStartSimulation(String moteTypeID,
      Vector<String> projectDirs, Vector<String> sensors,
      Vector<String> coreInterfaces, Vector<String> userProcesses,
      boolean addAutostartProcesses, int numberOfNodes, double areaSideLength,
      int delayTime, boolean simulationStarting, String filename,
      String contikiPath) {

    // Create GUI and GUI frame (not visible yet)
    JFrame.setDefaultLookAndFeelDecorated(true);
    JDialog.setDefaultLookAndFeelDecorated(true);
    logger.info("> Creating GUI and main frame (invisible)");
    frame = new JFrame("COOJA Simulator");
    frame.setDefaultCloseOperation(JFrame.DO_NOTHING_ON_CLOSE);

    // Create and set up the content pane.
    JDesktopPane desktop = new JDesktopPane();
    desktop.setDragMode(JDesktopPane.OUTLINE_DRAG_MODE);
    GUI gui = new GUI(desktop, false); // loads external settings and creates initial
    // project config

    // Add menu bar
    frame.setSize(700, 700);
    frame.addWindowListener(gui.guiEventHandler);

    JComponent newContentPane = gui.getDesktopPane();
    newContentPane.setOpaque(true);
    frame.setContentPane(newContentPane);
    frame.setLocationRelativeTo(null);

    // Set manual Contiki path if specified
    if (contikiPath != null)
      setExternalToolsSetting("PATH_CONTIKI", contikiPath);

    // Parse project directories and create config
    if (projectDirs == null) {
      projectDirs = new Vector<String>();
      projectDirs.add(".");
    }

    // TODO Should add user prop projects as well here...
    logger.info("> Reparsing project directories and creating config");
    for (String projectDir : projectDirs) {
      logger.info(">> Adding: " + projectDir);
      
      // Check if config file exists
      File configFile = new File(projectDir + File.separatorChar + PROJECT_CONFIG_FILENAME);
      if (!configFile.exists()) {
        logger.debug(">>> Creating empty cooja.config file");
        try {
          configFile.createNewFile();
        } catch (IOException e) {
          logger.fatal(">> Error when creating cooja.config file, aborting");
          return false;
        }
      }
      gui.currentProjectDirs.add(new File(projectDir));
    }
    boolean parsedProjects = gui.reparseProjectConfig();
    if (!parsedProjects) {
      logger.fatal(">> Error when parsing project directories, aborting");
      return false;
    }

    // Check file permissions and paths
    logger.info("> Checking paths and file permissions");
    if (moteTypeID == null)
      moteTypeID = "mtype1";
    File contikiBaseDir = new File(getExternalToolsSetting("PATH_CONTIKI"));
    File contikiCoreDir = new File(contikiBaseDir,
        getExternalToolsSetting("PATH_COOJA_CORE_RELATIVE"));
    File libFile = new File(ContikiMoteType.tempOutputDirectory, moteTypeID
        + ContikiMoteType.librarySuffix);
    File mapFile = new File(ContikiMoteType.tempOutputDirectory, moteTypeID
        + ContikiMoteType.mapSuffix);
    File depFile = new File(ContikiMoteType.tempOutputDirectory, moteTypeID
        + ContikiMoteType.dependSuffix);
    if (libFile.exists())
      libFile.delete();
    if (depFile.exists())
      depFile.delete();
    if (mapFile.exists())
      mapFile.delete();
    if (libFile.exists()) {
      logger.fatal(">> Can't delete output file, aborting: " + libFile);
      return false;
    }
    if (depFile.exists()) {
      logger.fatal(">> Can't delete output file, aborting: " + depFile);
      return false;
    }
    if (mapFile.exists()) {
      logger.fatal(">> Can't delete output file, aborting: " + mapFile);
      return false;
    }

    // Search for main file in current directory (or arg)
    File mainProcessFile = new File(filename);
    logger.info(">> Searching main process file: "
        + mainProcessFile.getAbsolutePath());
    if (!mainProcessFile.exists()) {
      logger.info(">> Searching main process file: "
          + mainProcessFile.getAbsolutePath());
      boolean foundFile = false;
      for (String projectDir : projectDirs) {
        mainProcessFile = new File(projectDir, filename);
        logger.info(">> Searching main process file: "
            + mainProcessFile.getAbsolutePath());
        if (mainProcessFile.exists()) {
          foundFile = true;
          break;
        }
      }
      if (!foundFile) {
        logger.fatal(">> Could not locate main process file, aborting");
        return false;
      }
    }

    // Setup compilation arguments
    logger.info("> Setting up compilation arguments");
    Vector<File> filesToCompile = new Vector<File>();
    filesToCompile.add(mainProcessFile); // main process file
    for (String projectDir : projectDirs)
      // project directories
      filesToCompile.add(new File(projectDir));
    String[] projectSources = // project config sources
    gui.getProjectConfig().getStringArrayValue(ContikiMoteType.class,
        "C_SOURCES");
    for (String projectSource : projectSources) {
      if (!projectSource.equals("")) {
        File file = new File(projectSource);
        if (file.getParent() != null) {
          // Find which project directory added this file
          File projectDir = gui.getProjectConfig().getUserProjectDefining(
              ContikiMoteType.class, "C_SOURCES", projectSource);
          if (projectDir != null) {
            // We found a project directory - Add it
            filesToCompile.add(new File(projectDir.getPath(), file
                .getParent()));
          }
        }
        filesToCompile.add(new File(file.getName()));
      }
    }

    // Scan for sensors
    if (sensors == null) {
      logger.info("> Scanning for sensors");
      sensors = new Vector<String>();
      Vector<String[]> scannedSensorInfo = ContikiMoteTypeDialog
          .scanForSensors(contikiCoreDir);
      for (String projectDir : projectDirs)
        // project directories
        scannedSensorInfo.addAll(ContikiMoteTypeDialog.scanForSensors(new File(
            projectDir)));

      for (String[] sensorInfo : scannedSensorInfo) {
        // logger.info(">> Found and added: " + sensorInfo[1] + " (" +
        // sensorInfo[0] + ")");
        sensors.add(sensorInfo[1]);
      }
    }

    // Scan for core interfaces
    if (coreInterfaces == null) {
      logger.info("> Scanning for core interfaces");
      coreInterfaces = new Vector<String>();
      Vector<String[]> scannedCoreInterfaceInfo = ContikiMoteTypeDialog
          .scanForInterfaces(contikiCoreDir);
      for (String projectDir : projectDirs)
        // project directories
        scannedCoreInterfaceInfo.addAll(ContikiMoteTypeDialog
            .scanForInterfaces(new File(projectDir)));

      for (String[] coreInterfaceInfo : scannedCoreInterfaceInfo) {
        // logger.info(">> Found and added: " + coreInterfaceInfo[1] + " (" +
        // coreInterfaceInfo[0] + ")");
        coreInterfaces.add(coreInterfaceInfo[1]);
      }
    }

    // Scan for mote interfaces
    logger.info("> Loading mote interfaces");
    String[] moteInterfaces = gui.getProjectConfig().getStringArrayValue(
        ContikiMoteType.class, "MOTE_INTERFACES");
    Vector<Class<? extends MoteInterface>> moteIntfClasses = new Vector<Class<? extends MoteInterface>>();
    for (String moteInterface : moteInterfaces) {
      try {
        Class<? extends MoteInterface> newMoteInterfaceClass = gui
            .tryLoadClass(gui, MoteInterface.class, moteInterface);
        moteIntfClasses.add(newMoteInterfaceClass);
        // logger.info(">> Loaded mote interface: " + newMoteInterfaceClass);
      } catch (Exception e) {
        logger.fatal(">> Failed to load mote interface, aborting: "
            + moteInterface + ", " + e.getMessage());
        return false;
      }
    }

    // Scan for processes
    if (userProcesses == null) {
      logger.info("> Scanning for user processes");
      userProcesses = new Vector<String>();
      Vector<String> autostartProcesses = new Vector<String>();
      Vector<String[]> scannedProcessInfo = ContikiMoteTypeDialog
          .scanForProcesses(contikiCoreDir);
      for (String projectDir : projectDirs)
        // project directories
        scannedProcessInfo.addAll(ContikiMoteTypeDialog
            .scanForProcesses(new File(projectDir)));

      for (String[] processInfo : scannedProcessInfo) {
        if (processInfo[0].equals(mainProcessFile.getName())) {
          logger.info(">> Found and added: " + processInfo[1] + " ("
              + processInfo[0] + ")");
          userProcesses.add(processInfo[1]);

          if (addAutostartProcesses) {
            // Parse any autostart processes
            try {
              // logger.info(">>> Parsing " + processInfo[0] + " for autostart
              // processes");
              Vector<String> autostarters = ContikiMoteTypeDialog
                  .parseAutostartProcesses(mainProcessFile);
              if (autostarters != null)
                autostartProcesses.addAll(autostarters);
            } catch (Exception e) {
              logger
                  .fatal(">>> Error when parsing autostart processes, aborting: "
                      + e);
              return false;
            }
          }

        } else {
          // logger.info(">> Found and ignored: " + processInfo[1] + " (" +
          // processInfo[0] + ")");
        }
      }

      if (addAutostartProcesses) {
        // Add autostart process sources if found
        logger.info("> Adding autostart processes");
        for (String autostartProcess : autostartProcesses) {
          boolean alreadyExists = false;
          for (String existingProcess : userProcesses) {
            if (existingProcess.equals(autostartProcess)) {
              alreadyExists = true;
              break;
            }
          }
          if (!alreadyExists) {
            userProcesses.add(autostartProcess);
            logger.info(">> Added autostart process: " + autostartProcess);
          }
        }
      }

    }

    // Generate Contiki main source file
    logger.info("> Generating Contiki main source file");
    if (!ContikiMoteType.tempOutputDirectory.exists())
      ContikiMoteType.tempOutputDirectory.mkdir();
    if (!ContikiMoteType.tempOutputDirectory.exists()) {
      logger.fatal(">> Could not create output directory: "
          + ContikiMoteType.tempOutputDirectory);
      return false;
    }

    try {
      String generatedFilename = ContikiMoteTypeDialog.generateSourceFile(
          moteTypeID, sensors, coreInterfaces, userProcesses);
      // logger.info(">> Generated source file: " + generatedFilename);
    } catch (Exception e) {
      logger.fatal(">> Error during file generation, aborting: "
          + e.getMessage());
      return false;
    }

    // Compile library
    logger.info("> Compiling library");
    boolean compilationSucceded = ContikiMoteTypeDialog.compileLibrary(
        moteTypeID, contikiBaseDir, filesToCompile, false, null, System.err);
    if (!libFile.exists() || !depFile.exists() || !mapFile.exists())
      compilationSucceded = false;

    if (compilationSucceded) {
      // logger.info(">> Compilation complete");
    } else {
      logger.fatal(">> Error during compilation, aborting");
      return false;
    }

    // Create mote type
    logger.info("> Creating mote type");
    ContikiMoteType moteType;
    try {
      moteType = new ContikiMoteType(moteTypeID);
    } catch (MoteTypeCreationException e) {
      logger.fatal("Exception when creating mote type: " + e);
      return false;
    }
    moteType.setDescription("Mote type: " + filename);
    moteType.setContikiBaseDir(contikiBaseDir.getPath());
    moteType.setContikiCoreDir(contikiCoreDir.getPath());
    moteType.setProjectDirs(new Vector<File>());
    moteType.setCompilationFiles(filesToCompile);
    moteType.setConfig(gui.getProjectConfig());
    moteType.setProcesses(userProcesses);
    moteType.setSensors(sensors);
    moteType.setCoreInterfaces(coreInterfaces);
    moteType.setMoteInterfaces(moteIntfClasses);

    // Create simulation
    logger.info("> Creating simulation");
    Simulation simulation = new Simulation(gui);
    simulation.setTitle("Quickstarted: " + filename);
    simulation.setDelayTime(delayTime);
    simulation.setSimulationTime(0);
    simulation.setTickTime(1);
    String radioMediumClassName = null;
    try {
      radioMediumClassName = gui.getProjectConfig().getStringArrayValue(
          GUI.class, "RADIOMEDIUMS")[0];
      Class<? extends RadioMedium> radioMediumClass = gui.tryLoadClass(gui,
          RadioMedium.class, radioMediumClassName);

      RadioMedium radioMedium = RadioMedium.generateRadioMedium(
          radioMediumClass, simulation);
      simulation.setRadioMedium(radioMedium);
    } catch (Exception e) {
      logger.fatal(">> Failed to load radio medium, aborting: "
          + radioMediumClassName + ", " + e);
      return false;
    }

    // Create nodes
    logger.info("> Creating motes");
    Vector<ContikiMote> motes = new Vector<ContikiMote>();
    Random random = new Random();
    int nextMoteID = 1;
    int nextIP = 0;
    for (int i = 0; i < numberOfNodes; i++) {
      ContikiMote mote = (ContikiMote) moteType.generateMote(simulation);

      // Set random position
      if (mote.getInterfaces().getPosition() != null)
        mote.getInterfaces().getPosition().setCoordinates(
            random.nextDouble() * areaSideLength,
            random.nextDouble() * areaSideLength, 0);

      // Set unique mote ID's
      if (mote.getInterfaces().getMoteID() != null)
        mote.getInterfaces().getMoteID().setMoteID(nextMoteID++);

      // Set unique IP address
      if (mote.getInterfaces().getIPAddress() != null) {
        mote.getInterfaces().getIPAddress().setIPNumber((char) 10,
            (char) ((nextIP / (254 * 255)) % 255),
            (char) ((nextIP / 254) % 255), (char) (nextIP % 254 + 1));
        nextIP++;
      }

      motes.add(mote);
    }

    // Add mote type and motes to simulation
    logger.info("> Adding motes and mote type to simulation");
    simulation.addMoteType(moteType);
    for (Mote mote : motes) {
      simulation.addMote(mote);
    }

    // Add simulation to GUI
    logger.info("> Adding simulation to GUI");
    gui.setSimulation(simulation);

    // Start plugins and try to place them wisely
    logger.info("> Starting plugin and showing GUI");
    VisPlugin plugin = (VisPlugin) gui.startPlugin(VisState.class, gui, simulation, null);
    plugin.setLocation(350, 20);
    plugin = (VisPlugin) gui.startPlugin(VisTraffic.class, gui, simulation, null);
    plugin.setLocation(350, 340);
    plugin = (VisPlugin) gui.startPlugin(LogListener.class, gui, simulation, null);
    plugin.setLocation(20, 420);

    // Finally show GUI
    frame.setVisible(true);

    if (simulationStarting) {
      simulation.startSimulation();
    }
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
   * Register new IP distributor class
   * 
   * @param ipDistributorClass
   *          Class to register
   * @return True if class was registered
   */
  public boolean registerIPDistributor(
      Class<? extends IPDistributor> ipDistributorClass) {
    // Check that vector constructor exists
    try {
      ipDistributorClass.getConstructor(new Class[] { Vector.class });
    } catch (Exception e) {
      logger.fatal("No vector constructor found of IP distributor: "
          + ipDistributorClass);
      return false;
    }

    ipDistributorClasses.add(ipDistributorClass);
    return true;
  }

  /**
   * Unregister all IP distributors.
   */
  public void unregisterIPDistributors() {
    ipDistributorClasses.clear();
  }

  /**
   * @return All registered IP distributors
   */
  public Vector<Class<? extends IPDistributor>> getRegisteredIPDistributors() {
    return ipDistributorClasses;
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
   * Builds new project configuration using current project directories settings.
   * Reregisters mote types, plugins, IP distributors, positioners and radio
   * mediums. This method may still return true even if all classes could not be
   * registered, but always returns false if all project directory configuration
   * files were not parsed correctly.
   * 
   * Any registered temporary plugins will be saved and reregistered.
   * 
   * @return True if external configuration files were found and parsed OK
   */
  public boolean reparseProjectConfig() {
    // Backup temporary plugins
    Vector<Class<? extends Plugin>> oldTempPlugins = (Vector<Class<? extends Plugin>>) pluginClassesTemporary
        .clone();

    // Reset current configuration
    unregisterMoteTypes();
    unregisterPlugins();
    unregisterIPDistributors();
    unregisterPositioners();
    unregisterRadioMediums();

    try {
      // Read default configuration
      projectConfig = new ProjectConfig(true);
    } catch (FileNotFoundException e) {
      logger.fatal("Could not find default project config file: "
          + PROJECT_DEFAULT_CONFIG_FILENAME);
      return false;
    } catch (IOException e) {
      logger.fatal("Error when reading default project config file: "
          + PROJECT_DEFAULT_CONFIG_FILENAME);
      return false;
    }

    // Append project directory configurations
    for (File projectDir : currentProjectDirs) {

      try {
        // Append config to general config
        projectConfig.appendProjectDir(projectDir);
      } catch (FileNotFoundException e) {
        logger.fatal("Could not find project config file: " + projectDir);
        return false;
      } catch (IOException e) {
        logger
            .fatal("Error when reading project config file: " + projectDir);
        return false;
      }
    }

    // Create class loader
    projectDirClassLoader = createClassLoader(currentProjectDirs);

    // Register mote types
    String[] moteTypeClassNames = projectConfig.getStringArrayValue(GUI.class,
        "MOTETYPES");
    if (moteTypeClassNames != null) {
      for (String moteTypeClassName : moteTypeClassNames) {
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
    registerPlugin(SimControl.class, false); // Not in menu
    registerPlugin(SimInformation.class, false); // Not in menu
    registerPlugin(MoteTypeInformation.class, false); // Not in menu
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

    // Reregister temporary plugins again
    if (oldTempPlugins != null) {
      for (Class<? extends Plugin> pluginClass : oldTempPlugins) {
        if (registerTemporaryPlugin(pluginClass)) {
          // logger.info("Reregistered temporary plugin class: " +
          // getDescriptionOf(pluginClass));
        } else
          logger.warn("Could not reregister temporary plugin class: "
              + getDescriptionOf(pluginClass));
      }
    }

    // Register IP distributors
    String[] ipDistClassNames = projectConfig.getStringArrayValue(GUI.class,
        "IP_DISTRIBUTORS");
    if (ipDistClassNames != null) {
      for (String ipDistClassName : ipDistClassNames) {
        Class<? extends IPDistributor> ipDistClass = tryLoadClass(this,
            IPDistributor.class, ipDistClassName);

        if (ipDistClass != null) {
          registerIPDistributor(ipDistClass);
          // logger.info("Loaded IP distributor class: " + ipDistClassName);
        } else {
          logger
              .warn("Could not load IP distributor class: " + ipDistClassName);
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

    return true;
  }

  /**
   * Returns the current project configuration common to the entire simulator.
   * 
   * @return Current project configuration
   */
  public ProjectConfig getProjectConfig() {
    return projectConfig;
  }

  /**
   * Returns the current project directories common to the entire simulator.
   * 
   * @return Current project directories.
   */
  public Vector<File> getProjectDirs() {
    return currentProjectDirs;
  }

  // // PLUGIN METHODS ////

  /**
   * Show a started plugin in working area.
   * 
   * @param plugin
   *          Internal frame to add
   */
  public void showPlugin(VisPlugin plugin) {
    int nrFrames = myDesktopPane.getAllFrames().length;
    myDesktopPane.add(plugin);

    // Set standard size if not specified by plugin itself
    if (plugin.getWidth() <= 0 || plugin.getHeight() <= 0) {
      plugin.setSize(FRAME_STANDARD_WIDTH, FRAME_STANDARD_HEIGHT);
    }

    // Set location if not already visible
    if (!plugin.isVisible()) {
      plugin.setLocation((nrFrames + 1) * FRAME_NEW_OFFSET, (nrFrames + 1)
          * FRAME_NEW_OFFSET);
      plugin.setVisible(true);
    }

    // Deselect all other plugins before selecting the new one
    try {
      for (JInternalFrame existingPlugin : myDesktopPane.getAllFrames()) {
        existingPlugin.setSelected(false);
      }
      plugin.setSelected(true);
    } catch (Exception e) {
      // Ignore
    }

    // Mote plugin to front
    myDesktopPane.moveToFront(plugin);
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
  public void removePlugin(Plugin plugin, boolean askUser) {
    // Clear any allocated resources and remove plugin
    plugin.closePlugin();
    startedPlugins.remove(plugin);

    // Dispose plugin if it has visualizer
    if (plugin instanceof VisPlugin) {
      ((VisPlugin) plugin).dispose();
    }

    if (getSimulation() != null && askUser && startedPlugins.isEmpty()) {
      String s1 = "Remove";
      String s2 = "Cancel";
      Object[] options = { s1, s2 };
      int n = JOptionPane.showOptionDialog(frame,
          "You have an active simulation.\nDo you want to remove it?",
          "Remove current simulation?", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, options, s1);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }
      doRemoveSimulation(false);
    }
  }

  /**
   * Starts a plugin of given plugin class with given arguments.
   * 
   * @param pluginClass
   *          Plugin class
   * @param gui
   *          GUI passed as argument to all plugins
   * @param simulation
   *          Simulation passed as argument to mote and simulation plugins
   * @param mote
   *          Mote passed as argument to mote plugins
   * @return Start plugin if any
   */
  public Plugin startPlugin(Class<? extends Plugin> pluginClass, GUI gui,
      Simulation simulation, Mote mote) {

    // Check that plugin class is registered
    if (!pluginClasses.contains(pluginClass)) {
      logger.fatal("Plugin class not registered: " + pluginClass);
      return null;
    }

    // Check that visualizer plugin is not started without GUI
    if (!isVisualized()) {
      try {
        pluginClass.asSubclass(VisPlugin.class);

        // Cast succeded, plugin is visualizer plugin!
        logger.fatal("Can't start visualizer plugin (no GUI): " + pluginClass);
        return null;
      } catch (ClassCastException e) {
      }
    }

    // Construct plugin depending on plugin type
    Plugin newPlugin = null;
    int pluginType = pluginClass.getAnnotation(PluginType.class).value();

    try {
      if (pluginType == PluginType.MOTE_PLUGIN) {
        if (mote == null) {
          logger.fatal("Can't start mote plugin (no mote selected)");
          return null;
        }

        newPlugin = pluginClass.getConstructor(
            new Class[] { Mote.class, Simulation.class, GUI.class })
            .newInstance(mote, simulation, gui);

        // Tag plugin with mote
        newPlugin.tagWithObject(mote);
      } else if (pluginType == PluginType.SIM_PLUGIN
          || pluginType == PluginType.SIM_STANDARD_PLUGIN) {
        if (simulation == null) {
          logger.fatal("Can't start simulation plugin (no simulation)");
          return null;
        }

        newPlugin = pluginClass.getConstructor(
            new Class[] { Simulation.class, GUI.class }).newInstance(
            simulation, gui);
      } else if (pluginType == PluginType.COOJA_PLUGIN
          || pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        if (gui == null) {
          logger.fatal("Can't start COOJA plugin (no GUI)");
          return null;
        }

        newPlugin = pluginClass.getConstructor(new Class[] { GUI.class })
            .newInstance(gui);
      }
    } catch (Exception e) {
      logger.fatal("Exception thrown when starting plugin: " + e);
      e.printStackTrace();
      return null;
    }

    if (newPlugin == null)
      return null;

    // Add to active plugins list
    startedPlugins.add(newPlugin);

    // Show plugin if visualizer type
    if (newPlugin instanceof VisPlugin) {
      myGUI.showPlugin((VisPlugin) newPlugin);
    }

    return newPlugin;
  }

  /**
   * Register a plugin to be included in the GUI. The plugin will be visible in
   * the menubar.
   * 
   * @param newPluginClass
   *          New plugin to register
   * @return True if this plugin was registered ok, false otherwise
   */
  public boolean registerPlugin(Class<? extends Plugin> newPluginClass) {
    return registerPlugin(newPluginClass, true);
  }

  /**
   * Register a temporary plugin to be included in the GUI. The plugin will be
   * visible in the menubar. This plugin will automatically be unregistered if
   * the current simulation is removed.
   * 
   * @param newPluginClass
   *          New plugin to register
   * @return True if this plugin was registered ok, false otherwise
   */
  public boolean registerTemporaryPlugin(Class<? extends Plugin> newPluginClass) {
    if (pluginClasses.contains(newPluginClass))
      return false;

    boolean returnVal = registerPlugin(newPluginClass, true);
    if (!returnVal)
      return false;

    pluginClassesTemporary.add(newPluginClass);
    return true;
  }

  /**
   * Unregister a plugin class. Removes any plugin menu items links as well.
   * 
   * @param pluginClass
   *          Plugin class to unregister
   */
  public void unregisterPlugin(Class<? extends Plugin> pluginClass) {

    // Remove (if existing) plugin class menu items
    for (Component menuComponent : menuPlugins.getMenuComponents()) {
      if (menuComponent.getClass().isAssignableFrom(JMenuItem.class)) {
        JMenuItem menuItem = (JMenuItem) menuComponent;
        if (menuItem.getClientProperty("class").equals(pluginClass))
          menuPlugins.remove(menuItem);
      }
    }
    if (menuMotePluginClasses.contains(pluginClass))
      menuMotePluginClasses.remove(pluginClass);

    // Remove from plugin vectors (including temporary)
    if (pluginClasses.contains(pluginClass))
      pluginClasses.remove(pluginClass);
    if (pluginClassesTemporary.contains(pluginClass))
      pluginClassesTemporary.remove(pluginClass);
  }

  /**
   * Register a plugin to be included in the GUI.
   * 
   * @param newPluginClass
   *          New plugin to register
   * @param addToMenu
   *          Should this plugin be added to the dedicated plugins menubar?
   * @return True if this plugin was registered ok, false otherwise
   */
  private boolean registerPlugin(Class<? extends Plugin> newPluginClass,
      boolean addToMenu) {

    // Get description annotation (if any)
    String description = getDescriptionOf(newPluginClass);

    // Get plugin type annotation (required)
    int pluginType;
    if (newPluginClass.isAnnotationPresent(PluginType.class)) {
      pluginType = newPluginClass.getAnnotation(PluginType.class).value();
    } else {
      pluginType = PluginType.UNDEFINED_PLUGIN;
    }

    // Check that plugin type is valid and constructor exists
    try {
      if (pluginType == PluginType.MOTE_PLUGIN) {
        newPluginClass.getConstructor(new Class[] { Mote.class,
            Simulation.class, GUI.class });
      } else if (pluginType == PluginType.SIM_PLUGIN
          || pluginType == PluginType.SIM_STANDARD_PLUGIN) {
        newPluginClass
            .getConstructor(new Class[] { Simulation.class, GUI.class });
      } else if (pluginType == PluginType.COOJA_PLUGIN
          || pluginType == PluginType.COOJA_STANDARD_PLUGIN) {
        newPluginClass.getConstructor(new Class[] { GUI.class });
      } else {
        logger.fatal("Could not find valid plugin type annotation in class "
            + newPluginClass);
        return false;
      }
    } catch (NoSuchMethodException e) {
      logger.fatal("Could not find valid constructor in class "
          + newPluginClass + ": " + e);
      return false;
    }

    if (addToMenu && menuPlugins != null) {
      // Create 'start plugin'-menu item
      JMenuItem menuItem = new JMenuItem(description);
      menuItem.setActionCommand("start plugin");
      menuItem.putClientProperty("class", newPluginClass);
      menuItem.addActionListener(guiEventHandler);

      menuPlugins.add(menuItem);

      if (pluginType == PluginType.MOTE_PLUGIN) {
        // Disable previous menu item and add new item to mote plugins menu
        menuItem.setEnabled(false);
        menuItem.setToolTipText("Mote plugin");
        menuMotePluginClasses.add(newPluginClass);
      }
    }

    pluginClasses.add(newPluginClass);
    return true;
  }

  /**
   * Unregister all plugin classes, including temporary plugins.
   */
  public void unregisterPlugins() {
    if (menuPlugins != null) {
      menuPlugins.removeAll();
      menuMotePluginClasses.clear();
    }
    pluginClasses.clear();
    pluginClassesTemporary.clear();
  }

  /**
   * Return a mote plugins submenu for given mote.
   * 
   * @param mote Mote
   * @return Mote plugins menu
   */
  public JMenu createMotePluginsSubmenu(Mote mote) {
    JMenu menuMotePlugins = new JMenu("Open mote plugin for " + mote);

    for (Class<? extends Plugin> motePluginClass: menuMotePluginClasses) {
      JMenuItem menuItem = new JMenuItem(getDescriptionOf(motePluginClass));
      menuItem.setActionCommand("start plugin");
      menuItem.putClientProperty("class", motePluginClass);
      menuItem.putClientProperty("mote", mote);
      menuItem.addActionListener(guiEventHandler);
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

  public void setSimulation(Simulation sim) {
    if (sim != null) {
      doRemoveSimulation(false);
    }
    mySimulation = sim;

    // Set frame title
    if (frame != null)
      frame.setTitle("COOJA Simulator" + " - " + sim.getTitle());

    // Open standard plugins (if none opened already)
    if (startedPlugins.size() == 0)
      for (Class<? extends Plugin> pluginClass : pluginClasses) {
        int pluginType = pluginClass.getAnnotation(PluginType.class).value();
        if (pluginType == PluginType.SIM_STANDARD_PLUGIN) {
          startPlugin(pluginClass, this, sim, null);
        }
      }
  }

  /**
   * Creates a new mote type of the given mote type class.
   * 
   * @param moteTypeClass
   *          Mote type class
   */
  public void doCreateMoteType(Class<? extends MoteType> moteTypeClass) {
    if (mySimulation == null) {
      logger.fatal("Can't create mote type (no simulation)");
      return;
    }

    // Stop simulation (if running)
    mySimulation.stopSimulation();

    // Create mote type
    MoteType newMoteType = null;
    boolean moteTypeOK = false;
    try {
      newMoteType = moteTypeClass.newInstance();
      moteTypeOK = newMoteType.configureAndInit(frame, mySimulation,
          isVisualized());
    } catch (InstantiationException e) {
      logger.fatal("Exception when creating mote type: " + e);
      return;
    } catch (IllegalAccessException e) {
      logger.fatal("Exception when creating mote type: " + e);
      return;
    } catch (MoteTypeCreationException e) {
      logger.fatal("Exception when creating mote type: " + e);
      return;
    }

    // Add mote type to simulation
    if (newMoteType != null && moteTypeOK) {
      mySimulation.addMoteType(newMoteType);
    }
  }

  /**
   * Remove current simulation
   * 
   * @param askForConfirmation
   *          Should we ask for confirmation if a simulation is already active?
   */
  public void doRemoveSimulation(boolean askForConfirmation) {

    if (mySimulation != null) {
      if (askForConfirmation) {
        String s1 = "Remove";
        String s2 = "Cancel";
        Object[] options = { s1, s2 };
        int n = JOptionPane.showOptionDialog(frame,
            "You have an active simulation.\nDo you want to remove it?",
            "Remove current simulation?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, s1);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
      }

      // Close all started non-GUI plugins
      for (Object startedPlugin : startedPlugins.toArray()) {
        int pluginType = startedPlugin.getClass().getAnnotation(
            PluginType.class).value();
        if (pluginType != PluginType.COOJA_PLUGIN
            && pluginType != PluginType.COOJA_STANDARD_PLUGIN)
          removePlugin((Plugin) startedPlugin, false);
      }

      // Delete simulation
      mySimulation.deleteObservers();
      mySimulation.stopSimulation();
      mySimulation = null;

      // Unregister temporary plugin classes
      Enumeration<Class<? extends Plugin>> pluginClasses = pluginClassesTemporary
          .elements();
      while (pluginClasses.hasMoreElements()) {
        unregisterPlugin(pluginClasses.nextElement());
      }

      // Reset frame title
      frame.setTitle("COOJA Simulator");
    }
  }

  /**
   * Load a simulation configuration file from disk
   * 
   * @param askForConfirmation Ask for confirmation before removing any current simulation
   * @param quick Quick-load simulation
   * @param configFile Configuration file to load, if null a dialog will appear
   */
  public void doLoadConfig(boolean askForConfirmation, final boolean quick, File configFile) {

    if (CoreComm.hasLibraryBeenLoaded()) {
      JOptionPane
      .showMessageDialog(
          frame,
          "Shared libraries has already been loaded.\nYou need to restart the simulator!",
          "Can't load simulation", JOptionPane.ERROR_MESSAGE);
      return;
    }

    if (askForConfirmation && mySimulation != null) {
      String s1 = "Remove";
      String s2 = "Cancel";
      Object[] options = { s1, s2 };
      int n = JOptionPane.showOptionDialog(frame,
          "You have an active simulation.\nDo you want to remove it?",
          "Remove current simulation?", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, options, s1);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }
    }

    doRemoveSimulation(false);

    // Check already selected file, or select file using filechooser
    if (configFile != null) {
      if (!configFile.exists() || !configFile.canRead()) {
        logger.fatal("No read access to file");
        return;
      }
    } else {
      JFileChooser fc = new JFileChooser();

      fc.setFileFilter(GUI.SAVED_SIMULATIONS_FILES);

      // Suggest file using history
      Vector<File> history = getFileHistory();
      if (history != null && history.size() > 0) {
        File suggestedFile = getFileHistory().firstElement();
        fc.setSelectedFile(suggestedFile);
      }

      int returnVal = fc.showOpenDialog(frame);
      if (returnVal == JFileChooser.APPROVE_OPTION) {
        configFile = fc.getSelectedFile();

        // Try adding extension if not founds
        if (!configFile.exists()) {
          configFile = new File(configFile.getParent(), configFile.getName()
              + SAVED_SIMULATIONS_FILES);
        }

        if (!configFile.exists() || !configFile.canRead()) {
          logger.fatal("No read access to file");
          return;
        }

      } else {
        logger.info("Load command cancelled by user...");
        return;
      }
    }

    // Load simulation in separate thread, while showing progress monitor
    final JDialog progressDialog = new JDialog(frame, "Loading", true);
    final File fileToLoad = configFile;
    final Thread loadThread = new Thread(new Runnable() {
      public void run() {
        Simulation newSim = null;
        try {
          newSim = loadSimulationConfig(fileToLoad, quick);
          addToFileHistory(fileToLoad);
          if (progressDialog != null && progressDialog.isDisplayable()) {
            progressDialog.dispose();
          }
        } catch (UnsatisfiedLinkError e) {
          showErrorDialog(frame, "Simulation load error", e, false);

          if (progressDialog != null && progressDialog.isDisplayable()) 
            progressDialog.dispose();
          newSim = null;
        } catch (SimulationCreationException e) {
          showErrorDialog(frame, "Simulation load error", e, false);

          if (progressDialog != null && progressDialog.isDisplayable()) 
            progressDialog.dispose();
          newSim = null;
        }
        if (newSim != null) {
          myGUI.setSimulation(newSim);
        }
      }
    });

    JPanel progressPanel = new JPanel(new BorderLayout());
    JProgressBar progressBar;
    JButton button;

    progressBar = new JProgressBar(0, 100);
    progressBar.setValue(0);
    progressBar.setIndeterminate(true);
    button = new JButton("Cancel");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (loadThread != null && loadThread.isAlive()) {
          loadThread.interrupt();
          doRemoveSimulation(false);
        }
        if (progressDialog != null && progressDialog.isDisplayable()) {
          progressDialog.dispose();
        }
      }
    });

    progressPanel.add(BorderLayout.CENTER, progressBar);
    progressPanel.add(BorderLayout.SOUTH, button);
    progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));

    progressPanel.setVisible(true);

    progressDialog.getContentPane().add(progressPanel);
    progressDialog.pack();

    progressDialog.getRootPane().setDefaultButton(button);
    progressDialog.setLocationRelativeTo(frame);
    progressDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
    loadThread.start();
    if (quick)
      progressDialog.setVisible(true);
  }
  
  /**
   * Reloads current simulation.
   * This may include recompiling libraries and renaming mote type identifiers.
   */
  private void reloadCurrentSimulation() {
    if (getSimulation() == null) {
      logger.fatal("No simulation to reload");
      return;
    }
    
    final JDialog progressDialog = new JDialog(frame, "Reloading", true);
    final Thread loadThread = new Thread(new Runnable() {
      public void run() {

        // Create mappings to new mote type identifiers
        Properties moteTypeNames = new Properties();
        for (MoteType moteType: getSimulation().getMoteTypes()) {
          if (moteType.getClass().equals(ContikiMoteType.class)) {
            // Suggest new identifier
            int counter = 0;
            String testIdentifier = "";
            boolean identifierOK = false;
            while (!identifierOK) {
              counter++;
              testIdentifier = ContikiMoteTypeDialog.ID_PREFIX + counter;
              identifierOK = true;

              // Check if identifier already reserved for some other type
              if (moteTypeNames.containsValue(testIdentifier)) {
                identifierOK = false;
              }

              // Check if identifier is already used by some other type
              for (MoteType existingMoteType : getSimulation().getMoteTypes()) {
                if (existingMoteType != moteType
                    && existingMoteType.getIdentifier().equals(testIdentifier)) {
                  identifierOK = false;
                  break;
                }
              }

              // Check if library file with given identifier has already been loaded
              if (identifierOK
                  && CoreComm.hasLibraryFileBeenLoaded(new File(
                      ContikiMoteType.tempOutputDirectory,
                      testIdentifier
                      + ContikiMoteType.librarySuffix))) {
                identifierOK = false;
              }
            }

            moteTypeNames.setProperty(moteType.getIdentifier(), testIdentifier);
          }
        }

        // Get current simulation configuration
        Element simulationElement = new Element("simulation");
        simulationElement.addContent(getSimulation().getConfigXML());
        Collection<Element> pluginsConfig = getPluginsConfigXML();

        // Scan and replace old configuration mote type names
        Element root = new Element("simconf");
        root.addContent(simulationElement);

        if (pluginsConfig != null)
          root.addContent(pluginsConfig);

        Document doc = new Document(root);
        XMLOutputter outputter = new XMLOutputter();
        outputter.setFormat(Format.getPrettyFormat());
        String configXML = outputter.outputString(doc);

        Enumeration oldNames = moteTypeNames.keys();
        while (oldNames.hasMoreElements()) {
          String oldName = (String) oldNames.nextElement();
          configXML = configXML.replaceAll(">" + oldName + "<", ">" + moteTypeNames.get(oldName) + "<");
        }

        // Reload altered simulation config
        boolean shouldRetry = false;
        do {
          try {
            shouldRetry = false;
            myGUI.doRemoveSimulation(false);
            Simulation newSim = loadSimulationConfig(new StringReader(configXML), true);
            myGUI.setSimulation(newSim);
          } catch (UnsatisfiedLinkError e) {
            shouldRetry = showErrorDialog(frame, "Simulation reload error", e, true);

            myGUI.doRemoveSimulation(false);
          } catch (SimulationCreationException e) {
            shouldRetry = showErrorDialog(frame, "Simulation reload error", e, true);

            myGUI.doRemoveSimulation(false);
          }
        } while (shouldRetry);

        if (progressDialog != null && progressDialog.isDisplayable()) { 
          progressDialog.dispose();
        }
      }
    });

    // Display progress dialog while reloading
    JProgressBar progressBar = new JProgressBar(0, 100);
    progressBar.setValue(0);
    progressBar.setIndeterminate(true);

    JButton button = new JButton("Cancel");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (loadThread != null && loadThread.isAlive()) {
          loadThread.interrupt();
          doRemoveSimulation(false);
        }
        if (progressDialog != null && progressDialog.isDisplayable()) { 
          progressDialog.dispose();
        }
      }
    });

    JPanel progressPanel = new JPanel(new BorderLayout());
    progressPanel.add(BorderLayout.CENTER, progressBar);
    progressPanel.add(BorderLayout.SOUTH, button);
    progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));

    progressPanel.setVisible(true);

    progressDialog.getContentPane().add(progressPanel);
    progressDialog.pack();

    progressDialog.getRootPane().setDefaultButton(button);
    progressDialog.setLocationRelativeTo(frame);
    progressDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);

    loadThread.start();
    progressDialog.setVisible(true);
  }

  /**
   * Save current simulation configuration to disk
   * 
   * @param askForConfirmation
   *          Ask for confirmation before overwriting file
   */
  public void doSaveConfig(boolean askForConfirmation) {
    if (mySimulation != null) {
      mySimulation.stopSimulation();

      JFileChooser fc = new JFileChooser();

      fc.setFileFilter(GUI.SAVED_SIMULATIONS_FILES);

      // Suggest file using history
      Vector<File> history = getFileHistory();
      if (history != null && history.size() > 0) {
        File suggestedFile = getFileHistory().firstElement();
        fc.setSelectedFile(suggestedFile);
      }

      int returnVal = fc.showSaveDialog(myDesktopPane);
      if (returnVal == JFileChooser.APPROVE_OPTION) {
        File saveFile = fc.getSelectedFile();
        if (!fc.accept(saveFile)) {
          saveFile = new File(saveFile.getParent(), saveFile.getName()
              + SAVED_SIMULATIONS_FILES);
        }

        if (saveFile.exists()) {
          if (askForConfirmation) {
            String s1 = "Overwrite";
            String s2 = "Cancel";
            Object[] options = { s1, s2 };
            int n = JOptionPane
                .showOptionDialog(
                    frame,
                    "A file with the same name already exists.\nDo you want to remove it?",
                    "Overwrite existing file?", JOptionPane.YES_NO_OPTION,
                    JOptionPane.QUESTION_MESSAGE, null, options, s1);
            if (n != JOptionPane.YES_OPTION) {
              return;
            }
          }
        }

        if (!saveFile.exists() || saveFile.canWrite()) {
          saveSimulationConfig(saveFile);
          addToFileHistory(saveFile);
        } else
          logger.fatal("No write access to file");

      } else {
        logger.info("Save command cancelled by user...");
      }
    }
  }

  /**
   * Add new mote to current simulation
   */
  public void doAddMotes(MoteType moteType) {
    if (mySimulation != null) {
      mySimulation.stopSimulation();

      Vector<Mote> newMotes = AddMoteDialog.showDialog(frame, mySimulation,
          moteType);
      if (newMotes != null) {
        for (Mote newMote : newMotes)
          mySimulation.addMote(newMote);
      }

    } else
      logger.warn("No simulation active");
  }

  /**
   * Create a new simulation
   * 
   * @param askForConfirmation
   *          Should we ask for confirmation if a simulation is already active?
   */
  public void doCreateSimulation(boolean askForConfirmation) {
    if (askForConfirmation && mySimulation != null) {
      String s1 = "Remove";
      String s2 = "Cancel";
      Object[] options = { s1, s2 };
      int n = JOptionPane.showOptionDialog(frame,
          "You have an active simulation.\nDo you want to remove it?",
          "Remove current simulation?", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, options, s1);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }
    }

    // Create new simulation
    doRemoveSimulation(false);
    Simulation newSim = new Simulation(this);
    boolean createdOK = CreateSimDialog.showDialog(frame, newSim);
    if (createdOK) {
      myGUI.setSimulation(newSim);
    }
  }

  /**
   * Quit program
   * 
   * @param askForConfirmation
   *          Should we ask for confirmation before quitting?
   */
  public void doQuit(boolean askForConfirmation) {
    if (askForConfirmation) {
      String s1 = "Quit";
      String s2 = "Cancel";
      Object[] options = { s1, s2 };
      int n = JOptionPane.showOptionDialog(frame, "Sure you want to quit?",
          "Close COOJA Simulator", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, options, s1);
      if (n != JOptionPane.YES_OPTION)
        return;
    }

    // Clean up resources
    Object[] plugins = startedPlugins.toArray();
    for (Object plugin : plugins)
      removePlugin((Plugin) plugin, false);

    // Restore last frame size and position
    if (frame != null) {
      setExternalToolsSetting("FRAME_POS_X", "" + frame.getLocationOnScreen().x);
      setExternalToolsSetting("FRAME_POS_Y", "" + frame.getLocationOnScreen().y);
      setExternalToolsSetting("FRAME_WIDTH", "" + frame.getWidth());
      setExternalToolsSetting("FRAME_HEIGHT", "" + frame.getHeight());
      saveExternalToolsUserSettings();
    }
    
    System.exit(0);
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
    return currentExternalToolsSettings.getProperty(name);
  }

  /**
   * @param name
   *          Name of setting
   * @param defaultValue
   *          Default value
   * @return Value
   */
  public static String getExternalToolsSetting(String name, String defaultValue) {
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
    String filename = GUI.EXTERNAL_TOOLS_LINUX_SETTINGS_FILENAME;
    if (System.getProperty("os.name").startsWith("Win"))
      filename = GUI.EXTERNAL_TOOLS_WIN32_SETTINGS_FILENAME;

    try {
      InputStream in = GUI.class.getResourceAsStream(filename);
      if (in == null) {
        throw new FileNotFoundException(filename + " not found");
      }
      Properties settings = new Properties();
      settings.load(in);
      in.close();

      currentExternalToolsSettings = settings;
      defaultExternalToolsSettings = (Properties) currentExternalToolsSettings.clone();
    } catch (IOException e) {
      // Error while importing default properties
      logger.warn(
          "Error when reading external tools settings from " + filename, e);
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
  private static void loadExternalToolsUserSettings() {
    try {
      FileInputStream in = new FileInputStream(externalToolsUserSettingsFile);
      Properties settings = new Properties();
      settings.load(in);
      in.close();

      Enumeration en = settings.keys();
      while (en.hasMoreElements()) {
        String key = (String) en.nextElement();
        setExternalToolsSetting(key, settings.getProperty(key));
      }

    } catch (FileNotFoundException e) {
      // No default configuration file found, using default
    } catch (IOException e) {
      // Error while importing saved properties, using default
      logger.warn("Error when reading default settings from " + externalToolsUserSettingsFile);
    }
  }

  /**
   * Save external tools user settings to file.
   */
  public static void saveExternalToolsUserSettings() {
    try {
      FileOutputStream out = new FileOutputStream(externalToolsUserSettingsFile);
      
      Properties differingSettings = new Properties();
      Enumeration keyEnum = currentExternalToolsSettings.keys();
      while (keyEnum.hasMoreElements()) {
        String key = (String) keyEnum.nextElement();
        String defaultSetting = getExternalToolsDefaultSetting(key, "");
        String currentSetting = getExternalToolsSetting(key, "");
        if (!defaultSetting.equals(currentSetting)) {
          differingSettings.setProperty(key, currentSetting);
        }
      }
      
      differingSettings.store(out, "COOJA External Tools (User specific)");
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

  private class GUIEventHandler implements ActionListener, WindowListener {
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
      myGUI.doQuit(true);
    }

    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("new sim")) {
        myGUI.doCreateSimulation(true);
      } else if (e.getActionCommand().equals("close sim")) {
        myGUI.doRemoveSimulation(true);
      } else if (e.getActionCommand().equals("confopen sim")) {
        myGUI.doLoadConfig(true, false, null);
      } else if (e.getActionCommand().equals("confopen last sim")) {
        File file = (File) ((JMenuItem) e.getSource()).getClientProperty("file");
        myGUI.doLoadConfig(true, false, file);
      } else if (e.getActionCommand().equals("open sim")) {
        myGUI.doLoadConfig(true, true, null);
      } else if (e.getActionCommand().equals("open last sim")) {
        File file = (File) ((JMenuItem) e.getSource()).getClientProperty("file");
        myGUI.doLoadConfig(true, true, file);
      } else if (e.getActionCommand().equals("save sim")) {
        myGUI.doSaveConfig(true);
      } else if (e.getActionCommand().equals("quit")) {
        myGUI.doQuit(true);
      } else if (e.getActionCommand().equals("create mote type")) {
        myGUI.doCreateMoteType((Class<? extends MoteType>) ((JMenuItem) e
            .getSource()).getClientProperty("class"));
      } else if (e.getActionCommand().equals("add motes")) {
        myGUI.doAddMotes((MoteType) ((JMenuItem) e.getSource())
            .getClientProperty("motetype"));
      } else if (e.getActionCommand().equals("edit paths")) {
        ExternalToolsDialog.showDialog(frame);
      } else if (e.getActionCommand().equals("close plugins")) {
        Object[] plugins = startedPlugins.toArray();
        for (Object plugin : plugins)
          removePlugin((Plugin) plugin, false);
      } else if (e.getActionCommand().equals("remove all motes")) {
        if (getSimulation() != null) {
          if (getSimulation().isRunning())
            getSimulation().stopSimulation();
          
          while (getSimulation().getMotesCount() > 0) {
            getSimulation().removeMote(getSimulation().getMote(0));
          }
        }
      } else if (e.getActionCommand().equals("manage projects")) {
        Vector<File> newProjects = ProjectDirectoriesDialog.showDialog(frame,
            currentProjectDirs, null);
        if (newProjects != null) {
          currentProjectDirs = newProjects;
          reparseProjectConfig();
        }
      } else if (e.getActionCommand().equals("start plugin")) {
        Class<? extends VisPlugin> pluginClass = (Class<? extends VisPlugin>) ((JMenuItem) e
            .getSource()).getClientProperty("class");
        Mote mote = (Mote) ((JMenuItem) e.getSource()).getClientProperty("mote");
        startPlugin(pluginClass, myGUI, mySimulation, mote);
      } else
        logger.warn("Unhandled action: " + e.getActionCommand());
    }
  }

  // // VARIOUS HELP METHODS ////

  /**
   * Help method that tries to load and initialize a class with given name.
   * 
   * @param <N>
   *          Class extending given class type
   * @param classType
   *          Class type
   * @param className
   *          Class name
   * @return Class extending given class type or null if not found
   */
  public <N extends Object> Class<? extends N> tryLoadClass(
      Object callingObject, Class<N> classType, String className) {

    if (callingObject != null) {
      try {
        return callingObject.getClass().getClassLoader().loadClass(className)
            .asSubclass(classType);
      } catch (ClassNotFoundException e) {
      }
    }

    try {
      return Class.forName(className).asSubclass(classType);
    } catch (ClassNotFoundException e) {
    }

    try {
      if (projectDirClassLoader != null) {
        return projectDirClassLoader.loadClass(className).asSubclass(
            classType);
      }
    } catch (ClassNotFoundException e) {
    }

    return null;
  }

  public ClassLoader createProjectDirClassLoader(Vector<File> projectsDirs) {
    if (projectDirClassLoader == null) {
      reparseProjectConfig();
    }
    return createClassLoader(projectDirClassLoader, projectsDirs);
  }

  private ClassLoader createClassLoader(Vector<File> currentProjectDirs) {
    return createClassLoader(ClassLoader.getSystemClassLoader(),
        currentProjectDirs);
  }

  private File findJarFile(File projectDir, String jarfile) {
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

  private ClassLoader createClassLoader(ClassLoader parent,
      Vector<File> projectDirs) {
    if (projectDirs == null || projectDirs.isEmpty()) {
      return parent;
    }

    // Combine class loader from all project directories (including any
    // specified JAR files)
    ArrayList<URL> urls = new ArrayList<URL>();
    for (int j = projectDirs.size() - 1; j >= 0; j--) {
      File projectDir = projectDirs.get(j);
      try {
        urls.add((new File(projectDir, "java")).toURL());

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
            urls.add(jarpath.toURL());
          }
        }

      } catch (Exception e) {
        logger.fatal("Error when trying to read JAR-file in " + projectDir
            + ": " + e);
      }
    }

    return new URLClassLoader((URL[]) urls.toArray(new URL[urls.size()]),
        parent);
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
   * Load configurations and create a GUI.
   * 
   * @param args
   *          null
   */
  public static void main(String[] args) {

    // Configure logger
    if ((new File(LOG_CONFIG_FILE)).exists()) {
      DOMConfigurator.configure(LOG_CONFIG_FILE);
    } else {
      // Used when starting from jar
      DOMConfigurator.configure(GUI.class.getResource("/" + LOG_CONFIG_FILE));
    }

    // Check for Contiki path argument
    for (int i = 1; i < args.length; i++) {
      if (args[i].startsWith("-contiki=")) {
        String arg = args[i].substring("-contiki=".length());
        GUI.specifiedContikiPath = arg;
      }
    }
    
    // Check if simulator should be quick-started
    if (args.length > 0 && args[0].startsWith("-quickstart=")) {
      String filename = args[0].substring("-quickstart=".length());

      String moteTypeID = "mtype1";
      Vector<String> projectDirs = null;
      Vector<String> sensors = null;
      Vector<String> coreInterfaces = null;
      Vector<String> userProcesses = null;
      boolean addAutostartProcesses = true;
      int numberOfNodes = 100;
      double areaSideLength = 100;
      int delayTime = 5;
      boolean startSimulation = true;
      String contikiPath = null;

      // Parse arguments
      for (int i = 1; i < args.length; i++) {

        if (args[i].startsWith("-id=")) {
          moteTypeID = args[i].substring("-id=".length());

        } else if (args[i].startsWith("-projects=")) {
          String arg = args[i].substring("-projects=".length());
          String[] argArray = arg.split(",");
          projectDirs = new Vector<String>();
          for (String argValue : argArray)
            projectDirs.add(argValue);

        } else if (args[i].startsWith("-sensors=")) {
          String arg = args[i].substring("-sensors=".length());
          String[] argArray = arg.split(",");
          sensors = new Vector<String>();
          for (String argValue : argArray)
            sensors.add(argValue);

        } else if (args[i].startsWith("-interfaces=")) {
          String arg = args[i].substring("-interfaces=".length());
          String[] argArray = arg.split(",");
          coreInterfaces = new Vector<String>();
          for (String argValue : argArray)
            coreInterfaces.add(argValue);

        } else if (args[i].startsWith("-processes=")) {
          String arg = args[i].substring("-processes=".length());
          String[] argArray = arg.split(",");
          userProcesses = new Vector<String>();
          for (String argValue : argArray)
            userProcesses.add(argValue);

        } else if (args[i].equals("-noautostartscan")) {
          addAutostartProcesses = false;

        } else if (args[i].equals("-paused")) {
          startSimulation = false;

        } else if (args[i].startsWith("-nodes=")) {
          String arg = args[i].substring("-nodes=".length());
          numberOfNodes = Integer.parseInt(arg);

        } else if (args[i].startsWith("-contiki=")) {
          String arg = args[i].substring("-contiki=".length());
          contikiPath = arg;

        } else if (args[i].startsWith("-delay=")) {
          String arg = args[i].substring("-delay=".length());
          delayTime = Integer.parseInt(arg);

        } else if (args[i].startsWith("-side=")) {
          String arg = args[i].substring("-side=".length());
          areaSideLength = Double.parseDouble(arg);

        } else {
          logger.fatal("Unknown argument, aborting: " + args[i]);
          System.exit(1);
        }
      }

      boolean ok = quickStartSimulation(moteTypeID, projectDirs, sensors,
          coreInterfaces, userProcesses, addAutostartProcesses, numberOfNodes,
          areaSideLength, delayTime, startSimulation, filename, contikiPath);
      if (!ok)
        System.exit(1);

    } else if (args.length > 0 && args[0].startsWith("-nogui")) {

      // Parse arguments
      for (int i = 1; i < args.length; i++) {

        if (args[i].startsWith("-external_tools_config=")) {
          String arg = args[i].substring("-external_tools_config=".length());
          File specifiedExternalToolsConfigFile = new File(arg);
          if (!specifiedExternalToolsConfigFile.exists()) {
            logger.fatal("Specified external tools configuration not found: " + specifiedExternalToolsConfigFile);
            specifiedExternalToolsConfigFile = null;
            System.exit(1);
          } else {
            GUI.externalToolsUserSettingsFile = specifiedExternalToolsConfigFile;
          }
        }

      }
      
      // No GUI start-up
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          new GUI(null, false);
        }
      });

    } else {

      // Regular start-up
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          createAndShowGUI();
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
    try {
      SAXBuilder builder = new SAXBuilder();
      Document doc = builder.build(file);
      Element root = doc.getRootElement();

      return loadSimulationConfig(root, quick);
    } catch (JDOMException e) {
      logger.fatal("Config not wellformed: " + e.getMessage());
      return null;
    } catch (IOException e) {
      logger.fatal("IOException: " + e.getMessage());
      return null;
    }
  }

  private Simulation loadSimulationConfig(StringReader stringReader, boolean quick) 
  throws SimulationCreationException {
    try {
      SAXBuilder builder = new SAXBuilder();
      Document doc = builder.build(stringReader);
      Element root = doc.getRootElement();

      return loadSimulationConfig(root, quick);
    } catch (JDOMException e) {
      throw (SimulationCreationException) new SimulationCreationException(
          "Configuration file not wellformed: " + e.getMessage()).initCause(e);
    } catch (IOException e) {
      throw (SimulationCreationException) new SimulationCreationException(
          "IO Exception: " + e.getMessage()).initCause(e);
    }
  }

  private Simulation loadSimulationConfig(Element root, boolean quick)
  throws SimulationCreationException {
    Simulation newSim = null;

    try {
      // Check that config file version is correct
      if (!root.getName().equals("simconf")) {
        logger.fatal("Not a valid COOJA simulation config!");
        return null;
      }

      // Create new simulation from config
      for (Object element : root.getChildren()) {
        if (((Element) element).getName().equals("simulation")) {
          Collection<Element> config = ((Element) element).getChildren();
          newSim = new Simulation(this);
          boolean createdOK = newSim.setConfigXML(config, !quick);
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

    try {
      // Create simulation configL
      Element root = new Element("simconf");
      Element simulationElement = new Element("simulation");
      simulationElement.addContent(mySimulation.getConfigXML());
      root.addContent(simulationElement);

      // Create started plugins config
      Collection<Element> pluginsConfig = getPluginsConfigXML();
      if (pluginsConfig != null)
        root.addContent(pluginsConfig);

      // Create and write to document
      Document doc = new Document(root);
      FileOutputStream out = new FileOutputStream(file);
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

  /**
   * Returns started plugins config.
   * 
   * @return Config or null
   */
  public Collection<Element> getPluginsConfigXML() {
    Vector<Element> config = new Vector<Element>();

    // Loop through all started plugins
    // (Only return config of non-GUI plugins)
    Element pluginElement, pluginSubElement;
    for (Plugin startedPlugin : startedPlugins) {
      int pluginType = startedPlugin.getClass().getAnnotation(PluginType.class)
          .value();

      // Ignore GUI plugins
      if (pluginType == PluginType.COOJA_PLUGIN
          || pluginType == PluginType.COOJA_STANDARD_PLUGIN)
        continue;

      pluginElement = new Element("plugin");
      pluginElement.setText(startedPlugin.getClass().getName());

      // Create mote argument config (if mote plugin)
      if (pluginType == PluginType.MOTE_PLUGIN
          && startedPlugin.getTag() != null) {
        pluginSubElement = new Element("mote_arg");
        Mote taggedMote = (Mote) startedPlugin.getTag();
        for (int moteNr = 0; moteNr < mySimulation.getMotesCount(); moteNr++) {
          if (mySimulation.getMote(moteNr) == taggedMote) {
            pluginSubElement.setText(Integer.toString(moteNr));
            pluginElement.addContent(pluginSubElement);
            break;
          }
        }
      }

      // Create plugin specific configuration
      Collection pluginXML = startedPlugin.getConfigXML();
      if (pluginXML != null) {
        pluginSubElement = new Element("plugin_config");
        pluginSubElement.addContent(pluginXML);
        pluginElement.addContent(pluginSubElement);
      }

      // If plugin is visualizer plugin, create visualization arguments
      if (startedPlugin instanceof VisPlugin) {
        VisPlugin startedVisPlugin = (VisPlugin) startedPlugin;

        pluginSubElement = new Element("width");
        pluginSubElement.setText("" + startedVisPlugin.getSize().width);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("z");
        pluginSubElement.setText(""
            + getDesktopPane().getComponentZOrder(startedVisPlugin));
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("height");
        pluginSubElement.setText("" + startedVisPlugin.getSize().height);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("location_x");
        pluginSubElement.setText("" + startedVisPlugin.getLocation().x);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("location_y");
        pluginSubElement.setText("" + startedVisPlugin.getLocation().y);
        pluginElement.addContent(pluginSubElement);

        pluginSubElement = new Element("minimized");
        pluginSubElement.setText(new Boolean(startedVisPlugin.isIcon())
            .toString());
        pluginElement.addContent(pluginSubElement);
      }

      config.add(pluginElement);
    }

    return config;
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

    for (Element pluginElement : configXML.toArray(new Element[0])) {
      if (pluginElement.getName().equals("plugin")) {

        // Read plugin class
        String pluginClassName = pluginElement.getText().trim();
        Class<? extends Plugin> visPluginClass = tryLoadClass(this,
            Plugin.class, pluginClassName);
        if (visPluginClass == null) {
          logger.fatal("Could not load plugin class: " + pluginClassName);
          return false;
        }

        // Parse plugin mote argument (if any)
        Mote mote = null;
        for (Element pluginSubElement : (List<Element>) pluginElement
            .getChildren()) {
          if (pluginSubElement.getName().equals("mote_arg")) {
            int moteNr = Integer.parseInt(pluginSubElement.getText());
            if (moteNr >= 0 && moteNr < simulation.getMotesCount()) {
              mote = simulation.getMote(moteNr);
            }
          }
        }

        // Start plugin (before applying rest of config)
        Plugin startedPlugin = startPlugin(visPluginClass, this, simulation,
            mote);

        // Apply plugin specific configuration
        for (Element pluginSubElement : (List<Element>) pluginElement
            .getChildren()) {
          if (pluginSubElement.getName().equals("plugin_config")) {
            startedPlugin.setConfigXML(pluginSubElement.getChildren(),
                visAvailable);
          }
        }

        // If plugin is visualizer plugin, parse visualization arguments
        if (startedPlugin instanceof VisPlugin) {
          Dimension size = new Dimension(100, 100);
          Point location = new Point(100, 100);
          VisPlugin startedVisPlugin = (VisPlugin) startedPlugin;

          for (Element pluginSubElement : (List<Element>) pluginElement
              .getChildren()) {

            if (pluginSubElement.getName().equals("width") && size != null) {
              size.width = Integer.parseInt(pluginSubElement.getText());
              startedVisPlugin.setSize(size);
            } else if (pluginSubElement.getName().equals("height")) {
              size.height = Integer.parseInt(pluginSubElement.getText());
              startedVisPlugin.setSize(size);
            } else if (pluginSubElement.getName().equals("z")) {
              int zOrder = Integer.parseInt(pluginSubElement.getText());
              // Save z order as temporary client property
              startedVisPlugin.putClientProperty("zorder", zOrder);
            } else if (pluginSubElement.getName().equals("location_x")) {
              location.x = Integer.parseInt(pluginSubElement.getText());
              startedVisPlugin.setLocation(location);
            } else if (pluginSubElement.getName().equals("location_y")) {
              location.y = Integer.parseInt(pluginSubElement.getText());
              startedVisPlugin.setLocation(location);
            } else if (pluginSubElement.getName().equals("minimized")) {
              try {
                startedVisPlugin.setIcon(Boolean.parseBoolean(pluginSubElement
                    .getText()));
              } catch (PropertyVetoException e) {
                // Ignoring
              }
            } else if (pluginSubElement.getName().equals("plugin_config")) {
              startedVisPlugin.setConfigXML(pluginSubElement.getChildren(),
                  visAvailable);
            }
          }
        }

      }
    }

    // For all started visplugins, check if they have a zorder property
    try {
      for (JInternalFrame plugin : getDesktopPane().getAllFrames()) {
        if (plugin.getClientProperty("zorder") != null) {
          getDesktopPane().setComponentZOrder(plugin,
              ((Integer) plugin.getClientProperty("zorder")).intValue());
          plugin.putClientProperty("zorder", null);
        }
      }
    } catch (Exception e) {
      // Ignore errors
    }

    return true;
  }
  
  class SimulationCreationException extends Exception {
    public SimulationCreationException(String message) {
      super(message);
    }
  }

  /**
   * Shows a simple dialog with information about the thrown exception. A user
   * may watch the stack trace and, if the exception is a
   * MoteTypeCreationException, watch compilation output.
   * 
   * @param parentComponent
   *          Parent component
   * @param title
   *          Title of error window
   * @param exception
   *          Exception causing window to be shown
   * @param retryAvailable
   *          If true, a retry option is available
   */
  public static boolean showErrorDialog(Component parentComponent,
      final String title, Throwable exception, boolean retryAvailable) {

    MessageList compilationOutput = null;
    MessageList stackTrace = null;
    String message = title;

    // Create message
    if (exception != null)
      message = exception.getMessage();

    // Create stack trace message list
    if (exception != null) {
      stackTrace = new MessageList();
      PrintStream printStream = stackTrace.getInputStream(MessageList.NORMAL);
      exception.printStackTrace(printStream);
    }

    // Create compilation out message list (if available)
    if (exception != null && exception instanceof MoteTypeCreationException
        && ((MoteTypeCreationException) exception).hasCompilationOutput()) {
      compilationOutput = ((MoteTypeCreationException) exception)
          .getCompilationOutput();
    } else if (exception != null
        && exception.getCause() != null
        && exception.getCause() instanceof MoteTypeCreationException
        && ((MoteTypeCreationException) exception.getCause())
            .hasCompilationOutput()) {
      compilationOutput = ((MoteTypeCreationException) exception.getCause())
          .getCompilationOutput();
    }

    // Create error dialog
    final JDialog errorDialog;
    if (parentComponent instanceof Dialog)
      errorDialog = new JDialog((Dialog) parentComponent, title, true);
    else if (parentComponent instanceof Frame)
      errorDialog = new JDialog((Frame) parentComponent, title, true);
    else if (parentComponent instanceof Window)
      errorDialog = new JDialog((Window) parentComponent, title,
          ModalityType.APPLICATION_MODAL);
    else {
      logger.fatal("Bad parent for error dialog");
      errorDialog = new JDialog((Frame) null, title + " (Java stack trace)");
    }

    final JPanel errorPanel = new JPanel();
    errorPanel.setLayout(new BoxLayout(errorPanel, BoxLayout.Y_AXIS));
    errorPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
    Box messageBox = Box.createHorizontalBox();

    // Icon myIcon = (Icon)DefaultLookup.get(errorPanel, null,
    // "OptionPane.errorIcon");
    // messageBox.add(new JLabel(myIcon));
    messageBox.add(Box.createHorizontalGlue());
    messageBox.add(new JLabel(message));
    messageBox.add(Box.createHorizontalGlue());

    Box buttonBox = Box.createHorizontalBox();
    if (compilationOutput != null) {
      final MessageList listToDisplay = compilationOutput;
      final String titleToDisplay = title + " (Compilation output)";
      JButton showCompilationButton = new JButton("Show compilation output");
      showCompilationButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          JDialog messageListDialog = new JDialog(errorDialog, titleToDisplay);

          JPanel messageListPanel = new JPanel(new BorderLayout());

          messageListPanel.add(BorderLayout.CENTER, new JScrollPane(
              listToDisplay));
          messageListPanel.setBorder(BorderFactory.createEmptyBorder(20, 20,
              20, 20));
          messageListPanel.setVisible(true);

          messageListDialog.getContentPane().add(messageListPanel);
          messageListDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
          messageListDialog.pack();
          messageListDialog.setLocationRelativeTo(errorDialog);

          Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment()
              .getMaximumWindowBounds();
          if (maxSize != null
              && (messageListDialog.getSize().getWidth() > maxSize.getWidth() || messageListDialog
                  .getSize().getHeight() > maxSize.getHeight())) {
            Dimension newSize = new Dimension();
            newSize.height = Math.min((int) maxSize.getHeight(),
                (int) messageListDialog.getSize().getHeight());
            newSize.width = Math.min((int) maxSize.getWidth(),
                (int) messageListDialog.getSize().getWidth());
            messageListDialog.setSize(newSize);
          }

          messageListDialog.setVisible(true);
        }
      });
      buttonBox.add(showCompilationButton);
    }

    if (stackTrace != null) {
      final MessageList listToDisplay = stackTrace;
      final String titleToDisplay = title + " (Java stack trace)";
      JButton showTraceButton = new JButton("Show Java stack trace");
      showTraceButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          JDialog messageListDialog = new JDialog(errorDialog, titleToDisplay);

          JPanel messageListPanel = new JPanel(new BorderLayout());

          messageListPanel.add(BorderLayout.CENTER, new JScrollPane(
              listToDisplay));
          messageListPanel.setBorder(BorderFactory.createEmptyBorder(20, 20,
              20, 20));
          messageListPanel.setVisible(true);

          messageListDialog.getContentPane().add(messageListPanel);
          messageListDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
          messageListDialog.pack();
          messageListDialog.setLocationRelativeTo(errorDialog);

          Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment()
              .getMaximumWindowBounds();
          if (maxSize != null
              && (messageListDialog.getSize().getWidth() > maxSize.getWidth() || messageListDialog
                  .getSize().getHeight() > maxSize.getHeight())) {
            Dimension newSize = new Dimension();
            newSize.height = Math.min((int) maxSize.getHeight(),
                (int) messageListDialog.getSize().getHeight());
            newSize.width = Math.min((int) maxSize.getWidth(),
                (int) messageListDialog.getSize().getWidth());
            messageListDialog.setSize(newSize);
          }

          messageListDialog.setVisible(true);
        }
      });
      buttonBox.add(showTraceButton);
    }

    if (retryAvailable) {
      JButton retryButton = new JButton("Retry");
      retryButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          errorDialog.dispose();
          errorDialog.setTitle("-RETRY-");
        }
      });
      buttonBox.add(retryButton);
    }

    JButton closeButton = new JButton("Close");
    closeButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        errorDialog.dispose();
      }
    });
    buttonBox.add(closeButton);

    errorPanel.add(messageBox);
    errorPanel.add(Box.createVerticalStrut(20));
    errorPanel.add(buttonBox);

    errorDialog.getContentPane().add(errorPanel);
    errorDialog.pack();
    errorDialog.setLocationRelativeTo(parentComponent);
    errorDialog.setVisible(true);

    if (errorDialog.getTitle().equals("-RETRY-"))
      return true;
    return false;
  }

}
