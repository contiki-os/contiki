/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 */

package org.contikios.cooja.interfaces;

import org.contikios.cooja.*;

/**
 * Represents a mote's internal clock. Notice that the overall
 * simulation time and the mote's own time may differ.
 *
 * This observable never notifies.
 *
 * @author Fredrik Osterlind
 *         Andreas Löscher
 */
@ClassDescription("Clock")
public abstract class Clock extends MoteInterface {

  /**
   * Set mote's time to given time.
   *
   * @param newTime Time
   */
  public abstract void setTime(long newTime);

  /**
   * @return Current time
   */
  public abstract long getTime();

  /**
   * Set time drift.
   *
   * @param timeDrift Time drift
   */
  public abstract void setDrift(long timeDrift);

  /**
   * The clock drift provides information about the mote's internal time,
   * and can the used to calculate for instance its startup time.
   * 
   * The startup time is the negative drift time.
   * 
   * The mote internal time can be calculated by:
   * [current simulation time] + [mote drift].
   * 
   * @see Simulation#getSimulationTime()
   * @return Time drift
   */
  public abstract long getDrift();


  /**
   * The clock deviation is a factor that represents with how much speed the
   * mote progresses through the simulation in relation to the simulation speed.
   *
   * A value of 1.0 results in the mote beeing simulated with the same speed
   * as the simulation. A value of 0.5 results in the mote beeing simulation
   * at half of the simulation speed.
   *
   *  @param deviation Deviation factor
   */
  public abstract void setDeviation(double deviation);

  /**
   * Get deviation factor
   */
  public abstract double getDeviation();
}
