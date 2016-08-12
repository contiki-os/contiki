/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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

package org.contikios.cooja.mspmote.interfaces;

import java.util.Collection;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.Clock;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Cycle clock")
public class MspClock extends Clock {
  private static Logger logger = Logger.getLogger(MspClock.class);

  private Simulation simulation;
  
  private long timeDrift; /* Microseconds */
  private double deviation;

  public MspClock(Mote mote) {
    simulation = mote.getSimulation();
    deviation = 1.0;
  }

  public void setTime(long newTime) {
    logger.fatal("Can't change emulated CPU time");
  }

  public long getTime() {
    return simulation.getSimulationTime() + timeDrift;
  }

  public void setDrift(long drift) {
    timeDrift = drift;
  }

  public long getDrift() {
    return timeDrift;
  }
  
  public void setDeviation(double deviation) {
    assert (deviation>0.0) && (deviation<=1.0);
    this.deviation = deviation;
  }

  public double getDeviation() {
    return deviation;
  }
}
