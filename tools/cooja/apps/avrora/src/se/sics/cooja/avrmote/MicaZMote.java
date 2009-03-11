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
 * $Id: MicaZMote.java,v 1.3 2009/03/11 14:12:19 fros4943 Exp $
 */

package se.sics.cooja.avrmote;
import java.util.Collection;
import java.util.Observer;
import java.util.Random;
import java.util.Vector;
import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteInterfaceHandler;
import se.sics.cooja.MoteMemory;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.avrmote.interfaces.MicaClock;
import se.sics.cooja.avrmote.interfaces.MicaZLED;
import se.sics.cooja.avrmote.interfaces.MicaZRadio;
import se.sics.cooja.interfaces.Position;
import avrora.sim.*;
import avrora.sim.platform.*;
import avrora.sim.mcu.*;
import avrora.core.*;


/**
 * @author Joakim Eriksson
 */
public class MicaZMote implements Mote {
  private static Logger logger = Logger.getLogger(MicaZMote.class);

  /* 8 MHz according to Contiki config */
  public static long NR_CYCLES_PER_MSEC = 8000;

  /* Cycle counter */
  public long cycleCounter = 0;
  public long cycleDrift = 0;

  private Simulation mySimulation = null;
  private Microcontroller myCpu = null;
  private MicaZ micaZ = null;
  private LoadableProgram program = null;
  private Interpreter interpreter = null;

  private MicaZMoteType myMoteType = null;

  /* Stack monitoring variables */
  private boolean stopNextInstruction = false;

  private MoteInterfaceHandler myMoteInterfaceHandler;

  /**
   * Abort current tick immediately.
   * May for example be called by a breakpoint handler.
   */
  public void stopNextInstruction() {
    stopNextInstruction = true;
  }

  public MicaZ getMicaZ() {
    return micaZ;
  }

  public MicaZMote() {
    mySimulation = null;
    myCpu = null;
    myMoteInterfaceHandler = null;
  }

  public MicaZMote(Simulation simulation, MicaZMoteType type) {
    mySimulation = simulation;
    myMoteType = type;
  }

  protected void initMote() {
    if (myMoteType != null) {
      initEmulator(myMoteType.getContikiFirmwareFile().getName());
      myMoteInterfaceHandler = createMoteInterfaceHandler();
    }
  }

  protected boolean initEmulator(String fileELF) {
    try {
      prepareMote(fileELF);
    } catch (Exception e) {
      logger.fatal("Error when creating MicaZ mote:", e);
      return false;
    }
    return true;
  }


  public Simulation getSimulation() {
    return mySimulation;
  }

  public void setSimulation(Simulation simulation) {
    mySimulation = simulation;
  }

  /**
   * Prepares CPU, memory and ELF module.
   *
   * @param fileELF ELF file
   * @param cpu MSP430 cpu
   * @throws Exception
   */
  protected void prepareMote(String file) throws Exception {
    program = new LoadableProgram(file);
    program.load();
    PlatformFactory factory = new MicaZ.Factory();
    micaZ = (MicaZ) factory.newPlatform(1, program.getProgram());
    myCpu = micaZ.getMicrocontroller();
    Simulator sim = myCpu.getSimulator();
    interpreter = sim.getInterpreter();
//     State state = interpreter.getState();
  }

  public void setState(State newState) {
    logger.warn("MicaZ motes can't change state");
  }

  public State getState() {
    return Mote.State.ACTIVE;
  }

  /* called when moteID is updated */
  public void idUpdated(int newID) {
  }

  public MoteType getType() {
    return myMoteType;
  }

  public void setType(MoteType type) {
    //myMoteType = (MspMoteType) type;
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

  /* return false when done - e.g. true means more work to do before finished with this tick */

  private boolean firstTick = true;
  private long cyclesExecuted = 0;
  public boolean tick(long simTime) {
    if (stopNextInstruction) {
      stopNextInstruction = false;
      throw new RuntimeException("MSPSim requested simulation stop");
    }

    /* Nodes may be added in an ongoing simulation:
     * Update cycle drift to current simulation time */
    if (firstTick) {
      firstTick = false;
      cycleDrift += (-NR_CYCLES_PER_MSEC*simTime);
    }

    long maxSimTimeCycles = NR_CYCLES_PER_MSEC * (simTime + 1) + cycleDrift;

    if (maxSimTimeCycles <= cycleCounter) {
      return false;
    }

    // Leave control to emulated CPU
    cycleCounter += 1;

    if (cyclesExecuted > cycleCounter) {
      /* CPU already ticked too far - just wait it out */
      return true;
    }
    //    myMoteInterfaceHandler.doActiveActionsBeforeTick();

    cyclesExecuted += interpreter.step();

    //cpu.step(cycleCounter);

    /* Check if radio has pending incoming bytes */
//     if (myRadio != null && myRadio.hasPendingBytes()) {
//       myRadio.tryDeliverNextByte(cpu.cycles);
//     }

    return true;
  }

  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {

        setSimulation(simulation);
        myMoteType = (MicaZMoteType) simulation.getMoteType(element.getText());
        getType().setIdentifier(element.getText());

        try {
          prepareMote(myMoteType.getContikiFirmwareFile().getName());
        } catch (Exception e) {
          // TODO Auto-generated catch block
          e.printStackTrace();
        }
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

  private MoteInterfaceHandler createMoteInterfaceHandler() {
    System.out.println("Creating mote interface handler....");
    MoteInterfaceHandler moteInterfaceHandler = new MoteInterfaceHandler();

      // Add position interface
      Position motePosition = new Position(this);
      Random random = new Random(); /* Do not use main random generator for positioning */
      motePosition.setCoordinates(random.nextDouble()*100, random.nextDouble()*100, random.nextDouble()*100);
      moteInterfaceHandler.addInterface(motePosition);

      // Add LED interface
      moteInterfaceHandler.addInterface(new MicaZLED(micaZ));
      // Add Radio interface
      moteInterfaceHandler.addInterface(new MicaZRadio(this));
      // Add Radio interface
      moteInterfaceHandler.addInterface(new MicaClock(this));


      return moteInterfaceHandler;
  }

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

      Collection<Element> interfaceXML = moteInterface.getConfigXML();
      if (interfaceXML != null) {
        element.addContent(interfaceXML);
        config.add(element);
      }
    }

    return config;
  }

  public MoteMemory getMemory() {
    return null;
  }

  public void setMemory(MoteMemory memory) {
  }

}
