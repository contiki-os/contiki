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
 * $Id: ContikiVib.java,v 1.8 2009/10/27 10:11:17 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.event.*;
import java.util.Collection;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMote;
import se.sics.cooja.contikimote.ContikiMoteInterface;

/**
 * Vibration sensor mote interface.
 *
 * Contiki variables:
 * <ul>
 * <li>char simVibChanged (1=changed, else not changed)
 * <li>char simVibIsActive (1=active, else inactive)
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>vib_interface
 * </ul>
 * <p>
 *
 * This observable never notifies.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Vibration sensor")
public class ContikiVib extends MoteInterface implements ContikiMoteInterface {
  private static Logger logger = Logger.getLogger(ContikiVib.class);

  /**
   * Approximate energy consumption of an active vibration sensor. ESB measured
   * energy consumption is 1.58 mA.
   */
  public final double ENERGY_CONSUMPTION_VIB_mA;

  private double energyActiveVibPerTick = -1;

  private ContikiMote mote;
  private SectionMoteMemory moteMem;
  private double myEnergyConsumption = 0.0;

  /**
   * Creates an interface to the vibration sensor at mote.
   *
   * @param mote
   *          Vib's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiVib(Mote mote) {
    // Read class configurations of this mote type
    ENERGY_CONSUMPTION_VIB_mA = mote.getType().getConfig().getDoubleValue(
        ContikiVib.class, "ACTIVE_CONSUMPTION_mA");

    this.mote = (ContikiMote) mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();

    if (energyActiveVibPerTick < 0) {
      energyActiveVibPerTick = ENERGY_CONSUMPTION_VIB_mA * 0.001;
    }
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"vib_interface"};
  }

  /**
   * Simulates a change in the vibration sensor.
   */
  public void triggerChange() {
    mote.getSimulation().scheduleEvent(vibrateEvent, mote.getSimulation().getSimulationTime());
  }
  
  private TimeEvent vibrateEvent = new MoteTimeEvent(mote, 0) {
    public void execute(long t) {
      doTriggerChange();
    }
  };
  
  public void doTriggerChange() { 
    if (moteMem.getByteValueOf("simVibIsActive") == 1) {
      moteMem.setByteValueOf("simVibChanged", (byte) 1);

      mote.requestImmediateWakeup();
    }
  }

  /* TODO Energy consumption */

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JButton clickButton = new JButton("Vibrate!");

    panel.add(clickButton);

    clickButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        triggerChange();
      }
    });

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public double energyConsumption() {
    return myEnergyConsumption;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
