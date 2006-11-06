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
 * $Id: ContikiMoteType.java,v 1.4 2006/11/06 18:01:56 fros4943 Exp $
 */

package se.sics.cooja.contikimote;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.io.*;
import java.security.*;
import java.util.*;
import java.util.regex.*;
import javax.swing.*;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * The Contiki mote type holds the native library used to communicate with an
 * underlying Contiki system. All communication with that system should always
 * pass through this mote type.
 * <p>
 * This type also contains information about which processes, sensors and core
 * interfaces a mote of this type has, as well as where the Contiki OS, COOJA
 * core files and an optional user platform are located.
 * <p>
 * All core communication with the Contiki mote should be via this class. When a
 * mote type is created it allocates a CoreComm to be used with this type, and
 * loads a map file. The map file is used to map variable names to addresses.
 * <p>
 * When a new mote type is created an initialization function is run on the
 * Contiki system in order to create the initial memory. When a new mote is
 * created the createInitialMemory() method should be called to get this initial
 * memory for the mote.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Contiki Mote Type")
public class ContikiMoteType implements MoteType {
  private static Logger logger = Logger.getLogger(ContikiMoteType.class);

  /**
   * Map file suffix
   */
  final static public String mapSuffix = ".map";

  /**
   * Library file suffix
   */
  final static public String librarySuffix = ".library";

  /**
   * Make dependency file suffix
   */
  final static public String dependSuffix = ".a";

  /**
   * Temporary output directory
   */
  final static public File tempOutputDirectory = new File("obj_cooja");

  // Regular expressions for parsing the map file
  final static private String bssSectionAddrRegExp = "^.bss[ \t]*0x([0-9A-Fa-f]*)[ \t]*0x[0-9A-Fa-f]*[ \t]*$";
  final static private String bssSectionSizeRegExp = "^.bss[ \t]*0x[0-9A-Fa-f]*[ \t]*0x([0-9A-Fa-f]*)[ \t]*$";
  final static private String dataSectionAddrRegExp = "^.data[ \t]*0x([0-9A-Fa-f]*)[ \t]*0x[0-9A-Fa-f]*[ \t]*$";
  final static private String dataSectionSizeRegExp = "^.data[ \t]*0x[0-9A-Fa-f]*[ \t]*0x([0-9A-Fa-f]*)[ \t]*$";
  final static private String varAddressRegExpPrefix = "^[ \t]*0x([0-9A-Fa-f]*)[ \t]*";
  final static private String varAddressRegExpSuffix = "[ \t]*$";
  final static private String varNameRegExp = "^[ \t]*(0x[0-9A-Fa-f]*)[ \t]*([^ ]*)[ \t]*$";
  final static private String varSizeRegExpPrefix = "^";
  final static private String varSizeRegExpSuffix = "[ \t]*(0x[0-9A-Fa-f]*)[ \t]*[^ ]*[ \t]*$";

  // Regular expressions for parsing nm response
  final static private String nmRegExp = "^([0-9A-Fa-f][0-9A-Fa-f]*)[ \t][^Tt][ \t]([^ ._][^ ]*)";
  
  // Mote type specific data
  private String identifier = null;
  private String description = null;
  private String contikiBaseDir = null;
  private String contikiCoreDir = null;
  private Vector<File> userPlatformDirs = null;
  private Vector<String> processes = null;
  private Vector<String> sensors = null;
  private Vector<String> coreInterfaces = null;
  private Vector<Class<? extends MoteInterface>> moteInterfaces = null;
  private boolean hasSystemSymbols = false;
  
  // Simulation holding this mote type
  private Simulation mySimulation = null;

  // Type specific class configuration
  private PlatformConfig myConfig = null;

  // Core communication variables
  private String libraryClassName = null;
  private int offsetRelToAbs = 0;
  private CoreComm myCoreComm = null;

  // Variable name to address mappings
  private Properties varAddresses = new Properties();

  // Initial memory for all motes of this type
  private SectionMoteMemory initialMemory = null;

  /**
   * Creates a new uninitialized Contiki mote type. This mote type needs to load
   * a library file and parse a map file before it can be used.
   */
  public ContikiMoteType() {
  }

  /**
   * Creates a new Contiki mote type. This type uses two external files: a map
   * file for parsing relative addresses of Contiki variables (identifier +
   * ".map") and a library file with an actual compiled Contiki system
   * (identifier + ".library")
   * 
   * @param identifier
   *          Unique identifier for this mote type
   */
  public ContikiMoteType(String identifier) {
    doInit(identifier);
  }

  public Mote generateMote(Simulation simulation) {
    return new ContikiMote(this, simulation);
  }

  public boolean configureAndInit(JFrame parentFrame, Simulation simulation) {
    return ContikiMoteTypeDialog.showDialog(parentFrame, simulation, this);
  }

  /**
   * This is an mote type initialization method and should normally never be
   * called by any other part than the mote type constructor. It is called from
   * the constructor with an identifier argument. but not from the standard
   * constructor. This method may be called from the simulator when loading
   * configuration files, and the libraries must be recompiled.
   * 
   * This method allocates a core communicator, loads the Contiki library file,
   * loads and parses the map file, creates a variable name to address mapping
   * of the Contiki system and finally creates the Contiki mote initial memory.
   * 
   * @param identifier
   *          Mote type identifier
   * @return True if initialization ok, false otherwise
   */
  protected boolean doInit(String identifier) {
    this.identifier = identifier;

    if (myCoreComm != null) {
      logger
          .fatal("Core communicator not null. Is library already loaded? Aborting");
      return false;
    }

    File libFile = new File(ContikiMoteType.tempOutputDirectory,
        identifier + librarySuffix);
    File mapFile = new File(ContikiMoteType.tempOutputDirectory,
        identifier + mapSuffix);

    // Check that library file exists
    if (!libFile.exists()) {
      logger.fatal("Library file could not be found: " + libFile);
      return false;
    }

    // Check that map file exists
    if (!mapFile.exists()) {
      logger.fatal("Map file could not be found: " + mapFile);
      return false;
    }

    // Allocate core communicator class
    libraryClassName = CoreComm.getAvailableClassName();
    myCoreComm = CoreComm.createCoreComm(libraryClassName, libFile);

    // Try load map file
    Vector<String> mapFileData = loadMapFile(mapFile);

    // Try load nm data
    Vector<String> nmData = loadNmData(libFile);

    // Create variable names to addresses mappings
    varAddresses.clear();
    if (mapFileData == null || !parseMapFileData(mapFileData, varAddresses)) {
      logger.fatal("Map file parsing failed");
    }
    logger.info("Testing experimental nm response parsing for finding variable addresses");
    if (nmData == null || !parseNmData(nmData, varAddresses)) {
      logger.fatal("Nm response parsing failed");
    }

    if (varAddresses.size() == 0) {
      logger.fatal("Variable name to addresses mappings could not be created");
      return false;
    }
    
    // Get offset between relative and absolute addresses
    offsetRelToAbs = getReferenceAbsAddr()
        - getRelVarAddr(mapFileData, "referenceVar");

    // Parse addresses of data and BSS memory sections
    int relDataSectionAddr = loadRelDataSectionAddr(mapFileData);
    int dataSectionSize = loadDataSectionSize(mapFileData);
    int relBssSectionAddr = loadRelBssSectionAddr(mapFileData);
    int bssSectionSize = loadBssSectionSize(mapFileData);

    if (relDataSectionAddr <= 0 || dataSectionSize <= 0
        || relBssSectionAddr <= 0 || bssSectionSize <= 0) {
      logger.fatal("Could not parse section addresses correctly");
      return false;
    }

    // Create initial memory
    byte[] initialDataSection = new byte[dataSectionSize];
    getCoreMemory(relDataSectionAddr
        + offsetRelToAbs, dataSectionSize, initialDataSection);
    byte[] initialBssSection = new byte[bssSectionSize];
    getCoreMemory(
        relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);
    initialMemory = new SectionMoteMemory(varAddresses);
    initialMemory.setMemorySegment(relDataSectionAddr, initialDataSection);
    initialMemory.setMemorySegment(relBssSectionAddr, initialBssSection);

    return false;
  }

  /**
   * Ticks the currently loaded mote. This should not be used directly, but
   * rather via ContikiMote.tick().
   */
  public void tick() {
    myCoreComm.tick();
  }

  /**
   * Creates and returns a copy of this mote type's initial memory (just after
   * the init function has been run). When a new mote is created it should get
   * it's memory from here.
   * 
   * @return Initial memory of a mote type
   */
  public SectionMoteMemory createInitialMemory() {
    return initialMemory.clone();
  }

  /**
   * Copy given memory to the Contiki system. This should not be used directly,
   * but instead via ContikiMote.setMemory().
   * 
   * @param mem
   *          New memory
   */
  public void setCoreMemory(SectionMoteMemory mem) {
    for (int i = 0; i < mem.getNumberOfSections(); i++) {
      setCoreMemory(mem.getStartAddrOfSection(i) + offsetRelToAbs, mem
          .getSizeOfSection(i), mem.getDataOfSection(i));
    }
  }

  /**
   * Parses specified map file data for variable name to addresses mappings. The
   * mappings are added to the given properties object.
   * 
   * @param mapFileData
   *          Contents of entire map file
   * @param varAddresses
   *          Properties that should contain the name to addresses mappings.
   */
  private boolean parseMapFileData(Vector<String> mapFileData, Properties varAddresses) {
    Vector<String> varNames = getMapFileVarNames(mapFileData);
    if (varNames == null || varNames.size() == 0)
      return false;
    
    for (String varName : varNames) {
      int varAddress = getMapFileVarAddress(mapFileData, varName);
      if (varAddress > 0) {
        varAddresses.put(varName, new Integer(varAddress));
      } else
        logger.warn("Parsed Contiki variable '" + varName
            + "' but could not find address");
    }

    return true;
  }

  /**
   * Parses specified nm data for variable name to addresses mappings. The
   * mappings are added to the given properties object.
   * 
   * @param nmData
   *          Response from nm command on object file
   * @param varAddresses
   *          Properties that should contain the name to addresses mappings.
   */
  private boolean parseNmData(Vector<String> nmData, Properties varAddresses) {
    int nrNew = 0, nrOld = 0, nrMismatch = 0;
    
    Pattern pattern = Pattern.compile(nmRegExp);
    for (String nmLine: nmData) {
      Matcher matcher = pattern.matcher(nmLine);
      
      if (matcher.find()) {
        //logger.debug("Parsing line: " + nmLine);
        String varName = matcher.group(2);
        int varAddress = Integer.parseInt(matcher.group(1), 16);
        
        if (!varAddresses.containsKey(varName)) {
          nrNew++;
          varAddresses.put(varName, new Integer(varAddress));
        } else {
          int oldAddress = (Integer) varAddresses.get(varName);
          if (oldAddress != varAddress) {
            logger.warn("Warning, nm response not matching previous entry of: " + varName);
            nrMismatch++;
          }
             
          nrOld++;
        }
      }
    }

    if (nrMismatch > 0)
      logger.debug("Nm response parsing summary: Added " + nrNew + " variables. Found " + nrOld + " old variables. MISMATCHING ADDRESSES: " + nrMismatch);
    else
      logger.debug("Nm response parsing summary: Added " + nrNew + " variables. Found " + nrOld + " old variables");

    return (nrNew + nrOld) > 0;
  }
  
  /**
   * Copy core memory to given memory. This should not be used directly, but
   * instead via ContikiMote.getMemory().
   * 
   * @param mem
   *          Memory to set
   */
  public void getCoreMemory(SectionMoteMemory mem) {
    for (int i = 0; i < mem.getNumberOfSections(); i++) {
      int startAddr = mem.getStartAddrOfSection(i);
      int size = mem.getSizeOfSection(i);
      byte[] data = mem.getDataOfSection(i);
      
      getCoreMemory(startAddr + offsetRelToAbs,
          size, data);
    }
  }

  public String getIdentifier() {
    return identifier;
  }

  public void setIdentifier(String identifier) {
    logger.warn("Contiki mote type is read-only");
  }

  /**
   * @param using Core library has system symbols information
   */
  public void setHasSystemSymbols(boolean using) {
    hasSystemSymbols = using;
  }
  
  /**
   * @return Whether core library has system symbols information
   */
  public boolean hasSystemSymbols() {
    return hasSystemSymbols;
  }
  
  
  /**
   * @return Contiki mote type's library class name
   */
  public String getLibraryClassName() {
    return libraryClassName;
  }

  /**
   * Get relative address of variable with given name.
   * 
   * @param varName
   *          Name of variable
   * @return Relative memory address of variable or -1 if not found
   */
  protected int getMapFileVarAddress(Vector<String> mapFileData, String varName) {
    int varAddr;
    String varAddrString;
    if ((varAddrString = varAddresses.getProperty(varName)) != null) {
      varAddr = Integer.parseInt(varAddrString);
      return varAddr;
    }

    String regExp = varAddressRegExpPrefix + varName + varAddressRegExpSuffix;
    String retString = getFirstMatchGroup(mapFileData, regExp, 1);

    if (retString != null) {
      varAddresses.setProperty(varName, Integer.toString(Integer.parseInt(
          retString.trim(), 16)));
      return Integer.parseInt(retString.trim(), 16);
    } else
      return -1;
  }

  private int getReferenceAbsAddr() {
    return myCoreComm.getReferenceAbsAddr();
  }

  private void getCoreMemory(int start, int length, byte[] data) {
    myCoreComm.getMemory(start, length, data);
  }

  private void setCoreMemory(int start, int length, byte[] mem) {
    myCoreComm.setMemory(start, length, mem);
  }

  private static String getFirstMatchGroup(Vector<String> lines, String regexp,
      int groupNr) {
    Pattern pattern = Pattern.compile(regexp);
    for (int i = 0; i < lines.size(); i++) {
      Matcher matcher = pattern.matcher(lines.elementAt(i));
      if (matcher.find()) {
        return matcher.group(groupNr);
      }
    }
    return null;
  }

  /**
   * Returns all variable names in both data and BSS section by parsing the map
   * file. These values should not be trusted completely as the parsing may
   * fail.
   * 
   * @return Variable names found in the data and bss section
   */
  private Vector<String> getMapFileVarNames(Vector<String> mapFileData) {

    Vector<String> varNames = getAllVariableNames(mapFileData,
        loadRelDataSectionAddr(mapFileData),
        loadRelDataSectionAddr(mapFileData) + loadDataSectionSize(mapFileData));

    varNames.addAll(getAllVariableNames(mapFileData,
        loadRelBssSectionAddr(mapFileData), loadRelBssSectionAddr(mapFileData)
            + loadBssSectionSize(mapFileData)));

    return varNames;
  }

  private Vector<String> getAllVariableNames(Vector<String> lines,
      int startAddress, int endAddress) {
    Vector<String> varNames = new Vector<String>();

    Pattern pattern = Pattern.compile(varNameRegExp);
    for (int i = 0; i < lines.size(); i++) {
      Matcher matcher = pattern.matcher(lines.elementAt(i));
      if (matcher.find()) {
        if (Integer.decode(matcher.group(1)).intValue() >= startAddress
            && Integer.decode(matcher.group(1)).intValue() <= endAddress) {
          varNames.add(matcher.group(2));
        }
      }
    }
    return varNames;
  }

  protected int getVariableSize(Vector<String> lines, String varName) {
    Pattern pattern = Pattern.compile(varSizeRegExpPrefix + varName
        + varSizeRegExpSuffix);
    for (int i = 0; i < lines.size(); i++) {
      Matcher matcher = pattern.matcher(lines.elementAt(i));
      if (matcher.find()) {
        return Integer.decode(matcher.group(1));
      }
    }
    return -1;
  }

  private static int loadRelDataSectionAddr(Vector<String> mapFileData) {
    String retString = getFirstMatchGroup(mapFileData, dataSectionAddrRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else
      return 0;
  }

  private static int loadDataSectionSize(Vector<String> mapFileData) {
    String retString = getFirstMatchGroup(mapFileData, dataSectionSizeRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else
      return 0;
  }

  private static int loadRelBssSectionAddr(Vector<String> mapFileData) {
    String retString = getFirstMatchGroup(mapFileData, bssSectionAddrRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else
      return 0;
  }

  private static int loadBssSectionSize(Vector<String> mapFileData) {
    String retString = getFirstMatchGroup(mapFileData, bssSectionSizeRegExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else
      return 0;
  }

  private static int getRelVarAddr(Vector<String> mapFileData, String varName) {
    String regExp = varAddressRegExpPrefix + varName + varAddressRegExpSuffix;
    String retString = getFirstMatchGroup(mapFileData, regExp, 1);

    if (retString != null)
      return Integer.parseInt(retString.trim(), 16);
    else
      return 0;
  }

  private static Vector<String> loadMapFile(File mapFile) {
    Vector<String> mapFileData = new Vector<String>();

    try {
      BufferedReader in = new BufferedReader(new InputStreamReader(
          new FileInputStream(mapFile)));

      while (in.ready()) {
        mapFileData.add(in.readLine());
      }
    } catch (FileNotFoundException e) {
      logger.fatal("File not found: " + e);
      return null;
    } catch (IOException e) {
      logger.fatal("IO error: " + e);
      return null;
    }

    return mapFileData;
  }

  private static Vector<String> loadNmData(File libraryFile) {
    Vector<String> nmData = new Vector<String>();

    try {
      String nmPath = GUI.getExternalToolsSetting("PATH_NM");
      String nmArgs = GUI.getExternalToolsSetting("NM_ARGS");

      if (nmPath == null || nmPath.equals(""))
        return null;

      String[] splittedNmArgs = nmArgs.split(" ");
      String[] nmExecArray = new String[1 + splittedNmArgs.length + 1];

      nmExecArray[0] = nmPath.trim();
      nmExecArray[nmExecArray.length-1] = libraryFile.getAbsolutePath();
      System.arraycopy(splittedNmArgs, 0, nmExecArray, 1, splittedNmArgs.length);

      String line;
      Process p = Runtime.getRuntime().exec(nmExecArray);
      BufferedReader input =
        new BufferedReader
        (new InputStreamReader(p.getInputStream()));
      p.getErrorStream().close(); // Ignore error stream
      while ((line = input.readLine()) != null) {
        nmData.add(line);
      }
      input.close();
    }
    catch (Exception err) {
      err.printStackTrace();
      return null;
    }
    
    if (nmData == null || nmData.size() == 0)
      return null;

    return nmData;
  }

  /**
   * Returns simulation holding this mote type
   * 
   * @return Simulation
   */
  public Simulation getSimulation() {
    return mySimulation;
  }

  /**
   * Sets simulation holding this mote type
   * 
   * @param simulation
   *          Simulation holding this mote type
   */
  public void setSimulation(Simulation simulation) {
    mySimulation = simulation;
  }

  public String getDescription() {
    return description;
  }

  public void setDescription(String newDescription) {
    description = newDescription;
  }

  /**
   * Returns path to contiki base dir
   * 
   * @return String containing path
   */
  public String getContikiBaseDir() {
    return contikiBaseDir;
  }

  /**
   * Sets contiki base dir to path.
   * 
   * @param path
   *          Contiki base dir
   */
  public void setContikiBaseDir(String path) {
    contikiBaseDir = path;
  }

  /**
   * Returns path to contiki core dir
   * 
   * @return String containing path
   */
  public String getContikiCoreDir() {
    return contikiCoreDir;
  }

  /**
   * Sets contiki core dir to path.
   * 
   * @param path
   *          Contiki core dir
   */
  public void setContikiCoreDir(String path) {
    contikiCoreDir = path;
  }

  /**
   * Returns user platform directories
   * 
   * @return User platform directories
   */
  public Vector<File> getUserPlatformDirs() {
    return userPlatformDirs;
  }

  /**
   * Sets user platform directories.
   * 
   * @param dirs
   *          New user platform directories
   */
  public void setUserPlatformDirs(Vector<File> dirs) {
    userPlatformDirs = dirs;
  }

  public PlatformConfig getConfig() {
    return myConfig;
  }

  /**
   * Sets mote type platform configuration. This may differ from the general
   * simulator platform configuration.
   * 
   * @param moteTypeConfig
   *          Platform configuration
   */
  public void setConfig(PlatformConfig moteTypeConfig) {
    myConfig = moteTypeConfig;
  }

  /**
   * Returns all processes of this mote type
   * 
   * @return All processes
   */
  public Vector<String> getProcesses() {
    return processes;
  }

  /**
   * Set startup processes
   * 
   * @param processes
   *          New startup processes
   */
  public void setProcesses(Vector<String> processes) {
    this.processes = processes;
  }

  /**
   * Returns all sensors of this mote type
   * 
   * @return All sensors
   */
  public Vector<String> getSensors() {
    return sensors;
  }

  /**
   * Set sensors
   * 
   * @param sensors
   *          New sensors
   */
  public void setSensors(Vector<String> sensors) {
    this.sensors = sensors;
  }

  /**
   * Returns all core interfaces of this mote type
   * 
   * @return All core interfaces
   */
  public Vector<String> getCoreInterfaces() {
    return coreInterfaces;
  }

  /**
   * Set core interfaces
   * 
   * @param coreInterfaces
   *          New core interfaces
   */
  public void setCoreInterfaces(Vector<String> coreInterfaces) {
    this.coreInterfaces = coreInterfaces;
  }

  /**
   * Returns all mote interfaces of this mote type
   * 
   * @return All mote interfaces
   */
  public Vector<Class<? extends MoteInterface>> getMoteInterfaces() {
    return moteInterfaces;
  }

  /**
   * Set mote interfaces of this mote type
   * 
   * @param moteInterfaces
   *          New mote interfaces
   */
  public void setMoteInterfaces(
      Vector<Class<? extends MoteInterface>> moteInterfaces) {
    this.moteInterfaces = moteInterfaces;
  }

  /**
   * Create a checksum of file. Used for checking if needed files are unchanged
   * when loading a saved simulation.
   * 
   * @param file
   *          File containg data to checksum
   * @return Checksum
   */
  protected byte[] createChecksum(File file) {
    int bytesRead = 1;
    byte[] readBytes = new byte[128];
    MessageDigest messageDigest;

    try {
      InputStream fileInputStream = new FileInputStream(file);
      messageDigest = MessageDigest.getInstance("MD5");

      while (bytesRead > 0) {
        bytesRead = fileInputStream.read(readBytes);
        if (bytesRead > 0)
          messageDigest.update(readBytes, 0, bytesRead);
      }
      fileInputStream.close();
    } catch (NoSuchAlgorithmException e) {
      return null;
    } catch (IOException e) {
      return null;
    }
    return messageDigest.digest();
  }

  /**
   * Returns a panel with interesting data for this mote type.
   * 
   * @return Mote type visualizer
   */
  public JPanel getTypeVisualizer() {
    JPanel panel = new JPanel();
    JLabel label = new JLabel();
    JPanel smallPane;

    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    // Identifier
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Identifier");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(identifier);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Description
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Description");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(description);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Contiki dir
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Contiki path");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(contikiBaseDir);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Library class name
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("JNI Class");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(libraryClassName);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Processes
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Processes");
    smallPane.add(BorderLayout.WEST, label);
    panel.add(smallPane);

    for (String process : processes) {
      smallPane = new JPanel(new BorderLayout());
      label = new JLabel(process);
      smallPane.add(BorderLayout.EAST, label);
      panel.add(smallPane);
    }

    // Sensors
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Sensors");
    smallPane.add(BorderLayout.WEST, label);
    panel.add(smallPane);

    for (String sensor : sensors) {
      smallPane = new JPanel(new BorderLayout());
      label = new JLabel(sensor);
      smallPane.add(BorderLayout.EAST, label);
      panel.add(smallPane);
    }

    // Core Interfaces
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Core interfaces");
    smallPane.add(BorderLayout.WEST, label);
    panel.add(smallPane);

    for (String mInterface : coreInterfaces) {
      smallPane = new JPanel(new BorderLayout());
      label = new JLabel(mInterface);
      smallPane.add(BorderLayout.EAST, label);
      panel.add(smallPane);
    }

    // Mote Interfaces
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Mote interfaces");
    smallPane.add(BorderLayout.WEST, label);
    panel.add(smallPane);

    for (Class moteInterface : moteInterfaces) {
      smallPane = new JPanel(new BorderLayout());
      label = new JLabel(moteInterface.getSimpleName());
      smallPane.add(BorderLayout.EAST, label);
      panel.add(smallPane);
    }

    panel.add(Box.createRigidArea(new Dimension(0, 5)));
    return panel;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Identifier
    element = new Element("identifier");
    element.setText(getIdentifier());
    config.add(element);

    // Description
    element = new Element("description");
    element.setText(getDescription());
    config.add(element);

    // Contiki base directory
    element = new Element("contikibasedir");
    element.setText(getContikiBaseDir());
    config.add(element);

    // Contiki core directory
    element = new Element("contikicoredir");
    element.setText(getContikiCoreDir());
    config.add(element);

    // User platform directory
    for (File userPlatform: userPlatformDirs) {
      element = new Element("userplatformdir");
      element.setText(userPlatform.getPath());
      config.add(element);
    }
    
    // Contiki processes
    for (String process : getProcesses()) {
      element = new Element("process");
      element.setText(process);
      config.add(element);
    }

    // Contiki sensors
    for (String sensor : getSensors()) {
      element = new Element("sensor");
      element.setText(sensor);
      config.add(element);
    }

    // Mote interfaces
    for (Class moteInterface : getMoteInterfaces()) {
      element = new Element("moteinterface");
      element.setText(moteInterface.getName());
      config.add(element);
    }

    // Core interfaces
    for (String coreInterface : getCoreInterfaces()) {
      element = new Element("coreinterface");
      element.setText(coreInterface);
      config.add(element);
    }

    // Has system symbols
    element = new Element("symbols");
    element.setText(new Boolean(hasSystemSymbols).toString());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML) {
    userPlatformDirs = new Vector<File>();
    processes = new Vector<String>();
    sensors = new Vector<String>();
    coreInterfaces = new Vector<String>();
    moteInterfaces = new Vector<Class<? extends MoteInterface>>();
    mySimulation = simulation;

    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("identifier")) {
        identifier = element.getText();
      } else if (name.equals("description")) {
        description = element.getText();
      } else if (name.equals("contikibasedir")) {
        contikiBaseDir = element.getText();
      } else if (name.equals("contikicoredir")) {
        contikiCoreDir = element.getText();
      } else if (name.equals("userplatformdir")) {
        userPlatformDirs.add(new File(element.getText()));
      } else if (name.equals("process")) {
        processes.add(element.getText());
      } else if (name.equals("sensor")) {
        sensors.add(element.getText());
      } else if (name.equals("symbols")) {
        hasSystemSymbols = Boolean.parseBoolean(element.getText());
      } else if (name.equals("coreinterface")) {
        coreInterfaces.add(element.getText());
      } else if (name.equals("moteinterface")) {
        Class<? extends MoteInterface> moteInterfaceClass = 
          GUI.currentGUI.tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.warn("Can't find mote interface class: " + element.getText());
        } else
          moteInterfaces.add(moteInterfaceClass);
      } else {
        logger.fatal("Unrecognized entry in loaded configuration: " + name);
      }
    }

    boolean createdOK = configureAndInit(GUI.frame, simulation);
    return createdOK;
  }

}
