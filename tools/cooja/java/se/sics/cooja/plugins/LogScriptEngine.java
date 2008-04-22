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
 * $Id: LogScriptEngine.java,v 1.1 2008/04/22 10:12:16 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.lang.reflect.UndeclaredThrowableException;
import java.util.*;
import javax.swing.*;
import javax.script.*;

import org.apache.log4j.Logger;
import se.sics.cooja.*;

/**
 * Executes JavaScripts on mote logs.
 *
 * @see ScriptRunner
 *
 * @author Fredrik Osterlind
 */
public class LogScriptEngine {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(LogScriptEngine.class);

  private ScriptEngineManager factory = new ScriptEngineManager();
  private ScriptEngine engine = factory.getEngineByName("JavaScript");

  private Observer logObserver = null;
  private Observer simObserver = null;
  private Observer guiObserver = null;

  private GUI gui;

  private Observer scriptedLogObserver = null;

  private Observer scriptLogObserver = null;

  private String scriptCode;

  private interface ScriptLog {
    public void log(String log);
  }

  public LogScriptEngine(GUI gui, String code) {
    this.gui = gui;
    this.scriptCode = code;

    /* Create GUI observer: keeps track of new simulations */
    guiObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        if (LogScriptEngine.this.gui.getSimulation() != null) {
          LogScriptEngine.this.gui.getSimulation().addObserver(simObserver);
        }

        registerLogObserver();
      }
    };

    /* Create simulation observer: keeps track of newly added nodes */
    simObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        registerLogObserver();
      }
    };

    /* Create log observer: watches all log interfaces */
    logObserver = new Observer() {
      public void update(Observable obs, Object obj) {
        try {
          if (scriptedLogObserver != null) {
            scriptedLogObserver.update(obs, obj);
          }
        } catch (UndeclaredThrowableException e) {
          JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
              e.getMessage(),
              "Script error", JOptionPane.ERROR_MESSAGE);
          unregisterLogObserver();
        }
      }
    };

    /* Create script engine */
    try {
      createScriptEngine(scriptCode);
    } catch (ScriptException e) {
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          e.getMessage(),
          "Script error", JOptionPane.ERROR_MESSAGE);
      unregisterLogObserver();
    }
  }

  public void setScriptLogObserver(Observer observer) {
    scriptLogObserver = observer;
  }

  private void unregisterLogObserver() {
    /* Unregister mote log observer */
    if (logObserver != null && gui.getSimulation() != null) {
      for (int i=0; i < gui.getSimulation().getMotesCount(); i++) {
        Mote mote = gui.getSimulation().getMote(i);
        if (mote.getInterfaces().getLog() != null) {
          mote.getInterfaces().getLog().deleteObserver(logObserver);
        }
      }
    }
  }

  private void registerLogObserver() {
    /* Register mote log observer */
    if (logObserver != null && gui.getSimulation() != null) {
      for (int i=0; i < gui.getSimulation().getMotesCount(); i++) {
        Mote mote = gui.getSimulation().getMote(i);
        if (mote.getInterfaces().getLog() != null) {
          mote.getInterfaces().getLog().addObserver(logObserver);
        }
      }
    }
  }

  /**
   * Activate script
   */
  public void activateScript() {
    gui.addObserver(guiObserver);

    if (gui.getSimulation() != null) {
      gui.getSimulation().addObserver(simObserver);
    }

    registerLogObserver();
  }

  /**
   * Deactivate script
   */
  public void deactiveScript() {
    gui.deleteObserver(guiObserver);

    if (gui.getSimulation() != null) {
      gui.getSimulation().deleteObserver(simObserver);
    }

    unregisterLogObserver();
  }

  private void createScriptEngine(String code) throws ScriptException {
    engine.eval("function update(obs, obj) { " +
        "if (obj != null) {" +
        "  mote = obj;" +
        "  id = mote.getInterfaces().getMoteID().getMoteID();" +
        "  msg = mote.getInterfaces().getLog().getLastLogMessages();" +
        "} else {" +
        "  return;" +
        "} " +
        code +
    " };");

    Invocable inv = (Invocable) engine;
    scriptedLogObserver = inv.getInterface(Observer.class);

    /* Create script logger */
    engine.put("log", new ScriptLog() {
      public void log(String msg) {
        if (scriptLogObserver != null) {
          scriptLogObserver.update(null, msg);
        }
      }
    });

    /* TODO Test script */
    logObserver.update(null, null);
  }

}
