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
 * $Id: Battery.java,v 1.2 2006/09/26 12:47:06 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.*;

import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * A Battery represents the energy source for a mote. This implementation has no
 * connection with any underlying simulated software, hence a mote does not know
 * the current energy levels.
 * <p>
 * This Battery decreases current energy left each tick depending on the current
 * mote state. If the mote is sleeping all passive interfaces' energy
 * consumptions will be summed up and detracted from the current energy. If the
 * mote is awake both the active and passive interfaces' energy consumptions
 * will be used. Also, the energy used by the CPU (depends on mote state) will
 * be detracted each tick.
 * <p>
 * This observable is changed and notifies observers every time the energy left
 * is changed. When current energy left has decreased below 0 the mote state is
 * set to dead.
 * 
 * @see MoteInterface
 * @see MoteInterface#energyConsumptionPerTick()
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Battery")
public class Battery extends MoteInterface implements PassiveMoteInterface {

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
      energyConsumptionAwakePerTick = ENERGY_CONSUMPTION_AWAKE_mA
          * mote.getSimulation().getTickTimeInSeconds();
      energyConsumptionLPMPerTick = ENERGY_CONSUMPTION_LPM_mA
          * mote.getSimulation().getTickTimeInSeconds();
    }

    this.mote = mote;
    myEnergy = INITIAL_ENERGY;
  }

  public void doActionsBeforeTick() {
    // Nothing to do
  }

  public void doActionsAfterTick() {
    lastEnergyConsumption = 0;
    
    // If infinite energy, do nothing
    if (hasInfiniteEnergy)
      return;

    // If mote is dead, do nothing
    if (mote.getState() == Mote.STATE_DEAD)
      return;

    // Check mote state
    if (mote.getState() == Mote.STATE_LPM) {
      // Mote is sleeping. Sum up energy usage.
      double totalEnergyConsumption = 0.0;
      totalEnergyConsumption += energyConsumptionLPMPerTick;

      for (MoteInterface passiveInterface : mote.getInterfaces()
          .getAllPassiveInterfaces()) {
        totalEnergyConsumption += passiveInterface.energyConsumptionPerTick();
      }

      decreaseEnergy(totalEnergyConsumption);
      lastEnergyConsumption += totalEnergyConsumption;
    } else {
      // Mote is awake. Sum up energy usage.
      double totalEnergyConsumption = 0.0;
      totalEnergyConsumption += energyConsumptionAwakePerTick;

      for (MoteInterface activeInterface : mote.getInterfaces()
          .getAllActiveInterfaces()) {
        totalEnergyConsumption += activeInterface.energyConsumptionPerTick();
      }
      for (MoteInterface passiveInterface : mote.getInterfaces()
          .getAllPassiveInterfaces()) {
        totalEnergyConsumption += passiveInterface.energyConsumptionPerTick();
      }

      decreaseEnergy(totalEnergyConsumption);
      lastEnergyConsumption += totalEnergyConsumption;
    }

    // Check if we are out of energy
    if (getCurrentEnergy() <= 0.0) {
      setChanged();
      notifyObservers();
      mote.setState(Mote.STATE_DEAD);
    }
  }

  /**
   * @param inf
   *          Set infinite energy state
   */
  public void setInfiniteEnergy(boolean inf) {
    hasInfiniteEnergy = inf;

    setChanged();
    notifyObservers();
  }

  /**
   * @return True if this battery has inifinite energy
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

  public void setConfigXML(Collection<Element> configXML) {
    for (Element element : configXML) {
      if (element.getName().equals("infinite")) {
        hasInfiniteEnergy = Boolean.parseBoolean(element.getText());
      }
    }
  }

}
