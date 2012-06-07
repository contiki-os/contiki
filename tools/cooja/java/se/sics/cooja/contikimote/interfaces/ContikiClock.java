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
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.Collection;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.SectionMoteMemory;
import se.sics.cooja.Simulation;
import se.sics.cooja.contikimote.ContikiMote;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Clock;
import se.sics.cooja.interfaces.PolledAfterAllTicks;
import se.sics.cooja.interfaces.PolledBeforeActiveTicks;

/**
 * Clock mote interface. Controls Contiki time.
 *
 * Contiki variables:
 * <ul>
 * <li>clock_time_t simCurrentTime
 * <li>clock_time_t simNextExpirationTime
 * <li>int simProcessRunValue
 * <li>int simEtimerPending
 * </ul>
 *
 * Core interface:
 * <ul>
 * <li>clock_interface
 * </ul>
 * <p>
 *
 * This observable never notifies.
 *
 * @author Fredrik Osterlind
 */
public class ContikiClock extends Clock implements ContikiMoteInterface, PolledBeforeActiveTicks, PolledAfterAllTicks {
  private static Logger logger = Logger.getLogger(ContikiClock.class);

  private Simulation simulation;
  private ContikiMote mote;
  private SectionMoteMemory moteMem;

  private long moteTime; /* Microseconds */
  private long timeDrift; /* Microseconds */

  /**
   * @param mote Mote
   *
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiClock(Mote mote) {
    this.simulation = mote.getSimulation();
    this.mote = (ContikiMote) mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
    timeDrift = 0;
    moteTime = 0;
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"clock_interface"};
  }

  public void setTime(long newTime) {
    moteTime = newTime;
    if (moteTime > 0) {
      moteMem.setIntValueOf("simCurrentTime", (int)(newTime/1000));
    }
  }

  public void setDrift(long drift) {
    this.timeDrift = drift - (drift % 1000); /* Round to ms */
    setTime(timeDrift);
  }

  public long getDrift() {
    return timeDrift;
  }

  public long getTime() {
    return moteTime;
  }

  public void doActionsBeforeTick() {
    /* Update time */
    setTime(mote.getSimulation().getSimulationTime() + timeDrift);
  }

  public void doActionsAfterTick() {

    /* Request next tick for remaining events / timers */
    int processRunValue = moteMem.getIntValueOf("simProcessRunValue");
    if (processRunValue != 0) {
      /* Handle next Contiki event in one millisecond */
      mote.scheduleNextWakeup(simulation.getSimulationTime() + Simulation.MILLISECOND);
      return;
    }

    int etimersPending = moteMem.getIntValueOf("simEtimerPending");
    if (etimersPending == 0) {
      /* No timers */
      return;
    }

    /* Request tick next wakeup time */
    int nextExpirationTime = moteMem.getIntValueOf("simNextExpirationTime");
    if (nextExpirationTime <= 0) {
      /*logger.warn("Event timer already expired, but has been delayed: " + nextExpirationTime);*/
      mote.scheduleNextWakeup(simulation.getSimulationTime() + Simulation.MILLISECOND);
      return;
    }

    mote.scheduleNextWakeup(simulation.getSimulationTime() + Simulation.MILLISECOND*nextExpirationTime);
  }


  public JPanel getInterfaceVisualizer() {
    return null;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
