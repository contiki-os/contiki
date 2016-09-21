/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * ProfilerCommands
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 12 maj 2008
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.cli;
import java.util.Properties;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.EventListener;
import se.sics.mspsim.core.EventSource;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.Profiler;
import se.sics.mspsim.profiler.SimpleProfiler;
import se.sics.mspsim.ui.CPUHeatMap;
import se.sics.mspsim.ui.WindowManager;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.StackMonitor;

/**
 *
 */
public class ProfilerCommands implements CommandBundle {

  public void setupCommands(final ComponentRegistry registry, CommandHandler ch) {
    final MSP430 cpu = registry.getComponent(MSP430.class);
    if (cpu != null) {
      ch.registerCommand("profile", new BasicCommand("show profile information",
          "[-clear] [-sort column] [-showcallers] [regexp]") {
        public int executeCommand(final CommandContext context) {
          Profiler profiler = cpu.getProfiler();
          if (profiler == null) {
            context.err.println("No profiler found.");
            return 1;
          }
          String namematch = null;
          String sortMode = null;
          String showCaller = null;
          int i;

          for (i = 0; i < context.getArgumentCount(); i++) {
            String value = context.getArgument(i);
            if ("-clear".equals(value)) {
              profiler.clearProfile();
              context.out.println("Cleared profile information.");
              return 0;
            } else if ("-sort".equals(value)) {
              if (context.getArgumentCount() > i + 1) { 
                sortMode = context.getArgument(i + 1);
                i++;
              } else {
                context.err.println("Missing mode argument for -sort.");
                return 1;
              }
            } else if ("-showcallers".equals(value)) {
              showCaller = value;
            } else if ("--".equals(value)) {
                /* Done with arguments */
                break;
            } else if (value.startsWith("-")) {
                /* Unknown option */
                context.err.println("Unknown option: " + value);
                return 1;
            } else {
                break;
            }
          }
          if (i < context.getArgumentCount()) {
              namematch = context.getArgument(i++);
              if (i < context.getArgumentCount()) {
                  // Multiple patterns
                  namematch = "(" + namematch;
                  for(; i < context.getArgumentCount(); i++) {
                      namematch += "|" + context.getArgument(i);
                  }
                  namematch += ')';
              }
          }

          Properties params = new Properties();
          if (namematch != null) { 
            params.put(Profiler.PARAM_FUNCTION_NAME_REGEXP, namematch);
          }
          if (showCaller != null) { 
            params.put(Profiler.PARAM_PROFILE_CALLERS, showCaller);
          }
          if (sortMode != null) { 
            params.put(Profiler.PARAM_SORT_MODE, sortMode);
          }
          profiler.printProfile(context.out, params);
          return 0;
        }
      });

      ch.registerCommand("stacktrace", new BasicCommand("show stack trace", "") {
        public int executeCommand(CommandContext context) {
          Profiler profiler = cpu.getProfiler();
          if (profiler == null) {
            context.err.println("No profiler found.");
            return 1;
          }
          profiler.printStackTrace(context.out);
          return 0;
        }
      });

      ch.registerCommand("stackprof", new BasicCommand("Start stack profiler", "") {
          public int executeCommand(CommandContext context) {
              new StackMonitor(cpu);
              return 0;
          }
      });
      
//      ch.registerCommand("irqprofile", new BasicCommand("show interrupt profile", "") {
//        public int executeCommand(CommandContext context) {
//          long[] time = cpu.getInterruptTime();
//          long[] ctr = cpu.getInterruptCount();
//          context.out.println("Interrupt statistics");
//          context.out.println("Vector\tAvg\tCount");
//          
//          for (int i = 0; i < ctr.length; i++) {
//            long avg = ctr[i] != 0 ? (time[i] / ctr[i]) : 0;
//            context.out.println(i + "\t" + avg + "\t" + ctr[i]);
//          }
//          return 0;
//        }
//
//      });
      
      ch.registerCommand("logevents", new BasicAsyncCommand("log events", "[chips...]") {
        Chip[] chips;
        EventListener eventListener;
        public int executeCommand(final CommandContext context) {
            if (context.getArgumentCount() == 0) {
                context.out.println("Available chips:");
                for(Chip chip : cpu.getChips()) {
                    String id = chip.getID();
                    String name = chip.getName();
                    if (id == name) {
                        context.out.println("  " + id);
                    } else {
                        context.out.println("  " + id + " (" + name + ')');
                    }
                }
                context.exit(0);
                return 0;
            }
            chips = new Chip[context.getArgumentCount()];
            for(int i = 0, n = chips.length; i < n; i++) {
                chips[i] = cpu.getChip(context.getArgument(i));
                if (chips[i] == null) {
                    context.err.println("Can not find chip: " + context.getArgument(i));
                    return 1;
                }
            }
            eventListener = new EventListener() {
                public void event(EventSource source, String event, Object data) {
                    context.out.println("Event:" + source.getName() + ":" + event);
                }
            };
            for (Chip chip : chips) {
                chip.addEventListener(eventListener);
            }
            return 0;
        }
        public void stopCommand(CommandContext context) {
            for (Chip chip : chips) {
                chip.removeEventListener(eventListener);
            }
        }
      });

      ch.registerCommand("tagprof", new BasicCommand("profile between two events", "") {
        public int executeCommand(CommandContext context) {
          String event1 = context.getArgument(0);
          String event2 = context.getArgument(1);
          String chip1[] = event1.split("\\.");
          String chip2[] = event2.split("\\.");
          Chip chipE1 = cpu.getChip(chip1[0]);
          if (chipE1 == null) {
            context.err.println("Can not find chip: " + chip1[0]);
            return 1;
          }
          Chip chipE2 = cpu.getChip(chip2[0]);
          if (chipE2 == null) {
            context.err.println("Can not find chip: " + chip2[0]);
            return 1;
          }
          Profiler profiler = cpu.getProfiler();
          SimpleProfiler sprof = (SimpleProfiler) profiler;
          sprof.addProfileTag(context.getArgument(2), chipE1, chip1[1],
              chipE2, chip2[1]);
          return 0;
        }
      });      

      ch.registerCommand("printtags", new BasicCommand("print tags profile", "") {
        public int executeCommand(CommandContext context) {
          Profiler profiler = cpu.getProfiler();
          SimpleProfiler sprof = (SimpleProfiler) profiler;
          sprof.printTagProfile(context.out);
          return 0;
        }
      });      

      
      ch.registerCommand("logcalls", new BasicAsyncCommand("log function calls", "") {
        public int executeCommand(CommandContext context) {
          Profiler profiler = cpu.getProfiler();
          if (profiler == null) {
            context.err.println("No profiler found.");
            return 1;
          }
          profiler.setLogger(context.out);
          return 0;
        }
        public void stopCommand(CommandContext context) {
          Profiler profiler = cpu.getProfiler();
          if (profiler != null) {
            profiler.setLogger(null);
          }
        }
      });
      
      ch.registerCommand("profiler", new BasicCommand("configure profiler",
          "<command> <arguments>") {
            public int executeCommand(CommandContext context) {
              // TODO: add more API's to the Profiler???
              SimpleProfiler profiler = (SimpleProfiler) cpu.getProfiler();
              if (profiler == null) {
                context.err.println("No profiler found.");
                return 1;
              }
              String cmd = context.getArgument(0);
              if ("hide".equals(cmd)) {
                for (int j = 1, n = context.getArgumentCount(); j < n; j++) {
                  profiler.addIgnoreFunction(context.getArgument(j));
                }
              } else if ("hideirq".equals(cmd)) {
                profiler.setHideIRQ(context.getArgumentAsBoolean(1));
              }
              return 0;
            }
      });
      
      ch.registerCommand("readmap", new BasicAsyncCommand("read map", "") {
          private CPUHeatMap hm;

          public int executeCommand(CommandContext context) {
              hm = new CPUHeatMap(cpu, registry.getComponent(WindowManager.class));
              cpu.addGlobalMonitor(hm);
              return 0;
          }

          public void stopCommand(CommandContext context) {
              if (hm != null) {
                  cpu.removeGlobalMonitor(hm);
                  hm.close();
                  hm = null;
              }
          }
      });
    }
  }

}
