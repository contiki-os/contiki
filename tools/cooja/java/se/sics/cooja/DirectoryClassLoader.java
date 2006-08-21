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
 * $Id: DirectoryClassLoader.java,v 1.1 2006/08/21 12:12:56 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.*;
import org.apache.log4j.Logger;

/**
 * Loads an external file from the given directory as a Java class.
 * 
 * @author Fredrik Osterlind
 */
public class DirectoryClassLoader extends ClassLoader {
  private static Logger logger = Logger.getLogger(DirectoryClassLoader.class);
  private File directory;

  /**
   * Creates a new class loader reading from given directory.
   * 
   * @param directory
   *          Directory
   */
  public DirectoryClassLoader(File directory) {
    super();
    this.directory = directory;
  }

  /**
   * Creates a new class loader reading from given directory, with the given
   * class loader as parent class loader.
   * 
   * @param parent
   *          Parent class loader
   * @param directory
   *          Directory
   */
  public DirectoryClassLoader(ClassLoader parent, File directory) {
    super(parent);
    this.directory = directory;
  }

  public Class<?> findClass(String name) throws ClassNotFoundException {
    String fullFilePath = directory.getPath() + File.separatorChar + name
        + ".class";

    // Read external file
    //logger.info("Directory class loader reading file: " + fullFilePath);
    byte[] classData = loadClassData(fullFilePath);
    if (classData == null) {
      throw new ClassNotFoundException();
    }

    // Create class
    return defineClass(name, classData, 0, classData.length);
  }

  private byte[] loadClassData(String name) {
		// Support for fill class names in configuration file
    // TODO Quick-fix (may contain bugs)
    name = name.replace('.', File.separatorChar);
    name = name.replace(File.separatorChar + "class", ".class");
    
    // Open file for read access
    File classFile = new File(name);
    InputStream inputStream = null;
    if (!classFile.exists()) {
      //logger.fatal("File " + classFile + " does not exist!");
      return null;
    }

    try {
      inputStream = new FileInputStream(classFile);

      if (inputStream == null) {
        logger.fatal("File input stream is null!");
        return null;
      }
    } catch (FileNotFoundException e) {
      logger.fatal("Could not open file (not found?)!");
      return null;
    }
    long fileSize = classFile.length();

    if (fileSize > Integer.MAX_VALUE) {
      logger.fatal("Class file is too large");
      return null;
    }

    // Read class data
    byte[] classData = new byte[(int) fileSize];
    int offset = 0;
    int numRead = 0;
    try {
      while (offset < classData.length
          && (numRead = inputStream.read(classData, offset, classData.length
              - offset)) >= 0) {
        offset += numRead;
      }

      inputStream.close();
    } catch (IOException e) {
      logger.fatal("Error when reading class file");
      return null;
    }

    // Ensure all the bytes have been read in
    if (offset < classData.length) {
      logger.fatal("Could not read entire class file");
      return null;
    }

    return classData;
  }
}
