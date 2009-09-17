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
 * $Id: MspMote.java,v 1.32 2009/09/17 10:50:11 fros4943 Exp $
 */

package se.sics.cooja.mspmote;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;
import java.util.Observable;
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
import se.sics.cooja.Watchpoint;
import se.sics.cooja.WatchpointMote;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.mspmote.interfaces.TR1001Radio;
import se.sics.cooja.mspmote.plugins.MspBreakpointContainer;
import se.sics.mspsim.cli.CommandHandler;
import se.sics.mspsim.cli.LineListener;
import se.sics.mspsim.cli.LineOutputStream;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.util.ConfigManager;
import se.sics.mspsim.util.DebugInfo;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.MapTable;

/**
 * @author Fredrik Osterlind
 */
public abstract class MspMote implements Mote, WatchpointMote {
  private static Logger logger = Logger.getLogger(MspMote.class);

  /* 3.900 MHz according to Contiki's speed sync loop*/
  public static long NR_CYCLES_PER_MSEC = 3900;

  /* Cycle counter */
  public long cycleCounter = 0;
  public long usDrift = 0; /* us */

  private Simulation mySimulation = null;
  private CommandHandler commandHandler;
  private LineListener commandListener;
  private MSP430 myCpu = null;
  private MspMoteType myMoteType = null;
  private MspMoteMemory myMemory = null;
  private MoteInterfaceHandler myMoteInterfaceHandler = null;
  private ELF myELFModule = null;

  /* Stack monitoring variables */
  private boolean stopNextInstruction = false;
  private boolean monitorStackUsage = false;
  private int stackPointerLow = Integer.MAX_VALUE;
  private int heapStartAddress;
  private StackOverflowObservable stackOverflowObservable = new StackOverflowObservable();

  private MspBreakpointContainer breakpointsContainer;

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
      initEmulator(myMoteType.getContikiFirmwareFile());
      myMoteInterfaceHandler = createMoteInterfaceHandler();
      
      /* Create watchpoint container */
      breakpointsContainer = new MspBreakpointContainer(this, getFirmwareDebugInfo(this));
    }
  }

  /**
   * Abort current tick immediately.
   * May for example be called by a breakpoint handler.
   */
  public void stopNextInstruction() {
    stopNextInstruction = true;
  }

  protected MoteInterfaceHandler createMoteInterfaceHandler() {
    return new MoteInterfaceHandler(this, getType().getMoteInterfaceClasses());
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

    heapStartAddress = map.heapStartAddress;
    myCpu.reset();
  }

  /* called when moteID is updated */
  public void idUpdated(int newID) {
  }

  public MoteType getType() {
    return myMoteType;
  }

  public void setType(MoteType type) {
    myMoteType = (MspMoteType) type;
  }

  public MoteInterfaceHandler getInterfaces() {
    return myMoteInterfaceHandler;
  }

  public void setInterfaces(MoteInterfaceHandler moteInterfaceHandler) {
    myMoteInterfaceHandler = moteInterfaceHandler;
  }

  /**
   * Initializes emulator by creating CPU, memory and node object.
   *
   * @param ELFFile ELF file
   * @return True if successful
   */
  protected abstract boolean initEmulator(File ELFFile);

  private int[] pcHistory = new int[5];

  /* return false when done - e.g. true means more work to do before finished with this tick */
  public boolean tick(long simTime) {
    if (stopNextInstruction) {
      stopNextInstruction = false;
      sendCLICommandAndPrint("trace 1000");
      throw new RuntimeException("MSPSim requested simulation stop");
    } 
    
    if (simTime + usDrift < 0) {
      return false;
    }
    
    long maxSimTimeCycles = (long)(NR_CYCLES_PER_MSEC * ((simTime+usDrift+Simulation.MILLISECOND)/(double)Simulation.MILLISECOND));
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

    /* Log recent program counter (PC) history */
    for (int i=pcHistory.length-1; i > 0; i--) {
      pcHistory[i] = pcHistory[i-1];
    }
    pcHistory[0] = cpu.reg[MSP430.PC];

    try {
      cpu.step(cycleCounter);
    } catch (EmulationException e) {
      if (e.getMessage().startsWith("Bad operation")) {
        /* Experimental: print program counter history */
        sendCLICommandAndPrint("trace 1000");
      }

      throw (RuntimeException)
      new RuntimeException("Emulated exception: " + e.getMessage()).initCause(e);
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

  private void sendCLICommandAndPrint(String comamnd) {
    /* Backup listener */
    LineListener oldListener = commandListener;

    
    setCLIListener(new LineListener() {
      public void lineRead(String line) {
        logger.fatal(line);
      }
    });
    sendCLICommand(comamnd);

    /* Restore listener */
    setCLIListener(oldListener);
  }
  
  public int getID() {
    return getInterfaces().getMoteID().getMoteID();
  }
  
  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {

        setSimulation(simulation);
        myMoteType = (MspMoteType) simulation.getMoteType(element.getText());
        getType().setIdentifier(element.getText());

        initEmulator(myMoteType.getContikiFirmwareFile());
        myMoteInterfaceHandler = createMoteInterfaceHandler();

        /* Create watchpoint container */
        breakpointsContainer = new MspBreakpointContainer(this, getFirmwareDebugInfo(this));
        
      } else if ("breakpoints".equals(element.getName())) {
        breakpointsContainer.setConfigXML(element.getChildren(), visAvailable);
      } else if (name.equals("interface_config")) {
        String intfClass = element.getText().trim();
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.MspIPAddress")) {
          intfClass = IPAddress.class.getName();
        }
        Class<? extends MoteInterface> moteInterfaceClass = simulation.getGUI().tryLoadClass(
              this, MoteInterface.class, intfClass);

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: " + intfClass);
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

    /* Breakpoints */
    element = new Element("breakpoints");
    element.addContent(breakpointsContainer.getConfigXML());
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


  /* Watchpoints: Forward to breakpoint container */
  public void addWatchpointListener(ActionListener listener) {
    breakpointsContainer.addWatchpointListener(listener);
  }

  public Watchpoint getLastWatchpoint() {
    return breakpointsContainer.getLastWatchpoint();
  }

  public Mote getMote() {
    return breakpointsContainer.getMote();
  }

  public ActionListener[] getWatchpointListeners() {
    return breakpointsContainer.getWatchpointListeners();
  }

  public void removeWatchpointListener(ActionListener listener) {
    breakpointsContainer.removeWatchpointListener(listener);
  }

  public MspBreakpointContainer getBreakpointsContainer() {
    return breakpointsContainer;
  }

  private static Hashtable<File, Hashtable<Integer, Integer>> getFirmwareDebugInfo(MspMote mote) {
    /* Fetch all executable addresses */
    ArrayList<Integer> addresses = mote.getELF().getDebug().getExecutableAddresses();

    Hashtable<File, Hashtable<Integer, Integer>> fileToLineHash =
      new Hashtable<File, Hashtable<Integer, Integer>>();

    for (int address: addresses) {
      DebugInfo info = mote.getELF().getDebugInfo(address);

      if (info != null && info.getPath() != null && info.getFile() != null && info.getLine() >= 0) {

        /* Nasty Cygwin-Windows fix */
        String path = info.getPath();
        if (path.contains("/cygdrive/")) {
          int index = path.indexOf("/cygdrive/");
          char driveCharacter = path.charAt(index+10);

          path = path.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
        }

        File file = new File(path, info.getFile());
        try {
          file = file.getCanonicalFile();
        } catch (IOException e) {
        }

        Hashtable<Integer, Integer> lineToAddrHash = fileToLineHash.get(file);
        if (lineToAddrHash == null) {
          lineToAddrHash = new Hashtable<Integer, Integer>();
          fileToLineHash.put(file, lineToAddrHash);
        }

        lineToAddrHash.put(info.getLine(), address);
      }
    }

    return fileToLineHash;
  }

}
