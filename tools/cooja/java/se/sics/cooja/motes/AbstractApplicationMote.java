/*
 * Copyright (c) 2007, Swedish Institute of Computer Science. All rights
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
 * $Id: AbstractApplicationMote.java,v 1.4 2009/03/09 15:38:10 fros4943 Exp $
 */

package se.sics.cooja.motes;

import java.util.*;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.Radio;

/**
 * Abstract application mote.
 *
 * Simplifies implementation of application level mote types.
 *
 * @author Fredrik Osterlind
 */
public abstract class AbstractApplicationMote implements Mote {

  private static Logger logger = Logger.getLogger(AbstractApplicationMote.class);

  private MoteType moteType = null;

  private SectionMoteMemory memory = null;

  protected MoteInterfaceHandler moteInterfaces = null;

  private Simulation simulation = null;

  private Observer radioDataObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      if (getInterfaces().getRadio().getLastEvent() != Radio.RadioEvent.RECEPTION_FINISHED) {
        return;
      }

      /* Called at incoming data packets */
      logger.info("Application mote received radio data:");
      byte[] packet = getInterfaces().getRadio().getLastPacketReceived().getPacketData();
      String data = "";
      for (byte b: packet) {
        data += (char)b;
      }
      logger.info(data);
    }
  };

  public AbstractApplicationMote() {
  }

  public AbstractApplicationMote(MoteType moteType, Simulation sim) {
    this.simulation = sim;
    this.moteType = moteType;

    // Create memory
    this.memory = new SectionMoteMemory(new Properties());

    // Create mote interfaces
    this.moteInterfaces = new MoteInterfaceHandler(this, moteType.getMoteInterfaceClasses());

    if (moteInterfaces.getRadio() != null) {
      moteInterfaces.getRadio().addObserver(radioDataObserver);
    }
  }

  public void setState(State newState) {
    logger.fatal("Application mote can not change state");
  }

  public State getState() {
    return Mote.State.ACTIVE;
  }

  public void addStateObserver(Observer newObserver) {
  }

  public void deleteStateObserver(Observer newObserver) {
  }

  public MoteInterfaceHandler getInterfaces() {
    return moteInterfaces;
  }

  public void setInterfaces(MoteInterfaceHandler moteInterfaceHandler) {
    moteInterfaces = moteInterfaceHandler;
  }

  public MoteMemory getMemory() {
    return memory;
  }

  public void setMemory(MoteMemory memory) {
    this.memory = (SectionMoteMemory) memory;
  }

  public MoteType getType() {
    return moteType;
  }

  public void setType(MoteType type) {
    moteType = type;
  }

  public Simulation getSimulation() {
    return simulation;
  }

  public void setSimulation(Simulation simulation) {
    this.simulation = simulation;
  }

  public boolean tick(long simTime) {
    moteInterfaces.doPassiveActionsBeforeTick();
    moteInterfaces.doActiveActionsBeforeTick();

    /* TODO Implement application functionality here */

    moteInterfaces.doActiveActionsAfterTick();
    moteInterfaces.doPassiveActionsAfterTick();
    return false;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // We need to save the mote type identifier
    element = new Element("motetype_identifier");
    element.setText(getType().getIdentifier());
    config.add(element);

    // Interfaces
    for (MoteInterface moteInterface: moteInterfaces.getInterfaces()) {
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
    this.simulation = simulation;
    this.memory = new SectionMoteMemory(new Properties());

    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        moteType = simulation.getMoteType(element.getText());
        this.moteInterfaces = new MoteInterfaceHandler(this, moteType.getMoteInterfaceClasses());
      } else if (name.equals("interface_config")) {
        Class<? extends MoteInterface> moteInterfaceClass =
          simulation.getGUI().tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.warn("Can't find mote interface class: " + element.getText());
          return false;
        }

        MoteInterface moteInterface = moteInterfaces.getInterfaceOfType(moteInterfaceClass);
        moteInterface.setConfigXML(element.getChildren(), visAvailable);
      }

    }
    return true;
  }

  public String toString() {
    if (getInterfaces().getMoteID() != null) {
      return "Application Mote, ID=" + getInterfaces().getMoteID().getMoteID();
    } else {
      return "Application Mote, ID=null";
    }
  }

}
