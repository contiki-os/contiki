/**
 * Copyright (c) 2007-2012, Swedish Institute of Computer Science.
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
 * -----------------------------------------------------------------
 *
 * Chip
 *
 * Author  : Joakim Eriksson
 * Created : 17 jan 2008
 */
package se.sics.mspsim.core;
import java.io.PrintStream;

import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.util.ArrayUtils;

/**
 * @author Joakim Eriksson, SICS
 * TODO: add a detailed state too (including a listener). State is not necessarily 
 * related to energy consumption, etc. but more detailed state of the Chip.
 * LPM1,2,3 / ON is OperatingModes as well as Transmitting, Listening and Off.
 * State can be things such as search for SFD (which is in mode Listen for CC2420).
 */
public abstract class Chip implements Loggable, EventSource {

  protected final String id;
  protected final String name;
  protected final MSP430Core cpu;

  private OperatingModeListener[] omListeners;
  private StateChangeListener stateListener;
  private ConfigurationChangeListener[] ccListeners;

  private EventListener eventListener;
  protected boolean sendEvents = false;
  private String[] modeNames = null;
  private int mode;
  private int chipState;
  protected EmulationLogger logger;
  private PrintStream log;
  protected boolean DEBUG = false;
  protected int logLevel;

  public Chip(String id, MSP430Core cpu) {
    this(id, id, cpu);
  }

  public Chip(String id, String name, MSP430Core cpu) {
    this.id = id;
    this.name = name;
    this.cpu = cpu;
    if (cpu != null) {
      logger = cpu.getLogger();
      cpu.addChip(this);
    }
  }

  public void notifyReset() {
  }

  public synchronized void addOperatingModeListener(OperatingModeListener listener) {
    omListeners = ArrayUtils.add(OperatingModeListener.class, omListeners, listener);
  }
  
  public synchronized void removeOperatingModeListener(OperatingModeListener listener) {
    omListeners = ArrayUtils.remove(omListeners, listener);
  }

  public synchronized void addStateChangeListener(StateChangeListener listener) {
      stateListener = StateChangeListener.Proxy.INSTANCE.add(stateListener, listener);
  }

  public synchronized void removeStateChangeListener(StateChangeListener listener) {
      stateListener = StateChangeListener.Proxy.INSTANCE.remove(stateListener, listener);
  }

  public synchronized void addConfigurationChangeListener(ConfigurationChangeListener listener) {
      ccListeners = ArrayUtils.add(ConfigurationChangeListener.class, ccListeners, listener);
  }

  public synchronized void removeConfigurationChangeListener(ConfigurationChangeListener listener) {
      ccListeners = ArrayUtils.remove(ccListeners, listener);
  }

  public int getMode() {
    return mode;
  }

  protected void setMode(int mode) {
    if (mode != this.mode) {
      this.mode = mode;
      OperatingModeListener[] listeners = omListeners;
      if (listeners != null) {
        for (int i = 0, n = listeners.length; i < n; i++) {
          listeners[i].modeChanged(this, mode);
        }
      }
    }
  }

  protected void setModeNames(String[] names) {
    modeNames = names;
  }

  public synchronized void addEventListener(EventListener listener) {
      eventListener = EventListener.Proxy.INSTANCE.add(eventListener, listener);
      sendEvents = eventListener != null;
  }

  public synchronized void removeEventListener(EventListener listener) {
      eventListener = EventListener.Proxy.INSTANCE.add(eventListener, listener);
      sendEvents = eventListener != null;
  }

  protected void sendEvent(String event, Object data) {
    EventListener listener = this.eventListener;
    if (listener != null) {
        listener.event(this, event, data);
    }
  }
  
  public String getModeName(int index) {
    if (modeNames == null) {
      return null;
    }
    return modeNames[index];
  }

  public int getModeByName(String mode) {
    if (modeNames != null) {
      for (int i = 0; i < modeNames.length; i++) {
        if (mode.equals(modeNames[i])) return i;
      }
    }
    try {
      // If it is just an integer it can be parsed!
      int modei = Integer.parseInt(mode);
      if (modei >= 0 && modei <= getModeMax()) {
        return modei;
      }
    } catch (Exception e) {
    }
    return -1;
  }

  /* Called by subclasses to inform about changes of state */
  protected void stateChanged(int newState) {
      if (chipState != newState) {
          int oldState = chipState;
          chipState = newState;
          /* inform listeners */
          StateChangeListener listener = stateListener;
          if (listener != null) {
              listener.stateChanged(this, oldState, chipState);
          }
      }
  }
  
  /* Called by subclasses to inform about changes of configuration */
  protected void configurationChanged(int parameter, int oldValue, int newValue) {
      ConfigurationChangeListener[] listeners = ccListeners;
      if (oldValue != newValue && listeners != null) {
          for (int i = 0, n = listeners.length; i < n; i++) {
              listeners[i].configurationChanged(this, parameter, oldValue, newValue);
          }
      }
  }

  /* interface for getting hold of configuration values - typically mapped to some kind of address */
  public abstract int getConfiguration(int parameter);
  
  
  public String getID() {
    return id;
  }

  public String getName() {
    return name;
  }

  public abstract int getModeMax();

  /* By default the cs is set high */
  public boolean getChipSelect() {
    return true;
  }
  
  public String info() {
    return "* no info";
  }

  public int getLogLevel() {
      return logLevel;
  }

  public void setLogLevel(int l) {
      logLevel = l;
      DEBUG = logLevel == Loggable.DEBUG;
  }
  
  protected void log(String msg) {
      logger.log(this, msg);
  }

  /* warn about anything above severe - but what types are severe? */
  protected void logw(WarningType type, String msg) {
      logger.logw(this, type, msg);
  }

}
