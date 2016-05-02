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
 */
package org.contikios.cooja.contikimote;

import java.awt.Container;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.reflect.Method;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Random;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.JComponent;
import javax.swing.JLabel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.AbstractionLevelDescription;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.CoreComm;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.ProjectConfig;
import org.contikios.cooja.mote.memory.SectionMoteMemory;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.dialogs.CompileContiki;
import org.contikios.cooja.dialogs.ContikiMoteCompileDialog;
import org.contikios.cooja.dialogs.MessageListUI;
import org.contikios.cooja.dialogs.MessageContainer;
import org.contikios.cooja.mote.memory.ArrayMemory;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.mote.memory.MemoryInterface.Symbol;
import org.contikios.cooja.mote.memory.MemoryLayout;
import org.contikios.cooja.mote.memory.UnknownVariableException;
import org.contikios.cooja.mote.memory.VarMemory;
import org.contikios.cooja.util.StringUtils;

/**
 * The Cooja mote type holds the native library used to communicate with an
 * underlying Contiki system. All communication with that system should always
 * pass through this mote type.
 * <p>
 * This type also contains information about sensors and mote interfaces a mote
 * of this type has.
 * <p>
 * All core communication with the Cooja mote should be via this class. When a
 * mote type is created it allocates a CoreComm to be used with this type, and
 * loads the variable and segments addresses.
 * <p>
 * When a new mote type is created an initialization function is run on the
 * Contiki system in order to create the initial memory. When a new mote is
 * created the createInitialMemory() method should be called to get this initial
 * memory for the mote.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Cooja mote")
@AbstractionLevelDescription("OS level")
public class ContikiMoteType implements MoteType {

  private static final Logger logger = Logger.getLogger(ContikiMoteType.class);

  public static final String ID_PREFIX = "mtype";

  /**
   * Library file suffix
   */
  final static public String librarySuffix = ".cooja";

  /**
   * Map file suffix
   */
  final static public String mapSuffix = ".map";

  /**
   * Make archive file suffix
   */
  final static public String dependSuffix = ".a";

  /**
   * Temporary output directory
   */
  final static public File tempOutputDirectory = new File("obj_cooja");

  /**
   * Communication stacks in Contiki.
   */
  public enum NetworkStack {

    DEFAULT, MANUAL;
    public String manualHeader = "netstack-conf-example.h";

    @Override
    public String toString() {
      if (this == DEFAULT) {
        return "Default (from contiki-conf.h)";
      } else if (this == MANUAL) {
        return "Manual netstack header";
      }
      return "[unknown]";
    }

    public String getHeaderFile() {
      if (this == DEFAULT) {
        return null;
      } else if (this == MANUAL) {
        return manualHeader;
      }
      return null;
    }

    public String getConfig() {
      if (this == DEFAULT) {
        return "DEFAULT";
      } else if (this == MANUAL) {
        return "MANUAL:" + manualHeader;
      }
      return "[unknown]";
    }

    public static NetworkStack parseConfig(String config) {
      if (config.equals("DEFAULT")) {
        return DEFAULT;
      } else if (config.startsWith("MANUAL")) {
        NetworkStack st = MANUAL;
        st.manualHeader = config.split(":")[1];
        return st;
      }

      /* TODO Backwards compatibility */
      logger.warn("Bad network stack config: '" + config + "', using default");
      return DEFAULT;
    }
  }

  private final String[] sensors = {"button_sensor", "pir_sensor", "vib_sensor"};

  private String identifier = null;
  private String description = null;
  private File fileSource = null;
  private File fileFirmware = null;
  private String compileCommands = null;

  /* For internal use only: using during Contiki compilation. */
  private File contikiApp = null; /* Contiki application: hello-world.c */

  public File libSource = null; /* JNI library: obj_cooja/mtype1.c */

  public File libFile = null; /* JNI library: obj_cooja/mtype1.lib */

  public File archiveFile = null; /* Contiki archive: obj_cooja/mtype1.a */

  public File mapFile = null; /* Contiki map: obj_cooja/mtype1.map */

  public String javaClassName = null; /* Loading Java class name: Lib1 */

  private String[] coreInterfaces = null;

  private ArrayList<Class<? extends MoteInterface>> moteInterfacesClasses = null;

  private boolean hasSystemSymbols = false;

  private NetworkStack netStack = NetworkStack.DEFAULT;

  // Type specific class configuration
  private ProjectConfig myConfig = null;

  private CoreComm myCoreComm = null;

  // Initial memory for all motes of this type
  private SectionMoteMemory initialMemory = null;

  /** Offset between native (cooja) and contiki address space */
  long offset;

  /**
   * Creates a new uninitialized Cooja mote type. This mote type needs to load
   * a library file and parse a map file before it can be used.
   */
  public ContikiMoteType() {
  }

  @Override
  public Mote generateMote(Simulation simulation) {
    return new ContikiMote(this, simulation);
  }

  @Override
  public boolean configureAndInit(Container parentContainer, Simulation simulation,
                                  boolean visAvailable) throws MoteTypeCreationException {
    myConfig = simulation.getCooja().getProjectConfig().clone();

    if (visAvailable) {

      if (getDescription() == null) {
        setDescription("Cooja Mote Type #" + (simulation.getMoteTypes().length + 1));
      }

      /* Compile Contiki from dialog */
      boolean compileOK
              = ContikiMoteCompileDialog.showDialog(parentContainer, simulation, this);
      if (!compileOK) {
        return false;
      }

    } else {
      if (getIdentifier() == null) {
        throw new MoteTypeCreationException("No identifier specified");
      }
      if (getContikiSourceFile() == null) {
        throw new MoteTypeCreationException("No Contiki application specified");
      }

      /* Create variables used for compiling Contiki */
      contikiApp = getContikiSourceFile();
      libSource = new File(
              contikiApp.getParentFile(),
              "obj_cooja/" + getIdentifier() + ".c");
      libFile = new File(
              contikiApp.getParentFile(),
              "obj_cooja/" + getIdentifier() + librarySuffix);
      archiveFile = new File(
              contikiApp.getParentFile(),
              "obj_cooja/" + getIdentifier() + dependSuffix);
      mapFile = new File(
              contikiApp.getParentFile(),
              "obj_cooja/" + getIdentifier() + mapSuffix);
      javaClassName = CoreComm.getAvailableClassName();

      if (javaClassName == null) {
        throw new MoteTypeCreationException("Could not allocate a core communicator.");
      }

      /* Delete output files */
      libSource.delete();
      libFile.delete();
      archiveFile.delete();
      mapFile.delete();

      /* Generate Contiki main source */
      /*try {
       CompileContiki.generateSourceFile(
       libSource,
       javaClassName,
       getSensors(),
       getCoreInterfaces()
       );
       } catch (Exception e) {
       throw (MoteTypeCreationException) new MoteTypeCreationException(
       "Error when generating Contiki main source").initCause(e);
       }*/

      /* Prepare compiler environment */
      String[][] env;
      try {
        env = CompileContiki.createCompilationEnvironment(
                getIdentifier(),
                contikiApp,
                mapFile,
                libFile,
                archiveFile,
                javaClassName);
        CompileContiki.redefineCOOJASources(
                this,
                env
        );
      } catch (Exception e) {
        throw new MoteTypeCreationException("Error when creating environment: " + e.getMessage(), e);
      }
      String[] envOneDimension = new String[env.length];
      for (int i = 0; i < env.length; i++) {
        envOneDimension[i] = env[i][0] + "=" + env[i][1];
      }

      /* Compile Contiki (may consist of several commands) */
      if (getCompileCommands() == null) {
        throw new MoteTypeCreationException("No compile commands specified");
      }
      final MessageListUI compilationOutput = new MessageListUI();
      String[] arr = getCompileCommands().split("\n");
      for (String cmd : arr) {
        if (cmd.trim().isEmpty()) {
          continue;
        }

        try {
          CompileContiki.compile(
                  cmd,
                  envOneDimension,
                  null /* Do not observe output firmware file */,
                  getContikiSourceFile().getParentFile(),
                  null,
                  null,
                  compilationOutput,
                  true
          );
        } catch (Exception e) {
          MoteTypeCreationException newException
                  = new MoteTypeCreationException("Mote type creation failed: " + e.getMessage());
          newException = (MoteTypeCreationException) newException.initCause(e);
          newException.setCompilationOutput(compilationOutput);

          /* Print last 10 compilation errors to console */
          MessageContainer[] messages = compilationOutput.getMessages();
          for (int i = messages.length - 10; i < messages.length; i++) {
            if (i < 0) {
              continue;
            }
            logger.fatal(">> " + messages[i]);
          }

          logger.fatal("Compilation error: " + e.getMessage());
          throw newException;
        }
      }

      /* Make sure compiled firmware exists */
      if (getContikiFirmwareFile() == null
              || !getContikiFirmwareFile().exists()) {
        throw new MoteTypeCreationException("Contiki firmware file does not exist: " + getContikiFirmwareFile());
      }
    }

    /* Load compiled library */
    doInit();
    return true;
  }

  public static File getExpectedFirmwareFile(File source) {
    File parentDir = source.getParentFile();
    String sourceNoExtension = source.getName().substring(0, source.getName().length() - 2);

    return new File(parentDir, sourceNoExtension + librarySuffix);
  }

  /**
   * For internal use.
   *
   * This method creates a core communicator linking a Contiki library and a
   * Java class.
   * It furthermore parses library Contiki memory addresses and creates the
   * initial memory.
   *
   * @throws MoteTypeCreationException
   */
  private void doInit() throws MoteTypeCreationException {

    if (myCoreComm != null) {
      throw new MoteTypeCreationException(
              "Core communicator already used: " + myCoreComm.getClass().getName());
    }

    if (getContikiFirmwareFile() == null
            || !getContikiFirmwareFile().exists()) {
      throw new MoteTypeCreationException("Library file could not be found: " + getContikiFirmwareFile());
    }

    if (this.javaClassName == null) {
      throw new MoteTypeCreationException("Unknown Java class library: " + this.javaClassName);
    }

    // Allocate core communicator class
    logger.info("Creating core communicator between Java class " + javaClassName + " and Contiki library '" + getContikiFirmwareFile().getPath() + "");
    myCoreComm = CoreComm.createCoreComm(this.javaClassName, getContikiFirmwareFile());

    /* Parse addresses using map file
     * or output of command specified in external tools settings (e.g. nm -a )
     */
    boolean useCommand = Boolean.parseBoolean(Cooja.getExternalToolsSetting("PARSE_WITH_COMMAND", "false"));

    SectionParser dataSecParser;
    SectionParser bssSecParser;
    SectionParser commonSecParser;
    SectionParser readonlySecParser = null;

    HashMap<String, Symbol> variables = new HashMap<>();
    if (useCommand) {
      /* Parse command output */
      String[] output = loadCommandData(getContikiFirmwareFile());
      if (output == null) {
        throw new MoteTypeCreationException("No parse command output loaded");
      }

      dataSecParser = new CommandSectionParser(
              output,
              Cooja.getExternalToolsSetting("COMMAND_DATA_START"),
              Cooja.getExternalToolsSetting("COMMAND_DATA_END"),
              Cooja.getExternalToolsSetting("COMMAND_VAR_SEC_DATA"));
      bssSecParser = new CommandSectionParser(
              output,
              Cooja.getExternalToolsSetting("COMMAND_BSS_START"),
              Cooja.getExternalToolsSetting("COMMAND_BSS_END"),
              Cooja.getExternalToolsSetting("COMMAND_VAR_SEC_BSS"));
      commonSecParser = new CommandSectionParser(
              output,
              Cooja.getExternalToolsSetting("COMMAND_COMMON_START"),
              Cooja.getExternalToolsSetting("COMMAND_COMMON_END"),
              Cooja.getExternalToolsSetting("COMMAND_VAR_SEC_COMMON"));
      /* XXX Currently Cooja tries to sync readonly memory */
      readonlySecParser = null;/* new CommandSectionParser(
              output,
              Cooja.getExternalToolsSetting("COMMAND_READONLY_START"),
              Cooja.getExternalToolsSetting("COMMAND_READONLY_END"),
              Cooja.getExternalToolsSetting("COMMAND_VAR_SEC_READONLY"));*/

    } else {
      /* Parse map file */
      if (mapFile == null
              || !mapFile.exists()) {
        throw new MoteTypeCreationException("Map file " + mapFile + " could not be found");
      }
      String[] mapData = loadMapFile(mapFile);
      if (mapData == null) {
        logger.fatal("No map data could be loaded");
        throw new MoteTypeCreationException("No map data could be loaded: " + mapFile);
      }

      dataSecParser = new MapSectionParser(
              mapData,
              Cooja.getExternalToolsSetting("MAPFILE_DATA_START"),
              Cooja.getExternalToolsSetting("MAPFILE_DATA_SIZE"));
      bssSecParser = new MapSectionParser(
              mapData,
              Cooja.getExternalToolsSetting("MAPFILE_BSS_START"),
              Cooja.getExternalToolsSetting("MAPFILE_BSS_SIZE"));
      commonSecParser = new MapSectionParser(
              mapData,
              Cooja.getExternalToolsSetting("MAPFILE_COMMON_START"),
              Cooja.getExternalToolsSetting("MAPFILE_COMMON_SIZE"));
      readonlySecParser = null;

    }

    /* We first need the value of Contiki's referenceVar, which tells us the
     * memory offset between Contiki's variable and the relative addresses that
     * were calculated directly from the library file.
     *
     * This offset will be used in Cooja in the memory abstraction to match
     * Contiki's and Cooja's address spaces */
    {
      SectionMoteMemory tmp = new SectionMoteMemory(variables);
      VarMemory varMem = new VarMemory(tmp);
      tmp.addMemorySection("tmp.data", dataSecParser.parse(0));

      tmp.addMemorySection("tmp.bss", bssSecParser.parse(0));

      try {
        int referenceVar = (int) varMem.getVariable("referenceVar").addr;
        myCoreComm.setReferenceAddress(referenceVar);
      } catch (UnknownVariableException e) {
        throw new MoteTypeCreationException("Error setting reference variable: " + e.getMessage(), e);
      } catch (RuntimeException e) {
          throw new MoteTypeCreationException("Error setting reference variable: " + e.getMessage(), e);
      }

      getCoreMemory(tmp);

      offset = varMem.getIntValueOf("referenceVar") & 0xFFFFFFFFL;
      logger.info(getContikiFirmwareFile().getName()
              + ": offsetting Cooja mote address space: 0x" + Long.toHexString(offset));
    }

    /* Create initial memory: data+bss+optional common */
    initialMemory = new SectionMoteMemory(variables);

    initialMemory.addMemorySection("data", dataSecParser.parse(offset));

    initialMemory.addMemorySection("bss", bssSecParser.parse(offset));

    initialMemory.addMemorySection("common", commonSecParser.parse(offset));

    if (readonlySecParser != null) {
      initialMemory.addMemorySection("readonly", readonlySecParser.parse(offset));
    }

    getCoreMemory(initialMemory);
  }

  /**
   * Abstract base class for concrete section parser class.
   */
  public static abstract class SectionParser {

    private final String[] mapFileData;
    protected int startAddr;
    protected int size;

    public SectionParser(String[] mapFileData) {
      this.mapFileData = mapFileData;
    }

    public String[] getData() {
      return mapFileData;
    }

    public int getStartAddr() {
      return startAddr;
    }

    public int getSize() {
      return size;
    }

    protected abstract void parseStartAddr();

    protected abstract void parseSize();

    abstract Map<String, Symbol> parseSymbols(long offset);

    protected int parseFirstHexInt(String regexp, String[] data) {
      String retString = getFirstMatchGroup(data, regexp, 1);

      if (retString == null || retString.equals("")) {
        return -1;
      }

      return Integer.parseInt(retString.trim(), 16);
    }

    public MemoryInterface parse(long offset) {

      /* Parse start address and size of section */
      parseStartAddr();
      parseSize();

      if (getStartAddr() < 0 || getSize() <= 0) {
        return null;
      }

      Map<String, Symbol> variables = parseSymbols(offset);

      logger.info(String.format("Parsed section at 0x%x ( %d == 0x%x bytes)",
                                getStartAddr() + offset,
                                getSize(),
                                getSize()));

      if (logger.isDebugEnabled()) {
        for (String var : variables.keySet()) {
          logger.debug(String.format("Found Symbol: %s, 0x%x, %d",
                                     var,
                                     variables.get(var).addr,
                                     variables.get(var).size));
        }
      }

      return new ArrayMemory(
              getStartAddr() + offset,
              getSize(),
              MemoryLayout.getNative(),
              variables);
    }

  }

  /**
   * Parses Map file for seciton data.
   */
  public static class MapSectionParser extends SectionParser {

    private final String startRegExp;
    private final String sizeRegExp;

    public MapSectionParser(String[] mapFileData, String startRegExp, String sizeRegExp) {
      super(mapFileData);
      this.startRegExp = startRegExp;
      this.sizeRegExp = sizeRegExp;
    }

    @Override
    protected void parseStartAddr() {
      if (startRegExp == null || startRegExp.equals("")) {
        startAddr = -1;
        return;
      }
      startAddr = parseFirstHexInt(startRegExp, getData());
    }

    @Override
    protected void parseSize() {
      if (sizeRegExp == null || sizeRegExp.equals("")) {
        size = -1;
        return;
      }
      size = parseFirstHexInt(sizeRegExp, getData());
    }

    @Override
    public Map<String, Symbol> parseSymbols(long offset) {
      Map<String, Symbol> varNames = new HashMap<>();

      Pattern pattern = Pattern.compile(Cooja.getExternalToolsSetting("MAPFILE_VAR_NAME"));
      for (String line : getData()) {
        Matcher matcher = pattern.matcher(line);
        if (matcher.find()) {
          if (Integer.decode(matcher.group(1)).intValue() >= getStartAddr()
                  && Integer.decode(matcher.group(1)).intValue() <= getStartAddr() + getSize()) {
            String varName = matcher.group(2);
            varNames.put(varName, new Symbol(
                    Symbol.Type.VARIABLE,
                    varName,
                    getMapFileVarAddress(getData(), varName) + offset,
                    getMapFileVarSize(getData(), varName)));
          }
        }
      }
      return varNames;
    }

    /**
     * Get relative address of variable with given name.
     *
     * @param varName Name of variable
     * @return Relative memory address of variable or -1 if not found
     */
    private int getMapFileVarAddress(String[] mapFileData, String varName) {

      String regExp = Cooja.getExternalToolsSetting("MAPFILE_VAR_ADDRESS_1")
              + varName
              + Cooja.getExternalToolsSetting("MAPFILE_VAR_ADDRESS_2");
      String retString = getFirstMatchGroup(mapFileData, regExp, 1);

      if (retString != null) {
        return Integer.parseInt(retString.trim(), 16);
      } else {
        return -1;
      }
    }

    private int getMapFileVarSize(String[] mapFileData, String varName) {
      Pattern pattern = Pattern.compile(
              Cooja.getExternalToolsSetting("MAPFILE_VAR_SIZE_1")
              + varName
              + Cooja.getExternalToolsSetting("MAPFILE_VAR_SIZE_2"));
      for (int idx = 0; idx < mapFileData.length; idx++) {
        String parseString = mapFileData[idx];
        Matcher matcher = pattern.matcher(parseString);
        if (matcher.find()) {
          return Integer.decode(matcher.group(1));
        }
        // second approach with lines joined
        if (idx < mapFileData.length - 1) {
          parseString += mapFileData[idx + 1];
        }
        matcher = pattern.matcher(parseString);
        if (matcher.find()) {
          return Integer.decode(matcher.group(1));
        }
      }
      return -1;
    }
  }

  /**
   * Parses command output for section data.
   */
  public static class CommandSectionParser extends SectionParser {

    private final String startRegExp;
    private final String endRegExp;
    private final String sectionRegExp;

    /**
     * Creates SectionParser based on output of configurable command.
     * 
     * @param mapFileData Map file lines as array of String
     * @param startRegExp Regular expression for parsing start of section
     * @param endRegExp Regular expression for parsing end of section
     * @param sectionRegExp Reqular expression describing symbol table section identifier (e.g. '[Rr]' for readonly)
     *        Will be used to replaced '<SECTION>'in 'COMMAND_VAR_NAME_ADDRESS_SIZE'
     */
    public CommandSectionParser(String[] mapFileData, String startRegExp, String endRegExp, String sectionRegExp) {
      super(mapFileData);
      this.startRegExp = startRegExp;
      this.endRegExp = endRegExp;
      this.sectionRegExp = sectionRegExp;
    }

    @Override
    protected void parseStartAddr() {
      if (startRegExp == null || startRegExp.equals("")) {
        startAddr = -1;
        return;
      }
      startAddr = parseFirstHexInt(startRegExp, getData());
    }

    @Override
    public void parseSize() {
      if (endRegExp == null || endRegExp.equals("")) {
        size = -1;
        return;
      }

      if (getStartAddr() < 0) {
        size = -1;
        return;
      }

      int end = parseFirstHexInt(endRegExp, getData());
      if (end < 0) {
        size = -1;
        return;
      }
      size = end - getStartAddr();
    }

    @Override
    public Map<String, Symbol> parseSymbols(long offset) {
      HashMap<String, Symbol> addresses = new HashMap<>();
      /* Replace "<SECTION>" in regexp by section specific regex */
      Pattern pattern = Pattern.compile(
              Cooja.getExternalToolsSetting("COMMAND_VAR_NAME_ADDRESS_SIZE")
                      .replace("<SECTION>", sectionRegExp));

      for (String line : getData()) {
        Matcher matcher = pattern.matcher(line);

        if (matcher.find()) {
          /* Line matched variable address */
          String symbol = matcher.group(1);
          long varAddr = Integer.parseInt(matcher.group(2), 16) + offset;
          int varSize;
          if (matcher.group(3) != null) {
           varSize = Integer.parseInt(matcher.group(3), 16);
          } else {
            varSize = -1;
          }

          /* XXX needs to be checked */
          if (!addresses.containsKey(symbol)) {
            addresses.put(symbol, new Symbol(Symbol.Type.VARIABLE, symbol, varAddr, varSize));
          } else {
            int oldAddress = (int) addresses.get(symbol).addr;
            if (oldAddress != varAddr) {
              /*logger.warn("Warning, command response not matching previous entry of: "
               + varName);*/
            }
          }
        }
      }

      return addresses;
    }
  }

  /**
   * Ticks the currently loaded mote. This should not be used directly, but
   * rather via {@link ContikiMote#execute(long)}.
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
   * Copy core memory to given memory. This should not be used directly, but
   * instead via ContikiMote.getMemory().
   *
   * @param mem
   *          Memory to set
   */
  public void getCoreMemory(SectionMoteMemory mem) {
    for (MemoryInterface section : mem.getSections().values()) {
      getCoreMemory(
              (int) (section.getStartAddr() - offset),
              section.getTotalSize(),
              section.getMemory());
    }
  }

  private void getCoreMemory(int relAddr, int length, byte[] data) {
    myCoreComm.getMemory(relAddr, length, data);
  }

  /**
   * Copy given memory to the Contiki system. This should not be used directly,
   * but instead via ContikiMote.setMemory().
   *
   * @param mem
   * New memory
   */
  public void setCoreMemory(SectionMoteMemory mem) {
    for (MemoryInterface section : mem.getSections().values()) {
      setCoreMemory(
              (int) (section.getStartAddr() - offset),
              section.getTotalSize(),
              section.getMemory());
    }
  }

  private void setCoreMemory(int relAddr, int length, byte[] mem) {
    myCoreComm.setMemory(relAddr, length, mem);
  }

  @Override
  public String getIdentifier() {
    return identifier;
  }

  @Override
  public void setIdentifier(String identifier) {
    this.identifier = identifier;
  }

  @Override
  public File getContikiSourceFile() {
    return fileSource;
  }

  @Override
  public void setContikiSourceFile(File file) {
    fileSource = file;
  }

  @Override
  public File getContikiFirmwareFile() {
    return fileFirmware;
  }

  @Override
  public void setContikiFirmwareFile(File file) {
    fileFirmware = file;
  }

  @Override
  public String getCompileCommands() {
    return compileCommands;
  }

  @Override
  public void setCompileCommands(String commands) {
    this.compileCommands = commands;
  }

  /**
   * @param symbols Core library has system symbols information
   */
  public void setHasSystemSymbols(boolean symbols) {
    hasSystemSymbols = symbols;
  }

  /**
   * @return Whether core library has system symbols information
   */
  public boolean hasSystemSymbols() {
    return hasSystemSymbols;
  }

  /**
   * @param netStack Contiki network stack
   */
  public void setNetworkStack(NetworkStack netStack) {
    this.netStack = netStack;
  }

  /**
   * @return Contiki network stack
   */
  public NetworkStack getNetworkStack() {
    return netStack;
  }

  private static String getFirstMatchGroup(String[] lines, String regexp, int groupNr) {
    if (regexp == null) {
      return null;
    }
    Pattern pattern = Pattern.compile(regexp);
    for (String line : lines) {
      Matcher matcher = pattern.matcher(line);
      if (matcher.find()) {
        return matcher.group(groupNr);
      }
    }
    return null;
  }

  public static String[] loadMapFile(File mapFile) {
    String contents = StringUtils.loadFromFile(mapFile);
    if (contents == null) {
      return null;
    }
    return contents.split("\n");
  }

  /**
   * Executes configured command on given file and returns the result.
   *
   * @param libraryFile Contiki library
   * @return Execution response, or null at failure
   */
  public static String[] loadCommandData(File libraryFile) {
    ArrayList<String> output = new ArrayList<>();

    try {
      String command = Cooja.getExternalToolsSetting("PARSE_COMMAND");
      if (command == null) {
        return null;
      }

      /* Prepare command */
      command = command.replace("$(LIBFILE)",
                                libraryFile.getName().replace(File.separatorChar, '/'));

      /* Execute command, read response */
      String line;
      Process p = Runtime.getRuntime().exec(
              command.split(" "),
              null,
              libraryFile.getParentFile()
      );
      BufferedReader input = new BufferedReader(
              new InputStreamReader(p.getInputStream())
      );
      p.getErrorStream().close();
      while ((line = input.readLine()) != null) {
        output.add(line);
      }
      input.close();

      if (output == null || output.isEmpty()) {
        return null;
      }
      return output.toArray(new String[0]);
    } catch (IOException err) {
      logger.fatal("Command error: " + err.getMessage(), err);
      return null;
    }
  }

  @Override
  public String getDescription() {
    return description;
  }

  @Override
  public void setDescription(String newDescription) {
    description = newDescription;
  }

  @Override
  public ProjectConfig getConfig() {
    return myConfig;
  }

  /**
   * Sets mote type project configuration. This may differ from the general
   * simulator project configuration.
   *
   * @param moteTypeConfig
   * Project configuration
   */
  public void setConfig(ProjectConfig moteTypeConfig) {
    myConfig = moteTypeConfig;
  }

  /**
   * Returns all sensors of this mote type
   *
   * @return All sensors
   */
  public String[] getSensors() {
    return sensors;
  }

  /**
   * Returns all core interfaces of this mote type
   *
   * @return All core interfaces
   */
  public String[] getCoreInterfaces() {
    return coreInterfaces;
  }

  /**
   * Set core interfaces
   *
   * @param coreInterfaces
   * New core interfaces
   */
  public void setCoreInterfaces(String[] coreInterfaces) {
    this.coreInterfaces = coreInterfaces;
  }

  @Override
  public Class<? extends MoteInterface>[] getMoteInterfaceClasses() {
    if (moteInterfacesClasses == null) {
      return null;
    }
    Class<? extends MoteInterface>[] arr = new Class[moteInterfacesClasses.size()];
    moteInterfacesClasses.toArray(arr);
    return arr;
  }

  @Override
  public void setMoteInterfaceClasses(Class<? extends MoteInterface>[] moteInterfaces) {
    this.moteInterfacesClasses = new ArrayList<>();
    this.moteInterfacesClasses.addAll(Arrays.asList(moteInterfaces));
  }

  /**
   * Create a checksum of file. Used for checking if needed files are unchanged
   * when loading a saved simulation.
   *
   * @param file
   * File containg data to checksum
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
        if (bytesRead > 0) {
          messageDigest.update(readBytes, 0, bytesRead);
        }
      }
      fileInputStream.close();
    } catch (NoSuchAlgorithmException | IOException e) {
      return null;
    }
    return messageDigest.digest();
  }

  /**
   * Generates a unique Cooja mote type ID.
   *
   * @param existingTypes Already existing mote types, may be null
   * @param reservedIdentifiers Already reserved identifiers, may be null
   * @return Unique mote type ID.
   */
  public static String generateUniqueMoteTypeID(MoteType[] existingTypes, Collection reservedIdentifiers) {
    String testID = "";
    boolean okID = false;

    while (!okID) {
      testID = ID_PREFIX + (new Random().nextInt(1000));
      okID = true;

      // Check if identifier is reserved
      if (reservedIdentifiers != null && reservedIdentifiers.contains(testID)) {
        okID = false;
      }

      if (!okID) {
        continue;
      }

      // Check if identifier is used
      if (existingTypes != null) {
        for (MoteType existingMoteType : existingTypes) {
          if (existingMoteType.getIdentifier().equals(testID)) {
            okID = false;
            break;
          }
        }
      }

      if (!okID) {
        continue;
      }

      // Check if identifier library has been loaded
      /* XXX Currently only checks the build directory! */
      File libraryFile = new File(
              ContikiMoteType.tempOutputDirectory,
              testID + ContikiMoteType.librarySuffix);
      if (libraryFile.exists() || CoreComm.hasLibraryFileBeenLoaded(libraryFile)) {
        okID = false;
      }
    }

    return testID;
  }

  /**
   * Returns a panel with interesting data for this mote type.
   *
   * @return Mote type visualizer
   */
  @Override
  public JComponent getTypeVisualizer() {
    StringBuilder sb = new StringBuilder();
    // Identifier
    sb.append("<html><table><tr><td>Identifier</td><td>")
            .append(getIdentifier()).append("</td></tr>");

    // Description
    sb.append("<tr><td>Description</td><td>")
            .append(getDescription()).append("</td></tr>");

    /* Contiki application */
    sb.append("<tr><td>Contiki application</td><td>")
            .append(getContikiSourceFile().getAbsolutePath()).append("</td></tr>");

    /* Contiki firmware */
    sb.append("<tr><td>Contiki firmware</td><td>")
            .append(getContikiFirmwareFile().getAbsolutePath()).append("</td></tr>");

    /* JNI class */
    sb.append("<tr><td>JNI library</td><td>")
            .append(this.javaClassName).append("</td></tr>");

    /* Contiki sensors */
    sb.append("<tr><td valign=\"top\">Contiki sensors</td><td>");
    for (String sensor : sensors) {
      sb.append(sensor).append("<br>");
    }
    sb.append("</td></tr>");

    /* Mote interfaces */
    sb.append("<tr><td valign=\"top\">Mote interface</td><td>");
    for (Class<? extends MoteInterface> moteInterface : moteInterfacesClasses) {
      sb.append(moteInterface.getSimpleName()).append("<br>");
    }
    sb.append("</td></tr>");

    /* Contiki core mote interfaces */
    sb.append("<tr><td valign=\"top\">Contiki's mote interface</td><td>");
    for (String coreInterface : getCoreInterfaces()) {
      sb.append(coreInterface).append("<br>");
    }
    sb.append("</td></tr>");

    JLabel label = new JLabel(sb.append("</table></html>").toString());
    label.setVerticalTextPosition(JLabel.TOP);
    return label;
  }

  @Override
  public Collection<Element> getConfigXML(Simulation simulation) {
    ArrayList<Element> config = new ArrayList<>();
    Element element;

    element = new Element("identifier");
    element.setText(getIdentifier());
    config.add(element);

    element = new Element("description");
    element.setText(getDescription());
    config.add(element);

    element = new Element("source");
    File file = simulation.getCooja().createPortablePath(getContikiSourceFile());
    element.setText(file.getPath().replaceAll("\\\\", "/"));
    config.add(element);

    element = new Element("commands");
    element.setText(compileCommands);
    config.add(element);

    for (Class<? extends MoteInterface> moteInterface : getMoteInterfaceClasses()) {
      element = new Element("moteinterface");
      element.setText(moteInterface.getName());
      config.add(element);
    }

    element = new Element("symbols");
    element.setText(new Boolean(hasSystemSymbols()).toString());
    config.add(element);

    if (getNetworkStack() != NetworkStack.DEFAULT) {
      element = new Element("netstack");
      element.setText(getNetworkStack().getConfig());
      config.add(element);
    }

    return config;
  }

  @Override
  public boolean setConfigXML(Simulation simulation,
                              Collection<Element> configXML, boolean visAvailable)
          throws MoteTypeCreationException {
    boolean warnedOldVersion = false;
    File oldVersionSource = null;

    moteInterfacesClasses = new ArrayList<Class<? extends MoteInterface>>();

    for (Element element : configXML) {
      String name = element.getName();
      switch (name) {
        case "identifier":
          identifier = element.getText();
          break;
        case "description":
          description = element.getText();
          break;
        case "contikiapp":
        case "source":
          File file = new File(element.getText());
          if (!file.exists()) {
            file = simulation.getCooja().restorePortablePath(file);
          } setContikiSourceFile(file);
          /* XXX Do not load the generated firmware. Instead, load the unique library file directly */
        File contikiFirmware = new File(
                getContikiSourceFile().getParentFile(),
                "obj_cooja/" + getIdentifier() + librarySuffix);
          setContikiFirmwareFile(contikiFirmware);
          break;
        case "commands":
          compileCommands = element.getText();
          break;
        case "symbols":
          hasSystemSymbols = Boolean.parseBoolean(element.getText());
          break;
        case "commstack":
          logger.warn("The Cooja communication stack config was removed: " + element.getText());
          logger.warn("Instead assuming default network stack.");
          netStack = NetworkStack.DEFAULT;
          break;
        case "netstack":
          netStack = NetworkStack.parseConfig(element.getText());
          break;
        case "moteinterface":
          String intfClass = element.getText().trim();
          /* Backwards compatibility: se.sics -> org.contikios */
          if (intfClass.startsWith("se.sics")) {
            intfClass = intfClass.replaceFirst("se\\.sics", "org.contikios");
          } Class<? extends MoteInterface> moteInterfaceClass
                = simulation.getCooja().tryLoadClass(
                        this, MoteInterface.class, intfClass);
        if (moteInterfaceClass == null) {
          logger.warn("Can't find mote interface class: " + intfClass);
        } else {
          moteInterfacesClasses.add(moteInterfaceClass);
        } break;
        case "contikibasedir":
        case "contikicoredir":
        case "projectdir":
        case "compilefile":
        case "process":
        case "sensor":
        case "coreinterface":
          /* Backwards compatibility: old cooja mote type is being loaded */
          if (!warnedOldVersion) {
            warnedOldVersion = true;
            logger.warn("Old simulation config detected: Cooja mote types may not load correctly");
          } if (name.equals("compilefile")) {
          if (element.getText().endsWith(".c")) {
            File potentialFile = new File(element.getText());
            if (potentialFile.exists()) {
              oldVersionSource = potentialFile;
            }
          }
        } break;
        default:
          logger.fatal("Unrecognized entry in loaded configuration: " + name);
          break;
      }
    }

    /* Create initial core interface dependencies */
    Class<? extends MoteInterface>[] arr
            = new Class[moteInterfacesClasses.size()];
    moteInterfacesClasses.toArray(arr);
    setCoreInterfaces(ContikiMoteType.getRequiredCoreInterfaces(arr));

    /* Backwards compatibility: old cooja mote type is being loaded */
    if (getContikiSourceFile() == null
            && warnedOldVersion
            && oldVersionSource != null) {
      /* Guess Contiki source */
      setContikiSourceFile(oldVersionSource);
      logger.info("Guessing Contiki source: " + oldVersionSource.getAbsolutePath());

      setContikiFirmwareFile(getExpectedFirmwareFile(oldVersionSource));
      logger.info("Guessing Contiki firmware: " + getContikiFirmwareFile().getAbsolutePath());

      /* Guess compile commands */
      String compileCommands
              = "make " + getExpectedFirmwareFile(oldVersionSource).getName() + " TARGET=cooja";
      logger.info("Guessing compile commands: " + compileCommands);
      setCompileCommands(compileCommands);
    }

    boolean createdOK = configureAndInit(Cooja.getTopParentContainer(), simulation, visAvailable);
    return createdOK;
  }

  public static String[] getRequiredCoreInterfaces(
          Class<? extends MoteInterface>[] moteInterfaces) {
    /* Extract Contiki dependencies from currently selected mote interfaces */
    ArrayList<String> coreInterfacesList = new ArrayList<>();
    for (Class<? extends MoteInterface> intf : moteInterfaces) {
      if (!ContikiMoteInterface.class.isAssignableFrom(intf)) {
        continue;
      }

      String[] deps;
      try {
        /* Call static method */
        Method m = intf.getDeclaredMethod("getCoreInterfaceDependencies", (Class[]) null);
        deps = (String[]) m.invoke(null, (Object[]) null);
      } catch (Exception e) {
        logger.warn("Could not extract Contiki dependencies of mote interface: " + intf.getName());
        e.printStackTrace();
        continue;
      }

      if (deps == null || deps.length == 0) {
        continue;
      }
      coreInterfacesList.addAll(Arrays.asList(deps));
    }

    String[] coreInterfaces = new String[coreInterfacesList.size()];
    coreInterfacesList.toArray(coreInterfaces);
    return coreInterfaces;
  }

}
