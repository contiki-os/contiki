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
 */

package org.contikios.cooja.contikimote;

import java.util.ArrayList;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;
import org.contikios.cooja.Mote;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.MoteInterfaceHandler;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.mote.memory.SectionMoteMemory;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.mote.memory.MemoryInterface;
import org.contikios.cooja.motes.AbstractWakeupMote;

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
public class ContikiMote extends AbstractWakeupMote implements Mote {
  private static Logger logger = Logger.getLogger(ContikiMote.class);

  private ContikiMoteType myType = null;
  private SectionMoteMemory myMemory = null;
  private MoteInterfaceHandler myInterfaceHandler = null;

  /**
   * Creates a new mote of given type.
   * Both the initial mote memory and the interface handler
   * are supplied from the mote type.
   *
   * @param moteType Mote type
   * @param sim Mote's simulation
   */
  public ContikiMote(ContikiMoteType moteType, Simulation sim) {
    setSimulation(sim);
    this.myType = moteType;
    this.myMemory = moteType.createInitialMemory();
    this.myInterfaceHandler = new MoteInterfaceHandler(this, moteType.getMoteInterfaceClasses());

    requestImmediateWakeup();
  }

  @Override
  public int getID() {
    return myInterfaceHandler.getMoteID().getMoteID();
  }

  @Override
  public MoteInterfaceHandler getInterfaces() {
    return myInterfaceHandler;
  }

  public void setInterfaces(MoteInterfaceHandler newInterfaces) {
    myInterfaceHandler = newInterfaces;
  }

  @Override
  public MemoryInterface getMemory() {
    return myMemory;
  }

  public void setMemory(SectionMoteMemory memory) {
    myMemory = memory;
  }

  @Override
  public MoteType getType() {
    return myType;
  }

  public void setType(MoteType type) {
    myType = (ContikiMoteType) type;
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
  @Override
  public void execute(long simTime) {

    /* Poll mote interfaces */
    myInterfaceHandler.doActiveActionsBeforeTick();
    myInterfaceHandler.doPassiveActionsBeforeTick();

    /* Check if pre-boot time */
    if (myInterfaceHandler.getClock().getTime() < 0) {
      scheduleNextWakeup(simTime + -myInterfaceHandler.getClock().getTime());
      return;
    }

    /* Copy mote memory to Contiki */
    myType.setCoreMemory(myMemory);

    /* Handle a single Contiki events */
    myType.tick();

    /* Copy mote memory from Contiki */
    myType.getCoreMemory(myMemory);

    /* Poll mote interfaces */
    myMemory.pollForMemoryChanges();
    myInterfaceHandler.doActiveActionsAfterTick();
    myInterfaceHandler.doPassiveActionsAfterTick();
  }

  /**
   * Returns the current Contiki mote config represented by XML elements.
   * This config also includes all mote interface configs.
   *
   * @return Current simulation config
   */
  @Override
  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    /* Mote interfaces */
    for (MoteInterface moteInterface: getInterfaces().getInterfaces()) {
      element = new Element("interface_config");
      element.setText(moteInterface.getClass().getName());

      Collection<Element> interfaceXML = moteInterface.getConfigXML();
      if (interfaceXML != null) {
        element.addContent(interfaceXML);
        config.add(element);
      }
    }

    return config;
  }

  @Override
  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    setSimulation(simulation);
    myMemory = myType.createInitialMemory();
    myInterfaceHandler = new MoteInterfaceHandler(this, myType.getMoteInterfaceClasses());

    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        /* Ignored: handled by simulation */
      } else if (name.equals("interface_config")) {
        String intfClass = element.getText().trim();

        /* Backwards compatibility: se.sics -> org.contikios */
        if (intfClass.startsWith("se.sics")) {
          intfClass = intfClass.replaceFirst("se\\.sics", "org.contikios");
        }

        Class<? extends MoteInterface> moteInterfaceClass =
            simulation.getCooja().tryLoadClass(this, MoteInterface.class, intfClass);

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: " + intfClass);
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

    requestImmediateWakeup();
    return true;
  }

  @Override
  public String toString() {
    return "Contiki " + getID();
  }

}
