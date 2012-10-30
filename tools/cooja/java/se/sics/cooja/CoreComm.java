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

import java.io.*;
import java.lang.reflect.*;
import java.net.*;
import java.util.Vector;

import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.contikimote.ContikiMoteType;
import se.sics.cooja.dialogs.MessageList;

/**
 * The purpose of corecomm's is communicating with a compiled Contiki system
 * using Java Native Interface (JNI). Each implemented class (named
 * Lib[number]), loads a shared library which belongs to one mote type. The
 * reason for this somewhat strange design is that once loaded, a native library
 * cannot be unloaded in Java (in the current versions available). Therefore if
 * we wish to load several libraries, the names and associated native functions
 * must have unique names. And those names are defined via the calling class in
 * JNI. For example, the corresponding function for a native tick method in
 * class Lib1 will be named Java_se_sics_cooja_corecomm_Lib1_tick. When creating
 * a new mote type, the main Contiki source file is generated with function
 * names compatible with the next available corecomm class. This also implies
 * that even if a mote type is deleted, a new one cannot be created using the
 * same corecomm class without restarting the JVM and thus the entire
 * simulation.
 *
 * Each implemented CoreComm class needs read access to the following core
 * variables:
 * <ul>
 * <li>referenceVar
 * </ul>
 * and the following native functions:
 * <ul>
 * <li>tick()
 * <li>init()
 * <li>getReferenceAbsAddr()
 * <li>getMemory(int start, int length, byte[] mem)
 * <li>setMemory(int start, int length, byte[] mem)
 *
 * @author Fredrik Osterlind
 */
public abstract class CoreComm {

  // Static pointers to current libraries
  private final static Vector<CoreComm> coreComms = new Vector<CoreComm>();

  private final static Vector<File> coreCommFiles = new Vector<File>();

  private static int fileCounter = 1;

  /**
   * Has any library been loaded? Since libraries can't be unloaded the entire
   * simulator may have to be restarted.
   *
   * @return True if any library has been loaded this session
   */
  public static boolean hasLibraryBeenLoaded() {
    return coreComms.size() > 0;
  }

  /**
   * Has given library file already been loaded during this session? A loaded
   * library can be removed, but not unloaded during one session. And a new
   * library file, named the same as an earlier loaded and removed file, can't
   * be loaded either.
   *
   * @param libraryFile
   *          Library file
   * @return True if a library has already been loaded from the given file's
   *         filename
   */
  public static boolean hasLibraryFileBeenLoaded(File libraryFile) {
    for (File loadedFile : coreCommFiles) {
      if (loadedFile.getName().equals(libraryFile.getName())) {
        return true;
      }
    }
    return false;
  }

  /**
   * Get the class name of next free core communicator class. If null is
   * returned, no classes are available.
   *
   * @return Class name
   */
  public static String getAvailableClassName() {
    return "Lib" + fileCounter;
  }

  /**
   * Generates new source file by reading default source template and replacing
   * the class name field.
   *
   * @param className
   *          Java class name (without extension)
   * @throws MoteTypeCreationException
   *           If error occurs
   */
  public static void generateLibSourceFile(String className)
      throws MoteTypeCreationException {
    BufferedWriter sourceFileWriter = null;
    BufferedReader templateFileReader = null;
    String destFilename = null;

    try {
      Reader reader;
      String mainTemplate = GUI
          .getExternalToolsSetting("CORECOMM_TEMPLATE_FILENAME");

      if ((new File(mainTemplate)).exists()) {
        reader = new FileReader(mainTemplate);
      } else {
        InputStream input = CoreComm.class
            .getResourceAsStream('/' + mainTemplate);
        if (input == null) {
          throw new FileNotFoundException(mainTemplate + " not found");
        }
        reader = new InputStreamReader(input);
      }

      templateFileReader = new BufferedReader(reader);
      destFilename = className + ".java";

      File dir = new File("se/sics/cooja/corecomm");
      if (!dir.exists()) {
        dir.mkdirs();
      }

      sourceFileWriter = new BufferedWriter(new OutputStreamWriter(
          new FileOutputStream("se/sics/cooja/corecomm/" + destFilename)));

      // Replace special fields in template
      String line;
      while ((line = templateFileReader.readLine()) != null) {
        line = line.replaceFirst("\\[CLASSNAME\\]", className);
        sourceFileWriter.write(line + "\n");
      }

      sourceFileWriter.close();
      templateFileReader.close();
    } catch (Exception e) {
      try {
        if (sourceFileWriter != null) {
          sourceFileWriter.close();
        }
        if (templateFileReader != null) {
          templateFileReader.close();
        }
      } catch (Exception e2) {
      }

      throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Could not generate corecomm source file: " + className + ".java")
          .initCause(e);
    }

    File genFile = new File("se/sics/cooja/corecomm/" + destFilename);
    if (genFile.exists()) {
      return;
    }

    throw new MoteTypeCreationException(
        "Could not generate corecomm source file: " + className + ".java");
  }

  /**
   * Compiles Java class.
   *
   * @param className
   *          Java class name (without extension)
   * @throws MoteTypeCreationException
   *           If Java class compilation error occurs
   */
  public static void compileSourceFile(String className)
      throws MoteTypeCreationException {
    MessageList compilationOutput = new MessageList();
    OutputStream compilationStandardStream = compilationOutput
        .getInputStream(MessageList.NORMAL);
    OutputStream compilationErrorStream = compilationOutput
        .getInputStream(MessageList.ERROR);

    File classFile = new File("se/sics/cooja/corecomm/" + className + ".class");

    try {
      int b;
      String[] cmd = new String[] { GUI.getExternalToolsSetting("PATH_JAVAC"),
          "-version", "se/sics/cooja/corecomm/" + className + ".java" };

      Process p = Runtime.getRuntime().exec(cmd, null, null);
      InputStream outputStream = p.getInputStream();
      InputStream errorStream = p.getErrorStream();
      while ((b = outputStream.read()) >= 0) {
        compilationStandardStream.write(b);
      }
      while ((b = errorStream.read()) >= 0) {
        compilationErrorStream.write(b);
      }
      p.waitFor();

      if (classFile.exists()) {
        return;
      }

      // Try including cooja.jar
      cmd = new String[] {
          GUI.getExternalToolsSetting("PATH_JAVAC"),
          "-version",
          "se/sics/cooja/corecomm/" + className + ".java",
          "-cp",
          GUI.getExternalToolsSetting("PATH_CONTIKI")
              + "/tools/cooja/dist/cooja.jar" };

      p = Runtime.getRuntime().exec(cmd, null, null);

      outputStream = p.getInputStream();
      errorStream = p.getErrorStream();
      while ((b = outputStream.read()) >= 0) {
        compilationStandardStream.write(b);
      }
      while ((b = errorStream.read()) >= 0) {
        compilationErrorStream.write(b);
      }
      p.waitFor();

      if (classFile.exists()) {
        return;
      }

    } catch (IOException e) {
      MoteTypeCreationException exception = (MoteTypeCreationException) new MoteTypeCreationException(
          "Could not compile corecomm source file: " + className + ".java")
          .initCause(e);
      exception.setCompilationOutput(compilationOutput);
      throw exception;
    } catch (InterruptedException e) {
      MoteTypeCreationException exception = (MoteTypeCreationException) new MoteTypeCreationException(
          "Could not compile corecomm source file: " + className + ".java")
          .initCause(e);
      exception.setCompilationOutput(compilationOutput);
      throw exception;
    }

    MoteTypeCreationException exception = new MoteTypeCreationException(
        "Could not compile corecomm source file: " + className + ".java");
    exception.setCompilationOutput(compilationOutput);
    throw exception;
  }

  /**
   * Loads given Java class file from disk.
   *
   * @param className Java class name
   * @return Loaded class
   * @throws MoteTypeCreationException If error occurs
   */
  public static Class<?> loadClassFile(String className)
      throws MoteTypeCreationException {
    Class<?> loadedClass = null;
    try {
      ClassLoader urlClassLoader = new URLClassLoader(
          new URL[] { new File(".").toURI().toURL() },
          CoreComm.class.getClassLoader());
      loadedClass = urlClassLoader.loadClass("se.sics.cooja.corecomm."
          + className);

    } catch (MalformedURLException e) {
      throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Could not load corecomm class file: " + className + ".class")
          .initCause(e);
    } catch (ClassNotFoundException e) {
      throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Could not load corecomm class file: " + className + ".class")
          .initCause(e);
    }
    if (loadedClass == null) {
      throw new MoteTypeCreationException(
          "Could not load corecomm class file: " + className + ".class");
    }

    return loadedClass;
  }

  /**
   * Create and return an instance of the core communicator identified by
   * className. This core communicator will load the native library libFile.
   *
   * @param className
   *          Class name of core communicator
   * @param libFile
   *          Native library file
   * @return Core Communicator
   */
  public static CoreComm createCoreComm(String className, File libFile)
      throws MoteTypeCreationException {
    generateLibSourceFile(className);

    compileSourceFile(className);

    Class newCoreCommClass = loadClassFile(className);

    try {
      Constructor constr = newCoreCommClass
          .getConstructor(new Class[] { File.class });
      CoreComm newCoreComm = (CoreComm) constr
          .newInstance(new Object[] { libFile });

      coreComms.add(newCoreComm);
      coreCommFiles.add(libFile);
      fileCounter++;

      return newCoreComm;
    } catch (Exception e) {
      throw (MoteTypeCreationException) new MoteTypeCreationException(
          "Error when creating corecomm instance: " + className).initCause(e);
    }
  }

  /**
   * Ticks a mote once. This should not be used directly, but instead via
   * {@link ContikiMoteType#tick()}.
   */
  public abstract void tick();

  /**
   * Initializes a mote by running a startup script in the core. (Should only be
   * run once, at the same time as the library is loaded)
   */
  protected abstract void init();

  /**
   * Sets the relative memory address of the reference variable.
   * Is used by Contiki to map between absolute and relative memory addresses.
   *
   * @param addr Relative address
   */
  public abstract void setReferenceAddress(int addr);

  /**
   * Fills an byte array with memory segment identified by start and length.
   *
   * @param relAddr Relative memory start address
   * @param length Length of segment
   * @param mem Array to fill with memory segment
   */
  public abstract void getMemory(int relAddr, int length, byte[] mem);

  /**
   * Overwrites a memory segment identified by start and length.
   *
   * @param relAddr Relative memory start address
   * @param length Length of segment
   * @param mem New memory segment data
   */
  public abstract void setMemory(int relAddr, int length, byte[] mem);

}
