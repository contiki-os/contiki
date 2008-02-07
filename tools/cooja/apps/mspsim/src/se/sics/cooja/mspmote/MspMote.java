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
 * $Id: MspMote.java,v 1.1 2008/02/07 14:53:29 fros4943 Exp $
 */

package se.sics.cooja.mspmote;

import java.io.*;
import java.util.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.mspmote.interfaces.TR1001Radio;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.MapTable;
import se.sics.mspsim.util.ELF;

/**
 * @author Fredrik Osterlind
 */
public abstract class MspMote implements Mote {
  private static Logger logger = Logger.getLogger(MspMote.class);

  /* 2.4 MHz */
  public static int NR_CYCLES_PER_MSEC = 2365;

  /* Cycle counter */
  public long cycleCounter = 0;

  private Simulation mySimulation = null;
  private MSP430 myCpu = null;
  private MspMoteType myMoteType = null;
  private MspMoteMemory myMemory = null;
  private MoteInterfaceHandler myMoteInterfaceHandler = null;
  private ELF myELFModule = null;

  protected TR1001Radio myRadio = null; /* TODO Only used by ESB (TR1001) */

  /* Stack monitoring variables */
  private boolean stopNextInstruction = false;
  private boolean monitorStackUsage = false;
  private int stackPointerLow = Integer.MAX_VALUE;
  private int heapStartAddress;
  private StackOverflowObservable stackOverflowObservable = new StackOverflowObservable();

  /**
   * Abort current tick immediately.
   * May for example be called by a breakpoint handler.
   */
  public void stopNextInstruction() {
    stopNextInstruction = true;
  }

  public MspMote() {
    myMoteType = null;
    mySimulation = null;
    myCpu = null;
    myMemory = null;
    myMoteInterfaceHandler = null;
  }

  public MspMote(MspMoteType moteType, Simulation simulation) {
    myMoteType = moteType;
    mySimulation = simulation;

    initEmulator(myMoteType.getELFFile());

    myMoteInterfaceHandler = createMoteInterfaceHandler();
  }

  /**
   * @return MSP430 CPU
   */
  public MSP430 getCPU() {
    return myCpu;
  }

  public MoteMemory getMemory() {
    return myMemory;
  }

  public void setMemory(MoteMemory memory) {
    myMemory = (MspMoteMemory) memory;
  }

  public Simulation getSimulation() {
    return mySimulation;
  }

  public void setSimulation(Simulation simulation) {
    mySimulation = simulation;
  }

  /* Stack monitoring variables */
  public class StackOverflowObservable extends Observable {
    public void signalStackOverflow() {
      setChanged();
      notifyObservers();
    }
  }

  /**
   * Enable/disable stack monitoring
   *
   * @param monitoring Monitoring enabled
   */
  public void monitorStack(boolean monitoring) {
    this.monitorStackUsage = monitoring;
    resetLowestStackPointer();
  }

  /**
   * @return Lowest SP since stack monitoring was enabled
   */
  public int getLowestStackPointer() {
    return stackPointerLow;
  }

  /**
   * Resets lowest stack pointer variable
   */
  public void resetLowestStackPointer() {
    stackPointerLow = Integer.MAX_VALUE;
  }

  /**
   * @return Stack overflow observable
   */
  public StackOverflowObservable getStackOverflowObservable() {
    return stackOverflowObservable;
  }

  /**
   * Creates MSP430 CPU object and address memory for current object.
   * This method should normally not be called from outside constructor.
   *
   * @param fileELF ELF file
   * @throws IOException File loading failed
   */
  protected void createCPUAndMemory(File fileELF) throws IOException {
    myCpu = new MSP430(0);
    myCpu.setMonitorExec(true);

    int[] memory = myCpu.getMemory();

    myELFModule = ELF.readELF(fileELF.getPath());
    myELFModule.loadPrograms(memory);
    MapTable map = myELFModule.getMap();
    myCpu.getDisAsm().setMap(map);
    myCpu.setMap(map);

    /* TODO Need new memory type including size and type as well */

    /* Create mote address memory */
    Properties varAddresses = new Properties();
    for (int i=0; i < map.functionNames.length; i++) {
      if (map.functionNames[i] != null) {
        varAddresses.put(map.functionNames[i], new Integer(i));
      }
    }
    myMemory = new MspMoteMemory(varAddresses, myCpu);

    myCpu.reset();
  }

  public void setState(State newState) {
    logger.warn("Msp motes can't change state");
  }

  public State getState() {
    return Mote.State.ACTIVE;
  }

  public MoteType getType() {
    return myMoteType;
  }

  public void setType(MoteType type) {
    myMoteType = (MspMoteType) type;
  }

  public void addStateObserver(Observer newObserver) {
  }

  public void deleteStateObserver(Observer newObserver) {
  }

  public MoteInterfaceHandler getInterfaces() {
    return myMoteInterfaceHandler;
  }

  public void setInterfaces(MoteInterfaceHandler moteInterfaceHandler) {
    myMoteInterfaceHandler = moteInterfaceHandler;
  }

  /**
   * Creates an interface handler object and registers interfaces to it.
   *
   * @return Interface handler
   */
  protected abstract MoteInterfaceHandler createMoteInterfaceHandler();

  /**
   * Initializes emulator by creating CPU, memory and node object.
   *
   * @param ELFFile ELF file
   * @return True if successful
   */
  protected abstract boolean initEmulator(File ELFFile);

  public void tick(int simTime) {
    // Let all interfaces act
    myMoteInterfaceHandler.doPassiveActionsBeforeTick();
    myMoteInterfaceHandler.doActiveActionsBeforeTick();

    stopNextInstruction = false;

    // Leave control to emulated CPU
    MSP430 cpu = getCPU();
    cycleCounter += NR_CYCLES_PER_MSEC;

    if (monitorStackUsage) {
      // CPU loop with stack observer
      int newStack;
      while (!stopNextInstruction && cpu.step() < cycleCounter) {
        /* Check if radio has pending incoming bytes */
        if (myRadio != null && myRadio.hasPendingBytes()) {
          myRadio.tryDeliverNextByte(cpu.cycles);
        }

        newStack = cpu.reg[MSP430.SP];
        if (newStack < stackPointerLow && newStack > 0) {
          stackPointerLow = cpu.reg[MSP430.SP];

          // Check if stack is writing in memory
          if (stackPointerLow < heapStartAddress) {
            stackOverflowObservable.signalStackOverflow();
            stopNextInstruction = true;
          }
        }
      }
    } else { /* Fast CPU loop */
      do {
        /* Check if radio has pending incoming bytes */
        if (myRadio != null && myRadio.hasPendingBytes()) {
          myRadio.tryDeliverNextByte(cpu.cycles);
        }
      } while (!stopNextInstruction && (cpu.step() < cycleCounter) );
    }

    // Reset abort flag
    stopNextInstruction = false;

    // Let all interfaces act after tick
    myMoteInterfaceHandler.doActiveActionsAfterTick();
    myMoteInterfaceHandler.doPassiveActionsAfterTick();
  }

  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {

        setSimulation(simulation);
        myMoteType = (MspMoteType) simulation.getMoteType(element.getText());
        getType().setIdentifier(element.getText());

        initEmulator(myMoteType.getELFFile());
        myMoteInterfaceHandler = createMoteInterfaceHandler();

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

    return true;
  }

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

}

