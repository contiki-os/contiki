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
 * $Id: PlatformConfig.java,v 1.3 2006/09/06 12:26:33 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.*;
import java.util.*;
import org.apache.log4j.Logger;

/**
 * A platform configuration may hold the configuration for one or several user
 * platforms as well as a general simulator configuration.
 * 
 * The configuration for a user platform may for example consist of which
 * plugins, interfaces and processes that the specific platform supplies. Each
 * user platform configuration is read from the property file cooja.config, a
 * file which is required in each user platform.
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
 * The simulator will hold a merged platform configuration, depending on which
 * user platforms are used. Additionally. each mote type may also have a
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
public class PlatformConfig {
  private static Logger logger = Logger.getLogger(PlatformConfig.class);

  private Properties myConfig = null;
  private Vector<File> myUserPlatformHistory = null;

  /**
   * Creates new platform configuration.
   * 
   * @param useDefault
   *          If true the default configuration will be loaded
   * @throws FileNotFoundException
   *           If file was not found
   * @throws IOException
   *           Stream read error
   */
  public PlatformConfig(boolean useDefault) throws IOException,
      FileNotFoundException {
    // Create empty configuration
    myConfig = new Properties();
    myUserPlatformHistory = new Vector<File>();

    if (useDefault) {
      InputStream input = GUI.class
          .getResourceAsStream(GUI.PLATFORM_DEFAULT_CONFIG_FILENAME);
      if (input != null) {
        try {
          appendConfigStream(input);
        } finally {
          input.close();
        }
      } else {
        throw new FileNotFoundException(GUI.PLATFORM_DEFAULT_CONFIG_FILENAME);
      }
    }
  }

  /**
   * Appends the given user platform's config file. Thus method also saved a
   * local history of which user platforms has been loaded.
   * 
   * @param userPlatform
   *          User platform
   * @return True if loaded OK
   * @throws FileNotFoundException
   *           If file was not found
   * @throws IOException
   *           Stream read error
   */
  public boolean appendUserPlatform(File userPlatform)
      throws FileNotFoundException, IOException {
    File userPlatformConfig = new File(userPlatform.getPath(),
        GUI.PLATFORM_CONFIG_FILENAME);
    myUserPlatformHistory.add(userPlatform);
    return appendConfigFile(userPlatformConfig);
  }


  /**
   * Returns the user platform earlier appended to this configuration that
   * defined the given key. If the key is of an array format and the given array
   * element is non-null, then the user platform that added this element will be
   * returned instead. If no such user platform can be found null is returned
   * instead.
   * 
   * @param callingClass
   *          Class which value belongs to
   * @param key
   *          Key
   * @param value
   *          Element of array
   * @return User platform
   */
  public File getUserPlatformDefining(Class callingClass, String key, String arrayElement) {

    // Check that key really exists in current config
    if (getStringValue(callingClass, key, null) == null) {
      return null;
    }
    
    // Check that element really exists, if any
    if (arrayElement != null) {
      String[] array = getStringArrayValue(callingClass, key);
      boolean foundValue = false;
      for (int c=0; c < array.length; c++) {
        if (array[c].equals(arrayElement))
          foundValue = true;
      }
      if (!foundValue) {
        return null;
      }
    }

    // Search in all user platform in reversed order
    try {
      PlatformConfig remadeConfig = new PlatformConfig(false);
      
      for (int i=myUserPlatformHistory.size()-1; i >= 0; i--) {
        remadeConfig.appendUserPlatform(myUserPlatformHistory.get(i));

        if (arrayElement != null) {
          // Look for array
          String[] array = remadeConfig.getStringArrayValue(callingClass, key);
          for (int c=0; c < array.length; c++) {
            if (array[c].equals(arrayElement))
              return myUserPlatformHistory.get(i);
          }
        } else {
          // Look for key
          if (remadeConfig.getStringValue(callingClass, key, null) != null) {
            return myUserPlatformHistory.get(i);
          }
        }
      }
      
    } catch (Exception e) {
      logger.fatal("Exception when searching in user platform history: " + e);
      return null;
    }
    
    return null;
  }

  /**
   * Loads the given property file and appends it to the current configuration.
   * If a property already exists it will be overwritten, unless the new value
   * begins with a '+' in which case the old value will be extended.
   * 
   * WARNING! The user platform history will not be saved if this method is
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
    FileInputStream in = new FileInputStream(propertyFile);
    return appendConfigStream(myConfig, in);
  }

  /**
   * Reads properties from the given stream and appends them to the current
   * configuration. If a property already exists it will be overwritten, unless
   * the new value begins with a '+' in which case the old value will be
   * extended.
   * 
   * WARNING! The user platform history will not be saved if this method is
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
        if (currentValues.getProperty(key) != null)
          currentValues.setProperty(key, currentValues.getProperty(key) + " "
              + property.substring(1).trim());
        else
          currentValues.setProperty(key, property.substring(1).trim());
      } else
        currentValues.setProperty(key, property);
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
      logger.warn("Could not find key named '" + callingClass.getName() + "."
          + id + "'");
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
    if (!myConfig.containsKey(name))
      logger.debug("Could not find key named '" + name + "'");

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
    if (stringVal == null)
      return new String[0];

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
    if (stringVal == null)
      return new String[0];

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
    if (str == null)
      return defaultValue;

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
    if (str == null)
      return defaultValue;

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
    if (str == null)
      return defaultValue;

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

  public PlatformConfig clone() {
    try {
      PlatformConfig clone = new PlatformConfig(false);
      clone.myConfig = (Properties) this.myConfig.clone();
      clone.myUserPlatformHistory = (Vector<File>) this.myUserPlatformHistory.clone();
      return clone;
    } catch (Exception e) {
      return null;
    }
  }
}
