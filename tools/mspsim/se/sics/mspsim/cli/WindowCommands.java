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
 *
 * $Id: WindowCommands.java 187 2008-03-17 19:34:12Z joxe $
 *
 * -----------------------------------------------------------------
 *
 * WindowCommands - 
 * 
 * Author  : Joakim Eriksson, Niclas Finne
 * Created : 9 april 2008
 * Updated : $Date: 2008-03-17 20:34:12 +0100 (Mon, 17 Mar 2008) $
 *           $Revision: 187 $
 */
package se.sics.mspsim.cli;

import java.util.Hashtable;

import se.sics.mspsim.ui.WindowManager;
import se.sics.mspsim.ui.WindowUtils;
import se.sics.mspsim.util.ComponentRegistry;

public class WindowCommands implements CommandBundle {

    private ComponentRegistry registry;
    private final Hashtable<String,Target> windowTargets = new Hashtable<String,Target>();

    public void setupCommands(ComponentRegistry registry, CommandHandler handler) {
        this.registry = registry;
        handler.registerCommand("window", new BasicLineCommand("redirect input to a window", "[-close|-clear|-list] [windowname]") {
            Target wt;
            CommandContext context;
            public int executeCommand(CommandContext context) {
                boolean close = false;
                boolean clear = false;
                boolean list = false;
                String windowName = null;
                this.context = context;
                for (int i = 0; i < context.getArgumentCount(); i++) {
                    String name = context.getArgument(i);
                    if ("-close".equals(name)) {
                        close = true;
                    } else if ("-clear".equals(name)) {
                        clear = true;
                    } else if ("-list".equals(name)) {
                        list = true;
                    } else if (windowName != null) {
                        context.err.println("illegal arguments");
                        context.exit(1);
                        return 1;
                    } else {
                        windowName = name;
                    }
                }
                if (list || windowName == null) {
                    Target tgts[];
                    synchronized (windowTargets) {
                        tgts = windowTargets.values().toArray(new Target[windowTargets.size()]);
                    }
                    if (tgts.length == 0) {
                        context.out.println("There are no open windows.");
                    } else {
                        context.out.println("Window Name   PIDs");
                        for (Target target : tgts) {
                            context.out.println(target.getStatus());
                        }
                    }
                    context.exit(0);
                    return 0;
                }
                if (close) {
                    Target target = windowTargets.get(windowName);
                    if (target != null) {
                        context.out.println("Closing window " + windowName);
                        target.close();
                        context.exit(0);
                        return 0;
                    }
                    context.err.println("Could not find the window " + windowName);
                    context.exit(1);
                    return 1;
                }
                wt = addTarget(context, windowName, clear);
                return 0;
            }

            public void lineRead(String line) {
                wt.lineRead(context, line);
            }

            public void stopCommand(CommandContext context) {
                if (wt != null) {
                    wt.removeContext(context);
                }
            }
        });

        handler.registerCommand("wclear", new BasicCommand("resets stored window positions", "") {
            public int executeCommand(CommandContext context) {
                WindowUtils.clearState();
                return 0;
            }
        });
    }

    protected Target addTarget(CommandContext context, String name, boolean clear) {
        Target target;
        WindowTarget wt = null;
        synchronized (windowTargets) {
            target = windowTargets.get(name);
            if (target == null) {
                target = wt = new WindowTarget(windowTargets, name);
            }
        }
        if (wt != null) {
            wt.init((WindowManager) registry.getComponent("windowManager"));
        }
        if (context.getPID() >= 0) {
            target.addContext(context);
        }
        if (clear) {
            target.lineRead(context, "#!clear");
        }
        return target;
    }

}
