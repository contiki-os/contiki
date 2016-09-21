/**
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 * -----------------------------------------------------------------
 *
 * GenericNode
 *
 * Author  : Joakim Eriksson
 */

package se.sics.mspsim.platform;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.net.URISyntaxException;
import java.net.URL;

import javax.swing.JFrame;

import se.sics.mspsim.cli.CommandHandler;
import se.sics.mspsim.cli.DebugCommands;
import se.sics.mspsim.cli.FileCommands;
import se.sics.mspsim.cli.MiscCommands;
import se.sics.mspsim.cli.NetCommands;
import se.sics.mspsim.cli.ProfilerCommands;
import se.sics.mspsim.cli.StreamCommandHandler;
import se.sics.mspsim.cli.WindowCommands;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.EmulationLogger;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.MSP430Config;
import se.sics.mspsim.core.MSP430Constants;
import se.sics.mspsim.extutil.highlight.HighlightSourceViewer;
import se.sics.mspsim.ui.ConsoleUI;
import se.sics.mspsim.ui.ControlUI;
import se.sics.mspsim.ui.JFrameWindowManager;
import se.sics.mspsim.ui.StackUI;
import se.sics.mspsim.ui.WindowUtils;
import se.sics.mspsim.util.ArgumentManager;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.ConfigManager;
import se.sics.mspsim.util.DefaultEmulationLogger;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.IHexReader;
import se.sics.mspsim.util.MapTable;
import se.sics.mspsim.util.OperatingModeStatistics;
import se.sics.mspsim.util.PluginRepository;
import se.sics.mspsim.util.StatCommands;

public abstract class GenericNode extends Chip implements Runnable {

  private static final String PROMPT = "MSPSim>";

  protected final MSP430 cpu;
  protected final ComponentRegistry registry;
  protected ConfigManager config;

  protected String firmwareFile = null;
  protected ELF elf;
  protected OperatingModeStatistics stats;


  public GenericNode(String id, MSP430Config config) {
    super(id, new MSP430(0, new ComponentRegistry(), config));
    this.cpu = (MSP430)super.cpu;
    this.registry = cpu.getRegistry();
  }

  public ComponentRegistry getRegistry() {
    return registry;
  }

  public MSP430 getCPU() {
    return cpu;
  }

  public abstract void setupNode();

  public void setCommandHandler(CommandHandler handler) {
    registry.registerComponent("commandHandler", handler);
  }

  public void setupArgs(ArgumentManager config) throws IOException {
    String[] args = config.getArguments();
    if (args.length == 0) {
      System.err.println("Usage: " + getClass().getName() + " <firmware>");
      System.exit(1);
    }
    firmwareFile = args[0];
    if (!(new File(firmwareFile)).exists()) {
      System.err.println("Could not find the firmware file '" + firmwareFile + "'.");
      System.exit(1);
    }
    if (config.getProperty("nogui") == null) {
      config.setProperty("nogui", "false");
    }
    /* Ensure auto-run of a start script */
    if (config.getProperty("autorun") == null) {
      File fp = new File("scripts/autorun.sc");
      if (fp.exists()) {
        config.setProperty("autorun", "scripts/autorun.sc");
      } else {
        try {
          File dir = new File(GenericNode.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getParentFile();
          fp = new File(dir, "scripts/autorun.sc");
          if (fp.exists()) {
            config.setProperty("autorun", fp.getAbsolutePath());
          }
        } catch (URISyntaxException e) {
          // Failed to find auto run script
        }
      }
    }

    int[] memory = cpu.memory;
    if (firmwareFile.endsWith("ihex")) {
      // IHEX Reading
      IHexReader reader = new IHexReader();
      reader.readFile(memory, firmwareFile);
    } else {
      loadFirmware(firmwareFile, memory);
    }
    if (args.length > 1) {
      MapTable map = new MapTable(args[1]);
      cpu.getDisAsm().setMap(map);
      registry.registerComponent("mapTable", map);
    }
    
    setup(config);


    if (!config.getPropertyAsBoolean("nogui", false)) {
      // Setup control and other UI components
      ControlUI control = new ControlUI();
      registry.registerComponent("controlgui", control);
      registry.registerComponent("stackchart", new StackUI(cpu));
      HighlightSourceViewer sourceViewer = new HighlightSourceViewer();
      if (firmwareFile != null) {
        // Add the firmware location to the search path
        File fp = new File(firmwareFile).getParentFile();
        if (fp != null) {
          try {
            // Get absolute path
            fp = fp.getCanonicalFile();
          } catch (Exception e) {
            // Ignore
          }
          sourceViewer.addSearchPath(fp);
        }
      }
      control.setSourceViewer(sourceViewer);
    }

    String script = config.getProperty("autorun");
    if (script != null) {
      File fp = new File(script);
      if (fp.canRead()) {
        CommandHandler ch = (CommandHandler) registry.getComponent("commandHandler");
        script = script.replace('\\', '/');
        System.out.println("Autoloading script: " + script);
        config.setProperty("autoloadScript", script);
        if (ch != null) {
          ch.lineRead("source \"" + script + '"');
        }
      }
    }
    config.setProperty("firmwareFile", firmwareFile);
    System.out.println("-----------------------------------------------");
    System.out.println("MSPSim " + MSP430Constants.VERSION + " starting firmware: " + firmwareFile);
    System.out.println("-----------------------------------------------");
    System.out.print(PROMPT);
    System.out.flush();
  }

  public void setup(ConfigManager config) {
    this.config = config;

    registry.registerComponent("cpu", cpu);
    registry.registerComponent("node", this);
    registry.registerComponent("config", config);
    
    CommandHandler ch = registry.getComponent(CommandHandler.class, "commandHandler");

    if (ch == null) {
        if (config.getPropertyAsBoolean("jconsole", false)) {
            ConsoleUI console = new ConsoleUI();
            PrintStream consoleStream = new PrintStream(console.getOutputStream());
            ch = new CommandHandler(consoleStream, consoleStream);
            JFrame w = new JFrame("ConsoleUI");
            w.add(console);
            w.setBounds(20, 20, 520, 400);
            w.setLocationByPlatform(true);
            String key = "console";
            WindowUtils.restoreWindowBounds(key, w);
            WindowUtils.addSaveOnShutdown(key, w);
            w.setVisible(true);
            console.setCommandHandler(ch);
        } else {
            ch = new StreamCommandHandler(System.in, System.out, System.err, PROMPT);
        }
        registry.registerComponent("commandHandler", ch);
    }
    
    stats = new OperatingModeStatistics(cpu);
    
    registry.registerComponent("pluginRepository", new PluginRepository());
    registry.registerComponent("debugcmd", new DebugCommands());
    registry.registerComponent("misccmd", new MiscCommands());
    registry.registerComponent("filecmd", new FileCommands());
    registry.registerComponent("statcmd", new StatCommands(cpu, stats));
    registry.registerComponent("wincmd", new WindowCommands());
    registry.registerComponent("profilecmd", new ProfilerCommands());
    registry.registerComponent("netcmd", new NetCommands());
    registry.registerComponent("windowManager", new JFrameWindowManager());

    // Monitor execution
    cpu.setMonitorExec(true);
    
    setupNode();

    registry.start();

    cpu.reset();
  }
  
 
  public void run() {
    if (!cpu.isRunning()) {
      try {
        cpu.cpuloop(); 
      } catch (Exception e) {
        /* what should we do here */
        e.printStackTrace();
      }
    }
  }
  
  public void start() {
    if (!cpu.isRunning()) {
      Thread thread = new Thread(this);
      // Set this thread to normal priority in case the start method was called
      // from the higher priority AWT thread.
      thread.setPriority(Thread.NORM_PRIORITY);
      thread.start();
    }
  }
  
  public void stop() {
    cpu.stop();
  }
  
  public void step() throws EmulationException {
    if (!cpu.isRunning()) {
      cpu.step();
    }
  }

  // A step that will break out of breakpoints!
  public void step(int nr) throws EmulationException {
    if (!cpu.isRunning()) {
      cpu.stepInstructions(nr);
    }
  }

  public ELF loadFirmware(URL url) throws IOException {
      return loadFirmware(url, cpu.memory);
  }

  @Deprecated public ELF loadFirmware(URL url, int[] memory) throws IOException {
    DataInputStream inputStream = new DataInputStream(url.openStream());
    ByteArrayOutputStream byteStream = new ByteArrayOutputStream();
    byte[] firmwareData = new byte[2048];
    int read;
    while ((read = inputStream.read(firmwareData)) != -1) {
      byteStream.write(firmwareData, 0, read);
    }
    inputStream.close();
    ELF elf = new ELF(byteStream.toByteArray());
    elf.readAll();
    return loadFirmware(elf, memory);
  }

  public ELF loadFirmware(String name) throws IOException {
      return loadFirmware(name, cpu.memory);
  }

  @Deprecated public ELF loadFirmware(String name, int[] memory) throws IOException {
    return loadFirmware(ELF.readELF(firmwareFile = name), memory);
  }

  public ELF loadFirmware(ELF elf) {
      return loadFirmware(elf, cpu.memory);
  }

  @Deprecated public ELF loadFirmware(ELF elf, int[] memory) {
    if (cpu.isRunning()) {
        stop();
    }
    this.elf = elf;
    elf.loadPrograms(memory);
    MapTable map = elf.getMap();
    cpu.getDisAsm().setMap(map);
    cpu.setMap(map);
    registry.registerComponent("elf", elf);
    registry.registerComponent("mapTable", map);
    return elf;
  }

  public int getConfiguration(int param) {
      return 0;
  }
}
