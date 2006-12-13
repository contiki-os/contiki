/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 * $Id: GUI.java,v 1.12 2006/12/13 11:57:04 fros4943 Exp $
 */

package se.sics.cooja;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.*;
import javax.swing.*;
import javax.swing.event.MenuEvent;
import javax.swing.event.MenuListener;
import javax.swing.filechooser.FileFilter;
import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;

import se.sics.cooja.contikimote.*;
import se.sics.cooja.dialogs.*;
import se.sics.cooja.plugins.*;

/**
 * Main file of COOJA Simulator.
 *
 * @author Fredrik Osterlind
 */
public class GUI extends JDesktopPane {

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

  /**
   * Logger settings filename.
   */
  public static final String LOG_CONFIG_FILE = "log4j_config.xml";

  /**
   * Default platform configuration filename.
   */
  public static final String PLATFORM_DEFAULT_CONFIG_FILENAME = "/cooja_default.config";

  /**
   * User platform configuration filename.
   */
  public static final String PLATFORM_CONFIG_FILENAME = "cooja.config";

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
   * Main frame for current GUI
   */
  public static JFrame frame;

  /**
   * Current active simulation
   */
  public static Simulation currentSimulation = null;

  /**
   * Current active GUI
   */
  public static GUI currentGUI = null;

  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(GUI.class);

  // External tools setting names
  private static Properties currentExternalToolsSettings;
  private static final String externalToolsSettingNames[] = new String[]{
      "PATH_CONTIKI", "PATH_COOJA_CORE_RELATIVE", "PATH_MAKE", "PATH_SHELL",
      "PATH_C_COMPILER", "COMPILER_ARGS", "PATH_LINKER", "PATH_NM", "NM_ARGS",
      "PATH_OBJDUMP", "OBJDUMP_ARGS", "LINKER_ARGS_1",
      "LINKER_ARGS_2", "CONTIKI_STANDARD_PROCESSES", "CMD_GREP_PROCESSES",
      "REGEXP_PARSE_PROCESSES", "CMD_GREP_INTERFACES",
      "REGEXP_PARSE_INTERFACES", "CMD_GREP_SENSORS", "REGEXP_PARSE_SENSORS",
      "CONTIKI_MAIN_TEMPLATE_FILENAME", "DEFAULT_USERPLATFORMS"};

  private static final int FRAME_NEW_OFFSET = 30;
  private static final int FRAME_STANDARD_WIDTH = 150;
  private static final int FRAME_STANDARD_HEIGHT = 300;

  private GUI myGUI;
  private Mote selectedMote = null;
  private GUIEventHandler guiEventHandler = new GUIEventHandler();

  private JMenu menuPlugins, menuMoteTypeClasses, menuMoteTypes;
  private JPopupMenu menuMotePlugins;

  // Platform configuration variables
  // Maintained via method reparsePlatformConfig()
  private PlatformConfig platformConfig;
  private Vector<File> currentUserPlatforms = new Vector<File>();
  private ClassLoader userPlatformClassLoader;

  private Vector<Class<? extends MoteType>> moteTypeClasses = new Vector<Class<? extends MoteType>>();
  private Vector<Class<? extends VisPlugin>> pluginClasses = new Vector<Class<? extends VisPlugin>>();
  private Vector<Class<? extends VisPlugin>> pluginClassesTemporary = new Vector<Class<? extends VisPlugin>>();
  private Vector<Class<? extends RadioMedium>> radioMediumClasses = new Vector<Class<? extends RadioMedium>>();
  private Vector<Class<? extends IPDistributor>> ipDistributorClasses = new Vector<Class<? extends IPDistributor>>();
  private Vector<Class<? extends Positioner>> positionerClasses = new Vector<Class<? extends Positioner>>();

  /**
   * Creates a new COOJA Simulator GUI.
   */
  public GUI() {
    myGUI = this;
    currentGUI = this;

    // Set drag frames to outlines only (faster)
    setDragMode(JDesktopPane.OUTLINE_DRAG_MODE);

    // Add menu bar
    frame.setJMenuBar(createMenuBar());

    frame.setSize(700, 700);

    frame.addWindowListener(guiEventHandler);

    // Load default and overwrite with user settings (if any)
    loadExternalToolsDefaultSettings();
    loadExternalToolsUserSettings();

    // Register default user platforms
    String defaultUserPlatforms = getExternalToolsSetting("DEFAULT_USERPLATFORMS", null);
    if (defaultUserPlatforms != null) {
      String[] defaultUserPlatformsArr = defaultUserPlatforms.split(";");
      if (defaultUserPlatformsArr.length > 0) {
        for (String defaultUserPlatform: defaultUserPlatformsArr) {
          File userPlatform = new File(defaultUserPlatform);
          if (userPlatform.exists() && userPlatform.isDirectory()) {
            currentUserPlatforms.add(userPlatform);
          }
        }
      }
    }
   
    // Load extendable parts (using current platform config)
    reparsePlatformConfig();

    // EXPERIMENTAL: Start all standard GUI plugins
    for (Class<? extends VisPlugin> visPluginClass : pluginClasses) {
      int pluginType = visPluginClass.getAnnotation(VisPluginType.class)
          .value();
      if (pluginType == VisPluginType.GUI_STANDARD_PLUGIN) {
        startPlugin(visPluginClass);
      }
    }

  }

  private JMenuBar createMenuBar() {
    JMenuBar menuBar = new JMenuBar();
    JMenu menu;
    JMenuItem menuItem;

    // File menu
    menu = new JMenu("File");
    menu.setMnemonic(KeyEvent.VK_F);
    menuBar.add(menu);

    menuItem = new JMenuItem("New simulation");
    menuItem.setMnemonic(KeyEvent.VK_N);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,
        ActionEvent.CTRL_MASK));
    menuItem.setActionCommand("new sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menuItem = new JMenuItem("Close simulation");
    menuItem.setMnemonic(KeyEvent.VK_C);
    menuItem.setActionCommand("close sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menuItem = new JMenuItem("Load simulation");
    menuItem.setMnemonic(KeyEvent.VK_L);
    menuItem.setActionCommand("load sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menuItem = new JMenuItem("Save simulation");
    menuItem.setMnemonic(KeyEvent.VK_S);
    menuItem.setActionCommand("save sim");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    menu.addSeparator();

    menuItem = new JMenuItem("Quit");
    menuItem.setMnemonic(KeyEvent.VK_Q);
    menuItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_Q,
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

        if (currentSimulation == null) {
          return;
        }

        // Recreate menu items
        JMenuItem menuItem;

        for (MoteType moteType : currentSimulation.getMoteTypes()) {
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

    menuItem = new JMenuItem("Manage user platforms");
    menuItem.setActionCommand("manage platforms");
    menuItem.addActionListener(guiEventHandler);
    menu.add(menuItem);

    // Mote plugins popup menu (not available via menu bar)
    menuMotePlugins = new JPopupMenu();
    menuMotePlugins.add(new JLabel("Open mote plugin:"));
    menuMotePlugins.add(new JSeparator());

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
    JComponent newContentPane = new GUI();
    newContentPane.setOpaque(true);
    frame.setContentPane(newContentPane);
    frame.setLocationRelativeTo(null);

    // Display the window.
    frame.setVisible(true);
  }

  /**
   * Quick-starts a simulation using given parameters. TODO Experimental code
   * 
   * @param moteTypeID
   *          Mote type ID (if null "mtype1" will be used)
   * @param userPlatforms
   *          GUI user platforms
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
   *          Main Contiki user process file
   * @param contikiPath
   *          Contiki path
   * @return True if simulation was quickstarted correctly
   */
  private static boolean quickStartSimulation(String moteTypeID,
      Vector<String> userPlatforms, Vector<String> sensors,
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
    GUI gui = new GUI(); // loads external settings and creates initial
    // platform config
    JComponent newContentPane = gui;
    newContentPane.setOpaque(true);
    frame.setContentPane(newContentPane);
    frame.setLocationRelativeTo(null);

    // Set manual Contiki path if specified
    if (contikiPath != null)
      setExternalToolsSetting("PATH_CONTIKI", contikiPath);

    // Parse user platforms and create config
    if (userPlatforms == null) {
      userPlatforms = new Vector<String>();
      userPlatforms.add(".");
    }
    
    //XXX Should add user prop platforms as well here...
    logger.info("> Reparsing user platforms and creating config");
    for (String userPlatform : userPlatforms) {
      logger.info(">> Adding: " + userPlatform);
      gui.currentUserPlatforms.add(new File(userPlatform));
    }
    boolean parsedPlatforms = gui.reparsePlatformConfig();
    if (!parsedPlatforms) {
      logger.fatal(">> Error when parsing platforms, aborting");
      return false;
    }

    // Check file permissions and paths
    logger.info("> Checking paths and file permissions");
    if (moteTypeID == null)
      moteTypeID = "mtype1";
    File contikiBaseDir = new File(getExternalToolsSetting("PATH_CONTIKI"));
    File contikiCoreDir = new File(contikiBaseDir,
        getExternalToolsSetting("PATH_COOJA_CORE_RELATIVE"));
    File libFile = new File(ContikiMoteType.tempOutputDirectory,
        moteTypeID + ContikiMoteType.librarySuffix);
    File mapFile = new File(ContikiMoteType.tempOutputDirectory,
        moteTypeID + ContikiMoteType.mapSuffix);
    File depFile = new File(ContikiMoteType.tempOutputDirectory,
        moteTypeID + ContikiMoteType.dependSuffix);
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
      for (String userPlatform : userPlatforms) {
        mainProcessFile = new File(userPlatform, filename);
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
    for (String userPlatform : userPlatforms)
      // user platforms
      filesToCompile.add(new File(userPlatform));
    String[] platformSources = // platform config sources
    gui.getPlatformConfig().getStringArrayValue(ContikiMoteType.class,
        "C_SOURCES");
    for (String platformSource : platformSources) {
      if (!platformSource.equals("")) {
        File file = new File(platformSource);
        if (file.getParent() != null) {
          // Find which user platform added this file
          File userPlatform = gui.getPlatformConfig().getUserPlatformDefining(
              ContikiMoteType.class, "C_SOURCES", platformSource);
          if (userPlatform != null) {
            // We found a user platform - Add directory
            filesToCompile.add(new File(userPlatform.getPath(), file
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
      for (String userPlatform : userPlatforms)
        // user platforms
        scannedSensorInfo.addAll(ContikiMoteTypeDialog.scanForSensors(new File(
            userPlatform)));

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
      for (String userPlatform : userPlatforms)
        // user platforms
        scannedCoreInterfaceInfo.addAll(ContikiMoteTypeDialog
            .scanForInterfaces(new File(userPlatform)));

      for (String[] coreInterfaceInfo : scannedCoreInterfaceInfo) {
        // logger.info(">> Found and added: " + coreInterfaceInfo[1] + " (" +
        // coreInterfaceInfo[0] + ")");
        coreInterfaces.add(coreInterfaceInfo[1]);
      }
    }

    // Scan for mote interfaces
    logger.info("> Loading mote interfaces");
    String[] moteInterfaces = gui.getPlatformConfig().getStringArrayValue(
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
      for (String userPlatform : userPlatforms)
        // user platforms
        scannedProcessInfo.addAll(ContikiMoteTypeDialog
            .scanForProcesses(new File(userPlatform)));

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
    ContikiMoteType moteType = new ContikiMoteType(moteTypeID);
    moteType.setDescription("Mote type: " + filename);
    moteType.setContikiBaseDir(contikiBaseDir.getPath());
    moteType.setContikiCoreDir(contikiCoreDir.getPath());
    moteType.setUserPlatformDirs(new Vector<File>());
    moteType.setConfig(gui.getPlatformConfig());
    moteType.setProcesses(userProcesses);
    moteType.setSensors(sensors);
    moteType.setCoreInterfaces(coreInterfaces);
    moteType.setMoteInterfaces(moteIntfClasses);

    // Create simulation
    logger.info("> Creating simulation");
    Simulation simulation = new Simulation();
    simulation.setTitle("Quickstarted: " + filename);
    simulation.setDelayTime(delayTime);
    simulation.setSimulationTime(0);
    simulation.setTickTime(1);
    String radioMediumClassName = null;
    try {
      radioMediumClassName = gui.getPlatformConfig().getStringArrayValue(
          GUI.class, "RADIOMEDIUMS")[0];
      Class<? extends RadioMedium> radioMediumClass = gui.tryLoadClass(gui,
          RadioMedium.class, radioMediumClassName);
      simulation.setRadioMedium(radioMediumClass.newInstance());
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

    // Start plugin and showing GUI
    logger.info("> Starting plugin and showing GUI");
    gui.startPlugin(VisState.class);
    frame.setVisible(true);

    if (simulationStarting) {
      simulation.startSimulation();
    }
    return true;
  }
  
  // // PLATFORM CONFIG AND EXTENDABLE PARTS METHODS ////

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
      ipDistributorClass.getConstructor(new Class[]{Vector.class});
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
          .getConstructor(new Class[]{int.class, double.class, double.class,
              double.class, double.class, double.class, double.class});
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
   * Builds new platform configuration using current user platforms settings.
   * Reregisters mote types, plugins, IP distributors, positioners and radio
   * mediums. This method may still return true even if all classes could not be
   * registered, but always returns false if all user platform configuration
   * files were not parsed correctly.
   *
   * Any registered temporary plugins will be saved and reregistered.
   *
   * @return True if external configuration files were found and parsed OK
   */
  public boolean reparsePlatformConfig() {
    // Backup temporary plugins
    Vector<Class<? extends VisPlugin>> oldTempPlugins = (Vector<Class<? extends VisPlugin>>) pluginClassesTemporary
        .clone();

    // Reset current configuration
    unregisterMoteTypes();
    unregisterPlugins();
    unregisterIPDistributors();
    unregisterPositioners();
    unregisterRadioMediums();

    try {
      // Read default configuration
      platformConfig = new PlatformConfig(true);
      // logger.info("Loading default platform configuration: " +
      // PLATFORM_DEFAULT_CONFIG_FILENAME);
    } catch (FileNotFoundException e) {
      logger.fatal("Could not find default platform config file: "
          + PLATFORM_DEFAULT_CONFIG_FILENAME);
      return false;
    } catch (IOException e) {
      logger.fatal("Error when reading default platform config file: "
          + PLATFORM_DEFAULT_CONFIG_FILENAME);
      return false;
    }

    // Append user platform configurations
    for (File userPlatform : currentUserPlatforms) {

      try {
        // Append config to general config
        // logger.info("Appending user platform configuration: " + userPlatform);
        platformConfig.appendUserPlatform(userPlatform);
      } catch (FileNotFoundException e) {
        logger.fatal("Could not find platform config file: "
            + userPlatform);
        return false;
      } catch (IOException e) {
        logger.fatal("Error when reading platform config file: "
            + userPlatform);
        return false;
      }
    }

    // Create class loader
    userPlatformClassLoader = createClassLoader(currentUserPlatforms);

    // Register mote types
    String[] moteTypeClassNames = platformConfig.getStringArrayValue(GUI.class,
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
    String[] pluginClassNames = platformConfig.getStringArrayValue(GUI.class,
        "PLUGINS");
    if (pluginClassNames != null) {
      for (String pluginClassName : pluginClassNames) {
        Class<? extends VisPlugin> pluginClass = tryLoadClass(this,
            VisPlugin.class, pluginClassName);

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
      for (Class<? extends VisPlugin> pluginClass : oldTempPlugins) {
        if (registerTemporaryPlugin(pluginClass)) {
          // logger.info("Reregistered temporary plugin class: " +
          // getDescriptionOf(pluginClass));
        } else
          logger.warn("Could not reregister temporary plugin class: "
              + getDescriptionOf(pluginClass));
      }
    }

    // Register IP distributors
    String[] ipDistClassNames = platformConfig.getStringArrayValue(GUI.class,
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
    String[] positionerClassNames = platformConfig.getStringArrayValue(
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
    String[] radioMediumsClassNames = platformConfig.getStringArrayValue(
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
   * Returns the current platform configuration common to the entire simulator.
   *
   * @return Current platform configuration
   */
  public PlatformConfig getPlatformConfig() {
    return platformConfig;
  }

  /**
   * Returns the current user platforms common to the entire simulator.
   *
   * @return Current user platforms.
   */
  public Vector<File> getUserPlatforms() {
    return currentUserPlatforms;
  }

  // // PLUGIN METHODS ////

  /**
   * Show a started plugin in working area.
   *
   * @param plugin
   *          Internal frame to add
   */
  public void showPlugin(VisPlugin plugin) {
    int nrFrames = this.getAllFrames().length;
    add(plugin);

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
      for (JInternalFrame existingPlugin: this.getAllFrames()) {
        existingPlugin.setSelected(false);
      }
      plugin.setSelected(true);
    } catch (Exception e) {
      // Ignore
    }

    // Mote plugin to front
    myGUI.moveToFront(plugin);
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
  public void removePlugin(VisPlugin plugin, boolean askUser) {
    // Clear any allocated resources and remove plugin
    plugin.closePlugin();
    plugin.dispose();

    if (askUser && myGUI.getAllFrames().length == 0) {
      String s1 = "Remove";
      String s2 = "Cancel";
      Object[] options = {s1, s2};
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
   * Starts a plugin of given plugin class. If the plugin is a mote plugin the
   * currently selected mote will be given as an argument.
   *
   * @param pluginClass
   *          Plugin class
   * @return True if plugin was started, false otherwise
   */
  protected boolean startPlugin(Class<? extends VisPlugin> pluginClass) {
    // Check that plugin class is registered
    if (!pluginClasses.contains(pluginClass)) {
      logger.fatal("Plugin class not registered: " + pluginClass);
      return false;
    }

    // Instantiate and show plugin differently depending on plugin type
    VisPlugin newPlugin = null;
    int pluginType = pluginClass.getAnnotation(VisPluginType.class).value();

    try {
      if (pluginType == VisPluginType.MOTE_PLUGIN) {
        if (selectedMote == null) {
          logger.fatal("Can't start mote plugin (no mote selected)");
          return false;
        }

        newPlugin = pluginClass.getConstructor(new Class[]{Mote.class})
            .newInstance(selectedMote);
        
        // Tag plugin with mote
        newPlugin.putClientProperty("mote", selectedMote);
        
        selectedMote = null;
      } else if (pluginType == VisPluginType.SIM_PLUGIN) {
        if (currentSimulation == null) {
          logger.fatal("Can't start simulation plugin (no simulation)");
          return false;
        }

        newPlugin = pluginClass.getConstructor(new Class[]{Simulation.class})
            .newInstance(currentSimulation);
      } else if (pluginType == VisPluginType.SIM_STANDARD_PLUGIN) {
        if (currentSimulation == null) {
          logger
              .fatal("Can't start simulation standard plugin (no simulation)");
          return false;
        }

        newPlugin = pluginClass.getConstructor(new Class[]{Simulation.class})
            .newInstance(currentSimulation);
      } else if (pluginType == VisPluginType.GUI_PLUGIN) {
        if (currentGUI == null) {
          logger.fatal("Can't start GUI plugin (no GUI)");
          return false;
        }

        newPlugin = pluginClass.getConstructor(new Class[]{GUI.class})
            .newInstance(currentGUI);
      } else if (pluginType == VisPluginType.GUI_STANDARD_PLUGIN) {
        if (currentGUI == null) {
          logger.fatal("Can't start GUI plugin (no GUI)");
          return false;
        }

        newPlugin = pluginClass.getConstructor(new Class[]{GUI.class})
            .newInstance(currentGUI);
      }
    } catch (Exception e) {
      logger.fatal("Exception thrown when starting plugin: " + e);
      return false;
    }

    if (newPlugin == null)
      return false;

    // Show plugin
    myGUI.showPlugin(newPlugin);
    return true;
  }

  /**
   * Register a plugin to be included in the GUI. The plugin will be visible in
   * the menubar.
   *
   * @param newPluginClass
   *          New plugin to register
   * @return True if this plugin was registered ok, false otherwise
   */
  public boolean registerPlugin(Class<? extends VisPlugin> newPluginClass) {
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
  public boolean registerTemporaryPlugin(
      Class<? extends VisPlugin> newPluginClass) {
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
  public void unregisterPlugin(Class<? extends VisPlugin> pluginClass) {

    // Remove (if existing) plugin class menu items
    for (Component menuComponent : menuPlugins.getMenuComponents()) {
      if (menuComponent.getClass().isAssignableFrom(JMenuItem.class)) {
        JMenuItem menuItem = (JMenuItem) menuComponent;
        if (menuItem.getClientProperty("class").equals(pluginClass))
          menuPlugins.remove(menuItem);
      }
    }
    for (MenuElement menuComponent : menuMotePlugins.getSubElements()) {
      if (menuComponent.getClass().isAssignableFrom(JMenuItem.class)) {
        JMenuItem menuItem = (JMenuItem) menuComponent;
        if (menuItem.getClientProperty("class").equals(pluginClass))
          menuPlugins.remove(menuItem);
      }
    }

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
  private boolean registerPlugin(Class<? extends VisPlugin> newPluginClass,
      boolean addToMenu) {

    // Get description annotation (if any)
    String description = getDescriptionOf(newPluginClass);

    // Get plugin type annotation (required)
    int pluginType;
    if (newPluginClass.isAnnotationPresent(VisPluginType.class)) {
      pluginType = newPluginClass.getAnnotation(VisPluginType.class).value();
    } else {
      pluginType = VisPluginType.UNDEFINED_PLUGIN;
    }

    // Check that plugin type is valid and constructor exists
    try {
      if (pluginType == VisPluginType.MOTE_PLUGIN) {
        newPluginClass.getConstructor(new Class[]{Mote.class});
      } else if (pluginType == VisPluginType.SIM_PLUGIN) {
        newPluginClass.getConstructor(new Class[]{Simulation.class});
      } else if (pluginType == VisPluginType.SIM_STANDARD_PLUGIN) {
        newPluginClass.getConstructor(new Class[]{Simulation.class});
      } else if (pluginType == VisPluginType.GUI_PLUGIN) {
        newPluginClass.getConstructor(new Class[]{GUI.class});
      } else if (pluginType == VisPluginType.GUI_STANDARD_PLUGIN) {
        newPluginClass.getConstructor(new Class[]{GUI.class});
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

    if (addToMenu) {
      // Create 'start plugin'-menu item
      JMenuItem menuItem = new JMenuItem(description);
      menuItem.setActionCommand("start plugin");
      menuItem.putClientProperty("class", newPluginClass);
      menuItem.addActionListener(guiEventHandler);

      menuPlugins.add(menuItem);

      if (pluginType == VisPluginType.MOTE_PLUGIN) {
        // Disable previous menu item and add new item to mote plugins menu
        menuItem.setEnabled(false);
        menuItem.setToolTipText("Mote plugin");

        menuItem = new JMenuItem(description);
        menuItem.setActionCommand("start plugin");
        menuItem.putClientProperty("class", newPluginClass);
        menuItem.addActionListener(guiEventHandler);
        menuMotePlugins.add(menuItem);
      }
    }

    pluginClasses.add(newPluginClass);
    return true;
  }

  /**
   * Unregister all plugin classes, including temporary plugins.
   */
  public void unregisterPlugins() {
    menuPlugins.removeAll();
    menuMotePlugins.removeAll();
    pluginClasses.clear();
    pluginClassesTemporary.clear();
  }

  /**
   * Show mote plugins menu for starting a mote plugin. All registered mote
   * plugins can be selected from.
   *
   * @param invoker
   *          Component that wants to display the menu
   * @param mote
   *          Mote of plugin selected
   * @param location
   *          Location of popup menu
   */
  public void showMotePluginsMenu(Component invoker, Mote mote, Point location) {
    menuMotePlugins.setInvoker(invoker);
    menuMotePlugins.setLocation(location);
    menuMotePlugins.setVisible(true);
    selectedMote = mote;
  }

  // // GUI CONTROL METHODS ////

  public void setSimulation(Simulation sim) {
    if (sim != null) {
      doRemoveSimulation(false);
    }
    currentSimulation = sim;

    // Set frame title
    frame.setTitle("COOJA Simulator" + " - " + sim.getTitle());

    // Open standard plugins (if none opened already)
    if (getAllFrames().length == 0)
      for (Class<? extends VisPlugin> visPluginClass : pluginClasses) {
        int pluginType = visPluginClass.getAnnotation(VisPluginType.class)
        .value();
        if (pluginType == VisPluginType.SIM_STANDARD_PLUGIN) {
          startPlugin(visPluginClass);
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
    if (currentSimulation == null) {
      logger.fatal("Can't create mote type (no simulation)");
      return;
    }

    // Stop simulation (if running)
    currentSimulation.stopSimulation();

    // Create mote type
    MoteType newMoteType = null;
    boolean moteTypeOK = false;
    try {
      newMoteType = moteTypeClass.newInstance();
      moteTypeOK = newMoteType.configureAndInit(frame, currentSimulation);
    } catch (Exception e) {
      logger.fatal("Exception when creating mote type: " + e);
      return;
    }

    // Add mote type to simulation
    if (newMoteType != null && moteTypeOK) {
      currentSimulation.addMoteType(newMoteType);
    }
  }

  /**
   * Remove current simulation
   *
   * @param askForConfirmation
   *          Should we ask for confirmation if a simulation is already active?
   */
  public void doRemoveSimulation(boolean askForConfirmation) {

    if (currentSimulation != null) {
      if (askForConfirmation) {
        String s1 = "Remove";
        String s2 = "Cancel";
        Object[] options = {s1, s2};
        int n = JOptionPane.showOptionDialog(frame,
            "You have an active simulation.\nDo you want to remove it?",
            "Remove current simulation?", JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, s1);
        if (n != JOptionPane.YES_OPTION) {
          return;
        }
      }

      // Close all started non-GUI plugins
      for (JInternalFrame openededFrame : myGUI.getAllFrames()) {
        // Check that frame is a plugin
        Class frameClass = openededFrame.getClass();
        if (pluginClasses.contains(frameClass)) {
          int pluginType = ((Class<? extends VisPlugin>) frameClass)
              .getAnnotation(VisPluginType.class).value();
          if (pluginType != VisPluginType.GUI_PLUGIN &&
              pluginType != VisPluginType.GUI_STANDARD_PLUGIN)
            removePlugin((VisPlugin) openededFrame, false);
        }
      }

      // Delete simulation
      currentSimulation.deleteObservers();
      currentSimulation.stopSimulation();
      currentSimulation = null;

      // Unregister temporary plugin classes
      Enumeration<Class<? extends VisPlugin>> pluginClasses = pluginClassesTemporary
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
   * @param askForConfirmation
   *          Should we ask for confirmation if a simulation is already active?
   */
  public void doLoadConfig(boolean askForConfirmation) {

    if (CoreComm.hasLibraryBeenLoaded()) {
      JOptionPane
          .showMessageDialog(
              frame,
              "Shared libraries has already been loaded.\nYou need to restart the simulator!",
              "Can't load simulation", JOptionPane.ERROR_MESSAGE);
      return;
    }

    if (askForConfirmation && currentSimulation != null) {
      String s1 = "Remove";
      String s2 = "Cancel";
      Object[] options = {s1, s2};
      int n = JOptionPane.showOptionDialog(frame,
          "You have an active simulation.\nDo you want to remove it?",
          "Remove current simulation?", JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE, null, options, s1);
      if (n != JOptionPane.YES_OPTION) {
        return;
      }
    }

    doRemoveSimulation(false);

    JFileChooser fc = new JFileChooser();

    fc.setFileFilter(GUI.SAVED_SIMULATIONS_FILES);

    int returnVal = fc.showOpenDialog(frame);
    if (returnVal == JFileChooser.APPROVE_OPTION) {
      File loadFile = fc.getSelectedFile();

      // Try adding extension if not founds
      if (!loadFile.exists()) {
        loadFile = new File(loadFile.getParent(), loadFile.getName()
            + SAVED_SIMULATIONS_FILES);
      }

      if (loadFile.exists() && loadFile.canRead()) {
        Simulation newSim = null;
        try {
          newSim = Simulation.loadSimulationConfig(loadFile);
        } catch (UnsatisfiedLinkError e) {
          logger.warn("Could not reopen libraries");
          newSim = null;
        }
        if (newSim != null) {
          myGUI.setSimulation(newSim);
        }
      } else
        logger.fatal("No read access to file");

    } else {
      logger.info("Load command cancelled by user...");
    }

  }

  /**
   * Save current simulation configuration to disk
   *
   * @param askForConfirmation
   *          Ask for confirmation before overwriting file
   */
  public void doSaveConfig(boolean askForConfirmation) {
    if (currentSimulation != null) {
      currentSimulation.stopSimulation();

      JFileChooser fc = new JFileChooser();

      fc.setFileFilter(GUI.SAVED_SIMULATIONS_FILES);

      int returnVal = fc.showSaveDialog(myGUI);
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
            Object[] options = {s1, s2};
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

        if (!saveFile.exists() || saveFile.canWrite())
          currentSimulation.saveSimulationConfig(saveFile);
        else
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
    if (currentSimulation != null) {
      currentSimulation.stopSimulation();

      Vector<Mote> newMotes = AddMoteDialog.showDialog(frame, moteType);
      if (newMotes != null) {
        for (Mote newMote : newMotes)
          currentSimulation.addMote(newMote);
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
    if (askForConfirmation && currentSimulation != null) {
      String s1 = "Remove";
      String s2 = "Cancel";
      Object[] options = {s1, s2};
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
    Simulation newSim = new Simulation();
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
    if (!askForConfirmation)
      System.exit(0);

    String s1 = "Quit";
    String s2 = "Cancel";
    Object[] options = {s1, s2};
    int n = JOptionPane.showOptionDialog(frame, "Sure you want to quit?",
        "Close COOJA Simulator", JOptionPane.YES_NO_OPTION,
        JOptionPane.QUESTION_MESSAGE, null, options, s1);
    if (n != JOptionPane.YES_OPTION)
      return;

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
    } catch (IOException e) {
      // Error while importing default properties
      logger.warn(
          "Error when reading external tools settings from " + filename, e);
    } finally {
      if (currentExternalToolsSettings == null) {
        currentExternalToolsSettings = new Properties();
      }
    }
  }

  /**
   * Load user values from external properties file
   */
  private static void loadExternalToolsUserSettings() {
    File configFile = new File(System.getProperty("user.home"), GUI.EXTERNAL_TOOLS_USER_SETTINGS_FILENAME);
    
    try {
      FileInputStream in = new FileInputStream(configFile);
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
      logger.warn("Error when reading default settings from " + configFile);
    }
  }

  /**
   * Save external tools user settings to file.
   */
  public static void saveExternalToolsUserSettings() {
    File configFile = new File(System.getProperty("user.home"), GUI.EXTERNAL_TOOLS_USER_SETTINGS_FILENAME);
    try {
      FileOutputStream out = new FileOutputStream(configFile);
      currentExternalToolsSettings.store(out, "COOJA User Settings");
      out.close();
    } catch (FileNotFoundException ex) {
      // Could not open settings file for writing, aborting
      logger.warn("Could not save external tools user settings to " + configFile
          + ", aborting");
    } catch (IOException ex) {
      // Could not open settings file for writing, aborting
      logger.warn("Error while saving external tools user settings to "
          + configFile + ", aborting");
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
      } else if (e.getActionCommand().equals("load sim")) {
        myGUI.doLoadConfig(true);
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
      } else if (e.getActionCommand().equals("manage platforms")) {
        Vector<File> newPlatforms = UserPlatformsDialog.showDialog(frame,
            currentUserPlatforms, null);
        if (newPlatforms != null) {
          currentUserPlatforms = newPlatforms;
          reparsePlatformConfig();
        }
      } else if (e.getActionCommand().equals("start plugin")) {
        Class<? extends VisPlugin> pluginClass = (Class<? extends VisPlugin>) ((JMenuItem) e
            .getSource()).getClientProperty("class");
        startPlugin(pluginClass);
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
      if (userPlatformClassLoader != null) {
	return userPlatformClassLoader.loadClass(className).asSubclass(classType);
      }
    } catch (ClassNotFoundException e) {
    }

    return null;
  }

  public ClassLoader createUserPlatformClassLoader(Vector<File> platformsList) {
    if (userPlatformClassLoader == null) {
      reparsePlatformConfig();
    }
    return createClassLoader(userPlatformClassLoader, platformsList);
  }

  private ClassLoader createClassLoader(Vector<File> currentUserPlatforms) {
    return createClassLoader(ClassLoader.getSystemClassLoader(), currentUserPlatforms);
  }

  private File findJarFile(File platformPath, String jarfile) {
    File fp = new File(jarfile);
    if (!fp.exists()) {
      fp = new File(platformPath, jarfile);
    }
    if (!fp.exists()) {
      fp = new File(platformPath, "java/" + jarfile);
    }
    if (!fp.exists()) {
      fp = new File(platformPath, "java/lib/" + jarfile);
    }
    if (!fp.exists()) {
      fp = new File(platformPath, "lib/" + jarfile);
    }
    return fp.exists() ? fp : null;
  }

  private ClassLoader createClassLoader(ClassLoader parent,
      Vector<File> platformsList) {
    if (platformsList == null || platformsList.isEmpty()) {
      return parent;
    }
    
    // Combine class loader from all user platforms (including any
    // specified JAR files)
    ArrayList<URL> urls = new ArrayList<URL>();
    for (int j = platformsList.size() - 1; j >= 0; j--) {
      File userPlatform = platformsList.get(j);
      try {
        urls.add((new File(userPlatform, "java")).toURL());
        
        // Read configuration to check if any JAR files should be loaded
        PlatformConfig userPlatformConfig = new PlatformConfig(false);
        userPlatformConfig.appendUserPlatform(userPlatform);
        String[] platformJarFiles = userPlatformConfig.getStringArrayValue(
            GUI.class, "JARFILES");
        if (platformJarFiles != null && platformJarFiles.length > 0) {
          for (String jarfile : platformJarFiles) {
            File jarpath = findJarFile(userPlatform, jarfile);
            if (jarpath == null) {
              throw new FileNotFoundException(jarfile);
            }
            urls.add(jarpath.toURL());
          }
        }
        
      } catch (Exception e) {
        logger.fatal("Error when trying to read JAR-file in " + userPlatform
            + ": " + e);
      }
    }
    
    return new URLClassLoader((URL[]) urls.toArray(new URL[urls.size()]), parent);
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

    // Check if simulator should be quick-started
    if (args.length > 0 && args[0].startsWith("-quickstart=")) {
      String filename = args[0].substring("-quickstart=".length());

      String moteTypeID = "mtype1";
      Vector<String> userPlatforms = null;
      Vector<String> sensors = null;
      Vector<String> coreInterfaces = null;
      Vector<String> userProcesses = null;
      boolean addAutostartProcesses = true;
      int numberOfNodes = 100;
      double areaSideLength = 100;
      int delayTime = 0;
      boolean startSimulation = true;
      String contikiPath = null;

      // Parse quick start arguments
      for (int i = 1; i < args.length; i++) {

        if (args[i].startsWith("-id=")) {
          moteTypeID = args[i].substring("-id=".length());

        } else if (args[i].startsWith("-platforms=")) {
          String arg = args[i].substring("-platforms=".length());
          String[] argArray = arg.split(",");
          userPlatforms = new Vector<String>();
          for (String argValue : argArray)
            userPlatforms.add(argValue);

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

      boolean ok = quickStartSimulation(moteTypeID, userPlatforms, sensors,
          coreInterfaces, userProcesses, addAutostartProcesses, numberOfNodes,
          areaSideLength, delayTime, startSimulation, filename, contikiPath);
      if (!ok)
        System.exit(1);

    } else {

      // Regular start-up
      javax.swing.SwingUtilities.invokeLater(new Runnable() {
        public void run() {
          createAndShowGUI();
        }
      });

    }
  }
    
}
