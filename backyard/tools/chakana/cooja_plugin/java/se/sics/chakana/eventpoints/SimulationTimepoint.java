/*
 * Copyright (c) 2007, Swedish Institute of Computer Science. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * $Id: SimulationTimepoint.java,v 1.1 2009/02/26 13:47:38 fros4943 Exp $
 */

package se.sics.chakana.eventpoints;

import se.sics.cooja.Simulation;

/**
 * TODO Document
 *
 * @author Fredrik Osterlind
 */
public class SimulationTimepoint implements Timepoint {
  private int myID = 0;
  private Simulation simulation;
  private int time;
  
  private String message;

  /**
   * TODO Document
   * 
   * @param simulation
   * @param time
   */
  public SimulationTimepoint(Simulation simulation, int time) {
    this.simulation = simulation;
    this.time = time;
  }

  public boolean evaluate() {
    if (simulation.getSimulationTime() >= time) {
      message = "Simulation time = " + simulation.getSimulationTime() + " >= " + time;
      return true;
    }
    return false;
  }
  
  public String getMessage() {
    return message;
  }
  
  public String toString() {
    return "Simulation timepoint: " + time;
  }

  public void setID(int id) {
    myID = id;
  }

  public int getID() {
    return myID;
  }
}
