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
 * $Id: CompileContiki.java,v 1.5 2010/03/10 07:49:25 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.Reader;

import javax.swing.Action;

import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.contikimote.ContikiMoteType;

/**
 * Contiki compiler library.
 * Uses notion of Contiki platforms to compile a Contiki firmware.
 *
 * @author Fredrik Osterlind
 */
public class CompileContiki {
  private static Logger logger = Logger.getLogger(CompileContiki.class);

  /**
   * Executes a Contiki compilation command.
   *
   * @param command Command
   * @param env (Optional) Environment. May be null.
   * @param outputFile Expected output. May be null.
   * @param directory Directory in which to execute command
   * @param onSuccess Action called if compilation succeeds
   * @param onFailure Action called if compilation fails
   * @param compilationOutput Is written both std and err process output
   * @param synchronous If true, method blocks until process completes
   * @return Sub-process if called asynchronously
   * @throws Exception If process returns error, or outputFile does not exist
   */
  public static Process compile(
      final String command,
      final String[] env,
      final File outputFile,
      final File directory,
      final Action onSuccess,
      final Action onFailure,
      final MessageList compilationOutput,
      boolean synchronous)
  throws Exception {

    compilationOutput.addMessage("", MessageList.NORMAL);
    compilationOutput.addMessage("> " + command, MessageList.NORMAL);
    logger.info("> " + command);

    final Process compileProcess;
    try {
      /* TODO Split into correct arguments: parse " and ' */
      compileProcess = Runtime.getRuntime().exec(command, env, directory);

      final BufferedReader processNormal = new BufferedReader(
          new InputStreamReader(compileProcess.getInputStream()));
      final BufferedReader processError = new BufferedReader(
          new InputStreamReader(compileProcess.getErrorStream()));

      if (outputFile != null) {
        if (outputFile.exists()) {
          outputFile.delete();
        }
        if (outputFile.exists()) {
          compilationOutput.addMessage("Error when deleting old " + outputFile.getName(), MessageList.ERROR);
          if (onFailure != null) {
            onFailure.actionPerformed(null);
          }
          throw new MoteTypeCreationException("Error when deleting old " + outputFile.getName());
        }
      }

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          try {
            String readLine;
            while ((readLine = processNormal.readLine()) != null) {
              if (compilationOutput != null) {
                compilationOutput.addMessage(readLine, MessageList.NORMAL);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read input stream thread");

      Thread readError = new Thread(new Runnable() {
        public void run() {
          try {
            String readLine;
            while ((readLine = processError.readLine()) != null) {
              if (compilationOutput != null) {
                compilationOutput.addMessage(readLine, MessageList.ERROR);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read error stream thread");

      final MoteTypeCreationException syncException = new MoteTypeCreationException("");
      Thread handleCompilationResultThread = new Thread(new Runnable() {
        public void run() {

          /* Wait for compilation to end */
          try {
            compileProcess.waitFor();
          } catch (Exception e) {
            compilationOutput.addMessage(e.getMessage(), MessageList.ERROR);
            syncException.setCompilationOutput(new MessageList());
            syncException.fillInStackTrace();
            return;
          }

          /* Check return value */
          if (compileProcess.exitValue() != 0) {
            compilationOutput.addMessage("Process returned error code " + compileProcess.exitValue(), MessageList.ERROR);
            if (onFailure != null) {
              onFailure.actionPerformed(null);
            }
            syncException.setCompilationOutput(new MessageList());
            syncException.fillInStackTrace();
            return;
          }

          if (outputFile == null) {
            /* No firmware to generate: OK */
            if (onSuccess != null) {
              onSuccess.actionPerformed(null);
            }
            return;
          }

          if (!outputFile.exists()) {
            compilationOutput.addMessage("No firmware file: " + outputFile, MessageList.ERROR);
            if (onFailure != null) {
              onFailure.actionPerformed(null);
            }
            syncException.setCompilationOutput(new MessageList());
            syncException.fillInStackTrace();
            return;
          }

          compilationOutput.addMessage("", MessageList.NORMAL);
          compilationOutput.addMessage("Compilation succeded", MessageList.NORMAL);
          if (onSuccess != null) {
            onSuccess.actionPerformed(null);
          }
        }
      }, "handle compilation results");

      readInput.start();
      readError.start();
      handleCompilationResultThread.start();

      if (synchronous) {
        try {
          handleCompilationResultThread.join();
        } catch (Exception e) {
          /* Make sure process has exited */
          compileProcess.destroy();

          String msg = e.getMessage();
          if (e instanceof InterruptedException) {
            msg = "Aborted by user";
          }
          throw (MoteTypeCreationException) new MoteTypeCreationException(
              "Compilation error: " + msg).initCause(e);
        }

        /* Detect error manually */
        if (syncException.hasCompilationOutput()) {
          throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Bad return value").initCause(syncException);
        }
      }
    } catch (IOException ex) {
      if (onFailure != null) {
        onFailure.actionPerformed(null);
      }
      throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Compilation error: " + ex.getMessage()).initCause(ex);
    }

    return compileProcess;
  }

  /**
   * Generate JNI enabled Contiki main source file.
   * Used by Contiki Mote Type.
   *
   * @param sourceFile Source file to generate
   * @param javaClass JNI Java class
   * @param sensors Contiki sensors
   * @param coreInterfaces Core interfaces
   * @throws Exception At error
   *
   * @see ContikiMoteType
   */
  public static void generateSourceFile(
      File sourceFile,
      String javaClass,
      String[] sensors,
      String[] coreInterfaces
  ) throws Exception {

    if (sourceFile == null) {
      throw new Exception("No output source file defined");
    }
    if (javaClass == null) {
      throw new Exception("No Java class defined");
    }
    if (sensors == null) {
      throw new Exception("No Contiki sensors defined");
    }
    if (coreInterfaces == null) {
      throw new Exception("No Contiki dependencies defined");
    }

    /* SENSORS */
    String sensorString = "";
    String externSensorDefs = "";
    for (String sensor : sensors) {
      if (!sensorString.equals("")) {
        sensorString += ", ";
      }
      sensorString += "&" + sensor;
      externSensorDefs += "extern const struct sensors_sensor " + sensor
      + ";\n";
    }

    if (!sensorString.equals("")) {
      sensorString = "SENSORS(" + sensorString + ");";
    } else {
      sensorString = "SENSORS(NULL);";
    }

    /* CORE INTERFACES */
    String interfaceString = "";
    String externInterfaceDefs = "";
    for (String coreInterface : coreInterfaces) {
      if (!interfaceString.equals("")) {
        interfaceString += ", ";
      }
      interfaceString += "&" + coreInterface;
      externInterfaceDefs += "SIM_INTERFACE_NAME(" + coreInterface + ");\n";
    }

    if (!interfaceString.equals("")) {
      interfaceString = "SIM_INTERFACES(" + interfaceString + ");";
    } else {
      interfaceString = "SIM_INTERFACES(NULL);";
    }

    /* If directory does not exist, try creating it */
    if (!sourceFile.getParentFile().exists()) {
      logger.info("Creating source file directory: " + sourceFile.getParentFile().getAbsolutePath());
      sourceFile.getParentFile().mkdir();
    }

    /* GENERATE SOURCE FILE */
    BufferedReader templateReader = null;
    BufferedWriter sourceFileWriter = null;
    try {
      Reader reader;
      String mainTemplate = GUI.getExternalToolsSetting("CONTIKI_MAIN_TEMPLATE_FILENAME");
      if ((new File(mainTemplate)).exists()) {
        reader = new FileReader(mainTemplate);
      } else {
        /* Try JAR, or fail */
        InputStream input = CompileContiki.class.getResourceAsStream('/' + mainTemplate);
        if (input == null) {
          throw new FileNotFoundException(mainTemplate + " not found");
        }
        reader = new InputStreamReader(input);
      }

      templateReader = new BufferedReader(reader);
      sourceFileWriter = new BufferedWriter(new OutputStreamWriter(
          new FileOutputStream(sourceFile)));

      // Replace special fields in template
      String line;
      while ((line = templateReader.readLine()) != null) {
        line = line.replaceFirst("\\[SENSOR_DEFINITIONS\\]", externSensorDefs);
        line = line.replaceFirst("\\[SENSOR_ARRAY\\]", sensorString);

        line = line.replaceFirst("\\[INTERFACE_DEFINITIONS\\]", externInterfaceDefs);
        line = line.replaceFirst("\\[INTERFACE_ARRAY\\]", interfaceString);

        line = line.replaceFirst("\\[CLASS_NAME\\]", javaClass);
        sourceFileWriter.write(line + "\n");
      }
      sourceFileWriter.close();
      templateReader.close();
    } catch (Exception e) {
      try {
        if (templateReader != null) {
          templateReader.close();
        }
        if (sourceFileWriter != null) {
          sourceFileWriter.close();
        }
      } catch (Exception e2) {
      }

      // Forward exception
      throw e;
    }

    if (!sourceFile.exists()) {
      throw new Exception("Output source file does not exist: " + sourceFile.getAbsolutePath());
    }

    logger.info("Generated Contiki main source: " + sourceFile.getName());
  }

  /**
   * Generate compiler environment using external tools settings.
   * Used by Contiki Mote Type.
   *
   * @param identifier Mote type identifier
   * @param contikiApp Contiki application source
   * @param mapFile Expected map file
   * @param libFile Expected JNI Contiki library
   * @param archiveFile Expected Contiki archive
   * @return Environment
   * @throws Exception At errors
   */
  public static String[][] createCompilationEnvironment(
      String identifier,
      File contikiApp,
      File mapFile,
      File libFile,
      File archiveFile)
  throws Exception {

    if (identifier == null) {
      throw new Exception("No identifier specified");
    }
    if (contikiApp == null) {
      throw new Exception("No Contiki application specified");
    }
    if (mapFile == null) {
      throw new Exception("No map file specified");
    }
    if (libFile == null) {
      throw new Exception("No library output specified");
    }
    if (archiveFile == null) {
      throw new Exception("No archive file specified");
    }

    boolean includeSymbols = false; /* TODO */

    /* Fetch configuration from external tools */
    String link1 = GUI.getExternalToolsSetting("LINK_COMMAND_1", "");
    String link2 = GUI.getExternalToolsSetting("LINK_COMMAND_2", "");
    String ar1 = GUI.getExternalToolsSetting("AR_COMMAND_1", "");
    String ar2 = GUI.getExternalToolsSetting("AR_COMMAND_2", "");
    String ccFlags = GUI.getExternalToolsSetting("COMPILER_ARGS", "");

    /* Replace MAPFILE variable */
    link1 = link1.replace("$(MAPFILE)", "obj_cooja/" + mapFile.getName());
    link2 = link2.replace("$(MAPFILE)", "obj_cooja/" + mapFile.getName());
    ar1 = ar1.replace("$(MAPFILE)", "obj_cooja/" + mapFile.getName());
    ar2 = ar2.replace("$(MAPFILE)", "obj_cooja/" + mapFile.getName());
    ccFlags = ccFlags.replace("$(MAPFILE)", "obj_cooja/" + mapFile.getName());

    /* Replace LIBFILE variable */
    link1 = link1.replace("$(LIBFILE)", "obj_cooja/" + libFile.getName());
    link2 = link2.replace("$(LIBFILE)", "obj_cooja/" + libFile.getName());
    ar1 = ar1.replace("$(LIBFILE)", "obj_cooja/" + libFile.getName());
    ar2 = ar2.replace("$(LIBFILE)", "obj_cooja/" + libFile.getName());
    ccFlags = ccFlags.replace("$(LIBFILE)", "obj_cooja/" + libFile.getName());

    /* Replace ARFILE variable */
    link1 = link1.replace("$(ARFILE)", "obj_cooja/" + archiveFile.getName());
    link2 = link2.replace("$(ARFILE)", "obj_cooja/" + archiveFile.getName());
    ar1 = ar1.replace("$(ARFILE)", "obj_cooja/" + archiveFile.getName());
    ar2 = ar2.replace("$(ARFILE)", "obj_cooja/" + archiveFile.getName());
    ccFlags = ccFlags.replace("$(ARFILE)", "obj_cooja/" + archiveFile.getName());

    /* Replace JAVA_HOME variable */
    String javaHome = System.getenv().get("JAVA_HOME");
    if (javaHome == null) {
      javaHome = "";
    }
    link1 = link1.replace("$(JAVA_HOME)", javaHome);
    link2 = link2.replace("$(JAVA_HOME)", javaHome);
    ar1 = ar1.replace("$(JAVA_HOME)", javaHome);
    ar2 = ar2.replace("$(JAVA_HOME)", javaHome);
    ccFlags = ccFlags.replace("$(JAVA_HOME)", javaHome);

    /* Strip away contiki application .c extension */
    String contikiAppNoExtension = contikiApp.getName().substring(0, contikiApp.getName().length()-2);
    String[][] env = new String[13][];
    env[0] = new String[] { "LIBNAME", identifier };
    env[1] = new String[] { "CONTIKI_APP", contikiAppNoExtension };
    env[2] = new String[] { "COOJA_SOURCEFILES", "" };
    env[3] = new String[] { "CC", GUI.getExternalToolsSetting("PATH_C_COMPILER") };
    env[4] = new String[] { "EXTRA_CC_ARGS", ccFlags };
    env[5] = new String[] { "LD", GUI.getExternalToolsSetting("PATH_LINKER") };
    env[6] = new String[] { "LINK_COMMAND_1", link1 };
    env[7] = new String[] { "LINK_COMMAND_2", link2 };
    env[8] = new String[] { "AR", GUI.getExternalToolsSetting("PATH_AR") };
    env[9] = new String[] { "AR_COMMAND_1", ar1 };
    env[10] = new String[] { "AR_COMMAND_2", ar2 };
    env[11] = new String[] { "SYMBOLS", includeSymbols?"1":"" };
    env[12] = new String[] { "PATH", System.getenv("PATH") };

    return env;
  }
}
