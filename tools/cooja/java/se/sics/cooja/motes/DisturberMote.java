/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
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
 * $Id: DisturberMote.java,v 1.4 2008/10/28 13:39:24 fros4943 Exp $
 */

package se.sics.cooja.motes;

import java.util.*;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.motes.DisturberRadio;

/**
 * A disturber mote is a purely Java-based mote. It is used to disturb
 * transmission of other nodes on a certain channel (currently this is
 * hard-coded in the DisturberRadio.
 *
 * @author Fredrik Osterlind, Thiemo Voigt
 */
public class DisturberMote implements Mote {

  private static Logger logger = Logger.getLogger(DisturberMote.class);

  private MoteType myType = null;

  private SectionMoteMemory myMemory = null;

  private MoteInterfaceHandler myInterfaceHandler = null;

  private Simulation mySim = null;

  private Random myRandom = new Random();

  private DisturberRadio myDisturberRadio;

  /**
   * Creates a new uninitialized dummy mote.
   */
  public DisturberMote() {
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
  public DisturberMote(MoteType moteType, Simulation sim) {
    mySim = sim;
    myType = moteType;

    // Create memory
    myMemory = new SectionMoteMemory(new Properties());

    // Create interface handler
    myInterfaceHandler = new MoteInterfaceHandler();
    Position myPosition = new Position(this);
    myPosition.setCoordinates(myRandom.nextDouble() * 100, myRandom
        .nextDouble() * 100, myRandom.nextDouble() * 100);
    myInterfaceHandler.addInterface(myPosition);

    // create interface handler for radio
    myDisturberRadio = new DisturberRadio(this);
    myInterfaceHandler.addInterface(myDisturberRadio);
  }

  public void setState(State newState) {
    logger.fatal("Disturber mote can not change state");
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

  public boolean tick(int simTime) {
    myInterfaceHandler.doPassiveActionsBeforeTick();
    myInterfaceHandler.doActiveActionsBeforeTick();
    myInterfaceHandler.doActiveActionsAfterTick();
    myInterfaceHandler.doPassiveActionsAfterTick();
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

    // Interfaces
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

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable) {

    // Set initial configuration
    mySim = simulation;
    myMemory = new SectionMoteMemory(new Properties());
    myInterfaceHandler = new MoteInterfaceHandler();
    Position myPosition = new Position(this);
    myInterfaceHandler.addInterface(myPosition);
    myDisturberRadio = new DisturberRadio(this);
    myInterfaceHandler.addInterface(myDisturberRadio);


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
      return "Disturber Mote, ID=" + getInterfaces().getMoteID().getMoteID();
    } else {
      return "Disturber Mote, ID=null";
    }
  }

}
