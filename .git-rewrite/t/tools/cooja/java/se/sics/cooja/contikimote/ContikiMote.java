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
 * $Id: ContikiMote.java,v 1.6 2007/07/13 09:08:24 fros4943 Exp $
 */

package se.sics.cooja.contikimote;

import java.util.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * A Contiki mote executes an actual Contiki system via
 * a loaded shared library and JNI.
 * It contains a section mote memory, a mote interface handler and a
 * Contiki mote type.
 * 
 * The mote type is responsible for the connection to the loaded
 * Contiki system.
 * 
 * When ticked a Contiki mote polls all interfaces, copies the mote
 * memory to the core, lets the Contiki system handle one event, 
 * fetches the updated memory and finally polls all interfaces again. 
 * The mote state is also updated during a mote tick.
 * 
 * @author      Fredrik Osterlind
 */
public class ContikiMote implements Mote {
  private static Logger logger = Logger.getLogger(ContikiMote.class);

  private ContikiMoteType myType = null;
  private SectionMoteMemory myMemory = null;
  private MoteInterfaceHandler myInterfaceHandler = null;
  private Simulation mySim = null;

  // Time to wake up if sleeping
  private int wakeUpTime = 0;

  private State myState = State.ACTIVE;

  // State observable
  private class StateObservable extends Observable {
    private void stateChanged() {
      setChanged();
      notifyObservers();
    }
  }
  private StateObservable stateObservable = new StateObservable();

  
  /**
   * Creates a new uninitialized Contiki mote.
   * 
   * This mote needs at least a type, a memory, a mote interface handler
   * and to be connected to a simulation.
   */
  public ContikiMote() {
  }
  
  /**
   * Creates a new mote of given type.
   * Both the initial mote memory and the interface handler
   * are supplied from the mote type.
   * 
   * @param moteType Mote type
   * @param sim Mote's simulation
   */
  public ContikiMote(ContikiMoteType moteType, Simulation sim) {
    this.mySim = sim;
    this.myType = moteType;
    this.myMemory = moteType.createInitialMemory();
    this.myInterfaceHandler = new MoteInterfaceHandler((Mote) this, moteType.getMoteInterfaces());
    
    myState = State.ACTIVE;
  }

  public void setState(State newState) {
    if (myState == State.DEAD) {
      return;
    }

    if (myState == State.ACTIVE && newState != State.ACTIVE) {
      myState = newState;
      stateObservable.stateChanged();
    }

    if (myState == State.LPM && newState != State.LPM) {
      myState = newState;
      stateObservable.stateChanged();
    }
    
    if (myState == State.DEAD) {
      mySim.getRadioMedium().unregisterMote(this, mySim);
    }
  }

  public State getState() {
    return myState;
  }

  public void addStateObserver(Observer newObserver) {
    stateObservable.addObserver(newObserver);
  }

  public void deleteStateObserver(Observer newObserver) {
    stateObservable.deleteObserver(newObserver);
  }
  
  public MoteInterfaceHandler getInterfaces() {
    return myInterfaceHandler;
  }
  
  public void setInterfaces(MoteInterfaceHandler newInterfaces) {
    myInterfaceHandler = newInterfaces;
  }

  public MoteMemory getMemory() {
    return myMemory;
  }

  public void setMemory(MoteMemory memory) {
    myMemory = (SectionMoteMemory) memory;
  }

  public MoteType getType() {
    return myType;
  }

  public void setType(MoteType type) {
    myType = (ContikiMoteType) type;
  }

  public Simulation getSimulation() {
    return mySim;
  }

  public void setSimulation(Simulation simulation) {
    mySim = simulation;
  }

  /**
   * Ticks this mote once. This is done by first polling all interfaces
   * and letting them act on the stored memory before the memory is set. Then
   * the mote is ticked, and the new memory is received.
   * Finally all interfaces are allowing to act on the new memory in order to
   * discover relevant changes. This method also checks if mote should go to sleep
   * depending on Contiki specifics; pending timers and polled processes.
   *
   * @param simTime Current simulation time
   */
  public void tick(int simTime) {
    State currentState = getState();
    
    // If mote is dead, do nothing at all
    if (currentState == State.DEAD)
      return;

    // If mote is sleeping and has a wake up time, should it wake up now?
    if (currentState == State.LPM && wakeUpTime > 0 && wakeUpTime <= simTime) {
      setState(State.ACTIVE);
      currentState = getState();
      wakeUpTime = 0;
    }

    // If mote is active..
    if (currentState == State.ACTIVE) {
      // Let all active interfaces act before tick
      // Observe that each interface may put the mote to sleep at this point
      myInterfaceHandler.doActiveActionsBeforeTick();
    }

    // And let passive interfaces act even if mote is sleeping
    myInterfaceHandler.doPassiveActionsBeforeTick();


    // If mote is still active, complete this tick
    currentState = getState();
    if (currentState == State.ACTIVE) {
      
      // Copy mote memory to core
      myType.setCoreMemory(myMemory);

      // Tick node
      myType.tick();

      // Fetch new updated memory from core
      myType.getCoreMemory(myMemory);

      // Let all active interfaces act again after tick
      myInterfaceHandler.doActiveActionsAfterTick();

    }

    // Finally let all passive interfaces act
    myInterfaceHandler.doPassiveActionsAfterTick();

    // If mote is awake, should it go to sleep?
    if (currentState == State.ACTIVE) {
      // Check if this mote should sleep (no more pending timers or processes to poll)
      int processRunValue = myMemory.getIntValueOf("simProcessRunValue");
      int etimersPending = myMemory.getIntValueOf("simEtimerPending");
      int nextExpirationTime = myMemory.getIntValueOf("simNextExpirationTime");
      
      if (processRunValue == 0 && etimersPending == 0) {
        setState(State.LPM);
        wakeUpTime = 0;
      }
      
      if (processRunValue == 0 && etimersPending == 1 && nextExpirationTime > 0) {
        setState(State.LPM);
        wakeUpTime = nextExpirationTime;
      }
      
    }
  }

  /**
   * Returns the current Contiki mote config represented by XML elements.
   * This config also includes all mote interface configs.
   *
   * @return Current simulation config
   */
  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    
    Element element;
    
    // Mote type identifier
    element = new Element("motetype_identifier");
    element.setText(getType().getIdentifier());
    config.add(element);

    // Active interface configs (if any)
    for (MoteInterface moteInterface: getInterfaces().getAllActiveInterfaces()) {
      element = new Element("interface_config");
      element.setText(moteInterface.getClass().getName());

      Collection interfaceXML = moteInterface.getConfigXML();
      if (interfaceXML != null) {
        element.addContent(interfaceXML);
        config.add(element);
      }
    }
   
    // Passive interface configs (if any)
    for (MoteInterface moteInterface: getInterfaces().getAllPassiveInterfaces()) {
      element = new Element("interface_config");
      element.setText(moteInterface.getClass().getName());

      Collection interfaceXML = moteInterface.getConfigXML();
      if (interfaceXML != null) {
        element.addContent(interfaceXML);
        config.add(element);
      }
    }
   
    return config;
  }
  
  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    mySim = simulation;
    myState = State.ACTIVE;
    
    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        myType = (ContikiMoteType) simulation.getMoteType(element.getText());
        myMemory = myType.createInitialMemory();
        myInterfaceHandler = new MoteInterfaceHandler((Mote) this, myType.getMoteInterfaces());

      } else if (name.equals("interface_config")) {
        Class<? extends MoteInterface> moteInterfaceClass = 
          simulation.getGUI().tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: " + element.getText().trim());
          return false;
        }
        
        MoteInterface moteInterface = myInterfaceHandler.getInterfaceOfType(moteInterfaceClass);
        if (moteInterface != null)
          moteInterface.setConfigXML(element.getChildren(), visAvailable);
        else
          logger.warn("Can't restore configuration for non-existing interface: " + moteInterfaceClass.getName());
      }
    }
    
    return true;
  }
  
  public String toString() {
    if (getInterfaces().getMoteID() != null) {
      return "Contiki Mote, ID=" + getInterfaces().getMoteID().getMoteID();
    } else
      return "Contiki Mote, ID=null";
  }
  
}
