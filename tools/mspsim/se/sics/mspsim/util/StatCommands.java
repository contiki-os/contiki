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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * StatCommands
 *
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : 11 March 2008
 * Updated : $Date$
 *           $Rev$
 */
package se.sics.mspsim.util;
import java.io.PrintStream;

import se.sics.mspsim.cli.BasicAsyncCommand;
import se.sics.mspsim.cli.BasicCommand;
import se.sics.mspsim.cli.BasicLineCommand;
import se.sics.mspsim.cli.CommandBundle;
import se.sics.mspsim.cli.CommandContext;
import se.sics.mspsim.cli.CommandHandler;
import se.sics.mspsim.core.Loggable;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.TimeEvent;

public class StatCommands implements CommandBundle {

  private final MSP430Core cpu;
  private final OperatingModeStatistics statistics;

  public StatCommands(MSP430Core cpu, OperatingModeStatistics statistics) {
    this.cpu = cpu;
    this.statistics = statistics;
  }

  public void setupCommands(ComponentRegistry registry, CommandHandler handler) {
    handler.registerCommand("info", new BasicCommand("show information about specified chip/loggable",
    "[unit...]") {

      @Override
      public int executeCommand(CommandContext context) {
        if (context.getArgumentCount() > 0) {
          for (int i = 0, n = context.getArgumentCount(); i < n; i++) {
            String unitName = context.getArgument(i);
            Loggable unit = cpu.getLoggable(unitName);
            if (unit == null) {
              context.out.println("  " + unitName + ": NOT FOUND");
            } else {
              String id = unit.getID();
              String name = unit.getName();
              if (id == name) {
                context.out.println(unit.getName() + ": " + unit.getClass().getName());
              } else {
                context.out.println(unit.getID() + " (" + unit.getName() + "): "
                    + unit.getClass().getName());
              }
              String info = unit.info();
              if (info != null) {
                context.out.println(info);
              }
            }
          }
        } else {
          Loggable[] units = cpu.getLoggables();
          if (units == null) {
            context.out.println("No loggables found.");
          } else {
            for (int i = 0, n = units.length; i < n; i++) {
              String id = units[i].getID();
              String name = units[i].getName();
              if (id == name) {
                  context.out.println("  " + id);
              } else {
                  context.out.println("  " + id + " (" + name + ')');
              }
            }
          }
        }
        return 0;
      }
        
    });

    handler.registerCommand("mult", new BasicLineCommand("multiply line of doubles",
        "[m1...mn]") {
          double[] multiplicator;
          private PrintStream out;
          public int executeCommand(CommandContext context) {
            this.out = context.out;
            int args = context.getArgumentCount();
            multiplicator = new double[args];
            for(int i = 0; i < args; i++) {
              multiplicator[i] = context.getArgumentAsDouble(i);
            }
            return 0;
          }
          public void lineRead(String line) {
            // Split & parse double on each + multiplicate...
            String[] parts = line.split(" ");
            for(int i = 0; i < parts.length; i++) {
              out.print(multiplicator[i % multiplicator.length] * Double.parseDouble(parts[i]) + " ");
            }
            out.println();
          }
          public void stopCommand(CommandContext context) {
          }
    });
    
    handler.registerCommand("duty", new BasicAsyncCommand("add a duty cycle sampler for operating modes to the specified chips",
        "<frequency> <chip> [chips...]") {

      private PrintStream out;
      private Object[] sources;
      private double frequency;
      private boolean isRunning = true;

      public int executeCommand(CommandContext context) {
        frequency = context.getArgumentAsDouble(0);
        if (frequency <= 0.0) {
          context.err.println("illegal frequency: " + context.getArgument(0));
          return 1;
        }
        sources = new Object[context.getArgumentCount() - 1];
        for (int i = 0, n = sources.length; i < n; i++) {
          String sName = context.getArgument(i + 1);
          if (sName.indexOf('.') >= 0) {
            String[] parts = sName.split("\\.");
            sources[i] = statistics.getDataSource(parts[0], parts[1]); 
            if (sources[i] == null) {
              context.err.println("could not find chip / mode combination " + sName);
              return 1;
            }
          } else {
            sources[i] = statistics.getMultiDataSource(sName); 
            if (sources[i] == null) {
              context.err.println("could not find chip " + sName);
              return 1;
            }
          }
        }
        this.out = context.out;

        cpu.scheduleTimeEventMillis(new TimeEvent(0) {

          @Override
          public void execute(long t) {
            if (isRunning) {
              cpu.scheduleTimeEventMillis(this, 1000.0 / frequency);
              for (int j = 0, n = sources.length; j < n; j++) {
                Object s = sources[j];
                if (j > 0) out.print(' ');
                if (s instanceof MultiDataSource) {
                  MultiDataSource ds = (MultiDataSource) s;
                  for (int k = 0, m = ds.getModeMax(); k <= m; k++) {
                    if (k > 0) out.print(' ');
                    out.print(((int) (ds.getDoubleValue(k) * 100.0 + 0.5)) / 100.0);       
                  }
                } else {
                  out.print( ((int) (((DataSource)s).getDoubleValue() * 100.0 + 0.5)) / 100.0);
                }
              }
              out.println();
            }
          }
        }, 1000.0 / frequency);
        return 0;
      }

      public void stopCommand(CommandContext context) {
        isRunning = false;
        context.exit(0);
      }
    });
  }

}
