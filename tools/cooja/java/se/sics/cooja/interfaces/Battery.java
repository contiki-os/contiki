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
 * $Id: Battery.java,v 1.8 2009/05/26 14:24:20 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMote;

/**
 * A Battery represents the energy source for a mote. This implementation has no
 * connection with underlying simulated software: a mote does not know about energy.
 * <p>
 * This Battery updates energy after each mote tick:
 * the energy consumption of each interface is summed up.
 * In addition, the Battery adds the CPU energy.
 * <p>
 *
 * This observable notifies every tick (relatively time-consuming).
 *
 * When the energy left is below 0 the mote is dead.
 *
 * @see MoteInterface
 * @see MoteInterface#energyConsumption()
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Battery")
public class Battery extends MoteInterface implements PolledAfterAllTicks {
  private static Logger logger = Logger.getLogger(Battery.class);

  /**
   * Approximate energy consumption of a mote's CPU in active mode (mA). ESB
   * measured energy consumption is 1.49 mA.
   */
  public final double CPU_ENERGY_CONSUMPTION_AWAKE_mA;

  /**
   * Approximate energy consumption of a mote's CPU in low power mode (mA). ESB
   * measured energy consumption is 1.34 mA.
   */
  public final double CPU_ENERGY_CONSUMPTION_LPM_mA;

  /**
   * Initial energy of battery in milli coulomb (mQ). ESB mote: 3 regular AA
   * batteries, each ~1.25 Ah. 3 * 1.25 Ah = 3 * 1.25 * 3600 Q = 13,500 Q =
   * 13,500,000 mQ
   */
  public final double INITIAL_ENERGY;

  private ContikiMote mote = null;

  private double cpuEnergyConsumptionLPMPerMs;
  private double cpuEnergyConsumptionAwakePerMs;

  private boolean hasInfiniteEnergy;

  private double cpuEnergyConsumption = 0;

  private double totalEnergyConsumption = 0;

  /**
   * Creates a new battery connected to given mote.
   *
   * @param mote Mote
   *
   * @see #energyConsumption
   * @see #INITIAL_ENERGY
   */
  public Battery(Mote mote) {
    /* Read configuration */
    CPU_ENERGY_CONSUMPTION_AWAKE_mA = mote.getType().getConfig().getDoubleValue(Battery.class, "CPU_AWAKE_mA");
    CPU_ENERGY_CONSUMPTION_LPM_mA = mote.getType().getConfig().getDoubleValue(Battery.class, "CPU_LPM_mA");
    INITIAL_ENERGY = mote.getType().getConfig().getDoubleValue(Battery.class, "INITIAL_ENERGY_mQ");
    hasInfiniteEnergy = mote.getType().getConfig().getBooleanValue(Battery.class, "INFINITE_ENERGY_bool");

    cpuEnergyConsumptionAwakePerMs = CPU_ENERGY_CONSUMPTION_AWAKE_mA * 0.001; /* TODO Voltage */
    cpuEnergyConsumptionLPMPerMs = CPU_ENERGY_CONSUMPTION_LPM_mA * 0.001; /* TODO Voltage */

    this.mote = (ContikiMote) mote;
  }

  public void doActionsAfterTick() {
    // If infinite energy, do nothing
    if (hasInfiniteEnergy) {
      return;
    }

    // If mote is dead, do nothing
    if (mote.getState() == Mote.State.DEAD) {
      return;
    }

    if (mote.getState() == Mote.State.ACTIVE) {
      cpuEnergyConsumption += cpuEnergyConsumptionLPMPerMs;
    } else {
      cpuEnergyConsumption += cpuEnergyConsumptionAwakePerMs;
    }

    totalEnergyConsumption = cpuEnergyConsumption;
    for (MoteInterface intf : mote.getInterfaces().getInterfaces()) {
      totalEnergyConsumption += intf.energyConsumption();
    }

    /* Check if we are out of energy */
    if (getEnergyConsumption() > INITIAL_ENERGY) {
      mote.scheduleImmediateWakeup();
    }

    setChanged();
    notifyObservers();
  }

  /**
   * @param inf Infinite energy
   */
  public void setInfiniteEnergy(boolean inf) {
    hasInfiniteEnergy = inf;

    setChanged();
    notifyObservers();
  }

  /**
   * @return True if battery has infinite energy
   */
  public boolean hasInfiniteEnergy() {
    return hasInfiniteEnergy;
  }

  /**
   * @return Initial energy
   */
  public double getInitialEnergy() {
    return INITIAL_ENERGY;
  }

  /**
   * @return Current energy consumption
   */
  public double getEnergyConsumption() {
    return totalEnergyConsumption;
  }

  /**
   * @return Energy left ratio
   */
  public double getEnergyLeftRatio() {
    return ((getInitialEnergy() - getEnergyConsumption()) / getInitialEnergy());
  }

  public JPanel getInterfaceVisualizer() {
    // Battery energy left
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    final JLabel initialLabel = new JLabel("");
    final JLabel energyLabel = new JLabel("");
    final JLabel leftLabel = new JLabel("");

    if (hasInfiniteEnergy()) {
      initialLabel.setText("[infinite energy]");
      energyLabel.setText("");
      leftLabel.setText("");
    } else {
      initialLabel.setText("Total energy (mQ): " + getInitialEnergy());
      energyLabel.setText("Consumed energy (mQ): " + getEnergyConsumption());
      leftLabel.setText("Energy left (%): " + (getEnergyLeftRatio() * 100));
    }

    panel.add(initialLabel);
    panel.add(energyLabel);
    panel.add(leftLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (hasInfiniteEnergy()) {
          initialLabel.setText("[infinite energy]");
          energyLabel.setText("");
          leftLabel.setText("");
        } else {
          initialLabel.setText("Total energy (mQ): " + getInitialEnergy());
          energyLabel.setText("Consumed energy (mQ): " + getEnergyConsumption());
          leftLabel.setText("Energy left (%): " + (getEnergyLeftRatio() * 100));
        }
      }
    });

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    return panel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public double energyConsumption() {
    return 0.0;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // Infinite boolean
    element = new Element("infinite");
    element.setText(Boolean.toString(hasInfiniteEnergy));
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      if (element.getName().equals("infinite")) {
        hasInfiniteEnergy = Boolean.parseBoolean(element.getText());
      }
    }
  }

}
