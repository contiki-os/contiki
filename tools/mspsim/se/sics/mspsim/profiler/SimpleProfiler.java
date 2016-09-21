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
 * $Id$
 *
 * -----------------------------------------------------------------
 *
 * SimpleProfiler
 *
 * Author  : Joakim Eriksson
 * Created : March 5, 2013
 */

package se.sics.mspsim.profiler;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.regex.Pattern;

import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.EventListener;
import se.sics.mspsim.core.EventSource;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.Profiler;
import se.sics.mspsim.profiler.CallEntry;
import se.sics.mspsim.profiler.CallEntry.CallCounter;
import se.sics.mspsim.util.ArrayUtils;
import se.sics.mspsim.util.MapEntry;
import se.sics.mspsim.util.StackMonitor;
import se.sics.mspsim.util.Utils;

public class SimpleProfiler implements Profiler, EventListener {
 
  private HashMap<MapEntry,CallEntry> profileData;
  private HashMap<String, TagEntry> tagProfiles;
  private HashMap<String, TagEntry> startTags;
  private HashMap<String, TagEntry> endTags;
  private HashMap<String, String> ignoreFunctions;
  private CallEntry[] callStack;
  private int cSP = 0;
  private MSP430Core cpu;
  private PrintStream logger;
  private boolean hideIRQ = false;

  private CallListener[] callListeners;

  /* statistics for interrupts */
  private long[] lastInterruptTime = new long[64];
  private long[] interruptTime = new long[64];
  private long[] interruptCount = new long[64];
  private int servicedInterrupt;
  private int interruptLevel;
  private int interruptFrom;
  private boolean newIRQ;

  private StackMonitor stackMonitor;
  
  public SimpleProfiler() {
    profileData = new HashMap<MapEntry, CallEntry>();
    tagProfiles = new HashMap<String, TagEntry>();
    startTags = new HashMap<String, TagEntry>();
    endTags = new HashMap<String, TagEntry>();
    ignoreFunctions = new HashMap<String, String>();
    callStack = new CallEntry[64];
    servicedInterrupt = -1;
  }

  public void setCPU(MSP430Core cpu) {
    this.cpu = cpu;
  }

  public void setStackMonitor(StackMonitor stackMonitor) {
      this.stackMonitor = stackMonitor;
  }

  public void setHideIRQ(boolean hide) {
    hideIRQ = hide;
  }
  
  public void addIgnoreFunction(String function) {
    ignoreFunctions.put(function, function);
  }
  
  public void profileCall(MapEntry entry, long cycles, int from) {
    if (cSP == callStack.length) {
      CallEntry[] tmp = new CallEntry[cSP + 64];
      System.arraycopy(callStack, 0, tmp, 0, cSP);
      callStack = tmp;
    }
    if (callStack[cSP] == null) {
      callStack[cSP] = new CallEntry();
    }

    int hide = 0;
    PrintStream logger = this.logger;
    if (logger != null) {
      /* hide this if last call was to be hidden */
      hide = (cSP == 0 || newIRQ) ? 0 : callStack[cSP - 1].hide;
      /* increase level of "hide" if last was hidden */
      if (hide > 0) hide++;
      if ((!hideIRQ || servicedInterrupt == -1) && hide == 0) {
        if (servicedInterrupt >= 0) logger.printf("[%2d] ", servicedInterrupt);
        printSpace(logger, (cSP - interruptLevel) * 2);
        logger.println("Call to $" + Utils.hex(entry.getAddress(), 4) +
                       ": " + entry.getInfo());
        if (ignoreFunctions.get(entry.getName()) != null) {
          hide = 1;
        }
      }
    }

    CallEntry ce = callStack[cSP++];
    ce.function = entry;
    ce.calls = 0;
    ce.cycles = cycles;
    ce.exclusiveCycles = cycles;
    ce.hide = hide;
    ce.fromPC = from;
    newIRQ = false;

    
    if (stackMonitor != null) {
        /* get the current stack MAX for previous function */
        if (cSP > 1) {
            callStack[cSP - 2].currentStackMax = stackMonitor.getProfStackMax();
        }
        /* start stack here! */
        ce.stackStart = stackMonitor.getStack();
        stackMonitor.setProfStackMax(stackMonitor.getStack());
    }

    CallListener[] listeners = callListeners;
    if (listeners != null) {
      for (int i = 0, n = listeners.length; i < n; i++) {
        listeners[i].functionCall(this, ce);
      }
    }
  }

  public void profileReturn(long cycles) {
    if (cSP <= 0) {
      /* the stack pointer might have been messed with? */
      PrintStream logger = this.logger;
      if (logger != null) {
          // logger.println("SimpleProfiler: Too many returns?");
      } else {
        // System.err.println("SimpleProfiler: Too many returns?");
      }
      return;
    }
    CallEntry cspEntry = callStack[--cSP];
    MapEntry fkn = cspEntry.function;
//     System.out.println("Profiler: return / call stack: " + cSP + ", " + fkn);
    
    long elapsed = cycles - cspEntry.cycles;
    long exElapsed = cycles - cspEntry.exclusiveCycles;
    if (cSP != 0) {
      callStack[cSP-1].exclusiveCycles += elapsed;
    }
    int maxUsage = 0;
    
    if (cspEntry.calls >= 0) {
      CallEntry ce = profileData.get(fkn);
      if (ce == null) {
        profileData.put(fkn, ce = new CallEntry());
        ce.function = fkn;
      }
      ce.cycles += elapsed;
      ce.exclusiveCycles += exElapsed;
      ce.calls++;
      
      if (stackMonitor != null) {
          maxUsage = stackMonitor.getProfStackMax() - cspEntry.stackStart;
          ce.stackStart = cspEntry.stackStart;
          if (maxUsage > ce.currentStackMax) {
              ce.currentStackMax = maxUsage;
          }
          if (cSP != 0) {
              /* put the max for previous function back into the max profiler */ 
              stackMonitor.setProfStackMax(callStack[cSP-1].currentStackMax);
          }
      }

      
      
      if (cSP != 0) {
        MapEntry caller = callStack[cSP-1].function;
        HashMap<MapEntry,CallCounter> callers = ce.callers;
        CallCounter numCalls = callers.get(caller);
        if (numCalls == null) {
          numCalls = new CallCounter();
          callers.put(caller, numCalls);
        }
        numCalls.count++;
      }

      PrintStream logger = this.logger;
      if (logger != null) {
        if ((cspEntry.hide <= 1) && (!hideIRQ || servicedInterrupt == -1)) {
          if (servicedInterrupt >= 0) logger.printf("[%2d] ",servicedInterrupt);
          printSpace(logger, (cSP - interruptLevel) * 2);
          logger.println("return from " + ce.function.getInfo() + " elapsed: " + elapsed + " maxStackUsage: " + maxUsage);
        }
      }

      CallListener[] listeners = callListeners;
      if (listeners != null) {
        for (int i = 0, n = listeners.length; i < n; i++) {
          listeners[i].functionReturn(this, cspEntry);
        }
      }
    }
    newIRQ = false;
  }

  public void profileInterrupt(int vector, long cycles) {
    servicedInterrupt = vector;
    interruptFrom = cpu.getPC(); 
    lastInterruptTime[servicedInterrupt] = cycles;
    interruptLevel = cSP;
    newIRQ = true;

    PrintStream logger = this.logger;
    if (logger != null && !hideIRQ) {
      logger.println("----- Interrupt vector " + vector + " start execution -----");
    }
  }
  
  public void profileRETI(long cycles) {
    if (servicedInterrupt > -1) {
      interruptTime[servicedInterrupt] += cycles - lastInterruptTime[servicedInterrupt];
      interruptCount[servicedInterrupt]++;
    }
    newIRQ = false;

    PrintStream logger = this.logger;
    if (logger != null && !hideIRQ) {
      logger.println("----- Interrupt vector " + servicedInterrupt + " returned - elapsed: " +
          (cycles - lastInterruptTime[servicedInterrupt]));
    }
    interruptLevel = 0;
    
    /* what if interrupt from interrupt ? */
    servicedInterrupt = -1;
  }

  public void resetProfile() {
    clearProfile();
    cSP = 0;
    servicedInterrupt = -1;
  }

  public void clearProfile() {
    if (profileData != null) {
      CallEntry[] entries =
        profileData.values().toArray(new CallEntry[profileData.size()]);
      for (int i = 0, n = entries.length; i < n; i++) {
        entries[i].cycles = 0;
        entries[i].calls = 0;
      }
      for (int i = 0, n = callStack.length; i < n; i++) {
        CallEntry e = callStack[i];
        if (e != null) {
          e.calls = -1;
        }
      }
    }
  }  

  public void printProfile(PrintStream out) {
    printProfile(out, new Properties());
  }

  public void printProfile(PrintStream out, Properties parameters) {
    String functionNameRegexp = parameters.getProperty(PARAM_FUNCTION_NAME_REGEXP);
    String profSort = parameters.getProperty(PARAM_SORT_MODE);
    boolean profCallers = parameters.getProperty(PARAM_PROFILE_CALLERS) != null;
    Pattern pattern = null;
    CallEntry[] entries = profileData.values().toArray(new CallEntry[profileData.size()]);

    Arrays.sort(entries, new CallEntryComparator(profSort));
    
    out.println("************************* Profile Data **************************************");
    out.println("Function                              Calls    Average       Total  Exclusive");

    if (functionNameRegexp != null && functionNameRegexp.length() > 0) {
      pattern = Pattern.compile(functionNameRegexp);
    }
    for (int i = 0, n = entries.length; i < n; i++) {
      int c = entries[i].calls;
      if (c > 0) {
        String functionName = entries[i].function.getName();
        if (pattern == null || pattern.matcher(functionName).find()) {
          String cyclesS = "" + entries[i].cycles;
          String exCyclesS = "" + entries[i].exclusiveCycles;
          String callS = "" + c;
          String avgS = "" + (c > 0 ? (entries[i].cycles / c) : 0);
          out.print(functionName);
          printSpace(out, 43 - functionName.length() - callS.length());
          out.print(callS);
          out.print(' ');
          printSpace(out, 10 - avgS.length());
          out.print(avgS);
          out.print(' ');
          printSpace(out, 11 - cyclesS.length());
          out.print(cyclesS);
          printSpace(out, 11 - exCyclesS.length());
          out.println(exCyclesS);
          if (profCallers) {
            printCallers(entries[i], out);
          }
        }
      }
    }
    if (pattern == null) {
      out.println("********** Profile IRQ **************************");
      out.println("Vector          Average    Calls  Tot.Cycles");
      for (int i = 0; i <= cpu.config.maxInterruptVector; i++) {
        out.print((i < 10 ? "0" : "") + i + "               ");
        out.printf("%4d ",(interruptCount[i] > 0 ? (interruptTime[i] / interruptCount[i]):0));
        out.printf("%8d   %8d",interruptCount[i],interruptTime[i]);
        out.println();
      }
    }
  }

  private void printCallers(CallEntry callEntry, PrintStream out) {
    HashMap<MapEntry,CallCounter> callers = callEntry.callers;
    List<Entry<MapEntry,CallCounter>> list = new ArrayList<Entry<MapEntry,CallCounter>>(callers.entrySet());
    Collections.sort(list, new Comparator<Entry<MapEntry,CallCounter>>() {
        public int compare(Entry<MapEntry,CallCounter> o1, Entry<MapEntry,CallCounter> o2) {
          return o2.getValue().compareTo(o1.getValue());
        }
    });
    for (Entry<MapEntry,CallCounter> entry : list) {
      String functionName = entry.getKey().getName();
      String callS = "" + entry.getValue().count;
      printSpace(out, 12 - callS.length());
      out.print(callS);
      printSpace(out, 2);
      out.print(functionName);
      out.println();
    }
  }

  private void printSpace(PrintStream out, int len) {
    for (int i = 0; i < len; i++) {
      out.print(' ');
    }
  }

  public void printStackTrace(PrintStream out) {
    int stackCount = cSP;
    out.println("Stack Trace: number of calls: " + stackCount
        + " PC: $" + Utils.hex(cpu.getPC(), 5));
    for (int i = 0; i < stackCount; i++) {
      CallEntry call = callStack[stackCount - i - 1];
      out.println("  " + call.function.getInfo()
          + " called from PC: $" + Utils.hex(call.fromPC, 5)
          + " (elapsed: " + (cpu.cpuCycles - call.cycles) + ')');
      if (stackCount - i - 1 == interruptLevel && servicedInterrupt != -1) {
        out.println(" *** Interrupt " + servicedInterrupt + " from PC: $" + Utils.hex(interruptFrom, 5));
      }
    }
  }

  private static class CallEntryComparator implements Comparator<CallEntry> {
    private int mode;
    
    public CallEntryComparator(String modeS) {
      if ("exclusive".equalsIgnoreCase(modeS)) {
        mode = 1;
      } else if ("calls".equalsIgnoreCase(modeS)) {
        mode = 2;
      } else if ("average".equalsIgnoreCase(modeS)) {
        mode = 3;
      } else if ("function".equalsIgnoreCase(modeS)) {
        mode = 4;
      } else {
        mode = 0;
      }
    }
    
    public int compare(CallEntry o1, CallEntry o2) {
      long diff;
      switch (mode) {
      case 1:
        diff = o2.exclusiveCycles - o1.exclusiveCycles;
        break;
      case 2:
        diff = o2.calls - o1.calls;
        break;
      case 3:
        diff = (o2.calls > 0 ? (o2.cycles / o2.calls) : 0) -
        (o1.calls > 0 ? (o1.cycles / o1.calls) : 0);
        break;
      case 4:
        return o1.function.getName().compareTo(o2.function.getName());
      default:
        diff = o2.cycles - o1.cycles;
      }
      if (diff > 0) return 1;
      if (diff < 0) return -1;
      return 0;
    }
  }
  

  private static class TagEntry implements Comparable<TagEntry> {
    public final String tag;
    long cycles;
    long lastCycles;
    int calls;

    public TagEntry(String tag) {
        this.tag = tag;
    }

    public int compareTo(TagEntry o) {
      long diff = o.cycles - cycles;
      if (diff > 0) return 1;
      if (diff < 0) return -1;
      return 0;
    }
  }

  
  
  public void setLogger(PrintStream out) {
    logger = out;
  }
  
  /* 
   * Tag profiling.
   */
  public void measureStart(String tag) {
    TagEntry tagEntry = tagProfiles.get(tag);
    if (tagEntry == null) {
      tagEntry = new TagEntry(tag);
      tagProfiles.put(tag, tagEntry);
    }
    /* only the first occurrence of event will set the lastCycles */
    if (tagEntry.lastCycles == 0) {
      tagEntry.lastCycles = cpu.cycles;
    }
  }
  
  public void measureEnd(String tag) {
    TagEntry tagEntry = tagProfiles.get(tag);
    if (tagEntry != null) {
      if (tagEntry.lastCycles != 0) {
        tagEntry.calls++;
        tagEntry.cycles += cpu.cycles - tagEntry.lastCycles;
        tagEntry.lastCycles = 0;
      }
    }
  }
  
  public void printTagProfile(PrintStream out) {
    TagEntry[] entries = tagProfiles.values().toArray(new TagEntry[tagProfiles.size()]);
    Arrays.sort(entries);
    for (TagEntry entry : entries) {
      out.println(entry.tag + "\t" + entry.calls + "\t" + entry.cycles);
    }
  }

  public void addProfileTag(String tag, Chip chip, String start,
      Chip chip2, String end) {
    System.out.println("Add profile: " + tag +
        " start: " + start + " end: " + end);
    TagEntry tagEntry = new TagEntry(tag);
    startTags.put(start, tagEntry);
    endTags.put(end, tagEntry);
    tagProfiles.put(tag, tagEntry);
    chip.addEventListener(this);
    chip2.addEventListener(this);
  }

  public void event(EventSource source, String event, Object data) {
    TagEntry tagEntry = null;
    if ((tagEntry = startTags.get(event)) != null) {
      /* only the first occurrence of event will set the lastCycles */
      if (tagEntry.lastCycles == 0) {
        tagEntry.lastCycles = cpu.cycles;
      }
    } else if ((tagEntry = endTags.get(event)) != null) {
      if (tagEntry.lastCycles != 0) {
        tagEntry.calls++;
        tagEntry.cycles += cpu.cycles - tagEntry.lastCycles;
        tagEntry.lastCycles = 0;
      }
    }
  }

  public synchronized void addCallListener(CallListener listener) {
    callListeners = ArrayUtils.add(CallListener.class, callListeners, listener);
  }

  public synchronized void removeCallListener(CallListener listener) {
    callListeners = ArrayUtils.remove(callListeners, listener);
  }

  public String getCall(int i) {
    return callStack[cSP - i - 1].function.getInfo();
  }

  public MapEntry getCallMapEntry(int i) {
    return callStack[cSP - i - 1].function;
  }

}
