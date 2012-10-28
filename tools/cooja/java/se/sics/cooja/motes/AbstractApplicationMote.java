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
 */

package se.sics.cooja.motes;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.Observable;
import java.util.Observer;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteInterfaceHandler;
import se.sics.cooja.MoteMemory;
import se.sics.cooja.MoteType;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.SectionMoteMemory;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.ApplicationRadio;
import se.sics.cooja.interfaces.ApplicationSerialPort;
import se.sics.cooja.interfaces.Radio;

/**
 * Abstract application mote.
 *
 * Simplifies implementation of application level mote types.
 *
 * @author Fredrik Osterlind
 */
public abstract class AbstractApplicationMote extends AbstractWakeupMote implements Mote {
  private static Logger logger = Logger.getLogger(AbstractApplicationMote.class);

  private MoteType moteType = null;

  private SectionMoteMemory memory = null;

  protected MoteInterfaceHandler moteInterfaces = null;

  /* Observe our own radio for incoming radio packets */
  private Observer radioDataObserver = new Observer() {
    public void update(Observable obs, Object obj) {
      ApplicationRadio radio = (ApplicationRadio) obs;
      if (radio.getLastEvent() == Radio.RadioEvent.RECEPTION_FINISHED) {
        /* only send in packets when they exist */
        if (radio.getLastPacketReceived() != null)
            receivedPacket(radio.getLastPacketReceived());
      } else if (radio.getLastEvent() == Radio.RadioEvent.TRANSMISSION_FINISHED) {
        sentPacket(radio.getLastPacketTransmitted());
      }
    }
  };

  public abstract void receivedPacket(RadioPacket p);
  public abstract void sentPacket(RadioPacket p);
  
  public AbstractApplicationMote() {
    moteInterfaces = new MoteInterfaceHandler(this, moteType.getMoteInterfaceClasses());
  }

  public AbstractApplicationMote(MoteType moteType, Simulation sim) {
    setSimulation(sim);
    this.moteType = moteType;
    this.memory = new SectionMoteMemory(new HashMap<String, Integer>(), 0);
    this.moteInterfaces = new MoteInterfaceHandler(this, moteType.getMoteInterfaceClasses());
    this.moteInterfaces.getRadio().addObserver(radioDataObserver);
    requestImmediateWakeup();
  }

  public void log(String msg) {
    ((ApplicationSerialPort)moteInterfaces.getLog()).triggerLog(msg);
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

  public Collection<Element> getConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    for (MoteInterface moteInterface: moteInterfaces.getInterfaces()) {
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

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable) {
    setSimulation(simulation);
    this.memory = new SectionMoteMemory(new HashMap<String, Integer>(), 0);
    moteInterfaces.getRadio().addObserver(radioDataObserver);

    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        /* Ignored: handled by simulation */
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
    requestImmediateWakeup();
    return true;
  }

  public int getID() {
    return moteInterfaces.getMoteID().getMoteID();
  }
  
  public String toString() {
    return "AppMote " + getID();
  }

  /* These methods should be overriden to allow application motes receiving serial data */
  public void writeArray(byte[] s) {
  }
  public void writeByte(byte b) {
  }
  public void writeString(String s) {
  }
}
