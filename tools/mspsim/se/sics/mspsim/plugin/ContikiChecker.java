/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * ContikiChecker
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 7 sep 2009
 * Updated : $Date$
 *           $Revision$
 */

package se.sics.mspsim.plugin;
import java.util.Hashtable;

import se.sics.mspsim.cli.BasicAsyncCommand;
import se.sics.mspsim.cli.CommandContext;
import se.sics.mspsim.cli.CommandHandler;
import se.sics.mspsim.core.MSP430;
import se.sics.mspsim.core.Memory.AccessMode;
import se.sics.mspsim.core.MemoryMonitor;
import se.sics.mspsim.core.Profiler;
import se.sics.mspsim.profiler.CallEntry;
import se.sics.mspsim.profiler.CallListener;
import se.sics.mspsim.util.ActiveComponent;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.MapEntry;

/**
 *
 */
public class ContikiChecker implements CallListener, ActiveComponent {

    private ComponentRegistry registry;

    private CommandContext context;
    private MemoryMonitor monitor;
    private MSP430 cpu;
    private Profiler profiler;

    private Hashtable<String,Integer> callTable = new Hashtable<String,Integer>();
    private int callCount = 0;

    public void init(String name, ComponentRegistry registry) {
        this.registry = registry;
    }

    public void start() {
        CommandHandler commandHandler = (CommandHandler) registry.getComponent("commandHandler");
        if (commandHandler != null) {
            commandHandler.registerCommand("contikichecker", new BasicAsyncCommand("enable the Contiki checker", "") {

                @Override
                public int executeCommand(final CommandContext context) {
                    if (profiler != null) {
                        context.err.println("already running");
                        return 1;
                    }
                    cpu = registry.getComponent(MSP430.class);
                    profiler = cpu.getProfiler();
                    if (profiler == null) {
                        context.err.println("no profiler available");
                        return 1;
                    }
                    ContikiChecker.this.context = context;
                    profiler.addCallListener(ContikiChecker.this);

                    context.out.println("Installing watchpoints...");
                    monitor = new MemoryMonitor.Adapter() {
                        @Override
                        public void notifyWriteBefore(int dstAddress, int data, AccessMode mode) {
                            context.out.println("Warning: write to " + dstAddress +
                                    " from " + profiler.getCall(0));
                                //profiler.printStackTrace(context.out);
                        }
                    };
                    for (int i = 0; i < 0x100; i++) {
                        cpu.addWatchPoint(i, monitor);
                    }
                    return 0;
                }

                public void stopCommand(CommandContext context) {
                    if (monitor != null) {
                        for (int i = 0; i < 0x100; i++) {
                            cpu.removeWatchPoint(i, monitor);
                        }
                        monitor = null;
                        cpu = null;
                    }
                    if (profiler != null) {
                        profiler.removeCallListener(ContikiChecker.this);
                        profiler = null;
                    }
                    ContikiChecker.this.context = null;
                }});
        }
    }

    public void functionCall(Profiler source, CallEntry callEntry) {
        MapEntry entry = callEntry.getFunction();
        // Check for function calls before <prefix>_init() has been called.
        String name = entry.getName();
        if (name != null && callTable.get(name) == null) {
            boolean addEntry = true;
            int index = name.indexOf('_');
            if (index > 0 && !name.endsWith("_init")) {
                int lastIndex = name.lastIndexOf('_');
                String init1 = name.substring(0, index + 1) + "init";
                String init2 = index != lastIndex ? (name.substring(0, lastIndex + 1) + "init") : null;
                if (callTable.get(init1) == null && (init2 == null || callTable.get(init2) == null)) {
                    // Warning, lookup in case an init function exists
                    if (context.getMapTable().getFunctionAddress(init1) > 0) {
                        //context.err.println("ContikiChecker: warning, " + name + " is called before " + init1);
                        //profiler.printStackTrace(context.err);
                        addEntry = false;
                    } else if (init2 != null && context.getMapTable().getFunctionAddress(init2) > 0) {
                        //context.err.println("ContikiChecker: warning, " + name + " is called before " + init2);
                        //profiler.printStackTrace(context.err);
                        addEntry = false;
                    }
                }
            }
            if (addEntry) {
                callTable.put(name, ++callCount);
            }
        }
    }

    public void functionReturn(Profiler source, CallEntry entry) {
        // Ignore returns
    }

}
