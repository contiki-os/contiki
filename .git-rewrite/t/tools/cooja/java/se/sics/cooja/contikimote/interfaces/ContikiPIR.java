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
 * $Id: ContikiPIR.java,v 1.3 2007/01/09 10:05:19 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.awt.event.*;
import java.util.Collection;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.PIR;

/**
 * This class represents a passive infrared sensor.
 * 
 * It needs read/write access to the following core variables:
 * <ul>
 * <li>char simPirChanged (1=changed, else not changed)
 * <li>char simPirIsActive (1=active, else inactive)
 * </ul>
 * <p>
 * Dependency core interfaces are:
 * <ul>
 * <li>pir_interface
 * </ul>
 * <p>
 * This observable notifies observers if a change is discovered by the PIR.
 * 
 * @author Fredrik Osterlind
 */
public class ContikiPIR extends PIR implements ContikiMoteInterface {
  private static Logger logger = Logger.getLogger(ContikiPIR.class);

  /**
   * Approximate energy consumption of an active PIR sensor. ESB measured energy
   * consumption is 0.4 mA. TODO Measure energy consumption
   */
  public final double ENERGY_CONSUMPTION_PIR_mA;

  private final boolean RAISES_EXTERNAL_INTERRUPT;

  private double energyActivePerTick = -1;

  private Mote mote;
  private SectionMoteMemory moteMem;
  private double myEnergyConsumption = 0.0;

  /**
   * Creates an interface to the pir at mote.
   * 
   * @param mote
   *          Button's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiPIR(Mote mote) {
    // Read class configurations of this mote type
    ENERGY_CONSUMPTION_PIR_mA = mote.getType().getConfig().getDoubleValue(
        ContikiPIR.class, "ACTIVE_CONSUMPTION_mA");
    RAISES_EXTERNAL_INTERRUPT = mote.getType().getConfig()
        .getBooleanValue(ContikiPIR.class, "EXTERNAL_INTERRUPT_bool");

    this.mote = mote;
    this.moteMem = (SectionMoteMemory) mote.getMemory();

    if (energyActivePerTick < 0)
      energyActivePerTick = ENERGY_CONSUMPTION_PIR_mA
          * mote.getSimulation().getTickTimeInSeconds();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[]{"pir_interface"};
  }

  public void triggerChange() {
    if (moteMem.getByteValueOf("simPirIsActive") == 1) {
      moteMem.setByteValueOf("simPirChanged", (byte) 1);

      // If mote is inactive, wake it up
      if (RAISES_EXTERNAL_INTERRUPT)
        mote.setState(Mote.State.ACTIVE);

      this.setChanged();
      this.notifyObservers();
    }
  }

  public void doActionsBeforeTick() {
    if (moteMem.getByteValueOf("simPirIsActive") == 1) {
      myEnergyConsumption = energyActivePerTick;
    } else
      myEnergyConsumption = 0.0;
  }

  public void doActionsAfterTick() {
    // Nothing to do
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel();
    final JButton clickButton = new JButton("Signal PIR");

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

  public double energyConsumptionPerTick() {
    return myEnergyConsumption;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
