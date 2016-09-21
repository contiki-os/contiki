/**
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 *
 * -----------------------------------------------------------------
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : Mon Feb 11 2008
 */
package se.sics.mspsim.cli;
import se.sics.mspsim.core.DbgInstruction;
import se.sics.mspsim.core.DisAsm;
import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.EmulationLogger.WarningType;
import se.sics.mspsim.core.LogListener;
import se.sics.mspsim.core.Loggable;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.MSP430Constants;
import se.sics.mspsim.core.Memory;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.Memory.AccessType;
import se.sics.mspsim.core.MemoryMonitor;
import se.sics.mspsim.core.RegisterMonitor;
import se.sics.mspsim.core.TimeEvent;
import se.sics.mspsim.platform.GenericNode;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.DebugInfo;
import se.sics.mspsim.util.ELF;
import se.sics.mspsim.util.GDBStubs;
import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.Utils;

public class DebugCommands implements CommandBundle {
  private long lastCall = 0;
  private long lastWall = 0;
  private ComponentRegistry registry;

  private ELF getELF() {
    return registry.getComponent(ELF.class);
  }

  public void setupCommands(ComponentRegistry registry, CommandHandler ch) {
    this.registry = registry;
    final MSP430 cpu = registry.getComponent(MSP430.class);
    final GenericNode node = registry.getComponent(GenericNode.class, "node");
    if (cpu != null) {
      ch.registerCommand("break", new BasicAsyncCommand("add a breakpoint to a given address or symbol",
          "<address or symbol>") {
        private int address;
        private MemoryMonitor monitor;
        public int executeCommand(final CommandContext context) {
          address = context.getArgumentAsAddress(0);
          if (address < 0) {
            context.err.println("unknown symbol: " + context.getArgument(0));
            return 1;
          }
          monitor = new MemoryMonitor.Adapter() {
              private long lastCycles = -1;
              @Override
              public void notifyReadBefore(int address, AccessMode mode, AccessType type) {
                  if (type == AccessType.EXECUTE && cpu.cycles != lastCycles) {
                      context.out.println("*** Break at $" + cpu.getAddressAsString(address));
                      cpu.triggBreakpoint();
                      lastCycles = cpu.cycles;
                  }
              }
          };
          cpu.addWatchPoint(address, monitor);
          context.err.println("Breakpoint set at $" + cpu.getAddressAsString(address));
          return 0;
        }
        public void stopCommand(CommandContext context) {
          cpu.removeWatchPoint(address, monitor);
        }
      });

      ch.registerCommand("watch",
          new BasicAsyncCommand("add a write/read watch to a given address or symbol", "<address or symbol> [length] [char | hex | break]") {
        int mode = 0;
        int address = 0;
        int length = 1;
        MemoryMonitor monitor;
        public int executeCommand(final CommandContext context) {
          address = context.getArgumentAsAddress(0);
          if (address < 0) {
            context.err.println("unknown symbol: " + context.getArgument(0));
            return -1;
          }
          if (context.getArgumentCount() > 1) {
              for (int i = 1; i < context.getArgumentCount(); i++) {
                  String modeStr = context.getArgument(i);
                  if (Character.isDigit(modeStr.charAt(0))) {
                      length = Integer.parseInt(modeStr);
                  } else if ("char".equals(modeStr)) {
                      mode = Utils.ASCII_UNMODIFIED; // 4
                  } else if ("break".equals(modeStr)) {
                      mode = 10;
                  } else if ("hex".equals(modeStr)) {
                      mode = Utils.HEX; // 2
                  }
              }
          }
          if (length < 1) {
              context.err.println("please specify a length of at least one byte");
              return -1;
          }
          monitor = new MemoryMonitor.Adapter() {
              private void cpuAction(AccessType type, int adr, int data) {
                  if (mode == 0 || mode == 10) {
                      int pc = cpu.getPC();
                      String adrStr = getSymOrAddr(cpu, context, adr);
                      String pcStr = getSymOrAddrELF(cpu, getELF(), pc);
                      String op = "op";
                      if (type == AccessType.READ) {
                          op = "Read";
                      } else if (type == AccessType.WRITE){
                          op = "Write";
                      }
                      context.out.println("*** " + op + " from " + pcStr +
                              ": " + adrStr + " = 0x" + Utils.hex(data, 4));
                      if (mode == 10) {
                          cpu.triggBreakpoint();
                      }
                  } else {
                      if (length > 1) {
                          Memory mem = cpu.getMemory();
                          for (int i = address; i < address + length; i++) {
                              context.out.print(Utils.toString(mem.get(i, AccessMode.BYTE), Utils.BYTE, mode));
                          }
                          context.out.println();
                      } else {
                          context.out.print(Utils.toString(data, Utils.BYTE, mode));
                      }
                  }
              }

            @Override
            public void notifyReadBefore(int addr, AccessMode mode, AccessType type) {
                cpuAction(AccessType.READ, addr, cpu.getMemory().get(addr, mode));
            }
            @Override
            public void notifyWriteBefore(int dstAddress, int data, AccessMode mode) {
                cpuAction(AccessType.WRITE, dstAddress, data);
            }
          };

          for (int i = 0; i < length; i++) {
              cpu.addWatchPoint(address + i, monitor);
          }
          if (length > 1) {
              context.err.println("Watch set at $" + cpu.getAddressAsString(address) + " - $" + cpu.getAddressAsString(address + length - 1));
          } else {
              context.err.println("Watch set at $" + cpu.getAddressAsString(address));
          }
          return 0;
        }

        public void stopCommand(CommandContext context) {
            for (int i = 0; i < length; i++) {
                cpu.removeWatchPoint(address + i, monitor);
            }
            context.exit(0);
        }
      });

      ch.registerCommand("watchreg",
          new BasicAsyncCommand("add a write watch to a given register", "<register> [int]") {
        int watchMode = 0;
        int register = 0;
        RegisterMonitor monitor;
        public int executeCommand(final CommandContext context) {
          register = context.getArgumentAsRegister(0);
          if (register < 0) {
            return -1;
          }
          if (context.getArgumentCount() > 1) {
            String modeStr = context.getArgument(1);
            if ("int".equals(modeStr)) {
              watchMode = 1;
            } else {
              context.err.println("illegal argument: " + modeStr);
              return -1;
            }
          }
          monitor = new RegisterMonitor.Adapter() {
            @Override
            public void notifyWriteBefore(int register, int data, AccessMode mode) {
                if (watchMode == 0) {
                    int pc = cpu.getPC();
                    String adrStr = getRegisterName(register);
                    String pcStr = getSymOrAddrELF(cpu, getELF(), pc);
                    context.out.println("*** Write from " + pcStr +
                            ": " + adrStr + " = " + data);
                } else {
                    context.out.println(data);
                }
            }
          };
          cpu.addRegisterWriteMonitor(register, monitor);
          context.err.println("Watch set for register " + getRegisterName(register));
          return 0;
        }

        public void stopCommand(CommandContext context) {
          cpu.removeRegisterWriteMonitor(register, monitor);
        }
      });

//      ch.registerCommand("clear", new BasicCommand("clear a breakpoint or watch from a given address or symbol", "<address or symbol>") {
//        public int executeCommand(final CommandContext context) {
//          int baddr = context.getArgumentAsAddress(0);
//          cpu.setBreakPoint(baddr, null);
//          return 0;
//        }
//      });

      ch.registerCommand("symbol", new BasicCommand("list matching symbols", "<regexp>") {
        public int executeCommand(final CommandContext context) {
          String regExp = context.getArgument(0);
          MapEntry[] entries = context.getMapTable().getEntries(regExp);
          if (entries.length == 0) {
              context.err.println("Could not find any symbols matching '" + regExp + '\'');
          } else {
              for (MapEntry mapEntry : entries) {
                  int address = mapEntry.getAddress();
                  String file = mapEntry.getFile();
                  if (file == null) {
                      file = "(unspecified)";
                  }
                  context.out.println(" " + mapEntry.getName() + " at $"
                          + cpu.getAddressAsString(address) + " ($"
                          + Utils.hex8(cpu.getMemory().get(address, AccessMode.BYTE))
                          + ' ' + Utils.hex8(cpu.getMemory().get(address + 1, AccessMode.BYTE)) + ") "
                          + mapEntry.getType() + " in file " + file);
              }
          }
          return 0;
        }
      });

      ch.registerCommand("debug", new BasicCommand("set debug to on or off", "[0/1]") {
          public int executeCommand(final CommandContext context) {
              if (context.getArgumentCount() > 0) {
                  cpu.setDebug(context.getArgumentAsBoolean(0));
              }
              context.out.println("Debug is set to " + cpu.getDebug());
              return 0;
          }
      });

      ch.registerCommand("line", new BasicCommand("print line number of address/symbol", "<address or symbol>") {
        public int executeCommand(final CommandContext context) {
          int adr = context.getArgumentAsAddress(0);
          DebugInfo di = getELF().getDebugInfo(adr);
          if (di == null) {
            /* quick hack to test next address too... - since something seems to be off by one sometimes... */
            di = getELF().getDebugInfo(adr + 1);
          }
          if (di != null) {
            di.getLine();
            context.out.println(di);
          } else {
            context.err.println("No line number found for: " + context.getArgument(0));
          }
          return 0;
        }
      });

      if (node != null) {
        ch.registerCommand("stop", new BasicCommand("stop the CPU", "") {
          public int executeCommand(CommandContext context) {
            if (!cpu.isRunning()) {
                context.err.println("CPU is not running");
                return 1;
            }
            node.stop();
            context.out.println("CPU stopped at: $" + cpu.getAddressAsString(cpu.getPC()));
            return 0;
          }
        });
        ch.registerCommand("start", new BasicCommand("start the CPU", "") {
          public int executeCommand(CommandContext context) {
            if (cpu.isRunning()) {
                context.err.println("cpu already running");
                return 1;
            }
            node.start();
            return 0;
          }
        });
        ch.registerCommand("throw", new BasicCommand("throw an Emulation Exception", "[message]") {
            public int executeCommand(CommandContext context) {
                final String msg = context.getArgumentCount() > 0 ? context.getArgument(0) : "by request";
                cpu.scheduleCycleEvent(new TimeEvent(0, "EmulationException") {
                    @Override public void execute(long t) {
                        throw new EmulationException(msg);
                    }}, cpu.cycles);
                return 0;
            }
        });

        ch.registerCommand("step", new BasicCommand("single step the CPU", "[number of instructions]") {
          public int executeCommand(CommandContext context) {
            int nr = context.getArgumentCount() > 0 ? context.getArgumentAsInt(0) : 1;
            long cyc = cpu.cycles;
            if (cpu.isRunning()) {
                context.err.println("Can not single step when emulation is running.");
                return -1;
            }
            try {
              node.step(nr);
            } catch (Exception e) {
              e.printStackTrace(context.out);
            }
            context.out.println("CPU stepped to: $" + cpu.getAddressAsString(cpu.getPC()) +
                " in " + (cpu.cycles - cyc) + " cycles (" + cpu.cycles + ")");
            return 0;
          }
        });

        ch.registerCommand("stepmicro", new BasicCommand("single the CPU specified no micros", "<micro skip> <micro step>") {
          public int executeCommand(CommandContext context) {
            long cyc = cpu.cycles;
            if (cpu.isRunning()) {
                context.err.println("Can not single step when emulation is running.");
                return -1;
            }
            long nxt = 0;
            try {
              nxt = cpu.stepMicros(context.getArgumentAsLong(0), context.getArgumentAsLong(1));
            } catch (Exception e) {
              e.printStackTrace(context.out);
            }
            context.out.println("CPU stepped to: $" + cpu.getAddressAsString(cpu.getPC()) +
                " in " + (cpu.cycles - cyc) + " cycles (" + cpu.cycles + ") - next exec time: " + nxt);
            return 0;
          }
        });

        ch.registerCommand("stack", new BasicCommand("show stack info", "") {
          public int executeCommand(CommandContext context) {
            int stackEnd = context.getMapTable().heapStartAddress;
            int stackStart = context.getMapTable().stackStartAddress;
            int current = cpu.getSP();
            context.out.println("Current stack: $" + cpu.getAddressAsString(current) + " (" + (stackStart - current) + " used of " + (stackStart - stackEnd) + ')');
            return 0;
          }
        });
        ch.registerCommand("print", new BasicCommand("print value of an address or symbol", "<address or symbol>") {
          public int executeCommand(CommandContext context) {
            int adr = context.getArgumentAsAddress(0);
            if (adr >= 0) {
              int value = cpu.memory[adr];
              if (adr >= 0x100 && adr + 1 < cpu.MAX_MEM) {
                  value |= cpu.memory[adr + 1] << 8;
              }
              context.out.println(context.getArgument(0) + " = $" + Utils.hex16(value));
              return 0;
            }
            context.err.println("unknown symbol: " + context.getArgument(0));
            return 1;
          }
        });
        ch.registerCommand("printreg", new BasicCommand("print value of an register", "[register]") {
          public int executeCommand(CommandContext context) {
              if (context.getArgumentCount() > 0) {
                  for (int i = 0, n = context.getArgumentCount(); i < n; i++) {
                      int register = context.getArgumentAsRegister(i);
                      if (i > 0) {
                          context.out.print((i % 6) == 0 ? "\n" : " ");
                      }
                      if (register >= 0) {
                          context.out.print(getRegisterName(i) + "=$" + Utils.hex(cpu.getRegister(register), 4));
                      } else {
                          context.out.print(context.getArgument(i) + "=<not a register>");
                      }
                  }
              } else {
                  for (int i = 0; i < 16; i++) {
                      if (i > 0) {
                          context.out.print((i % 6) == 0 ? "\n" : " ");
                      }
                      context.out.print(getRegisterName(i) + "=$" + Utils.hex(cpu.getRegister(i), 4));
                  }
              }
              context.out.println();
              return 0;
          }
        });
        ch.registerCommand("reset", new BasicCommand("reset the CPU", "") {
          public int executeCommand(CommandContext context) {
            cpu.reset();
            return 0;
          }
        });

        ch.registerCommand("time", new BasicCommand("print the elapse time and cycles", "") {
          public int executeCommand(CommandContext context) {
            long time = (long)cpu.getTimeMillis();
	    long wallDiff = System.currentTimeMillis() - lastWall;
            context.out.println("Emulated time elapsed: " + time + "(ms)  since last: " + (time - lastCall) + " ms" + " wallTime: " +
				wallDiff + " ms speed factor: " +
				(wallDiff == 0 ? "N/A" : "" + (time - lastCall) / wallDiff));
            lastCall = time;
            lastWall = System.currentTimeMillis();
            return 0;
          }
        });

        ch.registerCommand("mem", new BasicCommand("dump memory", "<start address> <num_entries> [type] [hex|char|dis]") {
          public int executeCommand(final CommandContext context) {
            int start = context.getArgumentAsAddress(0);
            if (start < 0) {
              context.err.println("Illegal start address: "
                                  + context.getArgument(0));
              return 1;
            }
            int count = context.getArgumentAsInt(1);
            int mode = Utils.DEC;
            int type = Utils.UBYTE;
            boolean signed = false;
            if (context.getArgumentCount() > 2) {
                int pos = 2;
                int acount = context.getArgumentCount();
                if (acount > 4) acount = 4;
                while (pos < acount) {
                    String tS = context.getArgument(pos++);
                    if ("ubyte".equals(tS)) {
                    } else if ("byte".equals(tS)) {
                        type = Utils.BYTE;
                    } else if ("word".equals(tS)) {
                        type = Utils.WORD;
                    } else if ("uword".equals(tS)) {
                        type = Utils.UWORD;
                    } else if ("hex".equals(tS)) {
                        mode = Utils.HEX;
                    } else if ("char".equals(tS)) {
                        mode = Utils.ASCII;
                        type = Utils.BYTE;
                    } else if ("dis".equals(tS)) {
                        mode = Utils.DIS_ASM;
                        type = Utils.WORD;
                    }
                }
            }
            // Does not yet handle signed data...
            DisAsm disAsm = cpu.getDisAsm();
            for (int i = 0; i < count; i++) {
                if (mode == Utils.DIS_ASM) {
                    DbgInstruction dbg = disAsm.disassemble(start, cpu.memory, cpu.reg, new DbgInstruction(),
                            0);
                    String fkn;
                    if ((fkn = dbg.getFunction()) != null) {
                        context.out.println("//// " + fkn);
                    }
                    context.out.println(dbg.getASMLine(false));
                    start += dbg.getSize();
                } else {
                    int data = 0;
                    data = cpu.memory[start++];
                    if (Utils.size(type) == 2) {
                        data = data  + (cpu.memory[start++] << 8);
                    }
                    context.out.print((mode != Utils.ASCII ? " " : "") + 
                            Utils.toString(data, type, mode));
                }
            }
            context.out.println();
            return 0;
          }
        });

        ch.registerCommand("mset", new BasicCommand("set memory", "<address> [type] <value> [value ...]") {
          public int executeCommand(final CommandContext context) {
            int count = context.getArgumentCount();
            int adr = context.getArgumentAsAddress(0);
            String arg2 = context.getArgument(1);
            int type = Utils.BYTE;
            int mode = Utils.DEC;
            boolean typeRead = false;
            if (count > 2) {
              if ("char".equals(arg2)) {
                mode = Utils.ASCII;
                typeRead = true;
              }
              if ("word".equals(arg2)) {
                type = Utils.WORD;
                typeRead = true;
              }
            }
            for (int i = typeRead ? 2 : 1; i < count; i++) {
              if (mode == Utils.DEC) {
                int val = context.getArgumentAsInt(i);
                AccessMode accessMode = Utils.size(type) == 2 || val > 0xff ? AccessMode.WORD : AccessMode.BYTE;
                try {
                  cpu.getMemory().set(adr, val, accessMode);
                  adr += accessMode.bytes;
                } catch (EmulationException e) {
                  e.printStackTrace(context.out);
                }
              } else if (mode == Utils.ASCII) {
                String data = context.getArgument(i);
                Memory mem = cpu.getMemory();
                for (int j = 0; j < data.length(); j++) {
                  mem.set(adr++, data.charAt(j), AccessMode.BYTE);
                }
              }
            }
            return 0;
          }});

        /******************************************************
         * handle external memory (flash, etc).
         ******************************************************/
        ch.registerCommand("xmem", new BasicCommand("dump flash memory", "<start address> <num_entries> [type]") {
          public int executeCommand(final CommandContext context) {
            se.sics.mspsim.chip.Memory xmem = DebugCommands.this.registry.getComponent(se.sics.mspsim.chip.Memory.class, "xmem");
            if (xmem == null) {
              context.err.println("No xmem component registered");
              return 0;
            }
            int start = context.getArgumentAsAddress(0);
            int count = context.getArgumentAsInt(1);
            int size = 1; // unsigned byte
            boolean signed = false;
            if (context.getArgumentCount() > 2) {
              String tS = context.getArgument(2);
              if ("byte".equals(tS)) {
                signed = true;
              } else if ("word".equals(tS)) {
                signed = true;
                size = 2;
              } else if ("uword".equals(tS)) {
                size = 2;
              }
            }
            // Does not yet handle signed data...
            for (int i = 0; i < count; i++) {
              int data = 0;
              data = xmem.readByte(start++);
              if (size == 2) {
                data = data  + (xmem.readByte(start++) << 8);
              }
              context.out.print(" " + data);
            }
            context.out.println();
            return 0;
          }
        });

        ch.registerCommand("xmset", new BasicCommand("set memory", "<address> <value> [type]") {
          public int executeCommand(final CommandContext context) {
            se.sics.mspsim.chip.Memory xmem = DebugCommands.this.registry.getComponent(se.sics.mspsim.chip.Memory.class, "xmem");
            if (xmem == null) {
              context.err.println("No xmem component registered");
              return 0;
            }
            int adr = context.getArgumentAsAddress(0);
            int val = context.getArgumentAsInt(1);
            boolean word = val > 0xff;
            if (word) {
              xmem.writeByte(adr, val >> 8);
              val = val & 0xff;
              adr++;
            }
            xmem.writeByte(adr, val & 0xff);
            return 0;
          }});

        ch.registerCommand("gdbstubs", new BasicCommand("open up a gdb stubs server for GDB remote debugging", "port") {
          private GDBStubs stubs = null;
          public int executeCommand(CommandContext context) {
            if (stubs != null) {
              context.err.println("GDBStubs already open");
            } else {
              int port = context.getArgumentAsInt(0);
              stubs = new GDBStubs();
              stubs.setupServer(cpu, port);
            }
            return 0;
          }
        });

        ch.registerCommand("log", new BasicAsyncCommand("log a loggable object", "[loggable...]" ) {
            private Loggable[] logs;
            private int[] logLevels;
            private LogListener logListener;

            @Override
            public int executeCommand(final CommandContext context) {
                if (context.getArgumentCount() == 0) {
                    Loggable[] loggable = cpu.getLoggables();
                    for (Loggable unit : loggable) {
                        String id = unit.getID();
                        String name = unit.getName();
                        if (id == name) {
                            context.out.println("  " + id);
                        } else {
                            context.out.println("  " + id + " (" + name + ')');
                        }
                    }
                    context.exit(0);
                    return 0;
                }

                final Loggable[] logs = new Loggable[context.getArgumentCount()];
                for(int i = 0, n = context.getArgumentCount(); i < n; i++) {
                    logs[i] = cpu.getLoggable(context.getArgument(i));
                    if (logs[i] == null) {
                        context.err.println("Can not find loggable '" + context.getArgument(i) + '\'');
                        return 1;
                    }
                }
                logListener = new LogListener() {

                    boolean isLogging(Loggable source) {
                        for(Loggable log : logs) {
                            if (source == log) {
                                return true;
                            }
                        }
                        return false;
                    }

                    @Override
                    public void log(Loggable source, String message) {
                        if (isLogging(source)) {
                            context.out.println(source.getID() + ": " + message);
                        }
                    }

                    @Override
                    public void logw(Loggable source, WarningType type,
                            String message) throws EmulationException {
                        if (isLogging(source)) {
                            context.out.println("# " + source.getID() + "[" + type + "]: " + message);
                        }
                    }

                };
                this.logs = logs;
                cpu.getLogger().addLogListener(logListener);
                logLevels = new int[logs.length];
                int i = 0;
                for(Loggable log : logs) {
                    logLevels[i++] = log.getLogLevel();
                    log.setLogLevel(Loggable.DEBUG);
                }
                return 0;
            }

            public void stopCommand(CommandContext context) {
                if (logListener != null) {
                    cpu.getLogger().removeLogListener(logListener);
                    logListener = null;
                }
                if (logs != null) {
                    int i = 0;
                    for(Loggable log : logs) {
                        if (log.getLogLevel() == Loggable.DEBUG) {
                            log.setLogLevel(logLevels[i]);
                        }
                        i++;
                    }
                }
            }
        });

        ch.registerCommand("trace", new BasicCommand("store a trace of execution positions.", "[trace size | show]") {
            @Override
            public int executeCommand(CommandContext context) {
                if (context.getArgumentCount() > 0) {
                    if ("show".equals(context.getArgument(0))) {
                        int size = cpu.getTraceSize();
                        if (size > 0) {
                            DisAsm disAsm = cpu.getDisAsm();
                            for (int i = 0; i < size; i++) {
                                int pc = cpu.getBackTrace(size - 1 - i);
                                DbgInstruction inst = disAsm.getDbgInstruction(pc, cpu);
                                inst.setPos(pc);
                                context.out.println(inst.getASMLine(false));
                            }
                            return 0;
                        }
                    } else {
                        int newSize = context.getArgumentAsInt(0, -1);
                        if (newSize < 0) {
                            return 1;
                        }
                        cpu.setTrace(newSize);
                    }
                }
                context.out.println("Trace size is set to " + cpu.getTraceSize() + " positions.");
                return 0;
            }
        });

        ch.registerCommand("events", new BasicCommand("print event queues", "") {
            @Override
            public int executeCommand(CommandContext context) {
                cpu.printEventQueues(context.out);
              return 0;
            }
          });
      }
    }
  }

  private static String getSymOrAddr(MSP430 cpu, CommandContext context, int adr) {
    MapEntry me = context.getMapTable().getEntry(adr);
    if (me != null) {
      return me.getName();
    }
    return '$' + cpu.getAddressAsString(adr);
  }

  private static String getSymOrAddrELF(MSP430 cpu, ELF elf, int adr) {
    DebugInfo me = elf.getDebugInfo(adr);
    if (me != null) {
      return me.toString();
    }
    return '$' + cpu.getAddressAsString(adr);
  }

  private static String getRegisterName(int register) {
    if (register >= 0 && register < MSP430Constants.REGISTER_NAMES.length) {
      return MSP430Constants.REGISTER_NAMES[register];
    }
    return "R" + register;
  }
}
