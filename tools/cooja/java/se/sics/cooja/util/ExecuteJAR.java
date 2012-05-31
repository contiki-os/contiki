/*
 * Copyright (c) 2009, Swedish Institute of Computer Science. All rights
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
 * $Id: ExecuteJAR.java,v 1.10 2010/03/19 15:02:26 fros4943 Exp $
 */

package se.sics.cooja.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URISyntaxException;
import java.security.AccessControlException;
import java.util.Enumeration;
import java.util.List;
import java.util.Properties;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;
import org.jdom.Attribute;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.GUI;
import se.sics.cooja.MoteType;
import se.sics.cooja.Plugin;
import se.sics.cooja.ProjectConfig;
import se.sics.cooja.Simulation;
import se.sics.cooja.dialogs.CompileContiki;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.dialogs.MessageList.MessageContainer;
import se.sics.cooja.plugins.ScriptRunner;

public class ExecuteJAR {
  private static Logger logger = Logger.getLogger(ExecuteJAR.class);

  public final static String SIMCONFIG_FILENAME = "simulation.csc";
  public final static String EXTERNALTOOLS_FILENAME = "exttools.config";
  public final static String PROJECT_DEFAULT_CONFIG_FILENAME = "cooja_default.config";

  public static void main(String[] args) {
    try {
      if ((new File(GUI.LOG_CONFIG_FILE)).exists()) {
        DOMConfigurator.configure(GUI.LOG_CONFIG_FILE);
      } else {
        DOMConfigurator.configure(GUI.class.getResource("/" + GUI.LOG_CONFIG_FILE));
      }
    } catch (AccessControlException e) {
      BasicConfigurator.configure();
    }

    if (args.length > 0) {
      /* Generate executable JAR */
      if (args.length != 2) {
        throw new RuntimeException(
            "Usage: [input .csc] [output .jar]"
        );
      }
      generate(new File(args[0]), new File(args[1]));
    } else {
      /* Run simulation */
      execute();
    }
  }

  private static void generate(File config, File jar) {
    if (!config.exists()) {
      throw new RuntimeException(
          "Simulation config not found: " + config.getAbsolutePath()
      );
    }

    /* Load simulation */
    logger.info("Loading " + config);
    GUI.externalToolsUserSettingsFile = new File(
        System.getProperty("user.home"), 
        GUI.EXTERNAL_TOOLS_USER_SETTINGS_FILENAME);
    Simulation s = GUI.quickStartSimulationConfig(config, false);
    if (s == null) {
      throw new RuntimeException(
          "Error when creating simulation"
      );
    }
    s.stopSimulation();

    try {
      buildExecutableJAR(s.getGUI(), jar);
    } catch (RuntimeException e) {
      logger.fatal(e.getMessage(), e);
      System.exit(1);
    }
    System.exit(0);
  }

  final static boolean OVERWRITE = false;
  private static void execute() {
    String executeDir = null;
    try {
      executeDir = new File(
          ExecuteJAR.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getName();
      if (!executeDir.endsWith(".jar")) {
        logger.fatal("Not a proper JAR archive: " + executeDir);
        System.exit(1);
      }
      executeDir = executeDir.substring(0, executeDir.length()-".jar".length());
      new File(executeDir).mkdir();
    } catch (URISyntaxException e1) {
      logger.fatal("Can't access JAR file name: " + e1.getMessage());
      System.exit(1);
    }

    /* Unpack JAR dependencies - only when they do not already exist! */
    try {
      InputStream inputStream;
      File diskFile = new File(executeDir, SIMCONFIG_FILENAME);
      if (OVERWRITE || !diskFile.exists()) {
        logger.info("Unpacking simulation config: " + SIMCONFIG_FILENAME + " -> " + diskFile.getName());
        inputStream = GUI.class.getResourceAsStream("/" + SIMCONFIG_FILENAME);
        byte[] fileData = ArrayUtils.readFromStream(inputStream);
        if (fileData == null) {
          logger.info("Failed extracting file (read fail)");
          System.exit(1);
        }
        boolean ok = ArrayUtils.writeToFile(diskFile, fileData);
        if (!ok) {
          logger.info("Failed extracting file (write fail)");
          System.exit(1);
        }
      } else {
        logger.info("Skip: simulation config already exists: " + diskFile);
      }

      diskFile = new File(executeDir, EXTERNALTOOLS_FILENAME);
      if (OVERWRITE || !diskFile.exists()) {
        logger.info("Unpacking external tools config: " + EXTERNALTOOLS_FILENAME + " -> " + diskFile.getName());
        inputStream = GUI.class.getResourceAsStream("/" + EXTERNALTOOLS_FILENAME);
        byte[] fileData = ArrayUtils.readFromStream(inputStream);
        if (fileData == null) {
          logger.info("Failed extracting file (read fail)");
          System.exit(1);
        }
        boolean ok = ArrayUtils.writeToFile(diskFile, fileData);
        if (!ok) {
          logger.info("Failed extracting file (write fail)");
          System.exit(1);
        }
      } else {
        logger.info("Skip: external tools config already exists: " + diskFile);
      }
      GUI.externalToolsUserSettingsFile = diskFile;

      /* Unpack files from JAR (with attribute EXPORT=copy) */
      SAXBuilder builder = new SAXBuilder();
      Document doc = builder.build(new File(executeDir, SIMCONFIG_FILENAME));
      handleExportAttributesFromJAR(doc.getRootElement(), new File(executeDir, SIMCONFIG_FILENAME), new File(executeDir));
    } catch (Exception e) {
      logger.fatal("Error when unpacking executable JAR: " + e.getMessage());
      return;
    }

    logger.info("Starting simulation");
    GUI.setLookAndFeel();
    GUI.quickStartSimulationConfig(new File(executeDir, SIMCONFIG_FILENAME), false);
  }

  /**
   * Builds executable JAR from current simulation
   * 
   * @param gui GUI. Must contain simulation
   * @param outputFile Output file
   */
  public static boolean buildExecutableJAR(GUI gui, File outputFile) {
    String executeDir = null;
    executeDir = outputFile.getName();
    if (!executeDir.endsWith(".jar")) {
      throw new RuntimeException("Not a proper JAR archive: " + executeDir);
    }
    executeDir = executeDir.substring(0, executeDir.length()-".jar".length());

    Simulation simulation = gui.getSimulation();
    if (simulation == null) {
      throw new RuntimeException(
          "No simulation active"
      );
    }

    /* Check dependencies: mote type */
    for (MoteType t: simulation.getMoteTypes()) {
      if (!t.getClass().getName().contains("SkyMoteType")) {
        throw new RuntimeException(
            "You simulation contains the mote type: " + GUI.getDescriptionOf(t.getClass()) + "\n" + 
            "Only the Sky Mote Type is currently supported.\n"
        );
      }
      logger.info("Checking mote types: '" + GUI.getDescriptionOf(t.getClass()) + "'");
    }

    /* Check dependencies: Contiki Test Editor */
    boolean hasTestEditor = false;
    for (Plugin startedPlugin : gui.getStartedPlugins()) {
      if (startedPlugin instanceof ScriptRunner) {
        hasTestEditor = true;
        break;
      }
    }
    logger.info("Checking that Contiki Test Editor exists: " + hasTestEditor);
    if (!hasTestEditor) {
      throw new RuntimeException(
          "The simulation needs at least one active Contiki Test Editor plugin.\n" +
          "The plugin is needed to control the non-visualized simulation."
      );
    }

    /* Create temporary directory */
    File workingDir;
    try {
      workingDir = File.createTempFile("cooja", ".tmp");
      workingDir.delete();
      workingDir.mkdir();
      logger.info("Creating temporary directory: " + workingDir.getAbsolutePath());
    } catch (IOException e1) {
      throw (RuntimeException) new RuntimeException(
          "Error when creating temporary directory: " + e1.getMessage()
      ).initCause(e1);
    }

    /* Unpacking project JARs */
    ProjectConfig config = gui.getProjectConfig();
    String[] projectJARs = config.getStringArrayValue(GUI.class.getName() + ".JARFILES");
    for (String jar: projectJARs) {
      /* Locate project */
      File project = config.getUserProjectDefining(GUI.class, "JARFILES", jar);
      File jarFile = new File(project, jar);
      if (!jarFile.exists()) {
        jarFile = new File(project, "lib/" + jar);
      }
      if (!jarFile.exists()) {
        throw new RuntimeException(
            "Project JAR could not be found: " + jarFile.getAbsolutePath()
        );
      }

      logger.info("Unpacking project JAR " + jar + " (" + project + ")");
      try {
        Process unjarProcess = Runtime.getRuntime().exec(
            new String[] { "jar", "xf", jarFile.getAbsolutePath()},
            null,
            workingDir
        );
        unjarProcess.waitFor();
      } catch (Exception e1) {
        throw (RuntimeException)  new RuntimeException(
            "Error unpacking JAR file: " + e1.getMessage()
        ).initCause(e1);
      }
    }

    /* Unpacking COOJA core JARs */
    String[] coreJARs = new String[] {
        "tools/cooja/lib/jdom.jar", "tools/cooja/lib/log4j.jar", "tools/cooja/dist/cooja.jar"
    };
    for (String jar: coreJARs) {
      File jarFile = new File(GUI.getExternalToolsSetting("PATH_CONTIKI"), jar);
      if (!jarFile.exists()) {
        throw new RuntimeException(
            "Project JAR could not be found: " + jarFile.getAbsolutePath()
        );
      }
      logger.info("Unpacking core JAR " + jar);
      try {
        Process unjarProcess = Runtime.getRuntime().exec(
            new String[] { "jar", "xf", jarFile.getAbsolutePath()},
            null,
            workingDir
        );
        unjarProcess.waitFor();
      } catch (Exception e1) {
        throw (RuntimeException) new RuntimeException(
            "Error unpacking JAR file: " + e1.getMessage()
        ).initCause(e1);
      }
    }

    /* Prepare simulation config */
    Element rootElement = gui.extractSimulationConfig();
    logger.info("Extracting simulation configuration");
    handleExportAttributesToJAR(rootElement, gui, workingDir);

    /* Save simulation config */
    File configFile = new File(workingDir, SIMCONFIG_FILENAME);
    try {
      Document doc = new Document(rootElement);
      FileOutputStream out = new FileOutputStream(configFile);
      XMLOutputter outputter = new XMLOutputter();
      outputter.setFormat(Format.getPrettyFormat());
      outputter.output(doc, out);
      out.close();
    } catch (Exception e1) {
      throw (RuntimeException) new RuntimeException(
          "Error when writing simulation configuration: " + configFile
      ).initCause(e1);
    }
    logger.info("Wrote simulation configuration: " + configFile.getName());

    /* Export external tools config (without projects) */
    try {
      File externalToolsConfig = new File(workingDir, EXTERNALTOOLS_FILENAME);
      FileOutputStream out = new FileOutputStream(externalToolsConfig);
      Properties differingSettings = new Properties();
      Enumeration<Object> keyEnum = GUI.currentExternalToolsSettings.keys();
      while (keyEnum.hasMoreElements()) {
        String key = (String) keyEnum.nextElement();
        String defaultSetting = GUI.getExternalToolsDefaultSetting(key, "");
        String currentSetting = GUI.currentExternalToolsSettings.getProperty(key, "");

        if (key.equals("DEFAULT_PROJECTDIRS")) {
          differingSettings.setProperty(key, "");
        } else if (!defaultSetting.equals(currentSetting)) {
          differingSettings.setProperty(key, currentSetting);
        }
      }

      differingSettings.store(out, "Cooja External Tools (User specific)");
      out.close();
      logger.info("Wrote external tools config: " + externalToolsConfig.getName());
    } catch (Exception e2) {
      throw (RuntimeException) new RuntimeException(
          "Error when writing external tools configuration: " + e2.getMessage()   
      ).initCause(e2);
    }

    /* Export current project configuration */
    try {
      ProjectConfig pConfig = gui.getProjectConfig().clone();
      Enumeration<String> pValues = pConfig.getPropertyNames();
      File newConfigFile = new File(workingDir, PROJECT_DEFAULT_CONFIG_FILENAME);
      Properties newConfig = new Properties();
      while (pValues.hasMoreElements()) {
        String name = pValues.nextElement();
        newConfig.setProperty(name, pConfig.getStringValue(name));
      }
      FileOutputStream out = new FileOutputStream(newConfigFile);
      newConfig.store(out, "Cooja Project Config");
      logger.info("Wrote project config: " + newConfigFile.getName());
    } catch (Exception e1) {
      e1.printStackTrace();
      throw (RuntimeException) new RuntimeException(
          "Error when writing project config: " + e1.getMessage()   
      ).initCause(e1);
    }
    
    /* Delete existing META-INF dir */
    File metaInfDir = new File(workingDir, "META-INF");
    if (metaInfDir.exists() && metaInfDir.isDirectory()) {
      if (!deleteDirectory(metaInfDir)) {
        if (!deleteDirectory(metaInfDir)) {
          deleteDirectory(metaInfDir);
        }
      }

    }

    /* Prepare JAR manifest */
    File manifestFile = new File(workingDir, "manifest.tmp");
    if (manifestFile.exists()) {
      manifestFile.delete();
    }
    StringBuilder sb = new StringBuilder();
    sb.append("Manifest-Version: 1.0\r\n");
    sb.append("Main-Class: " + ExecuteJAR.class.getName() + "\r\n");
    sb.append("Class-path: .\r\n");
    StringUtils.saveToFile(manifestFile, sb.toString());
    logger.info("Wrote manifest file: " + manifestFile.getName());

    /* Build executable JAR */
    if (outputFile.exists()) {
      outputFile.delete();
    }

    logger.info("Building executable JAR: " + outputFile);
    MessageList errors = new MessageList();
    try {
      CompileContiki.compile(
          "jar cfm " + outputFile.getAbsolutePath() + " manifest.tmp .",
          null,
          outputFile,
          workingDir,
          null,
          null,
          errors,
          true
      );
    } catch (Exception e) {
      logger.warn("Building executable JAR error: " + e.getMessage());
      MessageContainer[] err = errors.getMessages();
      for (int i=0; i < err.length; i++) {
        logger.fatal(">> " + err[i]);
      }
      
      /* Forward exception */
      throw (RuntimeException) 
      new RuntimeException("Error when building executable JAR: " + e.getMessage()).initCause(e);
    }

    /* Delete temporary working directory */
    logger.info("Deleting temporary files in: " + workingDir.getAbsolutePath());
    if (!deleteDirectory(workingDir)) {
      if (!deleteDirectory(workingDir)) {
        deleteDirectory(workingDir);
      }
    }

    /* We are done! */
    logger.info("Done! To run simulation: > java -jar " + outputFile.getName());
    return true;
  }

  private static void handleExportAttributesToJAR(Element e, GUI gui, File toDir) {
    /* Checks configuration for EXPORT attributes:
     * copy: file copy file to exported JAR, update file path.
     * discard: remove config element */
    
    for (Element c : ((List<Element>) e.getChildren()).toArray(new Element[0])) {
      Attribute a = c.getAttribute("EXPORT");
      if (a != null && a.getValue().equals("copy")) {
        /* Copy file to JAR */
        File file = gui.restorePortablePath(new File(c.getText()));
        if (!file.exists()) {
          throw new RuntimeException("File not found: " + file);
        }     
        byte[] data = ArrayUtils.readFromFile(file);
        if (data == null) {
          throw new RuntimeException("Could not copy file: " + file);
        }

        String newFilename = file.getName();
        while (new File(toDir, newFilename).exists()) {
          newFilename += "-1";
        }
        boolean ok = ArrayUtils.writeToFile(new File(toDir, newFilename), data);
        if (!ok) {
          throw new RuntimeException("Error when copying file: " + file);
        }
        logger.info("Simconfig: Copied file: " + file.getAbsolutePath() + " -> " + ("[CONFIG_DIR]/" + newFilename));
        ((Element)c).setText("[CONFIG_DIR]/" + newFilename);
      } else if (a != null && a.getValue().equals("discard")) {
        /* Remove config element */
        e.removeChild(c.getName());
        logger.info("Simconfig: Discarded element '" + c.getName() + "': " + c.getText());
        continue;
      } else if (a != null) {
        throw new RuntimeException("Unknown EXPORT attribute value: " + a.getValue());
      }
      
      /* Recursive search */
      handleExportAttributesToJAR(c, gui, toDir);
    }
  }
  
  private static void handleExportAttributesFromJAR(Element e, File config, File toDir) {
    for (Element c : ((List<Element>) e.getChildren()).toArray(new Element[0])) {
      Attribute a = c.getAttribute("EXPORT");
      if (a != null && a.getValue().equals("copy")) {
        /* Copy file from JAR */
        File file = GUI.restoreConfigRelativePath(config, new File(c.getText()));
        InputStream inputStream = GUI.class.getResourceAsStream("/" + file.getName());
        if (inputStream == null) {
          throw new RuntimeException("Could not unpack file: " + file);
        }
        byte[] fileData = ArrayUtils.readFromStream(inputStream);
        if (fileData == null) {
          logger.info("Failed unpacking file");
          throw new RuntimeException("Could not unpack file: " + file);
        }
        if (OVERWRITE || !file.exists()) {
          boolean ok = ArrayUtils.writeToFile(file, fileData);
          if (!ok) {
            throw new RuntimeException("Failed unpacking file: " + file);
          }
          logger.info("Unpacked file from JAR: " + file.getName());
        } else if (OVERWRITE) {
          logger.info("Skip: unpack file from JAR: " + file.getName());
        }
      }
      
      /* Recursive search */
      handleExportAttributesFromJAR(c, config, toDir);
    }
  }
  private static boolean deleteDirectory(File path) {
    if(path.exists()) {
      File[] files = path.listFiles();
      for(File file: files) {
        if(file.isDirectory()) {
          deleteDirectory(file);
        } else {
          file.delete();
        }
      }
    }
    return(path.delete());
  }

}
