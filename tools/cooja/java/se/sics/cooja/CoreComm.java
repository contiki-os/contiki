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
 * $Id: CoreComm.java,v 1.5 2007/03/24 00:41:10 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.*;
import java.lang.reflect.*;
import java.net.*;
import java.util.Vector;

/**
 * The purpose of corecomm's is communicating with a compiled Contiki system
 * using Java Native Interface (JNI). Each implemented class (named Lib[number]),
 * loads a shared library which belongs to one mote type. The reason for this
 * somewhat strange design is that once loaded, a native library cannot be
 * unloaded in Java (in the current versions available). Therefore if we wish to
 * load several libraries, the names and associated native functions must have
 * unique names. And those names are defined via the calling class in JNI. For
 * example, the corresponding function for a native tick method in class Lib1
 * will be named Java_se_sics_cooja_corecomm_Lib1_tick. When creating a new mote
 * type, the main Contiki source file is generated with function names
 * compatible with the next available corecomm class. This also implies that
 * even if a mote type is deleted, a new one cannot be created using the same
 * corecomm class without restarting the JVM and thus the entire simulation.
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
    for (File loadedFile : coreCommFiles)
      if (loadedFile != null && loadedFile.getName().equals(libraryFile.getName()))
        return true;
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
   *          Wanted class name
   * @return True if success, false otherwise
   */
  public static boolean generateLibSourceFile(String className) {
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
      if (!dir.exists()) 
        dir.mkdirs();
      
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
        if (sourceFileWriter != null)
          sourceFileWriter.close();
        if (templateFileReader != null)
          templateFileReader.close();
      } catch (Exception e2) {
        return false;
      }

      return false;
    }

    File genFile = new File("se/sics/cooja/corecomm/" + destFilename);
    if (genFile.exists())
      return true;

    return false;
  }

  /**
   * Compile given Java source file.
   * 
   * @param className Class name
   * @return True if success, false otherwise
   */
  private static boolean compileSourceFile(String className) {
    File classFile = new File("se/sics/cooja/corecomm/" + className + ".class");

    try {
      String[] cmd = new String[]{
          GUI.getExternalToolsSetting("PATH_JAVAC"),
          "se/sics/cooja/corecomm/" + className + ".java"};

      Process p = Runtime.getRuntime().exec(cmd, null, null);
      p.waitFor();
      
      if (classFile.exists())
        return true;
      
      // Try including cooja.jar
      cmd = new String[]{
          GUI.getExternalToolsSetting("PATH_JAVAC"),
          "se/sics/cooja/corecomm/" + className + ".java",
          "-cp",
          GUI.getExternalToolsSetting("PATH_CONTIKI") + "/tools/cooja/dist/cooja.jar"};

      p = Runtime.getRuntime().exec(cmd, null, null);
      p.waitFor();
      
    } catch (IOException e) {
      return false;
    } catch (InterruptedException e) {
      return false;
    }

    if (classFile.exists())
      return true;
    
    return false;
  }
  
  /**
   * Load given Java class file, making it ready to be used.
   * 
   * @param classFile Class file
   * @return Loaded class, or null
   */
  private static Class loadClassFile(String className) {
    Class loadedClass = null;
    try {
      ClassLoader urlClassLoader = new URLClassLoader(
          new URL[] { new File(".").toURL() }, 
          CoreComm.class.getClassLoader());
      loadedClass = urlClassLoader.loadClass("se.sics.cooja.corecomm." + className);
    } catch (MalformedURLException e) {
      return null;
    } catch (ClassNotFoundException e) {
      return null;
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
  public static CoreComm createCoreComm(String className, File libFile) {
    if (!generateLibSourceFile(className))
      return null;
    
    if (!compileSourceFile(className))
      return null;

    Class newCoreCommClass = loadClassFile(className);
    if (newCoreCommClass == null)
      return null;

    try {
      Constructor constr = newCoreCommClass.getConstructor(new Class[] { File.class });
      CoreComm newCoreComm = (CoreComm) constr.newInstance(new Object[] { libFile });

      coreComms.add(newCoreComm);
      coreCommFiles.add(libFile);
      fileCounter++;
      
      return newCoreComm;
    } catch (NoSuchMethodException e) {
      return null;
    } catch (InstantiationException e) {
      return null;
    } catch (InvocationTargetException e) {
      return null;
    } catch (IllegalAccessException e) {
      return null;
    }
  }
  
  /**
   * Ticks a mote once. This should not be used directly, but instead via
   * Mote.tick().
   */
  public abstract void tick();

  /**
   * Initializes a mote by running a startup script in the core. (Should only be
   * run once, at the same time as the library is loaded)
   */
  protected abstract void init();

  /**
   * Returns absolute memory location of the core variable referenceVar. Used to
   * get offset between relative and absolute memory addresses.
   * 
   * @return Absolute memory address
   */
  public abstract int getReferenceAbsAddr();

  /**
   * Fills an byte array with memory segment identified by start and length.
   * 
   * @param start
   *          Start address of segment
   * @param length
   *          Length of segment
   * @param mem
   *          Array to fill with memory segment
   */
  public abstract void getMemory(int start, int length, byte[] mem);

  /**
   * Overwrites a memory segment identified by start and length.
   * 
   * @param start
   *          Start address of segment
   * @param length
   *          Length of segment
   * @param mem
   *          New memory segment data
   */
  public abstract void setMemory(int start, int length, byte[] mem);

}
