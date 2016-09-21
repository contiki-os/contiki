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
 * PrefixConfigManager
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : Fri Oct 11 15:24:14 2002
 * Updated : $Date$
 *           $Revision$
 */
package se.sics.mspsim.util;

/**
 *
 */
public class PrefixConfigManager extends ConfigManager {

  private final ConfigManager config;
  private final String shortPrefix;
  private final String longPrefix;

  public PrefixConfigManager(ConfigManager config, String prefix, String name) {
    this(config, prefix, name, '.');
  }

  public PrefixConfigManager(ConfigManager config, String prefix,
                 String name, char separator) {
    if (config == null) {
      throw new NullPointerException();
    }
    this.config = config;
    prefix = prefix != null && prefix.length() > 0 ? (prefix + separator) : null;

    if (name != null && name.length() > 0) {
      this.longPrefix =	prefix == null ? (name + separator) : (prefix + name + separator);
      this.shortPrefix = prefix;
    } else if (prefix != null) {
      this.longPrefix = prefix;
      this.shortPrefix = null;
    } else {
      this.longPrefix = "";
      this.shortPrefix = null;
    }
  }

  public String getProperty(String name, String defaultValue) {
    String value = config.getProperty(longPrefix + name);
    if (value != null) {
      return value;
    }
    if (shortPrefix == null) {
      return defaultValue;
    }
    return config.getProperty(shortPrefix + name, defaultValue);
  }

  public void setProperty(String name, String value) {
    config.setProperty(longPrefix + name, value);
  }

}
