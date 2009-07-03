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
 * $Id: Simulation.java,v 1.50 2009/07/03 13:37:41 fros4943 Exp $
 */

package se.sics.cooja;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Random;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.dialogs.CreateSimDialog;

/**
 * A simulation consists of a number of motes and mote types.
 *
 * A simulation is observable:
 * changed simulation state, added or deleted motes etc are observed.
 * To track mote changes, observe the mote (interfaces) itself.
 *
 * @author Fredrik Osterlind
 */
public class Simulation extends Observable implements Runnable {
  public static final long MILLISECOND = 1000L;

  /*private static long EVENT_COUNTER = 0;*/
  
  private Vector<Mote> motes = new Vector<Mote>();

  private Vector<MoteType> moteTypes = new Vector<MoteType>();

  private int delayTime = 0;

  private long currentSimulationTime = 0;

  private String title = null;

  private RadioMedium currentRadioMedium = null;

  private static Logger logger = Logger.getLogger(Simulation.class);

  private boolean isRunning = false;

  private boolean stopSimulation = false;

  private Thread simulationThread = null;

  private GUI myGUI = null;

  private long randomSeed = 123456;

  private boolean randomSeedGenerated = false;

  private long maxMoteStartupDelay = 1000*MILLISECOND;

  private Random randomGenerator = new Random();

  private boolean hasMillisecondObservers = false;
  private MillisecondObservable millisecondObservable = new MillisecondObservable();
  private class MillisecondObservable extends Observable {
    private void newMillisecond(long time) {
      setChanged();
      notifyObservers(time);
    }
  }

  private EventQueue eventQueue = new EventQueue();

  /**
   * Add millisecond observer.
   * This observer is notified once every simulated millisecond.
   *
   * @see #deleteMillisecondObserver(Observer)
   * @param newObserver Observer
   */
  public void addMillisecondObserver(Observer newObserver) {
    millisecondObservable.addObserver(newObserver);
    hasMillisecondObservers = true;
    rescheduleEvents = true;
  }

  /**
   * Delete millisecond observer.
   *
   * @see #addMillisecondObserver(Observer)
   * @param observer Observer to delete
   */
  public void deleteMillisecondObserver(Observer observer) {
    millisecondObservable.deleteObserver(observer);
    hasMillisecondObservers = millisecondObservable.countObservers() > 0;
    rescheduleEvents = true;
  }


  /**
   * Schedule events.
   * This method is not thread-safe, and should only be invoked when the 
   * simulation is paused, or from inside the simulation loop.
   * 
   * @see #scheduleEvent(TimeEvent, long)
   * @param e Event
   * @param time Execution time
   */
  public void scheduleEventUnsafe(TimeEvent e, long time) {
    eventQueue.addEvent(e, time);
  }

  /**
   * Schedule event to be handled by event loop.
   *
   * @param e Event
   * @param time Execution time
   */
  public void scheduleEvent(TimeEvent e, long time) {
    if (Thread.currentThread() == simulationThread) {
      eventQueue.addEvent(e, time);
    } else {
      eventQueue.addEventUnsorted(e, time);
    }
  }

  private Mote[] emulatedMoteArray;
  private TimeEvent tickEmulatedMotesEvent = new TimeEvent(0) {
    public void execute(long t) {
      /*logger.info("MSP motes tick at: " + t);*/
      if (emulatedMoteArray.length == 0) {
        return;
      }

      /* Tick MSP motes */
      boolean wantMoreTicks = true;
      while (wantMoreTicks) {
        /* Tick all MSP motes until none need more ticks */
        wantMoreTicks = false;
        for (Mote element : emulatedMoteArray) {
          if (element.tick(currentSimulationTime)) {
            wantMoreTicks = true;
          }
        }
      }

      /* XXX Reschedule MSP motes (millisecond resolution) */
      scheduleEventUnsafe(this, t+1000);
    }
    public String toString() {
      return "MSPSIM ALL";
    }
  };

  private TimeEvent delayEvent = new TimeEvent(0) {
    public void execute(long t) {
      /*logger.info("Delay at: " + t);*/
      if (delayTime == 0) {
        return;
      }

      try { Thread.sleep(delayTime); } catch (InterruptedException e) { }
      scheduleEventUnsafe(this, t+MILLISECOND);
    }
    public String toString() {
      return "DELAY";
    }
  };

  private TimeEvent millisecondEvent = new TimeEvent(0) {
    public void execute(long t) {
      if (!hasMillisecondObservers) {
        return;
      }

      millisecondObservable.newMillisecond(getSimulationTime());
      scheduleEventUnsafe(this, t+MILLISECOND);
    }
    public String toString() {
      return "MILLISECOND: " + millisecondObservable.countObservers();
    }
  };
  
  private void recreateMoteLists() {
    /* Tick MSP motes separately */
    ArrayList<Mote> emulatedMotes = new ArrayList<Mote>();
    for (Mote mote: motes) {
      /* TODO: fixe an emulatedMote generic class */
      if (mote.getType().getClass().toString().contains(".mspmote.")) {
        emulatedMotes.add(mote);
      } else if (mote.getType().getClass().toString().contains(".avrmote.")) {
        emulatedMotes.add(mote);
      }
    }
    emulatedMoteArray = emulatedMotes.toArray(new Mote[emulatedMotes.size()]);
  }

  private boolean rescheduleEvents = false;
  public void run() {
    long lastStartTime = System.currentTimeMillis();
    logger.info("Simulation main loop started, system time: " + lastStartTime);
    isRunning = true;
    
    /* Schedule tick events */
    scheduleEventUnsafe(tickEmulatedMotesEvent, currentSimulationTime);
    scheduleEventUnsafe(delayEvent, currentSimulationTime - (currentSimulationTime % 1000) + 1000);
    scheduleEventUnsafe(millisecondEvent, currentSimulationTime - (currentSimulationTime % 1000) + 1000);

    /* Simulation starting */
    this.setChanged();
    this.notifyObservers(this);

    recreateMoteLists();

    try {
      TimeEvent nextEvent;
      while (isRunning) {

        if (rescheduleEvents) {
          rescheduleEvents = false;
          scheduleEventUnsafe(tickEmulatedMotesEvent, currentSimulationTime);
          scheduleEventUnsafe(delayEvent, currentSimulationTime - (currentSimulationTime % 1000) + 1000);
          scheduleEventUnsafe(millisecondEvent, currentSimulationTime - (currentSimulationTime % 1000) + 1000);
        }

        nextEvent = eventQueue.popFirst();
        if (nextEvent == null) {
          throw new RuntimeException("No more events");
        }

        currentSimulationTime = nextEvent.time;
        /*logger.info("Executing event #" + EVENT_COUNTER++ + " @ " + currentSimulationTime + ": " + nextEvent);*/
        nextEvent.execute(currentSimulationTime);

        if (stopSimulation) {
          isRunning = false;
        }
      }
    } catch (RuntimeException e) {
      logger.fatal("Simulation stopped due to error", e);

      if (!GUI.isVisualized()) {
	/* Quit simulator if in test mode */
	System.exit(1);
      }
    }
    isRunning = false;
    simulationThread = null;
    stopSimulation = false;

    // Notify observers simulation has stopped
    this.setChanged();
    this.notifyObservers(this);

    logger.info("Simulation main loop stopped, system time: "
        + System.currentTimeMillis() + "\tDuration: "
        + (System.currentTimeMillis() - lastStartTime) + " ms");
  }

  /**
   * Creates a new simulation
   */
  public Simulation(GUI gui) {
    myGUI = gui;
  }

  /**
   * Starts this simulation (notifies observers).
   */
  public void startSimulation() {
    if (!isRunning()) {
      isRunning = true;
      simulationThread = new Thread(this);
      simulationThread.start();
    }
  }

  /**
   * Stops this simulation (notifies observers).
   */
  public void stopSimulation() {
    if (isRunning()) {
      stopSimulation = true;

      /* Wait until simulation stops */
      if (Thread.currentThread() != simulationThread) {
        try {
          Thread simThread = simulationThread;
          if (simThread != null) {
            simThread.join();
          }
        } catch (InterruptedException e) {
        }
      }
    }
  }

  /**
   * Starts simulation if stopped, ticks all motes once, and finally stops
   * simulation again.
   */
  public void stepMillisecondSimulation() {
    TimeEvent stopEvent = new TimeEvent(0) {
      public void execute(long t) {
        /* Stop simulation */
        stopSimulation();
      }
    };
    scheduleEvent(stopEvent, getSimulationTime()+Simulation.MILLISECOND);

    /* Start simulation if not running */
    if (!isRunning()) {
      startSimulation();
    }
  }

  /**
   * @return GUI holding this simulation
   */
  public GUI getGUI() {
    return myGUI;
  }

  /**
   * @return Random seed
   */
  public long getRandomSeed() {
    return randomSeed;
  }

  /**
   * @return Random seed (converted to a string)
   */
  public String getRandomSeedString() {
    return Long.toString(randomSeed);
  }

  /**
   * @param randomSeed Random seed
   */
  public void setRandomSeed(long randomSeed) {
    this.randomSeed = randomSeed;
    randomGenerator.setSeed(randomSeed);
    logger.info("Simulation random seed: " + randomSeed);
  }

  /**
   * @param generated Autogenerated random seed at simulation load
   */
  public void setRandomSeedGenerated(boolean generated) {
    this.randomSeedGenerated = generated;
  }

  /**
   * @return Autogenerated random seed at simulation load
   */
  public boolean getRandomSeedGenerated() {
    return randomSeedGenerated;
  }

  public Random getRandomGenerator() {
    return randomGenerator;
  }

  /**
   * @return Maximum mote startup delay
   */
  public long getDelayedMoteStartupTime() {
    return maxMoteStartupDelay;
  }

  /**
   * @param maxMoteStartupDelay Maximum mote startup delay
   */
  public void setDelayedMoteStartupTime(long maxMoteStartupDelay) {
    this.maxMoteStartupDelay = Math.max(0, maxMoteStartupDelay);
  }

  private SimEventCentral eventCentral = new SimEventCentral(this);
  public SimEventCentral getEventCentral() {
    return eventCentral;
  }
  
  /**
   * Returns the current simulation config represented by XML elements. This
   * config also includes the current radio medium, all mote types and motes.
   *
   * @return Current simulation config
   */
  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();

    Element element;

    // Title
    element = new Element("title");
    element.setText(title);
    config.add(element);

    // Delay time
    element = new Element("delaytime");
    element.setText(Integer.toString(delayTime));
    config.add(element);

    // Random seed
    element = new Element("randomseed");
    if (randomSeedGenerated) {
      element.setText("generated");
    } else {
      element.setText(Long.toString(getRandomSeed()));
    }
    config.add(element);

    // Max mote startup delay
    element = new Element("motedelay_us");
    element.setText(Long.toString(maxMoteStartupDelay));
    config.add(element);

    // Radio Medium
    element = new Element("radiomedium");
    element.setText(currentRadioMedium.getClass().getName());

    Collection<Element> radioMediumXML = currentRadioMedium.getConfigXML();
    if (radioMediumXML != null) {
      element.addContent(radioMediumXML);
    }
    config.add(element);

    /* Event central */
    element = new Element("events");
    element.addContent(eventCentral.getConfigXML());
    config.add(element);

    // Mote types
    for (MoteType moteType : getMoteTypes()) {
      element = new Element("motetype");
      element.setText(moteType.getClass().getName());

      Collection<Element> moteTypeXML = moteType.getConfigXML();
      if (moteTypeXML != null) {
        element.addContent(moteTypeXML);
      }
      config.add(element);
    }

    // Motes
    for (Mote mote : motes) {
      element = new Element("mote");
      element.setText(mote.getClass().getName());

      Collection<Element> moteXML = mote.getConfigXML();
      if (moteXML != null) {
        element.addContent(moteXML);
      }
      config.add(element);
    }

    return config;
  }

  /**
   * Sets the current simulation config depending on the given configuration.
   * 
   * @param configXML Simulation configuration
   * @param visAvailable True if simulation is allowed to show visualizers
   * @param manualRandomSeed Simulation random seed. May be null, in which case the configuration is used
   * @return True if simulation was configured successfully
   * @throws Exception If configuration could not be loaded
   */
  public boolean setConfigXML(Collection<Element> configXML,
      boolean visAvailable, Long manualRandomSeed) throws Exception {

    // Parse elements
    for (Element element : configXML) {

      // Title
      if (element.getName().equals("title")) {
        title = element.getText();
      }

      // Delay time
      if (element.getName().equals("delaytime")) {
        delayTime = Integer.parseInt(element.getText());
      }

      // Random seed
      if (element.getName().equals("randomseed")) {
        if (manualRandomSeed != null) {
          setRandomSeed(manualRandomSeed);
        } else if (element.getText().equals("generated")) {
          randomSeedGenerated = true;
          setRandomSeed(new Random().nextLong());
        } else {
          setRandomSeed(Long.parseLong(element.getText()));
        }
      }

      // Max mote startup delay
      if (element.getName().equals("motedelay")) {
        maxMoteStartupDelay = Integer.parseInt(element.getText())*MILLISECOND;
      }
      if (element.getName().equals("motedelay_us")) {
        maxMoteStartupDelay = Integer.parseInt(element.getText());
      }

      // Radio medium
      if (element.getName().equals("radiomedium")) {
        String radioMediumClassName = element.getText().trim();
        Class<? extends RadioMedium> radioMediumClass = myGUI.tryLoadClass(
            this, RadioMedium.class, radioMediumClassName);

        if (radioMediumClass != null) {
          // Create radio medium specified in config
          try {
            currentRadioMedium = RadioMedium.generateRadioMedium(radioMediumClass, this);
          } catch (Exception e) {
            currentRadioMedium = null;
            logger.warn("Could not load radio medium class: " + radioMediumClassName);
          }
        }

        // Show configure simulation dialog
        boolean createdOK = false;
        if (visAvailable) {
          createdOK = CreateSimDialog.showDialog(GUI.getTopParentContainer(), this);
        } else {
          createdOK = true;
        }

        if (!createdOK) {
          logger.debug("Simulation not created, aborting");
          throw new Exception("Load aborted by user");
        }

        // Check if radio medium specific config should be applied
        if (radioMediumClassName.equals(currentRadioMedium.getClass().getName())) {
          currentRadioMedium.setConfigXML(element.getChildren(), visAvailable);
        } else {
          logger.info("Radio Medium changed - ignoring radio medium specific config");
        }
      }

      /* Event central */
      if (element.getName().equals("events")) {
        eventCentral.setConfigXML(this, element.getChildren(), visAvailable);
      }

      // Mote type
      if (element.getName().equals("motetype")) {
        String moteTypeClassName = element.getText().trim();

        Class<? extends MoteType> moteTypeClass = myGUI.tryLoadClass(this,
            MoteType.class, moteTypeClassName);

        if (moteTypeClass == null) {
          logger.fatal("Could not load mote type class: " + moteTypeClassName);
          return false;
        }

        MoteType moteType = moteTypeClass.getConstructor((Class[]) null).newInstance();

        boolean createdOK = moteType.setConfigXML(this, element.getChildren(),
            visAvailable);
        if (createdOK) {
          addMoteType(moteType);
        } else {
          logger
              .fatal("Mote type was not created: " + element.getText().trim());
          throw new Exception("All mote types were not recreated");
        }
      }

      // Mote
      if (element.getName().equals("mote")) {
        Class<? extends Mote> moteClass = myGUI.tryLoadClass(this, Mote.class,
            element.getText().trim());

        Mote mote = moteClass.getConstructor((Class[]) null).newInstance((Object[]) null);
        if (mote.setConfigXML(this, element.getChildren(), visAvailable)) {
          addMote(mote);
        } else {
          logger.fatal("Mote was not created: " + element.getText().trim());
          throw new Exception("All motes were not recreated");
        }
      }
    }

    setChanged();
    notifyObservers(this);
    return true;
  }

  /**
   * Removes a mote from this simulation
   *
   * @param mote
   *          Mote to remove
   */
  public void removeMote(final Mote mote) {

    /* Simulation is running, remove mote in simulation loop */
    TimeEvent removeMoteEvent = new TimeEvent(0) {
      public void execute(long t) {
        motes.remove(mote);
        currentRadioMedium.unregisterMote(mote, Simulation.this);
        setChanged();
        notifyObservers(mote);

        /* Loop through all scheduled events.
         * Delete all events associated with deleted mote. */
        TimeEvent ev = eventQueue.peekFirst();
        while (ev != null) {
          if (ev instanceof MoteTimeEvent) {
            if (((MoteTimeEvent)ev).getMote() == mote) {
              ev.remove();
            }
          }
          
          ev = ev.nextEvent;
        }
        
        recreateMoteLists();
      }
    };

    if (!isRunning()) {
      /* Simulation is stopped, remove mote immediately */
      removeMoteEvent.execute(0);
    } else {
      /* Schedule event */
      scheduleEvent(removeMoteEvent, Simulation.this.getSimulationTime());
    }
  }

  /**
   * Adds a mote to this simulation
   *
   * @param mote
   *          Mote to add
   */
  public void addMote(final Mote mote) {
    if (maxMoteStartupDelay > 0 && mote.getInterfaces().getClock() != null) {
      mote.getInterfaces().getClock().setDrift(
          - getSimulationTime()
          - randomGenerator.nextInt((int)maxMoteStartupDelay)
      );
    }

    if (!isRunning()) {
      /* Simulation is stopped, add mote immediately */
      motes.add(mote);
      currentRadioMedium.registerMote(mote, this);
      this.setChanged();
      this.notifyObservers(mote);
      return;
    }

    /* Simulation is running, add mote in simulation loop */
    TimeEvent addNewMoteEvent = new TimeEvent(0) {
      public void execute(long t) {
        motes.add(mote);
        currentRadioMedium.registerMote(mote, Simulation.this);
        recreateMoteLists();
        Simulation.this.setChanged();
        Simulation.this.notifyObservers(mote);
      }
      public String toString() {
        return "ADD MOTE";
      }
    };

    scheduleEvent(addNewMoteEvent, Simulation.this.getSimulationTime());
  }

  /**
   * Get a mote from this simulation.
   *
   * @param pos
   *          Internal list position of mote
   * @return Mote
   */
  public Mote getMote(int pos) {
    return motes.get(pos);
  }

  /**
   * Returns number of motes in this simulation.
   *
   * @return Number of motes
   */
  public int getMotesCount() {
    return motes.size();
  }

  /**
   * Returns all motes in this simulation.
   *
   * @return Motes
   */
  public Mote[] getMotes() {
    Mote[] arr = new Mote[motes.size()];
    motes.toArray(arr);
    return arr;
  }

  /**
   * Returns all mote types in simulation.
   *
   * @return All mote types
   */
  public MoteType[] getMoteTypes() {
    MoteType[] types = new MoteType[moteTypes.size()];
    moteTypes.toArray(types);
    return types;
  }

  /**
   * Returns mote type with given identifier.
   *
   * @param identifier
   *          Mote type identifier
   * @return Mote type or null if not found
   */
  public MoteType getMoteType(String identifier) {
    for (MoteType moteType : getMoteTypes()) {
      if (moteType.getIdentifier().equals(identifier)) {
        return moteType;
      }
    }
    return null;
  }

  /**
   * Adds given mote type to simulation.
   *
   * @param newMoteType Mote type
   */
  public void addMoteType(MoteType newMoteType) {
    moteTypes.add(newMoteType);

    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Set delay time in milliseconds.
   * The simulation loop sleeps this value every simulated millisecond.
   * 
   * @param delayTime New delay time (ms)
   */
  public void setDelayTime(int delayTime) {
    this.delayTime = delayTime;

    rescheduleEvents = true;

    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Returns current delay time.
   *
   * @return Delay time (ms)
   */
  public int getDelayTime() {
    return delayTime;
  }

  /**
   * Set simulation time to simulationTime.
   *
   * @param simulationTime
   *          New simulation time (ms)
   */
  public void setSimulationTime(int simulationTime) {
    currentSimulationTime = simulationTime;

    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Returns current simulation time.
   *
   * @return Simulation time (microseconds)
   */
  public long getSimulationTime() {
    return currentSimulationTime;
  }

  /**
   * Returns current simulation time rounded to milliseconds.
   * 
   * @see #getSimulationTime()
   * @return
   */
  public long getSimulationTimeMillis() {
    return currentSimulationTime / MILLISECOND;
  }

  /**
   * Changes radio medium of this simulation to the given.
   *
   * @param radioMedium
   *          New radio medium
   */
  public void setRadioMedium(RadioMedium radioMedium) {
    // Remove current radio medium from observing motes
    if (currentRadioMedium != null) {
      for (int i = 0; i < motes.size(); i++) {
        currentRadioMedium.unregisterMote(motes.get(i), this);
      }
    }

    // Change current radio medium to new one
    if (radioMedium == null) {
      logger.fatal("Radio medium could not be created!");
      return;
    }
    this.currentRadioMedium = radioMedium;

    // Add all current motes to the new radio medium
    for (int i = 0; i < motes.size(); i++) {
      currentRadioMedium.registerMote(motes.get(i), this);
    }
  }

  /**
   * Get currently used radio medium.
   *
   * @return Currently used radio medium
   */
  public RadioMedium getRadioMedium() {
    return currentRadioMedium;
  }

  /**
   * Return true is simulation is running.
   *
   * @return True if simulation is running
   */
  public boolean isRunning() {
    return isRunning && simulationThread != null;
  }

  /**
   * Get current simulation title (short description).
   *
   * @return Title
   */
  public String getTitle() {
    return title;
  }

  /**
   * Set simulation title.
   *
   * @param title
   *          New title
   */
  public void setTitle(String title) {
    this.title = title;
  }

}
