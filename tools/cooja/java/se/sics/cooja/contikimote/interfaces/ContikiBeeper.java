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
 * $Id: ContikiBeeper.java,v 1.1 2006/08/21 12:13:05 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.Collection;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Beeper;

/**
 * Ths class represents a beeper.
 * 
 * It needs read access to the following core variables:
 * <ul>
 * <li>char simBeeped (1=on, else off)
 * </ul>
 * <p>
 * Dependency core interfaces are:
 * <ul>
 * <li>beep_interface
 * </ul>
 * <p>
 * This observable is changed and notifies observers whenever the beeper changes
 * state.
 * 
 * @author Fredrik Osterlind
 */
public class ContikiBeeper extends Beeper implements ContikiMoteInterface {
  private Mote mote = null;
  private SectionMoteMemory moteMem = null;
  private static Logger logger = Logger.getLogger(ContikiBeeper.class);

  /**
   * Assuming beep always lasts for 0.1 seconds. ESB measured energy
   * consumption: 16.69 mA. Total energy consumption of a beep is then:
   * 0.1*16.69
   */
  private final double ENERGY_CONSUMPTION_BEEP;

  private double myEnergyConsumption = 0.0;

  private boolean justBeeped = false;
  
  /**
   * Creates an interface to the beeper at mote.
   * 
   * @param mote
   *          Beeper's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiBeeper(Mote mote) {
    // Read class configurations of this mote type
    ENERGY_CONSUMPTION_BEEP = mote.getType().getConfig().getDoubleValue(
        ContikiBeeper.class, "BEEP_CONSUMPTION_mQ");

    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();
  }

  public boolean isBeeping() {
    return justBeeped;
  }
  
  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"beep_interface"};
  }

  public void doActionsBeforeTick() {
    // Nothing to do
    justBeeped = false;
  }

  public void doActionsAfterTick() {
    if (moteMem.getByteValueOf("simBeeped") == 1) {
      this.setChanged();
      this.notifyObservers(mote);
      justBeeped = true;
      moteMem.setByteValueOf("simBeeped", (byte) 0);
      myEnergyConsumption = ENERGY_CONSUMPTION_BEEP;
    } else
      myEnergyConsumption = 0.0;
  }

  public JPanel getInterfaceVisualizer() {
    return null;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public double energyConsumptionPerTick() {
    return myEnergyConsumption;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML) {
  }

}
