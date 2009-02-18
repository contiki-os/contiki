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
 * $Id: DummyMote.java,v 1.7 2009/02/18 10:41:50 fros4943 Exp $
 */

package se.sics.cooja.motes;

import java.util.Collection;
import java.util.Observer;
import java.util.Properties;
import java.util.Random;
import java.util.Vector;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.Position;

/**
 * A dummy mote is a purely Java-based mote, and can be used as an example of
 * how to implement motes other than the usual Contiki mote.
 *
 * The dummy mote uses an empty section mote memory without any variable
 * mappings.
 *
 * The mote interface handler has a position interface, added when the mote is
 * constructed.
 *
 * When the dummy mote is ticked all (one!) interfaces are polled and a random
 * variable decides if the position should be changed. The node never leaves the
 * active state.
 *
 * @author Fredrik Osterlind
 */
public class DummyMote implements Mote {

  private static Logger logger = Logger.getLogger(DummyMote.class);

  private MoteType myType = null;
  private SectionMoteMemory myMemory = null;
  private MoteInterfaceHandler myInterfaceHandler = null;
  private Simulation mySim = null;

  private Random random = null;

  /**
   * Creates a new uninitialized dummy mote.
   */
  public DummyMote() {
  }

  /**
   * Creates a new dummy mote of the given type in the given simulation. An
   * empty mote memory and a position interface is added to this mote.
   *
   * @param moteType
   *          Mote type
   * @param sim
   *          Simulation
   */
  public DummyMote(MoteType moteType, Simulation sim) {
    mySim = sim;
    myType = moteType;
    random = mySim.getRandomGenerator();

    // Create memory
    myMemory = new SectionMoteMemory(new Properties());

    // Create interface handler
    myInterfaceHandler = new MoteInterfaceHandler();
    Position myPosition = new Position(this);
    myPosition.setCoordinates(
        random.nextDouble() * 100,
        random.nextDouble() * 100,
        random.nextDouble() * 100
    );
    myInterfaceHandler.addInterface(myPosition);
  }

  public void setState(State newState) {
    logger.fatal("Dummy mote can not change state");
  }

  public State getState() {
    return Mote.State.ACTIVE;
  }

  public void addStateObserver(Observer newObserver) {
  }

  public void deleteStateObserver(Observer newObserver) {
  }

  public MoteInterfaceHandler getInterfaces() {
    return myInterfaceHandler;
  }

  public void setInterfaces(MoteInterfaceHandler moteInterfaceHandler) {
    myInterfaceHandler = moteInterfaceHandler;
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
    myType = type;
  }

  public Simulation getSimulation() {
    return mySim;
  }

  public void setSimulation(Simulation simulation) {
    this.mySim = simulation;
  }

  public boolean tick(long simTime) {

    // Perform some dummy task
    if (random.nextDouble() > 0.9) {
      // Move mote randomly
      Position myPosition = myInterfaceHandler.getPosition();
      myPosition.setCoordinates(myPosition.getXCoordinate()
          + random.nextDouble() - 0.5, myPosition.getYCoordinate()
          + random.nextDouble() - 0.5, myPosition.getZCoordinate()
          + random.nextDouble() - 0.5);
    }
    return false;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // We need to save the mote type identifier
    element = new Element("motetype_identifier");
    element.setText(getType().getIdentifier());
    config.add(element);

    // The position interface should also save its config
    element = new Element("interface_config");
    element.setText(myInterfaceHandler.getPosition().getClass().getName());

    Collection interfaceXML = myInterfaceHandler.getPosition().getConfigXML();
    if (interfaceXML != null) {
      element.addContent(interfaceXML);
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable) {
    mySim = simulation;
    myMemory = new SectionMoteMemory(new Properties());
    random = mySim.getRandomGenerator();
    myInterfaceHandler = new MoteInterfaceHandler();
    myInterfaceHandler.addInterface(new Position(this));

    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        myType = simulation.getMoteType(element.getText());
      } else if (name.equals("interface_config")) {
        Class<? extends MoteInterface> moteInterfaceClass = simulation.getGUI()
            .tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.warn("Can't find mote interface class: " + element.getText());
          return false;
        }

        MoteInterface moteInterface = myInterfaceHandler
            .getInterfaceOfType(moteInterfaceClass);
        moteInterface.setConfigXML(element.getChildren(), visAvailable);
      }

    }
    return true;
  }

  public String toString() {
    if (getInterfaces().getMoteID() != null) {
      return "Dummy Mote, ID=" + getInterfaces().getMoteID().getMoteID();
    } else {
      return "Dummy Mote, ID=null";
    }
  }

}
