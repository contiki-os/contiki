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
 * $Id: Simulation.java,v 1.3 2006/11/08 21:28:51 fros4943 Exp $
 */

package se.sics.cooja;

import java.awt.Dimension;
import java.awt.Point;
import java.io.*;
import java.util.*;
import javax.swing.JInternalFrame;

import org.apache.log4j.Logger;
import org.jdom.*;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import se.sics.cooja.dialogs.*;

/**
 * A simulation contains motes and ticks them one by one. When all motes has
 * been ticked once, the simulation sleeps for some specified time, and the
 * current simulation time is updated. Some observers (tick observers) are also
 * notified.
 * 
 * When observing the simulation itself, the simulation state, added or deleted
 * motes etc are observed, as opposed to individual mote changes. Changes of
 * individual motes should instead be observed via corresponding mote
 * interfaces.
 * 
 * @author Fredrik Osterlind
 */
public class Simulation extends Observable implements Runnable {

  private Vector<Mote> motes = new Vector<Mote>();
  private Vector<MoteType> moteTypes = new Vector<MoteType>();

  private int delayTime = 100;
  private int currentSimulationTime = 0;
  private int tickTime = 1;
  private String title = null;

  // Radio Medium
  private RadioMedium currentRadioMedium = null;

  private static Logger logger = Logger.getLogger(Simulation.class);

  private boolean isRunning = false;
  private boolean stopSimulation = false;
  private Thread thread = null;

  // Tick observable
  private class TickObservable extends Observable {
    private void allTicksPerformed() {
      setChanged();
      notifyObservers();
    }
  }
  private TickObservable tickObservable = new TickObservable();

  /**
   * Add tick observer. This observer is notified once every tick loop, that is,
   * when all motes have been ticked.
   * 
   * @see #deleteTickObserver(Observer)
   * @param newObserver
   *          New observer
   */
  public void addTickObserver(Observer newObserver) {
    tickObservable.addObserver(newObserver);
  }

  /**
   * Delete an existing tick observer.
   * 
   * @see #addTickObserver(Observer)
   * @param observer
   *          Observer to delete
   */
  public void deleteTickObserver(Observer observer) {
    tickObservable.deleteObserver(observer);
  }

  public void run() {
    long lastStartTime = System.currentTimeMillis();
    logger.info("Simulation main loop started, system time: " + lastStartTime);
    isRunning = true;

    // Notify observers simulation is starting
    this.setChanged();
    this.notifyObservers(this);

    while (isRunning) {
      try {
        // Tick all motes
        for (Mote moteToTick : motes) {
          moteToTick.tick(currentSimulationTime);
        }

        // Increase simulation time
        currentSimulationTime += tickTime;

        // Notify tick observers
        tickObservable.allTicksPerformed();

        // Sleep
        if (delayTime > 0)
          Thread.sleep(delayTime);

        if (stopSimulation) {
          // We should only tick once (and we just did), so abort now
          stopSimulation = false;
          isRunning = false;
          thread = null;
        }

      } catch (InterruptedException e) {
        isRunning = false;
        thread = null;
        break;
      } catch (IllegalArgumentException e) {
        logger.warn("llegalArgumentException:" + e);
        isRunning = false;
        thread = null;
        break;
      } catch (IllegalMonitorStateException e) {
        logger.warn("IllegalMonitorStateException:" + e);
        isRunning = false;
        thread = null;
        break;
      }
    }

    isRunning = false;
    thread = null;
    stopSimulation = false;

    // Notify observers simulation has stopped
    this.setChanged();
    this.notifyObservers(this);

    logger.info("Simulation main loop stopped, system time: "
        + System.currentTimeMillis() + "\tDuration: "
        + (System.currentTimeMillis() - lastStartTime) + " ms");
  }

  /**
   * Creates a new simulation with a delay time of 1 second.
   */
  public Simulation() {
    // New simulation instance
  }

  /**
   * Starts this simulation (notifies observers).
   */
  public void startSimulation() {
    if (!isRunning()) {
      thread = new Thread(this);
      thread.start();
    }
  }

  /**
   * Stops this simulation (notifies observers).
   */
  public void stopSimulation() {
    if (isRunning()) {
      stopSimulation = true;
      thread.interrupt();

      // Wait until simulation stops
      if (Thread.currentThread() != thread)
        while (thread != null && thread.isAlive()) {
          try {
            Thread.sleep(10);
          } catch (InterruptedException e) {
          }
        }
    } // else logger.fatal("Could not stop simulation: isRunning=" + isRunning +
      // ", thread=" + thread);
  }

  /**
   * Starts simulation if stopped, ticks all motes once, and finally stop
   * simulation again.
   */
  public void tickSimulation() {
    stopSimulation = true;

    if (!isRunning()) {
      thread = new Thread(this);
      thread.start();
    }

    // Wait until simulation stops
    while (thread != null && thread.isAlive()) {
      try {
        Thread.sleep(10);
      } catch (InterruptedException e) {
      }
    }

  }

  /**
   * Loads a simulation configuration from given file.
   * 
   * When loading mote types, user must recompile the actual library of each
   * type. User may also change mote type settings at this point.
   * 
   * @see #saveSimulationConfig(File)
   * @param file
   *          File to read
   * @return New simulation or null if recompiling failed or aborted
   * @throws UnsatisfiedLinkError
   *           If associated libraries could not be loaded
   */
  public static Simulation loadSimulationConfig(File file)
      throws UnsatisfiedLinkError {

    Simulation newSim = null;

    try {
      // Open config file
      SAXBuilder builder = new SAXBuilder();
      Document doc = builder.build(file);
      Element root = doc.getRootElement();

      // Check that config file version is correct
      if (!root.getName().equals("simulation")) {
        logger.fatal("Not a COOJA simulation config xml file!");
        return null;
      }

      // Build new simulation
      Collection<Element> config = root.getChildren();
      newSim = new Simulation();
      boolean createdOK = newSim.setConfigXML(config);
      if (!createdOK) {
        logger.info("Simulation not loaded");
        return null;
      }

      // EXPERIMENTAL: Reload stored plugins
      boolean loadedPlugin = false;
      for (Element pluginElement: config.toArray(new Element[0])) {
        if (pluginElement.getName().equals("visplugin")) {
          Class<? extends VisPlugin> visPluginClass = 
            GUI.currentGUI.tryLoadClass(GUI.currentGUI, VisPlugin.class, pluginElement.getText().trim());

          try {
            VisPlugin openedPlugin = null;
            List list = pluginElement.getChildren();
            Dimension size = new Dimension(100, 100);
            Point location = new Point(100, 100);
            for (Element pluginSubElement: (List<Element>) pluginElement.getChildren()) {
              
              if (pluginSubElement.getName().equals("constructor")) {
                if (pluginSubElement.getText().equals("sim")) {
                  // Simulator plugin type
                  openedPlugin = visPluginClass.getConstructor(new Class[]{Simulation.class}).newInstance(newSim);
                  GUI.currentGUI.showPlugin(openedPlugin);
                } else if (pluginSubElement.getText().equals("gui")) {
                  // GUI plugin type
                  openedPlugin = visPluginClass.getConstructor(new Class[]{GUI.class}).newInstance(GUI.currentGUI);
                  GUI.currentGUI.showPlugin(openedPlugin);
                } else if (pluginSubElement.getText().startsWith("mote: ")) {
                  // Mote plugin type
                  String moteNrString = pluginSubElement.getText().substring("mote: ".length());
                  
                  int moteNr = Integer.parseInt(moteNrString);
                  Mote mote = newSim.getMote(moteNr);
                  openedPlugin = visPluginClass.getConstructor(new Class[]{Mote.class}).newInstance(mote);
                  GUI.currentGUI.showPlugin(openedPlugin);
                }
                
              } else if (pluginSubElement.getName().equals("width")) {
                size.width = Integer.parseInt(pluginSubElement.getText());
                openedPlugin.setSize(size);
              } else if (pluginSubElement.getName().equals("height")) {
                size.height = Integer.parseInt(pluginSubElement.getText());
                openedPlugin.setSize(size);
              } else if (pluginSubElement.getName().equals("location_x")) {
                location.x = Integer.parseInt(pluginSubElement.getText());
                openedPlugin.setLocation(location);
              } else if (pluginSubElement.getName().equals("location_y")) {
                location.y = Integer.parseInt(pluginSubElement.getText());
                openedPlugin.setLocation(location);
              } else if (pluginSubElement.getName().equals("minimized")) {
                openedPlugin.setIcon(Boolean.parseBoolean(pluginSubElement.getText()));
              } else if (pluginSubElement.getName().equals("visplugin_config")) {
                logger.fatal("NOT IMPLEMENTED: Not passing plugin specific data yet: " + pluginSubElement.getText());
              }
            }
          } catch (Exception e) {
            logger.fatal("Error when startup up saved plugins: " + e);
          }
        }
      }
    } catch (JDOMException e) {
      logger.fatal("File not wellformed: " + e.getMessage());
      return null;
    } catch (IOException e) {
      logger.fatal("No access to file: " + e.getMessage());
      return null;
    } catch (Exception e) {
      logger.fatal("Exception when loading file: " + e);
      e.printStackTrace();
      return null;
    }

    return newSim;
  }

  /**
   * Saves current simulation configuration to given file and notifies
   * observers.
   * 
   * @see #loadSimulationConfig(File file)
   * @see #getConfigXML()
   * @param file
   *          File to write
   */
  public void saveSimulationConfig(File file) {

    try {
      // Create simulation XML
      Element root = new Element("simulation");
      root.addContent(getConfigXML());

      // EXPERIMENTAL: Store opened plugins information
      Element pluginElement, pluginSubElement;
      if (GUI.currentGUI != null) {
        for (JInternalFrame openedFrame: GUI.currentGUI.getAllFrames()) {
          VisPlugin openedPlugin = (VisPlugin) openedFrame;
          int pluginType = openedPlugin.getClass().getAnnotation(VisPluginType.class).value();

          pluginElement = new Element("visplugin");
          pluginElement.setText(openedPlugin.getClass().getName());

          pluginSubElement = new Element("constructor");
          if (pluginType == VisPluginType.GUI_PLUGIN) {
            pluginSubElement.setText("gui");
            pluginElement.addContent(pluginSubElement);
          } else if (pluginType == VisPluginType.SIM_PLUGIN || 
              pluginType == VisPluginType.SIM_STANDARD_PLUGIN) {
            pluginSubElement.setText("sim");
            pluginElement.addContent(pluginSubElement);
          } else if (pluginType == VisPluginType.MOTE_PLUGIN) {
            if (openedPlugin.getClientProperty("mote") != null) {
              Mote taggedMote = (Mote) openedPlugin.getClientProperty("mote");
              for (int moteNr = 0; moteNr < getMotesCount(); moteNr++) {
                if (getMote(moteNr) == taggedMote) {
                  pluginSubElement.setText("mote: " + moteNr);
                  pluginElement.addContent(pluginSubElement);
                }
              }
            }
          }

          pluginSubElement = new Element("width");
          pluginSubElement.setText("" + openedPlugin.getSize().width);
          pluginElement.addContent(pluginSubElement);
          
          pluginSubElement = new Element("height");
          pluginSubElement.setText("" + openedPlugin.getSize().height);
          pluginElement.addContent(pluginSubElement);
          
          pluginSubElement = new Element("location_x");
          pluginSubElement.setText("" + openedPlugin.getLocation().x);
          pluginElement.addContent(pluginSubElement);
          
          pluginSubElement = new Element("location_y");
          pluginSubElement.setText("" + openedPlugin.getLocation().y);
          pluginElement.addContent(pluginSubElement);
          
          pluginSubElement = new Element("minimized");
          pluginSubElement.setText(new Boolean(openedPlugin.isIcon()).toString());
          pluginElement.addContent(pluginSubElement);
          
          Collection pluginXML = openedPlugin.getConfigXML();
          if (pluginXML != null) {
            pluginSubElement = new Element("visplugin_config");
            pluginSubElement.addContent(pluginXML);
            pluginElement.addContent(pluginSubElement);
          }

          root.addContent(pluginElement);
        }
      }
      
      // Create config
      Document doc = new Document(root);

      // Write to file
      FileOutputStream out = new FileOutputStream(file);
      XMLOutputter outputter = new XMLOutputter();
      outputter.setFormat(Format.getPrettyFormat());
      outputter.output(doc, out);

      out.close();

      logger.info("Saved to file: " + file.getAbsolutePath());
    } catch (Exception e) {
      logger.warn("Exception while saving simulation config: " + e);
    }

    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Returns the current simulation config represented by XML elements. This
   * config also includes the current radio medium, all mote types and motes.
   * 
   * @see #saveSimulationConfig(File file)
   * @return Current simulation config
   */
  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Title
    element = new Element("title");
    element.setText(title);
    config.add(element);

    // Delay time
    element = new Element("delaytime");
    element.setText(Integer.toString(delayTime));
    config.add(element);

    // Simulation time
    element = new Element("simtime");
    element.setText(Integer.toString(currentSimulationTime));
    config.add(element);

    // Tick time
    element = new Element("ticktime");
    element.setText(Integer.toString(tickTime));
    config.add(element);

    // Radio Medium
    element = new Element("radiomedium");
    element.setText(currentRadioMedium.getClass().getName());

    Collection radioMediumXML = currentRadioMedium.getConfigXML();
    if (radioMediumXML != null)
      element.addContent(radioMediumXML);
    config.add(element);

    // Mote types
    for (MoteType moteType : getMoteTypes()) {
      element = new Element("motetype");
      element.setText(moteType.getClass().getName());

      Collection moteTypeXML = moteType.getConfigXML();
      if (moteTypeXML != null)
        element.addContent(moteTypeXML);
      config.add(element);
    }

    // Motes
    for (Mote mote : motes) {
      element = new Element("mote");
      element.setText(mote.getClass().getName());

      Collection moteXML = mote.getConfigXML();
      if (moteXML != null)
        element.addContent(moteXML);
      config.add(element);
    }

    return config;
  }

  /**
   * Sets the current simulation config depending on the given XML elements.
   * 
   * @see #getConfigXML()
   * @param configXML
   *          Config XML elements
   */
  public boolean setConfigXML(Collection<Element> configXML) throws Exception {

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

      // Simulation time
      if (element.getName().equals("simtime")) {
        currentSimulationTime = Integer.parseInt(element.getText());
      }

      // Tick time
      if (element.getName().equals("ticktime")) {
        tickTime = Integer.parseInt(element.getText());
      }

      // Radio medium
      if (element.getName().equals("radiomedium")) {
        String radioMediumClassName = element.getText().trim();
        Class<? extends RadioMedium> radioMediumClass = GUI.currentGUI
            .tryLoadClass(this, RadioMedium.class, radioMediumClassName);

        if (radioMediumClass != null)
          // Create radio medium specified in config
          currentRadioMedium = radioMediumClass.newInstance();
        else
          logger.warn("Could not find radio medium class: "
              + radioMediumClassName);

        // Show configure simulation dialog
        boolean createdOK = CreateSimDialog.showDialog(GUI.frame, this);

        if (!createdOK) {
          logger.debug("Simulation not created, aborting");
          throw new Exception("Load aborted by user");
        }

        // Check if radio medium specific config should be applied
        if (radioMediumClassName
            .equals(currentRadioMedium.getClass().getName())) {
          currentRadioMedium.setConfigXML(element.getChildren());
        } else {
          logger
              .info("Radio Medium changed - ignoring radio medium specific config");
        }
      }

      // Mote type
      if (element.getName().equals("motetype")) {
        String moteTypeClassName = element.getText().trim();

        Class<? extends MoteType> moteTypeClass = GUI.currentGUI.tryLoadClass(
            this, MoteType.class, moteTypeClassName);

        if (moteTypeClass == null) {
          logger.fatal("Could not load mote type class: " + moteTypeClassName);
          return false;
        }

        MoteType moteType = moteTypeClass.getConstructor((Class[]) null)
            .newInstance();

        boolean createdOK = moteType.setConfigXML(this, element.getChildren());
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
        Class<? extends Mote> moteClass = GUI.currentGUI.tryLoadClass(this,
            Mote.class, element.getText().trim());

        Mote mote = moteClass.getConstructor((Class[]) null).newInstance();
        if (mote.setConfigXML(this, element.getChildren())) {
          addMote(mote);
        } else {
          logger.fatal("Mote was not created: " + element.getText().trim());
          throw new Exception("All motes were not recreated");
        }
      }
    }

    return true;
  }

  /**
   * Removes a mote from this simulation
   * 
   * @param mote
   *          Mote to remove
   */
  public void removeMote(Mote mote) {
    if (isRunning()) {
      stopSimulation();
      motes.remove(mote);
      startSimulation();
    } else
      motes.remove(mote);

    currentRadioMedium.unregisterMote(mote, this);
    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Adds a mote to this simulation
   * 
   * @param mote
   *          Mote to add
   */
  public void addMote(Mote mote) {
    if (isRunning()) {
      stopSimulation();
      motes.add(mote);
      startSimulation();
    } else
      motes.add(mote);

    currentRadioMedium.registerMote(mote, this);
    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Get a mote from this simulation.
   * 
   * @param pos
   *          Position of mote
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
   * Returns all mote types in simulation.
   * 
   * @return All mote types
   */
  public Vector<MoteType> getMoteTypes() {
    return moteTypes;
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
      if (moteType.getIdentifier().equals(identifier))
        return moteType;
    }
    return null;
  }

  /**
   * Adds given mote type to simulation.
   * 
   * @param newMoteType
   */
  public void addMoteType(MoteType newMoteType) {
    moteTypes.add(newMoteType);

    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Set delay time to delayTime. When all motes have been ticked, the
   * simulation waits for this time before ticking again.
   * 
   * @param delayTime
   *          New delay time (ms)
   */
  public void setDelayTime(int delayTime) {
    this.delayTime = delayTime;

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
   * @return Simulation time (ms)
   */
  public int getSimulationTime() {
    return currentSimulationTime;
  }

  /**
   * Set tick time to tickTime. The tick time is the simulated time every tick
   * takes. When all motes have been ticked, current simulation time is
   * increased with tickTime. Default tick time is 1 ms.
   * 
   * @see #getTickTime()
   * @see #getTickTimeInSeconds()
   * @param tickTime
   *          New tick time (ms)
   */
  public void setTickTime(int tickTime) {
    this.tickTime = tickTime;

    this.setChanged();
    this.notifyObservers(this);
  }

  /**
   * Changes radio medium of this simulation to the given.
   * 
   * @param radioMedium
   *          New radio medium
   */
  public void setRadioMedium(RadioMedium radioMedium) {
    // Remove current radio medium from observing motes
    if (currentRadioMedium != null)
      for (int i = 0; i < motes.size(); i++)
        currentRadioMedium.unregisterMote(motes.get(i), this);

    // Change current radio medium to new one
    if (radioMedium == null) {
      logger.fatal("Radio medium could not be created!");
      return;
    }
    this.currentRadioMedium = radioMedium;

    // Add all current motes to be observered by new radio medium
    for (int i = 0; i < motes.size(); i++)
      currentRadioMedium.registerMote(motes.get(i), this);
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
   * Get current tick time (ms).
   * 
   * @see #setTickTime(int)
   * @return Current tick time (ms)
   */
  public int getTickTime() {
    return tickTime;
  }

  /**
   * Get current tick time (seconds).
   * 
   * @see #setTickTime(int)
   * @return Current tick time (seconds)
   */
  public double getTickTimeInSeconds() {
    return ((double) tickTime) / 1000.0;
  }

  /**
   * Return true is simulation is running.
   * 
   * @return True if simulation is running
   */
  public boolean isRunning() {
    return isRunning && thread != null;
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
