/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * ConfigManager
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : Fri Oct 11 15:24:14 2002
 * Updated : $Date$
 *           $Revision$
 */
package se.sics.mspsim.util;
import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.net.URL;
import java.util.Properties;
import java.util.StringTokenizer;

public class ConfigManager {

  protected final ConfigManager parent;
  protected Properties properties = null;

  public ConfigManager() {
    this(null);
  }

  public ConfigManager(ConfigManager parent) {
    this.parent = parent;
  }



  // -------------------------------------------------------------------
  // Config file handling
  // -------------------------------------------------------------------

  protected String getBackupFile(String configFile) {
    int index = configFile.lastIndexOf('.');
    if (index > 0 && index < configFile.length() - 1) {
      return configFile.substring(0, index) + ".bak";
    }
    return null;
  }

  public boolean loadConfiguration(String configFile) {
    return loadConfiguration(new File(configFile));
  }

  public boolean loadConfiguration(File configFile) {
    try {
      InputStream input =
        new BufferedInputStream(new FileInputStream(configFile));
      try {
        loadConfiguration(input);
      } finally {
        input.close();
      }
      return true;
    } catch (FileNotFoundException e) {
      return false;
    } catch (IOException e) {
      throw new IllegalArgumentException("could not read config file '"
          + configFile + "': " + e);
    }
  }

  public boolean loadConfiguration(URL configURL) {
    try {
      InputStream input = new BufferedInputStream(configURL.openStream());
      try {
        loadConfiguration(input);
      } finally {
        input.close();
      }
      return true;
    } catch (FileNotFoundException e) {
      return false;
    } catch (IOException e) {
      throw new IllegalArgumentException("could not read config file '"
          + configURL + "': " + e);
    }
  }

  public void loadConfiguration(InputStream input) throws IOException {
    Properties p = new Properties();
    p.load(input);
    this.properties = p;
  }

  public boolean saveConfiguration(File filename, String comments) {
    if (properties != null) {
      OutputStream output = null;
      try {
        output = new FileOutputStream(filename);
        properties.store(output, comments);
        return true;
      } catch (IOException e) {
        e.printStackTrace();
      } finally {
        if (output != null) {
          try {
            output.close();
          } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
          }
        }
      }
    }
    return false;
  }


  // -------------------------------------------------------------------
  // Properties handling
  // -------------------------------------------------------------------

  /**
   * Returns the property names. Does not include inherited properties.
   *
   * @return an array with the non-inherited property names
   */
  public String[] getPropertyNames() {
    if (properties == null) {
      return new String[0];
    }
    synchronized (properties) {
      return properties.keySet().toArray(new String[properties.size()]);
    }
  }

  public String getProperty(String name) {
    return getProperty(name, null);
  }

  public String getProperty(String name, String defaultValue) {
    String value = (properties != null)
    ? properties.getProperty(name)
        : null;

    if (value == null || value.length() == 0) {
      value = parent != null
      ? parent.getProperty(name, defaultValue)
          : defaultValue;
    }
    return value;
  }

  public void setProperty(String name, String value) {
    if (properties == null) {
      synchronized (this) {
        if (properties == null) {
          properties = new Properties();
        }
      }
    }

    if (value == null) {
      properties.remove(name);
    } else {
      properties.put(name, value);
    }
  }

  public String[] getPropertyAsArray(String name) {
    return getPropertyAsArray(name, null);
  }

  public String[] getPropertyAsArray(String name, String defaultValue) {
    String valueList = getProperty(name, defaultValue);
    if (valueList != null) {
      StringTokenizer tok = new StringTokenizer(valueList, ", \t");
      int len = tok.countTokens();
      if (len > 0) {
        String[] values = new String[len];
        for (int i = 0; i < len; i++) {
          values[i] = tok.nextToken();
        }
        return values;
      }
    }
    return null;
  }

  public int getPropertyAsInt(String name, int defaultValue) {
    String value = getProperty(name, null);
    return value != null ? parseInt(name, value, defaultValue) : defaultValue;
  }

  public int[] getPropertyAsIntArray(String name) {
    return getPropertyAsIntArray(name, null);
  }

  public int[] getPropertyAsIntArray(String name, String defaultValue) {
    String valueList = getProperty(name, defaultValue);
    if (valueList != null) {
      return parseIntArray(valueList, defaultValue);
    } else if (defaultValue != null) {
      return parseIntArray(defaultValue, null);
    } else {
      return null;
    }
  }

  private int[] parseIntArray(String valueList, String secondaryValue) {
    StringTokenizer tok = new StringTokenizer(valueList, ", \t/");
    int len = tok.countTokens();
    if (len > 0) {
      try {
        int[] values = new int[len];
        for (int i = 0; i < len; i++) {
          values[i] = Integer.parseInt(tok.nextToken());
        }
        return values;
      } catch (NumberFormatException e) {
        // Ignore parse errors and try secondary value if specified and not already tried
      }
    }
    if(secondaryValue != null && !secondaryValue.equals(valueList)) {
      return parseIntArray(secondaryValue, null);
    }
    return null;
  }

  public long getPropertyAsLong(String name, long defaultValue) {
    String value = getProperty(name, null);
    return value != null
    ? parseLong(name, value, defaultValue)
        : defaultValue;
  }

  public float getPropertyAsFloat(String name, float defaultValue) {
    String value = getProperty(name, null);
    return value != null
    ? parseFloat(name, value, defaultValue)
        : defaultValue;
  }

  public double getPropertyAsDouble(String name, double defaultValue) {
    String value = getProperty(name, null);
    return value != null
    ? parseDouble(name, value, defaultValue)
        : defaultValue;
  }

  public boolean getPropertyAsBoolean(String name, boolean defaultValue) {
    String value = getProperty(name, null);
    return value != null
    ? parseBoolean(name, value, defaultValue)
        : defaultValue;
  }

  protected int parseInt(String name, String value, int defaultValue) {
    try {
      return Integer.parseInt(value);
    } catch (Exception e) {
      System.err.println("config '" + name + "' has a non-integer value '"
          + value + '\'');
    }
    return defaultValue;
  }

  protected long parseLong(String name, String value, long defaultValue) {
    try {
      return Long.parseLong(value);
    } catch (Exception e) {
      System.err.println("config '" + name + "' has a non-long value '"
          + value + '\'');
    }
    return defaultValue;
  }

  protected float parseFloat(String name, String value, float defaultValue) {
    try {
      return Float.parseFloat(value);
    } catch (Exception e) {
      System.err.println("config '" + name + "' has a non-float value '"
          + value + '\'');
    }
    return defaultValue;
  }

  protected double parseDouble(String name, String value,
      double defaultValue) {
    try {
      return Double.parseDouble(value);
    } catch (Exception e) {
      System.err.println("config '" + name + "' has a non-double value '"
          + value + '\'');
    }
    return defaultValue;
  }

  protected boolean parseBoolean(String name, String value,
      boolean defaultValue) {
    return "true".equals(value) || "yes".equals(value) || "1".equals(value);
  }

  public void print(PrintStream out) {
      properties.list(out);
  }

  
} // ConfigManager
