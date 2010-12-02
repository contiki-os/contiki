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
 * $Id: ProjectConfig.java,v 1.5 2010/12/02 15:28:06 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.*;
import java.util.*;

import org.apache.log4j.Logger;

/**
 * A project configuration may hold the configuration for one or several project
 * directories as well as a general simulator configuration.
 *
 * The configuration for a project directory may for example consist of which
 * plugins, interfaces and processes that the specific project directory supplies.
 * Each project directory configuration is read from the property file cooja.config, a
 * file which is required in each project directory.
 *
 * Values can be fetched as String, Boolean, Integer, Double or String array.
 *
 * Several configurations can be merged, together forming a final overall
 * configuration. The order of the how configurations are merged matter - later
 * values will overwrite earlier. For example merging two configurations with
 * the key 'SOMEKEY' in the following order:
 *
 * SOMEKEY = a b c
 *
 * SOMEKEY = d e
 *
 * will result in the final value "d e".
 *
 * If a specific value should be extended instead of overwritten, the value must
 * start with a single space-surrounded '+'. For example, merging two
 * configurations with the key as above in the following order:
 *
 * SOMEKEY = a b c
 *
 * SOMEKEY = + d e
 *
 * will result in the final value "a b c d e".
 *
 * The simulator will hold a merged project configuration, depending on which
 * project directories are used. Additionally. each mote type may also have a
 * configuration of its own, that differs from the general simulator
 * configuration.
 *
 * Often, but not necessarily, keys are named depending on which class is
 * associated with the information. For example, let's say a battery interface
 * wants to store its initial capacity (a double) using this approach. Data
 * stored in the external configuration file can look like the following:
 * se.sics.cooja.interfaces.Battery.initial_capacity 54.123321
 *
 * This value is then be read by: myMoteTypeConfig.getDoubleValue(Battery.class,
 * "initial_capacity");
 *
 * @author Fredrik Osterlind
 */
public class ProjectConfig {
  private static Logger logger = Logger.getLogger(ProjectConfig.class);

  private Properties myConfig = null;
  private Vector<File> myProjectDirHistory = null;

  /**
   * Creates new project configuration.
   *
   * @param useDefault
   *          If true the default configuration will be loaded
   * @throws FileNotFoundException
   *           If file was not found
   * @throws IOException
   *           Stream read error
   */
  public ProjectConfig(boolean useDefault) throws IOException,
      FileNotFoundException {
    // Create empty configuration
    myConfig = new Properties();
    myProjectDirHistory = new Vector<File>();


    if (useDefault) {
      InputStream input = GUI.class
          .getResourceAsStream(GUI.PROJECT_DEFAULT_CONFIG_FILENAME);
      if (input != null) {
        try {
          appendConfigStream(input);
        } finally {
          input.close();
        }
      } else {
        throw new FileNotFoundException(GUI.PROJECT_DEFAULT_CONFIG_FILENAME);
      }
    }
  }

  /**
   * Appends the given project directory's config file. This method also saves a
   * local history of which project directories has been loaded.
   *
   * @param projectDir
   *          Project directory
   * @return True if loaded OK
   * @throws FileNotFoundException
   *           If file was not found
   * @throws IOException
   *           Stream read error
   */
  public boolean appendProjectDir(File projectDir)
      throws FileNotFoundException, IOException {
    if (projectDir == null) {
      throw new FileNotFoundException("No project directory specified");
    }
    if (!projectDir.exists()) {
      throw new FileNotFoundException("Project directory does not exist: " + projectDir.getAbsolutePath());
    }
    
    File projectConfig = new File(projectDir.getPath(), GUI.PROJECT_CONFIG_FILENAME);
    if (!projectConfig.exists()) {
      throw new FileNotFoundException("Project config does not exist: " + projectConfig.getAbsolutePath());
    }
    myProjectDirHistory.add(projectDir);
    return appendConfigFile(projectConfig);
  }


  /**
   * Returns the project directory earlier appended to this configuration that
   * defined the given key. If the key is of an array format and the given array
   * element is non-null, then the project directory that added this element will be
   * returned instead. If no such project directory can be found null is returned
   * instead.
   *
   * @param callingClass
   *          Class which value belong to
   * @param key
   *          Key
   * @param arrayElement
   *          Value or array element
   * @return Project directory defining arguments or null
   */
  public File getUserProjectDefining(Class callingClass, String key, String arrayElement) {

    // Check that key really exists in current config
    if (getStringValue(callingClass, key, null) == null) {
      return null;
    }

    // Check that element really exists, if any
    if (arrayElement != null) {
      String[] array = getStringArrayValue(callingClass, key);
      boolean foundValue = false;
      for (String element : array) {
        if (element.equals(arrayElement)) {
          foundValue = true;
        }
      }
      if (!foundValue) {
        return null;
      }
    }

    // Search in all project directory in reversed order
    try {
      ProjectConfig remadeConfig = new ProjectConfig(false);

      for (int i=myProjectDirHistory.size()-1; i >= 0; i--) {
        remadeConfig.appendProjectDir(myProjectDirHistory.get(i));

        if (arrayElement != null) {
          // Look for array
          String[] array = remadeConfig.getStringArrayValue(callingClass, key);
          for (String element : array) {
            if (element.equals(arrayElement)) {
              return myProjectDirHistory.get(i);
            }
          }
        } else {
          // Look for key
          if (remadeConfig.getStringValue(callingClass, key, null) != null) {
            return myProjectDirHistory.get(i);
          }
        }
      }

    } catch (Exception e) {
      logger.fatal("Exception when searching in project directory history: " + e);
      return null;
    }

    return null;
  }

  /**
   * Loads the given property file and appends it to the current configuration.
   * If a property already exists it will be overwritten, unless the new value
   * begins with a '+' in which case the old value will be extended.
   *
   * WARNING! The project directory history will not be saved if this method is
   * called, instead the appendUserPlatform method should be used.
   *
   * @param propertyFile
   *          Property file to read
   * @return True if file was read ok, false otherwise
   * @throws FileNotFoundException
   *           If file was not found
   * @throws IOException
   *           Stream read error
   */
  public boolean appendConfigFile(File propertyFile)
      throws FileNotFoundException, IOException {
    if (!propertyFile.exists()) {
      logger.warn("Trying to import non-existant project configuration");
      return true;
    }

    FileInputStream in = new FileInputStream(propertyFile);
    return appendConfigStream(myConfig, in);
  }

  /**
   * Reads properties from the given stream and appends them to the current
   * configuration. If a property already exists it will be overwritten, unless
   * the new value begins with a '+' in which case the old value will be
   * extended.
   *
   * WARNING! The project directory history will not be saved if this method is
   * called, instead the appendUserPlatform method should be used.
   *
   * @param configFileStream
   *          Stream to read from
   * @return True if stream was read ok, false otherwise
   * @throws IOException
   *           Stream read error
   */
  public boolean appendConfigStream(InputStream configFileStream)
      throws IOException {
    return appendConfigStream(myConfig, configFileStream);
  }

  private static boolean appendConfigStream(Properties currentValues,
      InputStream configFileStream) throws IOException {

    // Read from stream
    Properties newProps = new Properties();
    newProps.load(configFileStream);
    configFileStream.close();

    // Read new properties
    Enumeration en = newProps.keys();
    while (en.hasMoreElements()) {
      String key = (String) en.nextElement();
      String property = newProps.getProperty(key);
      if (property.startsWith("+ ")) {
        if (currentValues.getProperty(key) != null) {
          currentValues.setProperty(key, currentValues.getProperty(key) + " "
              + property.substring(1).trim());
        } else {
          currentValues.setProperty(key, property.substring(1).trim());
        }
      } else {
        currentValues.setProperty(key, property);
      }
    }

    return true;
  }

  public boolean appendConfig(ProjectConfig config) {
  	Enumeration<String> propertyNames = config.getPropertyNames();
  	while (propertyNames.hasMoreElements()) {
  		String key = propertyNames.nextElement();
  		String property = config.getStringValue(key);
      if (property.startsWith("+ ")) {
        if (myConfig.getProperty(key) != null) {
        	myConfig.setProperty(key, myConfig.getProperty(key) + " "
              + property.substring(1).trim());
        } else {
        	myConfig.setProperty(key, property.substring(1).trim());
        }
      } else {
      	myConfig.setProperty(key, property);
      }
  	}
  	return true;
  }

  /**
   * @return All property names in configuration
   */
  public Enumeration<String> getPropertyNames() {
    return (Enumeration<String>) myConfig.propertyNames();
  }

  /**
   * Get string value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @param defaultValue
   *          Default value to return if id is not found
   * @return Value or defaultValue if id wasn't found
   */
  public String getStringValue(Class callingClass, String id,
      String defaultValue) {
    return getStringValue(myConfig, callingClass, id, defaultValue);
  }

  private static String getStringValue(Properties currentValues,
      Class callingClass, String id, String defaultValue) {
    String val = currentValues.getProperty(callingClass.getName() + "." + id);

    if (val == null) {
      /*logger.warn("Could not find key named '" + callingClass.getName() + "."
          + id + "'");*/
      return defaultValue;
    }

    return val;
  }

  /**
   * Returns value of given name.
   *
   * @param name
   *          Name
   * @return Value as string
   */
  public String getStringValue(String name) {
    if (!myConfig.containsKey(name)) {
      /*logger.debug("Could not find key named '" + name + "'");*/
    	return null;
    }

    return myConfig.getProperty(name);
  }

  /**
   * Get string value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @return Value or null if id wasn't found
   */
  public String getStringValue(Class callingClass, String id) {
    return getStringValue(callingClass, id, null);
  }

  /**
   * Get string array value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @return Value or null if id wasn't found
   */
  public String[] getStringArrayValue(Class callingClass, String id) {
    String stringVal = getStringValue(callingClass, id, null);
    if (stringVal == null) {
      return new String[0];
    }

    return getStringValue(callingClass, id, "").split(" ");
  }

  /**
   * Get string value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @return Value or null if id wasn't found
   */
  public String getValue(Class callingClass, String id) {
    return getStringValue(callingClass, id);
  }

  /**
   * Get string array value with given id.
   *
   * @param id
   *          Id of value to return
   * @return Value or null if id wasn't found
   */
  public String[] getStringArrayValue(String id) {
    String stringVal = getStringValue(id);
    if (stringVal == null) {
      return new String[0];
    }

    return getStringValue(id).split(" ");
  }

  /**
   * Get integer value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @param defaultValue
   *          Default value to return if id is not found
   * @return Value or defaultValue if id wasn't found
   */
  public int getIntegerValue(Class callingClass, String id, int defaultValue) {
    String str = getStringValue(callingClass, id);
    if (str == null) {
      return defaultValue;
    }

    return Integer.parseInt(str);
  }

  /**
   * Get integer value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @return Value or 0 if id wasn't found
   */
  public int getIntegerValue(Class callingClass, String id) {
    return getIntegerValue(callingClass, id, 0);
  }

  /**
   * Get double value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @param defaultValue
   *          Default value to return if id is not found
   * @return Value or defaultValue if id wasn't found
   */
  public double getDoubleValue(Class callingClass, String id,
      double defaultValue) {
    String str = getStringValue(callingClass, id);
    if (str == null) {
      return defaultValue;
    }

    return Double.parseDouble(str);
  }

  /**
   * Get double value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @return Value or 0.0 if id wasn't found
   */
  public double getDoubleValue(Class callingClass, String id) {
    return getDoubleValue(callingClass, id, 0.0);
  }

  /**
   * Get boolean value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @param defaultValue
   *          Default value to return if id is not found
   * @return Value or defaultValue if id wasn't found
   */
  public boolean getBooleanValue(Class callingClass, String id,
      boolean defaultValue) {
    String str = getStringValue(callingClass, id);
    if (str == null) {
      return defaultValue;
    }

    return Boolean.parseBoolean(str);
  }

  /**
   * Get boolean value with given id.
   *
   * @param callingClass
   *          Class which value belongs to
   * @param id
   *          Id of value to return
   * @return Value or false if id wasn't found
   */
  public boolean getBooleanValue(Class callingClass, String id) {
    return getBooleanValue(callingClass, id, false);
  }

  public ProjectConfig clone() {
    try {
      ProjectConfig clone = new ProjectConfig(false);
      clone.myConfig = (Properties) this.myConfig.clone();
      clone.myProjectDirHistory = (Vector<File>) this.myProjectDirHistory.clone();
      return clone;
    } catch (Exception e) {
      return null;
    }
  }
}
