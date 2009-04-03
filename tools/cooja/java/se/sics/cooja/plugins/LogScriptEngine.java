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
 * $Id: LogScriptEngine.java,v 1.10 2009/04/03 17:05:14 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.lang.reflect.UndeclaredThrowableException;
import java.util.*;
import java.util.concurrent.Semaphore;
import javax.swing.*;
import javax.script.*;

import org.apache.log4j.Logger;
import se.sics.cooja.*;

/**
 * Executes Contiki test scripts.
 *
 * @see ScriptRunner
 *
 * @author Fredrik Osterlind
 */
public class LogScriptEngine {
  private static final int DEFAULT_TIMEOUT = 1200000; /* 1200s = 20 minutes */

  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(LogScriptEngine.class);

  private ScriptEngineManager factory = new ScriptEngineManager();
  private ScriptEngine engine = factory.getEngineByName("JavaScript");

  private Observer logObserver = null;
  private Observer simObserver = null;
  private Observer guiObserver = null;

  private GUI gui;

  private Thread scriptThread = null;

  private Observer scriptLogObserver = null;

  private ScriptMote scriptMote;

  private boolean stopSimulation = false, quitCooja = false;

  private Semaphore semaphoreScript = null;
  private Semaphore semaphoreSim = null;

  private boolean scriptActive = false;

  private TimeEvent timeoutEvent = new TimeEvent(0) {
    public void execute(long t) {
      engine.put("TIMEOUT", true);
      stepScript();
    }
  };

  private interface ScriptLog {
    public void log(String log);
    public void testOK();
    public void testFailed();
    public void generateMessage(long delay, String msg);
  }

  private void stepScript() {
    /* Release script - halt simulation */
    semaphoreScript.release();

    /* ... script executing ... */

    try {
      semaphoreSim.acquire();
    } catch (InterruptedException e1) {
      e1.printStackTrace();
    }

    /* ... script is now again waiting for script semaphore ... */

    /* Check if test script requested us to stop */
    if (stopSimulation) {
      LogScriptEngine.this.gui.getSimulation().stopSimulation();
      stopSimulation = false;
    }
    if (quitCooja) {
      LogScriptEngine.this.gui.doQuit(false);
      quitCooja = false;
    }
  }

  public LogScriptEngine(GUI gui) {
    this.gui = gui;

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
        Mote mote = (Mote) obj;
        handleNewMoteOutput(
            mote,
            mote.getInterfaces().getMoteID().getMoteID(),
            mote.getSimulation().getSimulationTime(),
            mote.getInterfaces().getLog().getLastLogMessage()
        );
      }
    };
  }

  private void handleNewMoteOutput(Mote mote, int id, long time, String msg) {
    try {
      if (scriptThread == null ||
          !scriptThread.isAlive()) {
        logger.info("script thread not alive. try deactivating script.");
        /*scriptThread.isInterrupted()*/
        return;
      }

      /* Update script variables */
      engine.put("mote", mote);
      engine.put("id", id);
      engine.put("time", time);
      engine.put("msg", msg);

      stepScript();

    } catch (UndeclaredThrowableException e) {
      e.printStackTrace();
      JOptionPane.showMessageDialog(GUI.getTopParentContainer(),
          "See console for more information.",
          "Script error", JOptionPane.ERROR_MESSAGE);
      unregisterLogObserver();
      if (LogScriptEngine.this.gui.getSimulation() != null) {
        LogScriptEngine.this.gui.getSimulation().stopSimulation();
      }
    }
  }

  /**
   * Inject faked mote log output.
   * Should only be used for debugging!
   *
   * @param msg Log message
   * @param mote Mote
   */
  public void fakeMoteLogOutput(final String msg, final Mote mote) {
    gui.getSimulation().scheduleEvent(new TimeEvent(0) {
      public void execute(long time) {
        handleNewMoteOutput(
            mote,
            mote.getInterfaces().getMoteID().getMoteID(),
            mote.getSimulation().getSimulationTime(),
            msg
        );
      }
    }, gui.getSimulation().getSimulationTime());
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
   * Deactivate script
   */
  public void deactivateScript() {
    if (!scriptActive) {
      return;
    }
    scriptActive = false;

    if (semaphoreScript == null) {
      logger.warn("semaphoreScript is not initialized");
    }
    if (semaphoreSim == null) {
      logger.warn("semaphoreSim is not initialized");
    }
    if (scriptThread == null) {
      logger.warn("scriptThread is not initialized");
    }

    if (timeoutEvent != null) {
      timeoutEvent.remove();
      timeoutEvent = null;
    }

    gui.deleteObserver(guiObserver);

    if (gui.getSimulation() != null) {
      gui.getSimulation().deleteObserver(simObserver);
    }

    unregisterLogObserver();

    engine.put("SHUTDOWN", true);

    try {
      semaphoreScript.release(100);
    } catch (Exception e) {
    } finally {
      semaphoreScript = null;
    }
    try {
      semaphoreSim.release(100);
    } catch (Exception e) {
    } finally {
      semaphoreSim = null;
    }

    if (scriptThread != null && scriptThread != Thread.currentThread()) {
      try {
        scriptThread.join();
      } catch (InterruptedException e) {
        e.printStackTrace();
      } finally {
      }
    }
    scriptThread = null;

  }

  public void activateScript(String scriptCode) throws ScriptException {
    if (scriptActive) {
      return;
    }
    scriptActive = true;

    if (semaphoreScript != null) {
      logger.warn("semaphoreScript is already initialized");
    }
    if (semaphoreSim != null) {
      logger.warn("semaphoreSim is already initialized");
    }
    if (scriptThread != null) {
      logger.warn("scriptThread is already initialized");
    }

    /* Parse current script */
    ScriptParser parser = new ScriptParser(scriptCode);
    String jsCode = parser.getJSCode();

    long timeoutTime = parser.getTimeoutTime();
    if (gui.getSimulation() != null) {
      if (timeoutTime > 0) {
        gui.getSimulation().scheduleEvent(
            timeoutEvent,
            gui.getSimulation().getSimulationTime() + timeoutTime);
      } else {
        gui.getSimulation().scheduleEvent(
            timeoutEvent,
            gui.getSimulation().getSimulationTime() + DEFAULT_TIMEOUT);
      }
    }

    engine.eval(jsCode);

    /* Setup script control */
    semaphoreScript = new Semaphore(1);
    semaphoreSim = new Semaphore(1);
    engine.put("TIMEOUT", false);
    engine.put("SHUTDOWN", false);
    engine.put("SEMAPHORE_SCRIPT", semaphoreScript);
    engine.put("SEMAPHORE_SIM", semaphoreSim);

    try {
      semaphoreScript.acquire();
    } catch (InterruptedException e) {
      e.printStackTrace();
    }
    scriptThread = new Thread(new Runnable() {
      public void run() {
        /*logger.info("test script thread starts");*/
        try {
          ((Invocable)engine).getInterface(Runnable.class).run();

        } catch (RuntimeException e) {
          Throwable throwable = e;
          while (throwable.getCause() != null) {
            throwable = throwable.getCause();
          }

          if (throwable.getMessage() != null &&
              throwable.getMessage().contains("test script killed") ) {
            /*logger.info("test script thread terminated by exception");*/
          } else {
            if (!GUI.isVisualized()) {
              logger.fatal("Test script error, terminating Cooja.");
              logger.fatal("Script error:", e);
              System.exit(1);
            }

            logger.fatal("Script error:", e);
            gui.getSimulation().stopSimulation();
            deactivateScript();
          }
        }
        /*logger.info("test script thread exits");*/
      }
    });
    scriptThread.start(); /* Starts by acquiring semaphore (blocks) */
    while (!semaphoreScript.hasQueuedThreads()) {
      Thread.yield();
    }

    /* Setup simulation observers */
    gui.addObserver(guiObserver);

    if (gui.getSimulation() != null) {
      gui.getSimulation().addObserver(simObserver);
    }

    /* Create script output logger */
    engine.put("log", new ScriptLog() {
      public void log(String msg) {
        if (scriptLogObserver != null) {
          scriptLogObserver.update(null, msg);
        }
      }
      public void testOK() {
        log("TEST OK\n");

        if (GUI.isVisualized()) {
          log("[if test was run without visualization, COOJA would now have been terminated]\n");
          stopSimulation = true;
        } else {
          quitCooja = true;
        }

        /* Make sure simulation is actually stopped */
        gui.getSimulation().scheduleEvent(new TimeEvent(0) {
          public void execute(long time) {
            if (stopSimulation) {
              LogScriptEngine.this.gui.getSimulation().stopSimulation();
              stopSimulation = false;
            }
            if (quitCooja) {
              LogScriptEngine.this.gui.doQuit(false);
              quitCooja = false;
            }
          }
        }, gui.getSimulation().getSimulationTime());

        if (timeoutEvent != null) {
          timeoutEvent.remove();
        }

        semaphoreSim.release(100);
        throw new RuntimeException("test script killed");
      }
      public void testFailed() {
        log("TEST FAILED\n");

        if (GUI.isVisualized()) {
          log("[if test was run without visualization, COOJA would now have been terminated]\n");
          stopSimulation = true;
        } else {
          quitCooja = true;
        }

        /* Make sure simulation is actually stopped */
        gui.getSimulation().scheduleEvent(new TimeEvent(0) {
          public void execute(long time) {
            if (stopSimulation) {
              LogScriptEngine.this.gui.getSimulation().stopSimulation();
              stopSimulation = false;
            }
            if (quitCooja) {
              LogScriptEngine.this.gui.doQuit(false);
              quitCooja = false;
            }
          }
        }, gui.getSimulation().getSimulationTime());

        if (timeoutEvent != null) {
          timeoutEvent.remove();
        }

        semaphoreSim.release(100);
        throw new RuntimeException("test script killed");
      }

      public void generateMessage(long delay, final String msg) {
        final Mote currentMote = (Mote) engine.get("mote");

        TimeEvent generateEvent = new TimeEvent(0) {
          public void execute(long t) {
            if (scriptThread == null ||
                !scriptThread.isAlive()) {
              logger.info("script thread not alive. try deactivating script.");
              /*scriptThread.isInterrupted()*/
              return;
            }

            /* Update script variables */
            engine.put("mote", currentMote);
            engine.put("id", currentMote.getInterfaces().getMoteID().getMoteID());
            engine.put("time", currentMote.getSimulation().getSimulationTime());
            engine.put("msg", msg);

            stepScript();
          }
        };
        gui.getSimulation().scheduleEvent(
            generateEvent,
            gui.getSimulation().getSimulationTime() + delay);
      }
    });

    Hashtable<Object, Object> hash = new Hashtable<Object, Object>();
    engine.put("global", hash);

    scriptMote = new ScriptMote();
    engine.put("node", scriptMote);

    registerLogObserver();
  }

}
