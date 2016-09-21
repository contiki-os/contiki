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
 * ArgumentManager
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : Tue Apr 08 22:08:32 2003
 * Updated : $Date$
 *           $Revision$
 */
package se.sics.mspsim.util;
import java.util.ArrayList;

/**
 */
public class ArgumentManager extends ConfigManager {

  private String configName = "config";
  private String[] arguments;
  private boolean isConfigLoaded;

  public ArgumentManager() {
  }

  public ArgumentManager(ConfigManager parent) {
    super(parent);
  }

  public boolean isConfigLoaded() {
    return isConfigLoaded;
  }

  public String getConfigArgumentName() {
    return configName;
  }

  public void setConfigArgumentName(String configName) {
    this.configName = configName;
  }

  public String[] getArguments() {
    return arguments;
  }

  public void handleArguments(String[] args) {
    ArrayList<String> list = new ArrayList<String>();
    ArrayList<String> config = new ArrayList<String>();
    for (int i = 0, n = args.length; i < n; i++) {
      if (args[i].startsWith("-")) {
        String param = args[i].substring(1);
        String value = "";
        int index = param.indexOf('=');
        if (index >= 0) {
          if (index < param.length()) {
            value = param.substring(index + 1);
          }
          param = param.substring(0, index);
        }
        if (param.length() == 0) {
          throw new IllegalArgumentException("illegal argument: " + args[i]);
        }
        if (configName != null && configName.equals(param)) {
          if (value.length() == 0) {
            throw new IllegalArgumentException("no config file name specified");
          }
          if (!loadConfiguration(value)) {
            throw new IllegalArgumentException("failed to load configuration " + value);
          }
          isConfigLoaded = true;
        }
        config.add(param);
        config.add(value.length() > 0 ? value : "true");
      } else {
        // Normal argument
        list.add(args[i]);
      }
    }
    this.arguments = list.toArray(new String[list.size()]);
    for (int i = 0, n = config.size(); i < n; i += 2) {
      setProperty(config.get(i), config.get(i + 1));
    }
  }

} // ArgumentManager
