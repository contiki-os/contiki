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
 * $Id: MantisMote.java,v 1.2 2007/01/09 10:02:16 fros4943 Exp $
 */

package se.sics.cooja.mantismote;

import java.util.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * A Mantis mote simulation works the same way as the native Contiki mote. The
 * Mantis OS is compiled and linked together with a JNI-enabled object file
 * generated from COOJA.
 * 
 * Each tick the all interfaces are polled, the memory is copied and the tick is
 * forwarded to the core Mantis system.
 * 
 * A Mantis mote is always active.
 * 
 * @author Fredrik Osterlind
 */
public class MantisMote implements Mote {
  private static Logger logger = Logger.getLogger(MantisMote.class);

  private MantisMoteType myType = null;

  private SectionMoteMemory myMemory = null;

  private MoteInterfaceHandler myInterfaceHandler = null;

  private Simulation mySimulation = null;

  /**
   * Creates a new uninitialized Mantis mote.
   * 
   * This mote needs at least a type, a memory, a mote interface handler and to
   * be connected to a simulation.
   */
  public MantisMote() {
  }

  /**
   * Creates a new mote of given type. Both the initial mote memory and the
   * interface handler are supplied from the mote type.
   * 
   * @param moteType
   *          Mote type
   * @param sim
   *          Mote's simulation
   */
  public MantisMote(MantisMoteType moteType, Simulation sim) {
    this.mySimulation = sim;
    this.myType = moteType;
    this.myMemory = moteType.createInitialMemory();
    this.myInterfaceHandler = new MoteInterfaceHandler((Mote) this, moteType
        .getMoteInterfaces());
  }

  public void setState(State newState) {
    logger.fatal("Mantis motes can't change state");
  }

  public State getState() {
    return State.ACTIVE;
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
    myType = (MantisMoteType) type;
  }

  public Simulation getSimulation() {
    return mySimulation;
  }

  public void setSimulation(Simulation simulation) {
    mySimulation = simulation;
  }

  public void tick(int simTime) {
    // Poll all interfaces before tick
    myInterfaceHandler.doActiveActionsBeforeTick();
    myInterfaceHandler.doPassiveActionsBeforeTick();

    // Copy memory to core
    myType.setCoreMemory(myMemory);

    // Tick node
    myType.tick();

    // Fetch new updated memory from core
    myType.getCoreMemory(myMemory);

    // Poll all interfaces after tick
    myInterfaceHandler.doActiveActionsBeforeTick();
    myInterfaceHandler.doPassiveActionsBeforeTick();
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Mote type identifier
    element = new Element("motetype_identifier");
    element.setText(getType().getIdentifier());
    config.add(element);

    // Active interface configs (if any)
    for (MoteInterface moteInterface : getInterfaces().getAllActiveInterfaces()) {
      element = new Element("interface_config");
      element.setText(moteInterface.getClass().getName());

      Collection interfaceXML = moteInterface.getConfigXML();
      if (interfaceXML != null) {
        element.addContent(interfaceXML);
        config.add(element);
      }
    }

    // Passive interface configs (if any)
    for (MoteInterface moteInterface : getInterfaces()
        .getAllPassiveInterfaces()) {
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
    mySimulation = simulation;

    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        myType = (MantisMoteType) simulation.getMoteType(element.getText());
        myMemory = myType.createInitialMemory();
        myInterfaceHandler = new MoteInterfaceHandler((Mote) this, myType
            .getMoteInterfaces());

      } else if (name.equals("interface_config")) {
        Class<? extends MoteInterface> moteInterfaceClass = simulation.getGUI()
            .tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: "
              + element.getText().trim());
          return false;
        }

        MoteInterface moteInterface = myInterfaceHandler
            .getInterfaceOfType(moteInterfaceClass);
        if (moteInterface != null)
          moteInterface.setConfigXML(element.getChildren(), visAvailable);
        else
          logger
              .warn("Can't restore configuration for non-existing interface: "
                  + moteInterfaceClass.getName());
      }
    }

    return true;
  }

  public String toString() {
    if (getInterfaces().getMoteID() != null) {
      return "Mantis Mote, ID=" + getInterfaces().getMoteID().getMoteID();
    } else
      return "Mantis Mote, ID=null";
  }

}
