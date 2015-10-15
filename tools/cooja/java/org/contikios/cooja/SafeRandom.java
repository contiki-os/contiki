/*
 * Copyright (c) 2014, Friedrich-Alexander University Erlangen-Nuremberg.
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
 * 3. Neither the name of the university nor the names of its contributors
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

package org.contikios.cooja;

import java.util.Random;

/**
 * This ensures that the functions of the random number generator are
 * only called by the the thread initializing a simulation or the
 * simulation thread itself.
 * Rationale: By allowing another thread to use the random number
 * generator concurrency is introduced, thus it can not be guaranteed
 * that simulations are reproducible.
 *
 */
public class SafeRandom extends Random {
  
  Simulation sim = null;
  Thread initThread = null;
  Boolean simStarted = false;
  
  private void assertSimThread() {
    // sim can be null, because setSeed is called by the super-constructor.
    if(sim == null) return;
    
    // If we are in the simulation thread, everything is fine (the default)
    if(sim.isSimulationThread()) {
      simStarted = true;
      return;
    }
    
    // Simulation has not started yet. Allow one initialisation thread.
    if(!simStarted) {
      if(initThread == null) initThread = Thread.currentThread();
      if(Thread.currentThread() == initThread ) return;
    }
    
    //This is done via the GUI - reproducibility is lost anyway!
    if(javax.swing.SwingUtilities.isEventDispatchThread()) return;
    
    // Some other threads seems to access the PNRG. This must not happen.
    throw new RuntimeException("A random-function was not called from the simulation thread. This can break things!");
    
  }
  
  public SafeRandom(Simulation sim) {
    // assertSimThread is called by the super-constructor.
    super();
    this.sim = sim;
  }
  
  public SafeRandom(Simulation sim, long seed) {
    // assertSimThread is called by the super-constructor.
    super(seed);
    this.sim = sim;
  }
  
  synchronized public void setSeed(long seed) {
    assertSimThread();
    super.setSeed(seed);
  }
  
  /*
   * This function is called by all functions returning random numbers
   * @see java.util.Random#next(int)
   */
  protected int next(int bits) {
    assertSimThread();
    return super.next(bits);
  }
  
}
