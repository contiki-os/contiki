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
 * $Id: Battery.java,v 1.5 2008/10/28 12:30:48 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * A Battery represents the energy source for a mote. This implementation has no
 * connection with underlying simulated software: a mote does not know about energy.
 * <p>
 * This Battery updates energy after each mote tick:
 * the energy consumption of each interface is summed up.
 * In addtion, the energy used by the CPU (depends on mote state) is
 * detracted each tick.
 * <p>
 *
 * This observable notifies every tick (!).
 *
 * When energy left is below 0 the mote is dead.
 *
 * @see MoteInterface
 * @see MoteInterface#energyConsumptionPerTick()
 *
 * @author Fredrik Österlind
 */
@ClassDescription("Battery")
public class Battery extends MoteInterface implements PolledAfterAllTicks {

  /**
   * Approximate energy consumption of a mote's CPU in active mode (mA). ESB
   * measured energy consumption is 1.49 mA.
   */
  public final double ENERGY_CONSUMPTION_AWAKE_mA;

  /**
   * Approximate energy consumption of a mote's CPU in low power mode (mA). ESB
   * measured energy consumption is 1.34 mA.
   */
  public final double ENERGY_CONSUMPTION_LPM_mA;

  /**
   * Initial energy of battery in milli coulomb (mQ). ESB mote: 3 regular AA
   * batteries, each ~1.25 Ah. 3 * 1.25 Ah = 3 * 1.25 * 3600 Q = 13,500 Q =
   * 13,500,000 mQ
   */
  public final double INITIAL_ENERGY;

  private double energyConsumptionLPMPerTick = -1.0;
  private double energyConsumptionAwakePerTick = -1.0;

  private Mote mote = null;
  private static Logger logger = Logger.getLogger(Battery.class);

  private double myEnergy;
  private boolean hasInfiniteEnergy;
  private double lastEnergyConsumption = 0;

  /**
   * Creates a new battery connected to given mote.
   *
   * @see #INITIAL_ENERGY
   * @param mote
   *          Mote holding battery
   */
  public Battery(Mote mote) {
    // Read class configurations of this mote type
    ENERGY_CONSUMPTION_AWAKE_mA = mote.getType().getConfig()
        .getDoubleValue(Battery.class, "CPU_AWAKE_mA");
    ENERGY_CONSUMPTION_LPM_mA = mote.getType().getConfig().getDoubleValue(
        Battery.class, "CPU_LPM_mA");
    INITIAL_ENERGY = mote.getType().getConfig().getDoubleValue(
        Battery.class, "INITIAL_ENERGY_mQ");
    hasInfiniteEnergy = mote.getType().getConfig().getBooleanValue(
        Battery.class, "INFINITE_ENERGY_bool");

    if (energyConsumptionAwakePerTick < 0) {
      energyConsumptionAwakePerTick = ENERGY_CONSUMPTION_AWAKE_mA * 0.001;
      energyConsumptionLPMPerTick = ENERGY_CONSUMPTION_LPM_mA * 0.001;
    }

    this.mote = mote;
    myEnergy = INITIAL_ENERGY;
  }

  public void doActionsAfterTick() {
    lastEnergyConsumption = 0;

    // If infinite energy, do nothing
    if (hasInfiniteEnergy) {
      return;
    }

    // If mote is dead, do nothing
    if (mote.getState() == Mote.State.DEAD) {
      return;
    }

    double totalEnergyConsumption = 0.0;

    totalEnergyConsumption += energyConsumptionLPMPerTick;
    for (MoteInterface intf : mote.getInterfaces().getInterfaces()) {
      totalEnergyConsumption += intf.energyConsumptionPerTick();
    }

    decreaseEnergy(totalEnergyConsumption);
    lastEnergyConsumption += totalEnergyConsumption;

    // Check if we are out of energy
    if (getCurrentEnergy() <= 0.0) {
      setChanged();
      notifyObservers();
      mote.setState(Mote.State.DEAD);
    }
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
   * @return Current energy left
   */
  public double getCurrentEnergy() {
    return myEnergy;
  }

  private void decreaseEnergy(double consumption) {
    if (!hasInfiniteEnergy) {
      myEnergy -= consumption;
      setChanged();
      notifyObservers();
    }
  }

  public JPanel getInterfaceVisualizer() {
    // Battery energy left
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    final JLabel energyLabel = new JLabel("");
    final JLabel energyPercentLabel = new JLabel("");

    if (hasInfiniteEnergy()) {
      energyLabel.setText("INFINITE");
      energyPercentLabel.setText("");
    } else {
      energyLabel.setText("Energy left (mQ) = " + getCurrentEnergy());
      energyPercentLabel.setText("Energy left (%) = "
          + (getCurrentEnergy() / getInitialEnergy() * 100) + "%");
    }

    panel.add(energyLabel);
    panel.add(energyPercentLabel);

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (hasInfiniteEnergy()) {
          energyLabel.setText("INFINITE");
          energyPercentLabel.setText("");
        } else {
          energyLabel.setText("Energy left (mQ) = " + getCurrentEnergy());
          energyPercentLabel.setText("Energy left (%) = "
              + (getCurrentEnergy() / getInitialEnergy() * 100) + "%");
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

  public double energyConsumptionPerTick() {
    // The battery itself does not require any power.
    return 0.0;
  }

  public double getLastTotalEnergyConsumption() {
    return lastEnergyConsumption;
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
