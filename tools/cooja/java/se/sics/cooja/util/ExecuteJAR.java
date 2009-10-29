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
 * $Id: ExecuteJAR.java,v 1.1 2009/10/29 10:17:15 fros4943 Exp $
 */

package se.sics.cooja.util;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.AccessControlException;
import java.util.Enumeration;
import java.util.Properties;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.xml.DOMConfigurator;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.GUI;
import se.sics.cooja.MoteType;
import se.sics.cooja.Plugin;
import se.sics.cooja.ProjectConfig;
import se.sics.cooja.Simulation;
import se.sics.cooja.plugins.ScriptRunner;

public class ExecuteJAR {
  private static Logger logger = Logger.getLogger(ExecuteJAR.class);

  public final static String EXECUTABLE_JAR_FILENAME = "cooja_simulation.jar";
  public final static String EXTERNALTOOLS_FILENAME = "externaltools.config";
  public final static String SIMCONFIG_FILENAME = "simulation.csc";
  public final static String FIRMWARE_PREFIX = "firmware";
  public final static String FIRMWARE_SUFFIX = ".sky";

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
      /* Generate */
      logger.debug("Generating JAR");
      if (args.length != 1) {
        throw new RuntimeException(
            "Bad command line arguments: specify only one simulation config!"
        );
      }
      generate(new File(args[0]));
    } else {
      logger.debug("Executing JAR");
      execute();
    }
  }

  private static void generate(File file) {
    if (!file.exists()) {
      throw new RuntimeException(
          "Simulation config not found: " + file.getAbsolutePath()
      );
    }

    /* Load simulation */
    logger.debug("Loading " + file);
    Simulation s = GUI.quickStartSimulationConfig(file, false);
    if (s == null) {
      throw new RuntimeException(
          "Error when creating simulation"
      );
    }
    s.stopSimulation();
    logger.debug("Stopped simulation");

    buildExecutableJAR(s.getGUI(), new File("."));
    System.exit(1);
  }
  
  private static void execute() {
    /* Execute */
    GUI.externalToolsUserSettingsFile = new File(EXTERNALTOOLS_FILENAME);

    /* Unpack JAR dependencies - only when they do not already exist! */
    try {
      Process process;
      if (!new File(SIMCONFIG_FILENAME).exists()) {
        logger.info("Unpacking simulation config: " + SIMCONFIG_FILENAME);
        process = Runtime.getRuntime().exec(new String[] { 
            "jar", 
            "xf", 
            EXECUTABLE_JAR_FILENAME, 
            SIMCONFIG_FILENAME,
        });
        process.waitFor();
      } else {
        logger.info("Skip: simulation config alrady exists: " + SIMCONFIG_FILENAME);
      }

      if (!new File(EXTERNALTOOLS_FILENAME).exists()) {
        logger.info("Unpacking external tools config: " + EXTERNALTOOLS_FILENAME);
        process = Runtime.getRuntime().exec(new String[] { 
            "jar", 
            "xf", 
            EXECUTABLE_JAR_FILENAME, 
            EXTERNALTOOLS_FILENAME,
        });
        process.waitFor();
      } else {
        logger.info("Skip: external tools config alrady exists: " + EXTERNALTOOLS_FILENAME);
      }

      if (!new File(FIRMWARE_PREFIX + 0 + FIRMWARE_SUFFIX).exists()) {
        logger.info("Unpacking firmware files (max 10))");
        process = Runtime.getRuntime().exec(new String[] { 
            "jar", 
            "xf", 
            EXECUTABLE_JAR_FILENAME, 
            FIRMWARE_PREFIX + 0 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 1 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 2 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 3 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 4 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 5 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 6 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 7 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 8 + FIRMWARE_SUFFIX,
            FIRMWARE_PREFIX + 9 + FIRMWARE_SUFFIX,
        });
        process.waitFor();
      } else {
        logger.info("Skip: firmware files already exist");
      }
    } catch (Exception e) {
      System.err.println("Error when partially unpacking executable JAR: " + e.getMessage());
      return;
    }

    logger.info("Starting simulation");
    GUI.setLookAndFeel();
    GUI.quickStartSimulationConfig(new File(SIMCONFIG_FILENAME), false);
  }

  /**
   * Builds executable JAR from current simulation
   * 
   * @param gui GUI. Must contain simulation
   */
  public static void buildExecutableJAR(GUI gui, File outputDir) {
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
    logger.info("Simconfig: Stripping project references");
    rootElement.removeChildren("project"); /* Remove project references */
    for (Object simElement : rootElement.getChildren()) {
      if (((Element) simElement).getName().equals("simulation")) {
        int firmwareCounter = 0;
        for (Object moteTypeElement : ((Element)simElement).getChildren()) {
          if (((Element) moteTypeElement).getName().equals("motetype")) {
            logger.info("Simconfig: Stripping source references");
            logger.info("Simconfig: Stripping build commands");
            ((Element)moteTypeElement).removeChildren("source"); /* Remove source references */
            ((Element)moteTypeElement).removeChildren("commands"); /* Remove build commands */
            for (Object pathElement : ((Element)moteTypeElement).getChildren()) {
              if (((Element) pathElement).getName().equals("firmware")) {
                String newName = FIRMWARE_PREFIX + firmwareCounter++ + FIRMWARE_SUFFIX;
                /* Copy firmwares, and update firmware references */
                String firmwarePath = ((Element)pathElement).getText();
                File firmwareFile = gui.restorePortablePath(new File(firmwarePath));
                if (!firmwareFile.exists()) {
                  throw new RuntimeException(
                      "Firmware file does not exist: " + firmwareFile
                  );
                }     
                logger.info("Simconfig: Copying firmware file: " + firmwareFile.getAbsolutePath());
                byte[] data = ArrayUtils.readFromFile(firmwareFile);
                if (data == null) {
                  throw new RuntimeException(
                      "Error when reading firmware file: " + firmwareFile
                  );
                }
                boolean ok = ArrayUtils.writeToFile(new File(workingDir, newName), data);
                if (!ok) {
                  throw new RuntimeException(
                      "Error when writing firmware file: " + firmwareFile
                  );
                }
                logger.info("Simconfig: Update firmware path reference: " + firmwareFile.getAbsolutePath() + " -> " + newName);
                ((Element)pathElement).setText(newName);
              }
            }
          }
        }
      }
    }

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
    logger.info("Wrote simulation configuration: " + configFile.getAbsolutePath());

    /* Export external tools config (without projects) */
    try {
      File externalToolsConfig = new File(workingDir, EXTERNALTOOLS_FILENAME);
      logger.info("Exporting external tools config: " + externalToolsConfig);
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

      differingSettings.store(out, "COOJA External Tools (User specific)");
      out.close();
    } catch (Exception e2) {
      throw (RuntimeException) new RuntimeException(
          "Error when writing external tools configuration: " + e2.getMessage()   
      ).initCause(e2);
    }

    /* Prepare JAR manifest */
    File manifestFile = new File(workingDir, "manifest.tmp");
    if (manifestFile.exists()) {
      manifestFile.delete();
    }
    logger.info("Creating manifest file: " + manifestFile);
    StringBuilder sb = new StringBuilder();
    sb.append("Manifest-Version: 1.0\r\n");
    sb.append("Main-Class: " + ExecuteJAR.class.getName() + "\r\n");
    sb.append("Class-path: .\r\n");
    StringUtils.saveToFile(manifestFile, sb.toString());

    /* Build executable JAR */
    File executableJAR = new File(outputDir, EXECUTABLE_JAR_FILENAME);
    if (executableJAR.exists()) {
      executableJAR.delete();
    }

    logger.info("Building executable JAR: " + executableJAR);
    try {
      Process jarProcess = Runtime.getRuntime().exec(
          new String[] { "jar", "cfm", executableJAR.getAbsolutePath(), "manifest.tmp", "*"},
          null,
          workingDir
      );
      jarProcess.waitFor();
    } catch (Exception e1) {
      throw (RuntimeException) new RuntimeException(
          "Error when building executable JAR: " + e1.getMessage()
      ).initCause(e1);
    }    

    /* We are done! */
    logger.info("Done! You may remove the temporary working directory: " + workingDir.getAbsolutePath());
    logger.info("Run simulation: > java -jar " + EXECUTABLE_JAR_FILENAME);
  }
}
