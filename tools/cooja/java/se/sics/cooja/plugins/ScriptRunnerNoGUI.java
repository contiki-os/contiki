/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: ScriptRunnerNoGUI.java,v 1.7 2008/12/16 15:11:18 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.*;
import javax.swing.JInternalFrame;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.GUI.SimulationCreationException;

@ClassDescription("Test Script Editor (no GUI)")
@PluginType(PluginType.COOJA_PLUGIN)
public class ScriptRunnerNoGUI implements Plugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ScriptRunnerNoGUI.class);

  private GUI gui;

  private LogScriptEngine engine = null;

  private Object tag = null;

  private BufferedWriter logWriter = null;

  private final int TIMEOUT = 1200000; /* 1200s = 20 minutes */

  public ScriptRunnerNoGUI(GUI gui) {
    this.gui = gui;
  }

  public void closePlugin() {
    if (engine != null) {
      engine.deactiveScript();
      engine.setScriptLogObserver(null);
      engine = null;
    }
    if (logWriter != null) {
      try {
        logWriter.close();
        logWriter = null;
      } catch (IOException e) {
      }
    }
  }

  public JInternalFrame getGUI() {
    return null;
  }

  public boolean activateTest(File script, File log) {
    try {
      /* Load test script */
      BufferedReader in = new BufferedReader(new FileReader(script));
      String line, code = "";
      while ((line = in.readLine()) != null) {
        code += line + "\n";
      }
      in.close();

      /* Prepare test log */
      logWriter = new BufferedWriter(new FileWriter(log));

      /* Create script engine */
      engine = new LogScriptEngine(gui, code);
      engine.activateScript();
      engine.setScriptLogObserver(new Observer() {
        public void update(Observable obs, Object obj) {
          try {
            if (logWriter != null) {
              logWriter.write((String) obj);
              logWriter.flush();
            }
          } catch (IOException e) {
            logger.fatal("Error when writing to test log file: " + obj);
          }
        }
      });

    } catch (IOException e) {
      return false;
    } catch (UnsatisfiedLinkError e) {
      return false;
    }

    return true;
  }

  public boolean activateTest(File config, File script, File log) {
    try {
      /* Load simulation */
      final Simulation sim = gui.loadSimulationConfig(config, true);
      if (sim == null) {
        System.exit(1);
        return false;
      }
      gui.setSimulation(sim);

      /* Load test script */
      BufferedReader in = new BufferedReader(new FileReader(script));
      String line, code = "";
      while ((line = in.readLine()) != null) {
        code += line + "\n";
      }
      in.close();

      /* Prepare test log */
      logWriter = new BufferedWriter(new FileWriter(log));

      /* Create script engine */
      engine = new LogScriptEngine(gui, code);
      engine.activateScript();
      engine.setScriptLogObserver(new Observer() {
        public void update(Observable obs, Object obj) {
          try {
            if (logWriter != null) {
              logWriter.write((String) obj);
              logWriter.flush();
            }
          } catch (IOException e) {
            logger.fatal("Error when writing to test log file: " + obj);
          }
        }
      });

      /* Create timeout event */
      sim.scheduleEvent(new TimeEvent(0) {
        public void execute(long t) {
          try {
            logWriter.write("TEST TIMEOUT");
            logWriter.flush();
          } catch (IOException e) {
          }
          gui.doQuit(false);
        }
      }, TIMEOUT);

      /* Start simulation and leave control to script */
      sim.startSimulation();
    } catch (IOException e) {
      logger.fatal("Error when running script: " + e);
      System.exit(1);
      return false;
    } catch (UnsatisfiedLinkError e) {
      logger.fatal("Error when running script: " + e);
      System.exit(1);
      return false;
    } catch (SimulationCreationException e) {
      System.exit(1);
      logger.fatal("Error when running script: " + e);
      return false;
    }

    return true;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

  public void tagWithObject(Object tag) {
    this.tag = tag;
  }

  public Object getTag() {
    return tag;
  }

}
