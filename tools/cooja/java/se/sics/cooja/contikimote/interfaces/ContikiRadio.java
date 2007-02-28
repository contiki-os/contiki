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
 * $Id: ContikiRadio.java,v 1.11 2007/02/28 09:48:48 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.*;
import javax.swing.*;
import java.awt.event.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.PacketRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;

/**
 * This class represents a radio transciever. In order to simulate different
 * transmission rates, the underlying Contiki system can be locked in either
 * transmission or reception states (using multi-threading). When a transmission
 * is initiated, it will automatically lock the Contiki system. When a packet is
 * received by this radio the Contiki system, the entitiy transferring the
 * packet may explicitly lock the radio in receiving mode. After some time it
 * should then deliver the packet.
 * 
 * It needs read/write access to the following core variables:
 * <ul>
 * <li>char simTransmitting (1=mote radio is transmitting)
 * <li>char simReceiving (1=mote radio is receiving)
 * <p>
 * <li>int simInSize (size of received data packet)
 * <li>byte[] simInDataBuffer (data of received data packet)
 * <p>
 * <li>int simOutSize (size of transmitted data packet)
 * <li>byte[] simOutDataBuffer (data of transmitted data packet)
 * <p>
 * <li>char simRadioHWOn (radio hardware status (on/off))
 * <li>int simSignalStrength (heard radio signal strength)
 * <li>char simPower (number indicating power output)
 * <li>int simRadioChannel (number indicating current channel)
 * </ul>
 * <p>
 * Dependency core interfaces are:
 * <ul>
 * <li>radio_interface
 * </ul>
 * <p>
 * 
 * @author Fredrik Osterlind
 */
public class ContikiRadio extends Radio implements ContikiMoteInterface,
    PacketRadio {
  private Mote myMote;

  private SectionMoteMemory myMoteMemory;

  private static Logger logger = Logger.getLogger(ContikiRadio.class);

  /**
   * Approximate energy consumption of an active radio. ESB measured energy
   * consumption is 5 mA. TODO Measure energy consumption
   */
  public final double ENERGY_CONSUMPTION_RADIO_mA;

  private final boolean RAISES_EXTERNAL_INTERRUPT;

  private double energyListeningRadioPerTick = -1;

  private byte[] packetToMote = null;

  private byte[] packetFromMote = null;

  private boolean radioOn = true;

  private double myEnergyConsumption = 0.0;

  private boolean isTransmitting = false;

  private boolean isInterfered = false;

  private int transmissionEndTime = -1;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private int lastEventTime = 0;

  private int oldOutputPowerIndicator = -1;

  /**
   * Creates an interface to the radio at mote.
   * 
   * @param mote
   *          Radio's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public ContikiRadio(Mote mote) {
    // Read class configurations of this mote type
    ENERGY_CONSUMPTION_RADIO_mA = mote.getType().getConfig().getDoubleValue(
        ContikiRadio.class, "ACTIVE_CONSUMPTION_mA");
    RAISES_EXTERNAL_INTERRUPT = mote.getType().getConfig().getBooleanValue(
        ContikiRadio.class, "EXTERNAL_INTERRUPT_bool");

    this.myMote = mote;
    this.myMoteMemory = (SectionMoteMemory) mote.getMemory();

    // Calculate energy consumption of a listening radio
    if (energyListeningRadioPerTick < 0)
      energyListeningRadioPerTick = ENERGY_CONSUMPTION_RADIO_mA
          * mote.getSimulation().getTickTimeInSeconds();

    radioOn = myMoteMemory.getByteValueOf("simRadioHWOn") == 1;
  }

  /* Contiki mote interface support */
  public static String[] getCoreInterfaceDependencies() {
    return new String[] { "radio_interface" };
  }

  /* Packet radio support */
  public byte[] getLastPacketTransmitted() {
    return packetFromMote;
  }

  public byte[] getLastPacketReceived() {
    return packetToMote;
  }

  public void setReceivedPacket(byte[] data) {
    packetToMote = data;
  }

  /* General radio support */
  public boolean isTransmitting() {
    return isTransmitting;
  }

  public boolean isReceiving() {
    if (isLockedAtReceiving())
      return true;

    return myMoteMemory.getIntValueOf("simInSize") != 0;
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public int getChannel() {
    return myMoteMemory.getIntValueOf("simRadioChannel");
  }

  public void signalReceptionStart() {
    packetToMote = null;
    if (isInterfered() || isReceiving() || isTransmitting()) {
      interfereAnyReception();
      return;
    }
    lockInReceivingMode();

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    if (isInterfered() || packetToMote == null) {
      // Reset interfered flag
      isInterfered = false;

      // Reset data
      packetToMote = null;
      myMoteMemory.setIntValueOf("simInSize", 0);

      // Unlock (if locked)
      myMoteMemory.setByteValueOf("simReceiving", (byte) 0);

      return;
    }

    // Unlock (if locked)
    myMoteMemory.setByteValueOf("simReceiving", (byte) 0);

    // Set data
    myMoteMemory.setIntValueOf("simInSize", packetToMote.length);
    myMoteMemory.setByteArray("simInDataBuffer", packetToMote);

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    this.setChanged();
    this.notifyObservers();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    if (!isInterfered()) {
      isInterfered = true;

      lastEvent = RadioEvent.RECEPTION_INTERFERED;
      lastEventTime = myMote.getSimulation().getSimulationTime();
      this.setChanged();
      this.notifyObservers();
    }
  }

  public double getCurrentOutputPower() {
    // TODO Implement method
    logger.warn("Not implemeted, always returning 1.5 dBm");
    return 1.5;
  }

  public int getCurrentOutputPowerIndicator() {
    return (int) myMoteMemory.getByteValueOf("simPower");
  }

  public double getCurrentSignalStrength() {
    return myMoteMemory.getIntValueOf("simSignalStrength");
  }

  public void setCurrentSignalStrength(double signalStrength) {
    myMoteMemory.setIntValueOf("simSignalStrength", (int) signalStrength);
  }

  public Position getPosition() {
    return myMote.getInterfaces().getPosition();
  }

  /**
   * @return True if locked at transmitting
   */
  private boolean isLockedAtTransmitting() {
    return myMoteMemory.getByteValueOf("simTransmitting") == 1;
  }

  /**
   * @return True if locked at receiving
   */
  private boolean isLockedAtReceiving() {
    return myMoteMemory.getByteValueOf("simReceiving") == 1;
  }

  /**
   * Locks underlying Contiki system in receiving mode. This may, but does not
   * have to, be used during a simulated data transfer that takes longer than
   * one tick to complete. The system is unlocked by delivering the received
   * data to the mote.
   */
  private void lockInReceivingMode() {
    // If mote is inactive, try to wake it up
    if (myMote.getState() != Mote.State.ACTIVE) {
      if (RAISES_EXTERNAL_INTERRUPT)
        myMote.setState(Mote.State.ACTIVE);
      if (myMote.getState() != Mote.State.ACTIVE)
        return;
    }

    // Lock core radio in receiving loop
    myMoteMemory.setByteValueOf("simReceiving", (byte) 1);
  }

  public void doActionsBeforeTick() {
  }

  public void doActionsAfterTick() {
    // Check if radio hardware status changed
    if (radioOn != (myMoteMemory.getByteValueOf("simRadioHWOn") == 1)) {
      // Radio changed
      radioOn = !radioOn;

      if (!radioOn) {
        // Reset status
        myMoteMemory.setByteValueOf("simReceiving", (byte) 0);
        myMoteMemory.setIntValueOf("simInSize", 0);
        myMoteMemory.setByteValueOf("simTransmitting", (byte) 0);
        myMoteMemory.setIntValueOf("simOutSize", 0);
        isTransmitting = false;
        lastEvent = RadioEvent.HW_OFF;
      } else
        lastEvent = RadioEvent.HW_ON;

      lastEventTime = myMote.getSimulation().getSimulationTime();
      this.setChanged();
      this.notifyObservers();
    }
    if (!radioOn) {
      myEnergyConsumption = 0.0;
      return;
    }
    myEnergyConsumption = energyListeningRadioPerTick;

    // Check if radio output power changed
    if (myMoteMemory.getByteValueOf("simPower") != oldOutputPowerIndicator) {
      oldOutputPowerIndicator = myMoteMemory.getByteValueOf("simPower");
      lastEvent = RadioEvent.UNKNOWN;
      this.setChanged();
      this.notifyObservers();
    }

    // Are we transmitting but should stop?
    if (isTransmitting
        && myMote.getSimulation().getSimulationTime() >= transmissionEndTime) {
      myMoteMemory.setByteValueOf("simTransmitting", (byte) 0);
      myMoteMemory.setIntValueOf("simOutSize", 0);
      isTransmitting = false;

      lastEventTime = myMote.getSimulation().getSimulationTime();
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      // TODO Energy consumption of transmitted packet?
      this.setChanged();
      this.notifyObservers();
    }

    // Check if a new transmission should be started
    if (!isTransmitting && myMoteMemory.getByteValueOf("simTransmitting") == 1) {
      isTransmitting = true;
      int size = myMoteMemory.getIntValueOf("simOutSize");
      packetFromMote = myMoteMemory.getByteArray("simOutDataBuffer", size);

      // Assuming sending at 19.2 kbps, with manchester-encoding (x2) and 1
      // bit/byte UART overhead (x9 instead of x8)
      int duration = (int) ((2 * size * 9) / 19.2); // ms
      transmissionEndTime = myMote.getSimulation().getSimulationTime()
          + Math.max(1, duration);

      lastEventTime = myMote.getSimulation().getSimulationTime();

      lastEvent = RadioEvent.TRANSMISSION_STARTED;
      this.setChanged();
      this.notifyObservers();

      // Deliver packet right away
      lastEvent = RadioEvent.PACKET_TRANSMITTED;
      this.setChanged();
      this.notifyObservers();
    }
  }

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update SS");

    panel.add(statusLabel);
    panel.add(lastEventLabel);
    panel.add(ssLabel);
    panel.add(updateButton);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ssLabel.setText("Signal strength (not auto-updated): "
            + getCurrentSignalStrength() + " dBm");
      }
    });

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting())
          statusLabel.setText("Transmitting packet now!");
        else if (isReceiving())
          statusLabel.setText("Receiving packet now!");
        else if (radioOn)
          statusLabel.setText("Listening...");
        else
          statusLabel.setText("HW turned off");

        lastEventLabel.setText("Last event (time=" + lastEventTime + "): "
            + lastEvent);
        ssLabel.setText("Signal strength (not auto-updated): "
            + getCurrentSignalStrength() + " dBm");
      }
    });

    observer.update(null, null);

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
    return myEnergyConsumption;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
