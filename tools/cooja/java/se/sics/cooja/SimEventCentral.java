/*
 * Copyright (c) 2009, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: SimEventCentral.java,v 1.4 2010/03/26 09:12:20 fros4943 Exp $
 */

package se.sics.cooja;

import java.io.File;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.interfaces.Log;
import se.sics.cooja.util.ArrayUtils;

/**
 * Simulation event central. Simplifies implementations of plugins that observe
 * motes and mote interfaces by keeping track of added and removed motes. For a
 * selected set of interfaces, the event central also maintains an event
 * history.
 * 
 * @see LogOutputEvent
 * @author Fredrik Osterlind
 */
public class SimEventCentral {
  private static Logger logger = Logger.getLogger(SimEventCentral.class);

  private Simulation simulation;

  public SimEventCentral(Simulation simulation) {
    this.simulation = simulation;

    /* Default buffer sizes */
    logOutputBufferSize = Integer.parseInt(GUI.getExternalToolsSetting("BUFFERSIZE_LOGOUTPUT", "" + 40000));

    
    moteObservations = new ArrayList<MoteObservation>();

    /* Mote count: notifications */
    moteCountListeners = new MoteCountListener[0];

    /* Log output: notifications and history */
    logOutputListeners = new LogOutputListener[0];
    logOutputEvents = new ArrayDeque<LogOutputEvent>();
  }
  

  /* GENERIC */
  private static class MoteEvent {
    public static int _ID_COUNTER = 0; /* Debugging */
    public final int ID; /* Debugging */

    private final Mote mote;
    private final long time;

    public MoteEvent(Mote mote, long time) {
      ID = _ID_COUNTER++;

      this.mote = mote;
      this.time = time;
    }
    public Mote getMote() {
      return mote;
    }
    public long getTime() {
      return time;
    }

    public String toString() {
      return "" + ID;
    }
  }
  /** Help class for maintaining mote-specific observations */
  private static class MoteObservation {
    private final Mote mote;
    private final Observable observable;
    private final Observer observer;
    public MoteObservation(Mote mote, Observable observable, Observer observer) {
      this.mote = mote;
      this.observable = observable;
      this.observer = observer;

      observable.addObserver(observer);
    }
    public Mote getMote() {
      return mote;
    }
    public Observer getObserver() {
      return observer;
    }
    public void disconnect() {
      observable.deleteObserver(observer);
    }
  }
  private ArrayList<MoteObservation> moteObservations;

  
  /* ADDED/REMOVED MOTES */
  public interface MoteCountListener {
    public void moteWasAdded(Mote mote);
    public void moteWasRemoved(Mote mote);
  }
  private MoteCountListener[] moteCountListeners;
  private Observer moteCountObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      if (obj == null || !(obj instanceof Mote)) {
        return;
      }
      Mote evMote = (Mote) obj;

      /* Check whether mote was added or removed */
      Mote[] allMotes = simulation.getMotes();
      boolean exists = false;
      for (Mote m: allMotes) {
        if (m == evMote) {
          exists = true;
          break;
        }
      }

      if (exists) {
        /* Mote was added */
        moteWasAdded(evMote);

        /* Notify external listeners */
        for (MoteCountListener l: moteCountListeners) {
          l.moteWasAdded(evMote);
        }
      } else {
        /* Mote was removed */
        moteWasRemoved(evMote);

        /* Notify external listeners */
        for (MoteCountListener l: moteCountListeners) {
          l.moteWasRemoved(evMote);
        }
      }
    }
  };
  public void addMoteCountListener(MoteCountListener listener) {
    if (moteCountListeners.length == 0) {
      /* Observe simulation for added/removed motes */
      simulation.addObserver(moteCountObserver);
    }

    moteCountListeners = ArrayUtils.add(moteCountListeners, listener);
  }
  public void removeMoteCountListener(MoteCountListener listener) {
    moteCountListeners = ArrayUtils.remove(moteCountListeners, listener);

    if (moteCountListeners.length == 0) {
      /* Stop observing simulation for added/removed motes */
      simulation.deleteObserver(moteCountObserver);
    }
  }


  /* LOG OUTPUT */
  public static class LogOutputEvent extends MoteEvent {
    public final String msg;
    public LogOutputEvent(Mote mote, long time, String msg) {
      super(mote, time);
      this.msg = msg;
    }
    public String getMessage() {
      return msg;
    }
  }
  private int logOutputBufferSize;
  private ArrayDeque<LogOutputEvent> logOutputEvents;
  public interface LogOutputListener extends MoteCountListener {
    public void removedLogOutput(LogOutputEvent ev);
    public void newLogOutput(LogOutputEvent ev);
  }
  private LogOutputListener[] logOutputListeners;
  private Observer logOutputObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      Mote mote = (Mote) obj;
      String msg = ((Log) obs).getLastLogMessage();
      if (msg == null) {
        return;
      }
      if (msg.length() > 0 && msg.charAt(msg.length() - 1) == '\n') {
        msg = msg.substring(0, msg.length() - 1);
      }

      /* We may have to remove some events now */
      while (logOutputEvents.size() > logOutputBufferSize-1) {
        LogOutputEvent removed;
        synchronized (logOutputEvents) {
          removed = logOutputEvents.pollFirst();
        }
        if (removed == null) {
          break;
        }
        for (LogOutputListener l: logOutputListeners) {
          l.removedLogOutput(removed);
        }
      }

      /* Store log output, and notify listeners */
      LogOutputEvent ev = new LogOutputEvent(mote, simulation.getSimulationTime(), msg);
      synchronized (logOutputEvents) {
        logOutputEvents.add(ev);
      }
      for (LogOutputListener l: logOutputListeners) {
        l.newLogOutput(ev);
      }
    }
  };
  public void addLogOutputListener(LogOutputListener listener) {
    if (logOutputListeners.length == 0) {
      /* Start observing all log interfaces */
      Mote[] motes = simulation.getMotes();
      for (Mote m: motes) {
        for (MoteInterface mi: m.getInterfaces().getInterfaces()) {
          if (mi instanceof Log) {
            moteObservations.add(new MoteObservation(m, mi, logOutputObserver));
          }
        }
      }
    }

    logOutputListeners = ArrayUtils.add(logOutputListeners, listener);
    addMoteCountListener(listener);
  }
  public void removeLogOutputListener(LogOutputListener listener) {
    logOutputListeners = ArrayUtils.remove(logOutputListeners, listener);
    removeMoteCountListener(listener);

    if (logOutputListeners.length == 0) {
      /* Stop observing all log interfaces */
      MoteObservation[] observations = moteObservations.toArray(new MoteObservation[0]);
      for (MoteObservation o: observations) {
        if (o.getObserver() == logOutputObserver) {
          o.disconnect();
          moteObservations.remove(o);
        }
      }

      /* Clear logs (TODO config) */
      logOutputEvents.clear();
    }
  }

  public LogOutputEvent[] getLogOutputHistory() {
    synchronized (logOutputEvents) {
      return logOutputEvents.toArray(new LogOutputEvent[0]);
    }
  }
  public int getLogOutputBufferSize() {
    return logOutputBufferSize;
  }
  public void setLogOutputBufferSize(int size) {
    logOutputBufferSize = size;
    
    /* We may have to remove some events now */
    while (logOutputEvents.size() > logOutputBufferSize) {
      LogOutputEvent removed = logOutputEvents.pollFirst();
      if (removed == null) {
        break;
      }
      for (LogOutputListener l: logOutputListeners) {
        l.removedLogOutput(removed);
      }
    }
  }
  public int getLogOutputObservationsCount() {
    int count=0;
    MoteObservation[] observations = moteObservations.toArray(new MoteObservation[0]);
    for (MoteObservation o: observations) {
      if (o.getObserver() == logOutputObserver) {
        count++;
      }
    }
    return count;
  }

  
  /* HELP METHODS: MAINTAIN OBSERVERS */
  private void moteWasAdded(Mote mote) {
    if (logOutputListeners.length > 0) {
      /* Add another log output observation.
       * (Supports multiple log interfaces per mote) */
      for (MoteInterface mi: mote.getInterfaces().getInterfaces()) {
        if (mi instanceof Log) {
          moteObservations.add(new MoteObservation(mote, mi, logOutputObserver));
        }
      }
    }

    /* ... */
  }
  private void moteWasRemoved(Mote mote) {
    /* Disconnect and remove mote observations */
    MoteObservation[] observations = moteObservations.toArray(new MoteObservation[0]);
    for (MoteObservation o: observations) {
      if (o.getMote() == mote) {
        o.disconnect();
        moteObservations.remove(o);
      }
    }
  }

  public String toString() {
    return 
    "\nActive mote observations: " + moteObservations.size() +
    "\n" +
    "\nMote count listeners: " + moteCountListeners.length +
    "\n" +
    "\nLog output listeners: " + logOutputListeners.length +
    "\nLog output history: " + logOutputEvents.size()
    ;
  }
  

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    /* Log output buffer size */
    element = new Element("logoutput");
    element.setText("" + logOutputBufferSize);
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable)
      throws MoteTypeCreationException {
    for (Element element : configXML) {
      String name = element.getName();
      if (name.equals("logoutput")) {
        logOutputBufferSize = Integer.parseInt(element.getText());
      }
    }
    return true;
  }
  
}
