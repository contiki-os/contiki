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
 * $Id: CoreComm.java,v 1.3 2007/01/10 14:57:42 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.File;
import se.sics.cooja.corecomm.*;

/**
 * The purpose of corecomm's is communicating with a compiled Contiki system
 * using Java Native Interface (JNI). Each implemented class (named Lib[1-MAX]),
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
  /**
   * Maximum supported core communicators in a simulator.
   */
  private final static int MAX_LIBRARIES = 8;

  // Static pointers to current libraries
  private final static CoreComm[] coreComms = new CoreComm[MAX_LIBRARIES];

  private final static File[] coreCommFiles = new File[MAX_LIBRARIES];

  /**
   * Has any library been loaded? Since libraries can't be unloaded the entire
   * simulator may have to be restarted.
   * 
   * @return True if any library has been loaded this session
   */
  public static boolean hasLibraryBeenLoaded() {
    for (int i = 0; i < coreComms.length; i++)
      if (coreComms[i] != null)
        return true;
    return false;
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
    for (File libFile : coreCommFiles)
      if (libFile != null && libFile.getName().equals(libraryFile.getName()))
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
    if (coreComms[0] == null)
      return "Lib1";
    if (coreComms[1] == null)
      return "Lib2";
    if (coreComms[2] == null)
      return "Lib3";
    if (coreComms[3] == null)
      return "Lib4";
    if (coreComms[4] == null)
      return "Lib5";
    if (coreComms[5] == null)
      return "Lib6";
    if (coreComms[6] == null)
      return "Lib7";
    if (coreComms[7] == null)
      return "Lib8";

    return null;
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
    if (className.equals("Lib1") && coreComms[0] == null) {
      coreComms[0] = new Lib1(libFile);
      coreCommFiles[0] = libFile;
      return coreComms[0];
    }
    if (className.equals("Lib2") && coreComms[1] == null) {
      coreComms[1] = new Lib2(libFile);
      coreCommFiles[1] = libFile;
      return coreComms[1];
    }
    if (className.equals("Lib3") && coreComms[2] == null) {
      coreComms[2] = new Lib3(libFile);
      coreCommFiles[2] = libFile;
      return coreComms[2];
    }
    if (className.equals("Lib4") && coreComms[3] == null) {
      coreComms[3] = new Lib4(libFile);
      coreCommFiles[3] = libFile;
      return coreComms[3];
    }
    if (className.equals("Lib5") && coreComms[4] == null) {
      coreComms[4] = new Lib5(libFile);
      coreCommFiles[4] = libFile;
      return coreComms[4];
    }
    if (className.equals("Lib6") && coreComms[5] == null) {
      coreComms[5] = new Lib6(libFile);
      coreCommFiles[5] = libFile;
      return coreComms[5];
    }
    if (className.equals("Lib7") && coreComms[6] == null) {
      coreComms[6] = new Lib7(libFile);
      coreCommFiles[6] = libFile;
      return coreComms[6];
    }
    if (className.equals("Lib8") && coreComms[7] == null) {
      coreComms[7] = new Lib8(libFile);
      coreCommFiles[7] = libFile;
      return coreComms[7];
    }

    return null;
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
