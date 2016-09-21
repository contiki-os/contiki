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
 * -----------------------------------------------------------------
 *
 * MSP430
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2007
 */

package se.sics.mspsim.core;
import java.io.PrintStream;

import se.sics.mspsim.profiler.SimpleProfiler;
import se.sics.mspsim.util.ArrayUtils;
import se.sics.mspsim.util.ComponentRegistry;
import se.sics.mspsim.util.MapTable;

public class MSP430 extends MSP430Core {

  private int[] execCounter;
  private int[] trace;
  private int tracePos;
  
  private boolean debug = false;
  private boolean running = false;
  private boolean isBreaking = false;
  private double rate = 2.0;

  // Debug time - measure cycles
  private long lastCycles = 0;
  private long lastCpuCycles = 0;
  private long time;
  private long nextSleep = 0;
  private long nextOut = 0;

  private double lastCPUPercent = 0d;

  private DisAsm disAsm;

  private SimEventListener[] simEventListeners;

  /**
   * Creates a new <code>MSP430</code> instance.
   *
   */
  public MSP430(int type, ComponentRegistry registry, MSP430Config config) {
    super(type, registry, config);
    disAsm = new DisAsm();
  }

  public double getCPUPercent() {
    return lastCPUPercent;
  }

  public DisAsm getDisAsm() {
    return disAsm;
  }

  public void cpuloop() throws EmulationException {
    if (isRunning()) {
      throw new IllegalStateException("already running");
    }
    setRunning(true);
    try {
        // ??? - power-up  should be executed?!
        time = System.currentTimeMillis();
        run();
    } finally {
        setRunning(false);
    }
  }

  private void run() throws EmulationException {
      while (!isStopping) {

      if (cycles > nextOut && !debug) {
	printCPUSpeed(reg[PC]);
	nextOut = cycles + 20000007;
      }

      int pc = emulateOP(-1);
      if (pc >= 0) {
	if (execCounter != null) {
	  execCounter[pc]++;
	}
	if (trace != null) {
	    trace[tracePos++] = pc;
	    if (tracePos >= trace.length)
		tracePos = 0;
	}
	// -------------------------------------------------------------------
	// Debug information
	// -------------------------------------------------------------------
	if (debug) {
	    if (servicedInterrupt >= 0) {
	        disAsm.disassemble(pc, memory, reg, servicedInterrupt);
	    } else {
	        disAsm.disassemble(pc, memory, reg);
	    }
	}
      }

      /* Just a test to see if it gets down to a reasonable speed */
      if (cycles > nextSleep) {
	try {
	  Thread.sleep(100);
	} catch (Exception e) {
	}
	// Frequency = 100 * cycles ratio
	// Ratio = Frq / 100
	nextSleep = cycles + (long)(rate * dcoFrq / 10);
      }

//       if ((instruction & 0xff80) == CALL) {
// 	System.out.println("Call to PC = " + reg[PC]);
//       }
    }
    isStopping = isBreaking = false;
  }

  public long step() throws EmulationException {
    return stepMicros(1, 1);
  }

  public long stepInstructions(int count) throws EmulationException {
    if (isRunning()) {
      throw new IllegalStateException("step not possible when CPU is running");
    }
    setRunning(true);
    try {
    while (count-- > 0 && !isStopping) {
      int pc = emulateOP(-1);
      if (pc >= 0) {
        if (execCounter != null) {
          execCounter[pc]++;
        }
        if (trace != null) {
  	  trace[tracePos++] = pc;
  	  if (tracePos >= trace.length)
  	      tracePos = 0;
        }

        // -------------------------------------------------------------------
        // Debug information
        // -------------------------------------------------------------------
        if (debug) {
            if (servicedInterrupt >= 0) {
                disAsm.disassemble(pc, memory, reg, servicedInterrupt);
            } else {
                disAsm.disassemble(pc, memory, reg);
            }
        }
      }
    }
    } finally { 
        setRunning(false);
    }
    isStopping = isBreaking = false;
    return cycles;
  }
  
  /* this represents the micros time that was "promised" last time */
  /* NOTE: this is a delta compared to "current micros" 
   */
  long lastReturnedMicros;
  long lastMicrosCycles;
  boolean microClockReady = false;

  /* when DCO has changed speed, this method will be called */
  protected void dcoReset() {
      microClockReady = false;
  }
  
  /* 
   * Perform a single step (even if in LPM) but no longer than to maxCycles + 1 instr
   * Note: jumpMicros just jump the clock until that time
   * executeMicros also check eventQ, etc and executes instructions
   */
  long maxCycles = 0;
  public long stepMicros(long jumpMicros, long executeMicros) throws EmulationException {
    if (isRunning()) {
      throw new IllegalStateException("step not possible when CPU is running");
    }

    if (jumpMicros < 0) {
      throw new IllegalArgumentException("Can not jump a negative time: " + jumpMicros);
    }
    /* quick hack - if microdelta == 0 => ensure that we have correct zery cycles
     */
    if (!microClockReady) {
      lastMicrosCycles = maxCycles;
    }
    
    // Note: will be reset during DCO-syncs... => problems ???
    lastMicrosDelta += jumpMicros;

    if (microClockReady) {
    /* check that we did not miss any events (by comparing with last return value) */
    maxCycles = lastMicrosCycles + (lastMicrosDelta * dcoFrq) / 1000000;
    if (cpuOff) {
      if(maxCycles > nextEventCycles) {
        /* back this time again... */
        lastMicrosDelta -= jumpMicros;
        printEventQueues(System.out);
        throw new IllegalArgumentException("Jumping to a time that is further than possible in LPM maxCycles:" + 
            maxCycles + " cycles: " + cycles + " nextEventCycles: " + nextEventCycles);
      }
    } else if (maxCycles > cycles) {
      /* back this time again... */
      lastMicrosDelta -= jumpMicros;
      throw new IllegalArgumentException("Jumping to a time that is further than possible not LPM maxCycles:" + 
          maxCycles + " cycles: " + cycles);
    }

    }
    microClockReady = true;

    /* run until this cycle time */
    maxCycles = lastMicrosCycles + ((lastMicrosDelta + executeMicros) * dcoFrq) / 1000000;
    /*System.out.println("Current cycles: " + cycles + " additional micros: " + (jumpMicros) +
          " exec micros: " + executeMicros + " => Execute until cycles: " + maxCycles);*/


    while (cycles < maxCycles || (cpuOff && (nextEventCycles < cycles))) {
        int pc = emulateOP(maxCycles);
        if (pc >= 0) {
            if (execCounter != null) {
                execCounter[pc]++;
            }
            if (trace != null) {
              if (tracePos >= trace.length) {
                tracePos = 0;
              }
              trace[tracePos++] = pc;
            }
            // -------------------------------------------------------------------
            // Debug information
            // -------------------------------------------------------------------
            if (debug) {
              if (servicedInterrupt >= 0) {
                disAsm.disassemble(pc, memory, reg, servicedInterrupt);
              } else {
                disAsm.disassemble(pc, memory, reg);
              }
            }
        }

        if (isStopping) {
            isStopping = false;
            if (cycles < maxCycles || (cpuOff && (nextEventCycles < cycles))) {
                // Did not complete the execution cycle
                lastMicrosDelta -= jumpMicros;
            }
            if (isBreaking) {
                isBreaking = false;
                throw new BreakpointException();
            }
            lastReturnedMicros = 0;
            return 0;
        }
    }

    if (cpuOff && !(interruptsEnabled && servicedInterrupt == -1 && interruptMax >= 0)) {
      lastReturnedMicros = (1000000 * (nextEventCycles - cycles)) / dcoFrq;
    } else {
      lastReturnedMicros = 0;
    }
    
    if(cycles < maxCycles) {
      throw new RuntimeException("cycles < maxCycles : " + cycles + " < " + maxCycles);
    }
    if(lastReturnedMicros < 0) {
      throw new RuntimeException("lastReturnedMicros < 0 : " + lastReturnedMicros);
    }

    return lastReturnedMicros;
  }

  public void stop() {
      isStopping = true;
  }

  public void triggBreakpoint() {
      isBreaking = true;
      stop();
  }

  public int getDCOFrequency() {
    return dcoFrq;
  }
  public int getExecCount(int address) {
    if (execCounter != null) {
      return execCounter[address];
    }
    return 0;
  }

  public void setMonitorExec(boolean mon) {
    if (mon) {
      if (execCounter == null) {
	execCounter = new int[MAX_MEM];
      }
    } else {
      execCounter = null;
    }
  }

  public void setTrace(int size) {
      if (size == 0) {
	  trace = null;
      } else {
	  trace = new int[size];
      }
      tracePos = 0;
  }
  
  public int getBackTrace(int pos) {
      int tPos = tracePos - pos - 1;
      if (tPos < 0) {
	  tPos += trace.length;
      }
      return trace[tPos];
  }
  
  public int getTraceSize() {
      return trace == null ? 0 : trace.length;
  }

  
  private void printCPUSpeed(int pc) {
    // Passed time
    int td = (int)(System.currentTimeMillis() - time);
    // Passed total cycles
    long cd = (cycles - lastCycles);
    // Passed "active" CPU cycles
    long cpud = (cpuCycles - lastCpuCycles);

    if (td == 0 || cd == 0) {
      return;
    }

    if (DEBUGGING_LEVEL > 0) {
      System.out.println("Elapsed: " + td
			 +  " cycDiff: " + cd + " => " + 1000 * (cd / td )
			 + " cyc/s  cpuDiff:" + cpud + " => "
			 + 1000 * (cpud / td ) + " cyc/s  "
			 + (10000 * cpud / cd)/100.0 + "%");
    }
    lastCPUPercent = (10000 * cpud / cd) / 100.0;
    time = System.currentTimeMillis();
    lastCycles = cycles;
    lastCpuCycles = cpuCycles;
    if (DEBUGGING_LEVEL > 0) {
      disAsm.disassemble(pc, memory, reg);
    }
  }

  public void generateTrace(PrintStream out) {
    if (profiler != null && out != null) {
      profiler.printStackTrace(out);
    }
  }
  
  public boolean getDebug() {
    return debug;
  }

  public void setDebug(boolean db) {
    debug = db;
  }

  public void setMap(MapTable map) {
    this.map = map;
    /* When we got the map table we can also profile! */
    if (profiler == null) {
      setProfiler(new SimpleProfiler());
      profiler.setCPU(this);
    }
  }

  private void setRunning(boolean running) {
    if (this.running != running) {
      this.running = running;
      if (running) {
          isStopping = false;
          isBreaking = false;
      }
      SimEventListener[] listeners = this.simEventListeners;
      if (listeners != null) {
        SimEvent.Type type = running ? SimEvent.Type.START : SimEvent.Type.STOP;
        SimEvent event = new SimEvent(type);
        for(SimEventListener l : listeners) {
          l.simChanged(event);
        }
      }
    }
  }

  public boolean isRunning() {
    return running;
  }

  public double getExecutionRate() {
    return rate;
  }

  public void setExecutionRate(double rate) {
    this.rate = rate;
  }

  public synchronized void addSimEventListener(SimEventListener l) {
    simEventListeners = ArrayUtils.add(SimEventListener.class, simEventListeners, l);
  }

  public synchronized void removeSimEventListener(SimEventListener l) {
    simEventListeners = ArrayUtils.remove(simEventListeners, l);
  }

}
