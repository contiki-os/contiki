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
 */

package se.sics.cooja.mspmote;

import java.awt.Component;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ContikiError;
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
import se.sics.cooja.motes.AbstractEmulatedMote;
import se.sics.cooja.mspmote.interfaces.Msp802154Radio;
import se.sics.cooja.mspmote.interfaces.MspSerial;
import se.sics.cooja.mspmote.plugins.CodeVisualizerSkin;
import se.sics.cooja.mspmote.plugins.MspBreakpoint;
import se.sics.cooja.plugins.Visualizer;
import se.sics.mspsim.cli.CommandContext;
import se.sics.mspsim.cli.CommandHandler;
import se.sics.mspsim.cli.LineListener;
import se.sics.mspsim.cli.LineOutputStream;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.LogListener;
import se.sics.mspsim.core.Loggable;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.ui.ManagedWindow;
import se.sics.mspsim.ui.WindowManager;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.ConfigManager;
import se.sics.mspsim.util.DebugInfo;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.MapTable;
import se.sics.mspsim.util.SimpleProfiler;

/**
 * @author Fredrik Osterlind
 */
public abstract class MspMote extends AbstractEmulatedMote implements Mote, WatchpointMote {
  private static Logger logger = Logger.getLogger(MspMote.class);

  private final static int EXECUTE_DURATION_US = 1; /* We always execute in 1 us steps */

  {
    Visualizer.registerVisualizerSkin(CodeVisualizerSkin.class);
  }

  private CommandHandler commandHandler;
  private MSP430 myCpu = null;
  private MspMoteType myMoteType = null;
  private MspMoteMemory myMemory = null;
  private MoteInterfaceHandler myMoteInterfaceHandler = null;
  public ComponentRegistry registry = null;

  /* Stack monitoring variables */
  private boolean stopNextInstruction = false;

  public GenericNode mspNode = null;

  public MspMote(MspMoteType moteType, Simulation simulation) {
    this.simulation = simulation;
    myMoteType = moteType;

    /* Schedule us immediately */
    requestImmediateWakeup();
  }

  protected void initMote() {
    if (myMoteType != null) {
      initEmulator(myMoteType.getContikiFirmwareFile());
      myMoteInterfaceHandler = createMoteInterfaceHandler();

      /* TODO Create COOJA-specific window manager */
      registry.removeComponent("windowManager");
      registry.registerComponent("windowManager", new WindowManager() {
        public ManagedWindow createWindow(String name) {
          return new ManagedWindow() {
            public void setVisible(boolean b) {
              logger.warn("setVisible() ignored");
            }
            public void setTitle(String string) {
              logger.warn("setTitle() ignored");
            }
            public void setSize(int width, int height) {
              logger.warn("setSize() ignored");
            }
            public void setBounds(int x, int y, int width, int height) {
              logger.warn("setBounds() ignored");
            }
            public void removeAll() {
              logger.warn("removeAll() ignored");
            }
            public void pack() {
              logger.warn("pack() ignored");
            }
            public boolean isVisible() {
              logger.warn("isVisible() return false");
              return false;
            }
            public String getTitle() {
              logger.warn("getTitle() return \"\"");
              return "";
            }
            public void add(Component component) {
              logger.warn("add() ignored");
            }
          };
        }
      });

      try {
        debuggingInfo = ((MspMoteType)getType()).getFirmwareDebugInfo();
      } catch (IOException e) {
        throw (RuntimeException) new RuntimeException("Error: " + e.getMessage()).initCause(e);
      }
    }
  }

  /**
   * Abort execution immediately.
   * May for example be called by a breakpoint handler.
   */
  public void stopNextInstruction() {
    stopNextInstruction = true;
    getCPU().stop();
  }

  protected MoteInterfaceHandler createMoteInterfaceHandler() {
    return new MoteInterfaceHandler(this, getType().getMoteInterfaceClasses());
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
   * Prepares CPU, memory and ELF module.
   *
   * @param fileELF ELF file
   * @param cpu MSP430 cpu
   * @throws IOException Preparing mote failed
   */
  protected void prepareMote(File fileELF, GenericNode node) throws IOException {
    this.commandHandler = new CommandHandler(System.out, System.err);
    
    this.mspNode = node;
    
    node.setCommandHandler(commandHandler);

    ConfigManager config = new ConfigManager();
    node.setup(config);

    this.myCpu = node.getCPU();
    this.myCpu.setMonitorExec(true);
    this.myCpu.setTrace(0); /* TODO Enable */
    
    LogListener ll = new LogListener() {
      private Logger mlogger = Logger.getLogger("MSPSim");
      @Override
      public void log(Loggable source, String message) {
        mlogger.debug("" + getID() + ": " + source.getID() + ": " + message);
      }
      
      @Override
      public void logw(Loggable source, WarningType type, String message) throws EmulationException {
        mlogger.warn("" + getID() +": " + "# " + source.getID() + "[" + type + "]: " + message);
      }
    };
    
    this.myCpu.getLogger().addLogListener(ll);

    logger.info("Loading firmware from: " + fileELF.getAbsolutePath());
    GUI.setProgressMessage("Loading " + fileELF.getName());
    node.loadFirmware(((MspMoteType)getType()).getELF());

    /* Throw exceptions at bad memory access */
    /*myCpu.setThrowIfWarning(true);*/

    /* Create mote address memory */
    MapTable map = ((MspMoteType)getType()).getELF().getMap();
    MapEntry[] allEntries = map.getAllEntries();
    myMemory = new MspMoteMemory(this, allEntries, myCpu);

    myCpu.reset();
  }

  public CommandHandler getCLICommandHandler() {
    return commandHandler;
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

  private boolean booted = false;

  public void simTimeChanged(long diff) {
    /* Compensates for simulation time changes (without simulation execution) */
    lastExecute -= diff;
    nextExecute -= diff;
    scheduleNextWakeup(nextExecute);
  }

  private long lastExecute = -1; /* Last time mote executed */
  private long nextExecute;
  public void execute(long time) {
    execute(time, EXECUTE_DURATION_US);
  }
  public void execute(long t, int duration) {
    /* Wait until mote boots */
    if (!booted && myMoteInterfaceHandler.getClock().getTime() < 0) {
      scheduleNextWakeup(t - myMoteInterfaceHandler.getClock().getTime());
      return;
    }
    booted = true;

    if (stopNextInstruction) {
      stopNextInstruction = false;
      scheduleNextWakeup(t);
      throw new RuntimeException("MSPSim requested simulation stop");
    }

    if (lastExecute < 0) {
      /* Always execute one microsecond the first time */
      lastExecute = t;
    }
    if (t < lastExecute) {
      throw new RuntimeException("Bad event ordering: " + lastExecute + " < " + t);
    }

    /* Execute MSPSim-based mote */
    /* TODO Try-catch overhead */
    try {
      nextExecute =
        t + duration +
        myCpu.stepMicros(t - lastExecute, duration);
      lastExecute = t;
    } catch (EmulationException e) {
      String trace = e.getMessage() + "\n\n" + getStackTrace();
      throw (ContikiError)
      new ContikiError(trace).initCause(e);
    }

    /* Schedule wakeup */
    if (nextExecute < t) {
      throw new RuntimeException(t + ": MSPSim requested early wakeup: " + nextExecute);
    }
    /*logger.debug(t + ": Schedule next wakeup at " + nextExecute);*/
    scheduleNextWakeup(nextExecute);

    if (stopNextInstruction) {
      stopNextInstruction = false;
      throw new RuntimeException("MSPSim requested simulation stop");
    }

    /* XXX TODO Reimplement stack monitoring using MSPSim internals */
    /*if (monitorStackUsage) {
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
    }*/
  }

  public String getStackTrace() {
    return executeCLICommand("stacktrace");
  }

  public int executeCLICommand(String cmd, CommandContext context) {
    return commandHandler.executeCommand(cmd, context);
  }

  public String executeCLICommand(String cmd) {
    final StringBuilder sb = new StringBuilder();
    LineListener ll = new LineListener() {
      public void lineRead(String line) {
        sb.append(line).append("\n");
      }
    };
    PrintStream po = new PrintStream(new LineOutputStream(ll));
    CommandContext c = new CommandContext(commandHandler, null, "", new String[0], 1, null);
    c.out = po;
    c.err = po;

    if (0 != executeCLICommand(cmd, c)) {
      sb.append("\nWarning: command failed");
    }

    return sb.toString();
  }

  public int getCPUFrequency() {
    return myCpu.getDCOFrequency();
  }

  public int getID() {
    return getInterfaces().getMoteID().getMoteID();
  }

  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    setSimulation(simulation);
    if (myMoteInterfaceHandler == null) {
      myMoteInterfaceHandler = createMoteInterfaceHandler();
    }

    try {
      debuggingInfo = ((MspMoteType)getType()).getFirmwareDebugInfo();
    } catch (IOException e) {
      throw (RuntimeException) new RuntimeException("Error: " + e.getMessage()).initCause(e);
    }

    for (Element element: configXML) {
      String name = element.getName();

      if (name.equals("motetype_identifier")) {
        /* Ignored: handled by simulation */
      } else if ("breakpoints".equals(element.getName())) {
        setWatchpointConfigXML(element.getChildren(), visAvailable);
      } else if (name.equals("interface_config")) {
        String intfClass = element.getText().trim();
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.MspIPAddress")) {
          intfClass = IPAddress.class.getName();
        }
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.ESBLog")) {
          intfClass = MspSerial.class.getName();
        }
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.SkyByteRadio")) {
          intfClass = Msp802154Radio.class.getName();
        }
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.SkySerial")) {
          intfClass = MspSerial.class.getName();
        }
        Class<? extends MoteInterface> moteInterfaceClass = simulation.getGUI().tryLoadClass(
              this, MoteInterface.class, intfClass);

        if (moteInterfaceClass == null) {
          logger.fatal("Could not load mote interface class: " + intfClass);
          return false;
        }

        MoteInterface moteInterface = getInterfaces().getInterfaceOfType(moteInterfaceClass);
        if (moteInterface == null) {
            logger.fatal("Could not find mote interface of class: " + moteInterfaceClass);
            return false;
        }
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

    /* Breakpoints */
    element = new Element("breakpoints");
    element.addContent(getWatchpointConfigXML());
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

  public String getExecutionDetails() {
    return executeCLICommand("stacktrace");
  }

  public String getPCString() {
    int pc = myCpu.getPC();
    ELF elf = myCpu.getRegistry().getComponent(ELF.class);
    DebugInfo di = elf.getDebugInfo(pc);

    /* Following code examples from MSPsim, DebugCommands.java */
    if (di == null) {
      di = elf.getDebugInfo(pc + 1);
    }
    if (di == null) {
      /* Return PC value */
      SimpleProfiler sp = (SimpleProfiler)myCpu.getProfiler();
      try {
        MapEntry mapEntry = sp.getCallMapEntry(0);
        if (mapEntry != null) {
          String file = mapEntry.getFile();
          if (file != null) {
            if (file.indexOf('/') >= 0) {
              file = file.substring(file.lastIndexOf('/')+1);
            }
          }
          String name = mapEntry.getName();
          return file + ":?:" + name;
        }
        return String.format("*%02x", pc);
      } catch (Exception e) {
        return null;
      }
    }

    int lineNo = di.getLine();
    String file = di.getFile();
    file = file==null?"?":file;
    if (file.contains("/")) {
      /* strip path */
      file = file.substring(file.lastIndexOf('/')+1, file.length());
    }

    String function = di.getFunction();
    function = function==null?"":function;
    if (function.contains(":")) {
      /* strip arguments */
      function = function.substring(0, function.lastIndexOf(':'));
    }
    if (function.equals("* not available")) {
      function = "?";
    }
    return file + ":" + lineNo + ":" + function;

    /*return executeCLICommand("line " + myCpu.getPC());*/
  }


  /* WatchpointMote */
  private ArrayList<WatchpointListener> watchpointListeners = new ArrayList<WatchpointListener>();
  private ArrayList<MspBreakpoint> watchpoints = new ArrayList<MspBreakpoint>();
  private Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo = null;

  public void addWatchpointListener(WatchpointListener listener) {
    watchpointListeners.add(listener);
  }
  public void removeWatchpointListener(WatchpointListener listener) {
    watchpointListeners.remove(listener);
  }
  public WatchpointListener[] getWatchpointListeners() {
    return watchpointListeners.toArray(new WatchpointListener[0]);
  }

  public Watchpoint addBreakpoint(File codeFile, int lineNr, int address) {
    MspBreakpoint bp = new MspBreakpoint(this, address, codeFile, new Integer(lineNr));
    watchpoints.add(bp);

    for (WatchpointListener listener: watchpointListeners) {
      listener.watchpointsChanged();
    }
    return bp;
  }
  public void removeBreakpoint(Watchpoint watchpoint) {
    ((MspBreakpoint)watchpoint).unregisterBreakpoint();
    watchpoints.remove(watchpoint);

    for (WatchpointListener listener: watchpointListeners) {
      listener.watchpointsChanged();
    }
  }
  public Watchpoint[] getBreakpoints() {
    return watchpoints.toArray(new Watchpoint[0]);
  }

  public boolean breakpointExists(int address) {
    if (address < 0) {
      return false;
    }
    for (Watchpoint watchpoint: watchpoints) {
      if (watchpoint.getExecutableAddress() == address) {
        return true;
      }
    }
    return false;
  }
  public boolean breakpointExists(File file, int lineNr) {
    for (Watchpoint watchpoint: watchpoints) {
      if (watchpoint.getCodeFile() == null) {
        continue;
      }
      if (watchpoint.getCodeFile().compareTo(file) != 0) {
        continue;
      }
      if (watchpoint.getLineNumber() != lineNr) {
        continue;
      }
      return true;
    }
    return false;
  }

  public int getExecutableAddressOf(File file, int lineNr) {
    if (file == null || lineNr < 0 || debuggingInfo == null) {
      return -1;
    }

    /* Match file */
    Hashtable<Integer, Integer> lineTable = debuggingInfo.get(file);
    if (lineTable == null) {
      for (File f: debuggingInfo.keySet()) {
        if (f != null && f.getName().equals(file.getName())) {
          lineTable = debuggingInfo.get(f);
          break;
        }
      }
    }
    if (lineTable == null) {
      return -1;
    }

    /* Match line number */
    Integer address = lineTable.get(lineNr);
    if (address != null) {
      for (Integer l: lineTable.keySet()) {
        if (l != null && l.intValue() == lineNr) {
          /* Found line address */
          return lineTable.get(l);
        }
      }
    }

    return -1;
  }

  private long lastBreakpointCycles = -1;
  public void signalBreakpointTrigger(MspBreakpoint b) {
    if (lastBreakpointCycles == myCpu.cycles) {
      return;
    }

    lastBreakpointCycles = myCpu.cycles;
    if (b.stopsSimulation() && getSimulation().isRunning()) {
      /* Stop simulation immediately */
      stopNextInstruction();
    }

    /* Notify listeners */
    WatchpointListener[] listeners = getWatchpointListeners();
    for (WatchpointListener listener: listeners) {
      listener.watchpointTriggered(b);
    }
  }

  public Collection<Element> getWatchpointConfigXML() {
    ArrayList<Element> config = new ArrayList<Element>();
    Element element;

    for (MspBreakpoint breakpoint: watchpoints) {
      element = new Element("breakpoint");
      element.addContent(breakpoint.getConfigXML());
      config.add(element);
    }

    return config;
  }
  public boolean setWatchpointConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("breakpoint")) {
        MspBreakpoint breakpoint = new MspBreakpoint(this);
        if (!breakpoint.setConfigXML(element.getChildren(), visAvailable)) {
          logger.warn("Could not restore breakpoint: " + breakpoint);
        } else {
          watchpoints.add(breakpoint);
        }
      }
    }
    return true;
  }
}
