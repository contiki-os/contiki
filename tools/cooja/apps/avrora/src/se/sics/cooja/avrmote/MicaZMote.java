/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: MicaZMote.java,v 1.15 2010/10/04 12:54:01 joxe Exp $
 */

package se.sics.cooja.avrmote;

import java.io.File;
import java.util.ArrayList;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteInterfaceHandler;
import se.sics.cooja.MoteMemory;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.motes.AbstractEmulatedMote;
import avrora.arch.avr.AVRProperties;
import avrora.core.LoadableProgram;
import avrora.sim.AtmelInterpreter;
import avrora.sim.Simulator;
import avrora.sim.State;
import avrora.sim.mcu.AtmelMicrocontroller;
import avrora.sim.mcu.EEPROM;
import avrora.sim.platform.MicaZ;
import avrora.sim.platform.PlatformFactory;

/**
 * @author Joakim Eriksson, Fredrik Osterlind
 */
public class MicaZMote extends AbstractEmulatedMote implements Mote {
  private static Logger logger = Logger.getLogger(MicaZMote.class);

  /* 8 MHz according to Contiki config */
  public static long NR_CYCLES_PER_MSEC = 8000;

  private MoteInterfaceHandler myMoteInterfaceHandler;
  private AtmelMicrocontroller myCpu = null;
  private MicaZ micaZ = null;
  private LoadableProgram program = null;
  private AtmelInterpreter interpreter = null;
  private AvrMoteMemory myMemory = null;
  private AVRProperties avrProperties = null;
  private MicaZMoteType myMoteType = null;

  private EEPROM eeprom = null;
  
  /* Stack monitoring variables */
  private boolean stopNextInstruction = false;

  public MicaZMote() {
    myMoteType = null;
    myCpu = null;
    /* TODO myMemory = null; */
    myMoteInterfaceHandler = null;
  }

  public MicaZMote(Simulation simulation, MicaZMoteType type) {
    setSimulation(simulation);
    myMoteType = type;

    /* Schedule us immediately */
    requestImmediateWakeup();
  }

  protected boolean initEmulator(File fileELF) {
    try {
      prepareMote(fileELF);
    } catch (Exception e) {
      logger.fatal("Error when creating MicaZ mote: ", e);
      return false;
    }
    return true;
  }

  /**
   * Abort current tick immediately.
   * May for example be called by a breakpoint handler.
   */
  public void stopNextInstruction() {
    stopNextInstruction = true;
  }

  private MoteInterfaceHandler createMoteInterfaceHandler() {
    return new MoteInterfaceHandler(this, getType().getMoteInterfaceClasses());
  }

  public MicaZ getMicaZ() {
    return micaZ;
  }

  protected void initMote() {
    if (myMoteType != null) {
      initEmulator(myMoteType.getContikiFirmwareFile());
      myMoteInterfaceHandler = createMoteInterfaceHandler();
    }
  }

  /**
   * Prepares CPU, memory and ELF module.
   *
   * @param fileELF ELF file
   * @param cpu MSP430 cpu
   * @throws Exception
   */
  protected void prepareMote(File file) throws Exception {
    program = new LoadableProgram(file);
    program.load();
    PlatformFactory factory = new MicaZ.Factory();
    micaZ = (MicaZ) factory.newPlatform(1, program.getProgram());
    myCpu = (AtmelMicrocontroller) micaZ.getMicrocontroller();
    eeprom = (EEPROM) myCpu.getDevice("eeprom");
    
    avrProperties = (AVRProperties) myCpu.getProperties();
    Simulator sim = myCpu.getSimulator();
    interpreter = (AtmelInterpreter) sim.getInterpreter();
//     State state = interpreter.getState();
    myMemory = new AvrMoteMemory(program.getProgram().getSourceMapping(), avrProperties, interpreter);
  }

  public void setEEPROM(int address, int i) {
      byte[] eedata = eeprom.getContent();
      eedata[address] = (byte) i;
  }
  
  public void setState(State newState) {
    logger.warn("MicaZ motes can't change state");
  }

  public int getID() {
    return getInterfaces().getMoteID().getMoteID();
  }

  /* called when moteID is updated */
  public void idUpdated(int newID) {
      
  }

  public MoteType getType() {
    return myMoteType;
  }

  public void setType(MoteType type) {
    myMoteType = (MicaZMoteType) type;
  }

  public MoteInterfaceHandler getInterfaces() {
    return myMoteInterfaceHandler;
  }

  public void setInterfaces(MoteInterfaceHandler moteInterfaceHandler) {
    myMoteInterfaceHandler = moteInterfaceHandler;
  }

  private long cyclesExecuted = 0;
  private long cyclesUntil = 0;
  public void execute(long t) {
    /* Wait until mote boots */
    if (myMoteInterfaceHandler.getClock().getTime() < 0) {
      scheduleNextWakeup(t - myMoteInterfaceHandler.getClock().getTime());
      return;
    }

    if (stopNextInstruction) {
      stopNextInstruction = false;
      throw new RuntimeException("Avrora requested simulation stop");
    } 

    /* TODO Poll mote interfaces? */

    /* Execute one millisecond */
    cyclesUntil += NR_CYCLES_PER_MSEC;
    while (cyclesExecuted < cyclesUntil) {
      cyclesExecuted += interpreter.step();
    }

    /* TODO Poll mote interfaces? */

    /* Schedule wakeup every millisecond */
    /* TODO Optimize next wakeup time */
    scheduleNextWakeup(t + Simulation.MILLISECOND);
  }
  
  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    setSimulation(simulation);
    initEmulator(myMoteType.getContikiFirmwareFile());
    myMoteInterfaceHandler = createMoteInterfaceHandler();

    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        /* Ignored: handled by simulation */
      } else if (name.equals("interface_config")) {
        Class<? extends MoteInterface> moteInterfaceClass = simulation.getGUI().tryLoadClass(
              this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: " + element.getText().trim());
          return false;
        }

        MoteInterface moteInterface = getInterfaces().getInterfaceOfType(moteInterfaceClass);
        moteInterface.setConfigXML(element.getChildren(), visAvailable);
      }
    }

    /* Schedule us immediately */
    requestImmediateWakeup();
    return true;
  }

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

  public MoteMemory getMemory() {
    return myMemory;
  }

  public void setMemory(MoteMemory memory) {
    myMemory = (AvrMoteMemory) memory;
  }

  public String toString() {
    return "MicaZ " + getID();
  }

}
