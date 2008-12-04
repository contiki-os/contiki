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
 * $Id: MspMote.java,v 1.17 2008/12/04 13:14:34 fros4943 Exp $
 */

package se.sics.cooja.mspmote;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.net.URL;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import java.util.Vector;
import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteInterfaceHandler;
import se.sics.cooja.MoteMemory;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.mspmote.interfaces.TR1001Radio;
import se.sics.mspsim.cli.CommandHandler;
import se.sics.mspsim.cli.LineListener;
import se.sics.mspsim.cli.LineOutputStream;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.util.ConfigManager;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.MapTable;

/**
 * @author Fredrik Osterlind
 */
public abstract class MspMote implements Mote {
  private static Logger logger = Logger.getLogger(MspMote.class);

  /* 2.4576 MHz according to Contiki's speed sync loop*/
  public static long NR_CYCLES_PER_MSEC = 2458;

  /* Cycle counter */
  public long cycleCounter = 0;
  public long cycleDrift = 0;

  private Simulation mySimulation = null;
  private CommandHandler commandHandler;
  private LineListener commandListener;
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
  }

  protected void initMote() {
    if (myMoteType != null) {
      initEmulator(myMoteType.getELFFile());
      myMoteInterfaceHandler = createMoteInterfaceHandler();
    }
  }

  public void sendCLICommand(String line) {
    if (commandHandler != null) {
      commandHandler.lineRead(line);
    }
  }

  public boolean hasCLIListener() {
    return this.commandListener != null;
  }

  public void setCLIListener(LineListener listener) {
    this.commandListener = listener;
  }

  /**
   * @return MSP430 CPU
   */
  public MSP430 getCPU() {
    return myCpu;
  }

  public void setCPU(MSP430 cpu) {
    myCpu = cpu;
  }

  public MoteMemory getMemory() {
    return myMemory;
  }

  public void setMemory(MoteMemory memory) {
    myMemory = (MspMoteMemory) memory;
  }

  /**
   * @return ELF module
   */
  public ELF getELF() {
    return myELFModule;
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
   * Prepares CPU, memory and ELF module.
   *
   * @param fileELF ELF file
   * @param cpu MSP430 cpu
   * @throws IOException Preparing mote failed
   */
  protected void prepareMote(File fileELF, GenericNode node) throws IOException {
    LineOutputStream lout = new LineOutputStream(new LineListener() {
      @Override
      public void lineRead(String line) {
        LineListener listener = commandListener;
        if (listener != null) {
          listener.lineRead(line);
        }
      }});
    PrintStream out = new PrintStream(lout);
    this.commandHandler = new CommandHandler(out, out);
    node.setCommandHandler(commandHandler);

    ConfigManager config = new ConfigManager();
    node.setup(config);

    this.myCpu = node.getCPU();
    this.myCpu.setMonitorExec(true);

    int[] memory = myCpu.getMemory();
    if (GUI.isVisualizedInApplet()) {
      myELFModule = node.loadFirmware(new URL(GUI.getAppletCodeBase(), fileELF.getName()), memory);
    } else {
      myELFModule = node.loadFirmware(fileELF.getPath(), memory);
    }

    /* Throw exceptions at bad memory access */
    /*myCpu.setThrowIfWarning(true);*/

    /* Create mote address memory */
    MapTable map = myELFModule.getMap();
    MapEntry[] allEntries = map.getAllEntries();
    myMemory = new MspMoteMemory(allEntries, myCpu);

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

  /* return false when done - e.g. true means more work to do before finished with this tick */

  private boolean firstTick = true;
  public boolean tick(int simTime) {
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

    long maxSimTimeCycles = NR_CYCLES_PER_MSEC*(simTime+1) + cycleDrift;

    if (maxSimTimeCycles <= cycleCounter) {
      return false;
    }

    // Leave control to emulated CPU
    cycleCounter += 1;

    MSP430 cpu = getCPU();
    if (cpu.cycles > cycleCounter) {
      /* CPU already ticked too far - just wait it out */
      return true;
    }
    myMoteInterfaceHandler.doActiveActionsBeforeTick();

    cpu.step(cycleCounter);

    /* Check if radio has pending incoming bytes */
    if (myRadio != null && myRadio.hasPendingBytes()) {
      myRadio.tryDeliverNextByte(cpu.cycles);
    }

    if (monitorStackUsage) {
      int newStack = cpu.reg[MSP430.SP];
      if (newStack < stackPointerLow && newStack > 0) {
        stackPointerLow = cpu.reg[MSP430.SP];

        // Check if stack is writing in memory
        if (stackPointerLow < heapStartAddress) {
          stackOverflowObservable.signalStackOverflow();
          stopNextInstruction = true;
          getSimulation().stopSimulation();
        }
      }
    }

    return true;
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

}
