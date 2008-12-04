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
 *
 * $Id: ContikiClock.java,v 1.7 2008/12/04 14:03:42 joxe Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.Collection;
import javax.swing.JPanel;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Clock;
import se.sics.cooja.interfaces.PolledBeforeActiveTicks;

/**
 * Clock mote interface. Controls Contiki time.
 *
 * Contiki variables:
 * <ul>
 * <li>int simCurrentTime
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
 * @author Fredrik Österlind
 */
public class ContikiClock extends Clock implements ContikiMoteInterface, PolledBeforeActiveTicks {

  private Mote mote = null;
  private SectionMoteMemory moteMem = null;

  private int timeDrift = 0;

  /**
   * @param mote Mote
   *
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiClock(Mote mote) {
    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"clock_interface"};
  }

  public void setTime(long newTime) {
    /* TODO: check if this is correct even if newTime > MAX_INT */
    moteMem.setIntValueOf("simCurrentTime", (int)newTime);
  }

  public void setDrift(int timeDrift) {
    this.timeDrift = timeDrift;
  }

  public int getDrift() {
    return timeDrift;
  }

  public long getTime() {
    return moteMem.getIntValueOf("simCurrentTime");
  }

  public void doActionsBeforeTick() {
    /* Update time */
    long moteTime = mote.getSimulation().getSimulationTime() + timeDrift;

    if (moteTime > 0) {
      setTime(moteTime);
    }
  }

  public JPanel getInterfaceVisualizer() {
    return null;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public double energyConsumption() {
    return 0.0;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
