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
 * $Id: ContikiMote.java,v 1.14 2009/09/17 13:20:03 fros4943 Exp $
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
 *
 * @author      Fredrik Osterlind
 */
public class ContikiMote implements Mote {
  private static Logger logger = Logger.getLogger(ContikiMote.class);

  private ContikiMoteType myType = null;
  private SectionMoteMemory myMemory = null;
  private MoteInterfaceHandler myInterfaceHandler = null;
  private Simulation mySim = null;

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
    this.myInterfaceHandler = new MoteInterfaceHandler(this, moteType.getMoteInterfaceClasses());
    
    scheduleNextWakeup(mySim.getSimulationTime());
  }

  public int getID() {
    return myInterfaceHandler.getMoteID().getMoteID();
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
   * Ticks mote once. This is done by first polling all interfaces
   * and letting them act on the stored memory before the memory is set. Then
   * the mote is ticked, and the new memory is received.
   * Finally all interfaces are allowing to act on the new memory in order to
   * discover relevant changes. This method also schedules the next mote tick time
   * depending on Contiki specifics; pending timers and polled processes.
   *
   * @param simTime Current simulation time
   */
  public boolean tick(long simTime) {

    /* Poll mote interfaces */
    myInterfaceHandler.doActiveActionsBeforeTick();
    myInterfaceHandler.doPassiveActionsBeforeTick();

    /* Check if pre-boot time */
    if (myInterfaceHandler.getClock().getTime() < 0) {
      scheduleNextWakeup(simTime + -myInterfaceHandler.getClock().getTime());
      return false;
    }

    /* Copy mote memory to Contiki */
    myType.setCoreMemory(myMemory);

    /* Handle a single Contiki events */
    myType.tick();

    /* Copy mote memory from Contiki */
    myType.getCoreMemory(myMemory);

    /* Poll mote interfaces */
    myInterfaceHandler.doActiveActionsAfterTick();
    myInterfaceHandler.doPassiveActionsAfterTick();

    return false;
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

    // Mote interfaces
    for (MoteInterface moteInterface: getInterfaces().getInterfaces()) {
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

    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        myType = (ContikiMoteType) simulation.getMoteType(element.getText());
        myMemory = myType.createInitialMemory();
        myInterfaceHandler = new MoteInterfaceHandler(this, myType.getMoteInterfaceClasses());

      } else if (name.equals("interface_config")) {
        Class<? extends MoteInterface> moteInterfaceClass =
          simulation.getGUI().tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: " + element.getText().trim());
          return false;
        }

        MoteInterface moteInterface = myInterfaceHandler.getInterfaceOfType(moteInterfaceClass);
        if (moteInterface != null) {
          moteInterface.setConfigXML(element.getChildren(), visAvailable);
        } else {
          logger.warn("Can't restore configuration for non-existing interface: " + moteInterfaceClass.getName());
        }
      }
    }

    scheduleNextWakeup(mySim.getSimulationTime());
    return true;
  }

  public String toString() {
    return "Contiki " + getID();
  }

  private TimeEvent tickMoteEvent = new MoteTimeEvent(this, 0) {
    public void execute(long t) {
      /* Tick Contiki mote */
      tick(mySim.getSimulationTime());
    }
    public String toString() {
      return "CONTIKI TICK " + ContikiMote.this;
    }
  };

  public void scheduleImmediateWakeup() {
    scheduleNextWakeup(mySim.getSimulationTime());
  }
  
  public void scheduleNextWakeup(long time) {
    if (tickMoteEvent.isScheduled() &&
        tickMoteEvent.getTime() <= time) {
      /* Native tick events already scheduled */
      return;
    }

    /* Reschedule native mote event */
    /*logger.info("Rescheduled tick from " + tickMoteEvent.time + " to " + time);*/
    mySim.scheduleEventUnsafe(tickMoteEvent, time);
  }
}
