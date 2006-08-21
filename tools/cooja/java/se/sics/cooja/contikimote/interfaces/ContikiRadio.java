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
 * $Id: ContikiRadio.java,v 1.1 2006/08/21 12:13:05 fros4943 Exp $
 */

package se.sics.cooja.contikimote.interfaces;

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteInterface;
import se.sics.cooja.interfaces.Radio;

/**
 * This class represents a radio transciever.
 * 
 * It needs read/write access to the following core variables:
 * <ul>
 * <li>char simSentPacket (1=mote has new outgoing data, else no new outgoing
 * data)
 * <li>char simReceivedPacket (1=mote has new incoming data, else no new
 * incoming data)
 * <li>char simEtherBusy (1=ether is busy, MAC may try to resend later, else
 * not busy)
 * <li>int simReceivedPacketSize (size of new received data packet)
 * <li>int simSentPacketSize (size of new sent data packet)
 * <li>byte[] simSentPacketData (data of new sent data packet)
 * <li>byte[] simReceivedPacketData (data of new received data packet)
 * <li>char simRadioHWOn (radio hardware status (on/off))
 * </ul>
 * <p>
 * Dependency core interfaces are:
 * <ul>
 * <li>radio_interface
 * </ul>
 * <p>
 * This observable is changed and notifies observers whenever either the send
 * status or listen status is changed. If current listen status is HEARS_PACKET
 * just before a mote tick, the current packet data is transferred to the core.
 * Otherwise no data will be transferred. If core has sent a packet, current
 * sent status will be set to SENT_SOMETHING when returning from the mote tick
 * that sent the packet. This status will be reset to SENT_NOTHING just before
 * next tick.
 * 
 * @author Fredrik Osterlind
 */
public class ContikiRadio extends Radio implements ContikiMoteInterface {
  private Mote myMote;
  private SectionMoteMemory myMoteMemory;
  private static Logger logger = Logger.getLogger(ContikiRadio.class);

  /**
   * Approximate energy consumption of an active radio. ESB measured energy
   * consumption is 5 mA. TODO Measure energy consumption
   */
  public final double ENERGY_CONSUMPTION_RADIO_mA;

  private final boolean RAISES_EXTERNAL_INTERRUPT;

  private double energyActiveRadioPerTick = -1;
  
  private int mySendState = SENT_NOTHING;
  private int myListenState = HEARS_NOTHING;

  private byte[] packetToMote = null;
  private byte[] packetFromMote = null;

  private boolean radioOn = true;
  
  private double myEnergyConsumption=0.0;

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
    ENERGY_CONSUMPTION_RADIO_mA = mote.getType().getConfig().getDoubleValue(ContikiRadio.class, "ACTIVE_CONSUMPTION_mA");
    RAISES_EXTERNAL_INTERRUPT = mote.getType().getConfig().getBooleanValue(ContikiRadio.class, "EXTERNAL_INTERRUPT_bool");

    this.myMote = mote;
    this.myMoteMemory = (SectionMoteMemory) mote.getMemory();
    
    if (energyActiveRadioPerTick < 0)
      energyActiveRadioPerTick = ENERGY_CONSUMPTION_RADIO_mA * mote.getSimulation().getTickTimeInSeconds();
  }

  public static String[] getCoreInterfaceDependencies() {
    return new String[] { "radio_interface" };
  }

  public byte[] getLastPacketSent() {
    return packetFromMote;
  }

  public byte[] getLastPacketReceived() {
    return packetToMote;
  }
  
  public void receivePacket(byte[] data) {
    packetToMote = data;
  }

  public int getSendState() {
    return mySendState;
  }

  public int getListenState() {
    return myListenState;
  }

  public void setListenState(int newStatus) {
    if (newStatus != myListenState) {
      myListenState = newStatus;
      this.setChanged();
      this.notifyObservers(myMote.getInterfaces().getPosition());
    }

    // If mote is inactive, wake it up
    if (RAISES_EXTERNAL_INTERRUPT)
      myMote.setState(Mote.STATE_ACTIVE);
  }

  public void advanceListenState() {
    if (myListenState == HEARS_NOTHING) {
      setListenState(HEARS_PACKET);
    } else
      setListenState(HEARS_NOISE);
  }

  public void doActionsBeforeTick() {
    // If radio hardware is turned off, we don't need to do anything..
    if (!radioOn) {
      myEnergyConsumption = 0.0;
      return;
    }
    myEnergyConsumption = energyActiveRadioPerTick;

    // Set ether status
    if (getListenState() == HEARS_PACKET ||
        getListenState() == HEARS_NOISE ||
        getSendState() == SENT_SOMETHING) {
      myMoteMemory.setByteValueOf("simEtherBusy", (byte) 1);
    } else {
      myMoteMemory.setByteValueOf("simEtherBusy", (byte) 0);
    }

    if (getListenState() == HEARS_NOTHING) {
      // Haven't heard anything, nothing to do
    } else if (getListenState() == HEARS_PACKET) {
      // Heard only one packet, transfer to mote ok
      myMoteMemory.setByteValueOf("simReceivedPacket", (byte) 1);
      myMoteMemory.setIntValueOf("simReceivedPacketSize", packetToMote.length);
      myMoteMemory.setByteArray("simReceivedPacketData", packetToMote);
    } else if (getListenState() == HEARS_NOISE) {
      // Heard several packets or noise, transfer failed
    }
    
    // Reset send flag
    setSendStatus(SENT_NOTHING);
  }

  public void doActionsAfterTick() {
    // Check new radio hardware status
    if (myMoteMemory.getByteValueOf("simRadioHWOn") == 1) {
      radioOn = true;
    } else {
      radioOn = false;
      return;
    }

    // Reset listen flag
    setListenState(HEARS_NOTHING);

    if (fetchPacketFromCore()) {
      setSendStatus(SENT_SOMETHING);
    }
  }

  private void setSendStatus(int newStatus) {
    if (newStatus != mySendState) {
      mySendState = newStatus;
      this.setChanged();
      this.notifyObservers(myMote.getInterfaces().getPosition());
    }
  }

  private boolean fetchPacketFromCore() {
    if (myMoteMemory.getByteValueOf("simSentPacket") == 1) {
      // TODO Increase energy consumption, we are sending a packet...
      
      myMoteMemory.setByteValueOf("simSentPacket", (byte) 0);

      int size = myMoteMemory.getIntValueOf("simSentPacketSize");

      packetFromMote = myMoteMemory.getByteArray("simSentPacketData", size);

      myMoteMemory.setIntValueOf("simSentPacketSize", 0);

      return true;
    }
    return false;
  }
  
  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
    
    final JLabel listenLabel = new JLabel();
    final JLabel sendLabel = new JLabel();
    
    if (getListenState() == HEARS_NOISE)
      listenLabel.setText("Current listen status: hears noise");
    else if (getListenState() == HEARS_NOTHING)
      listenLabel.setText("Current listen status: hears nothing");
    else if (getListenState() == HEARS_PACKET)
      listenLabel.setText("Current listen status: hears a packet");
    
    if (getSendState() == SENT_NOTHING)
      sendLabel.setText("Current sending status: sent nothing");
    else if (getSendState() == SENT_SOMETHING)
      sendLabel.setText("Current sending status: sent a packet");
    
    panel.add(listenLabel);
    panel.add(sendLabel);
    
    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (getListenState() == HEARS_NOISE)
          listenLabel.setText("Current listen status: hears noise");
        else if (getListenState() == HEARS_NOTHING)
          listenLabel.setText("Current listen status: hears nothing");
        else if (getListenState() == HEARS_PACKET)
          listenLabel.setText("Current listen status: hears a packet");
        
        if (getSendState() == SENT_NOTHING)
          sendLabel.setText("Current sending status: sent nothing");
        else if (getSendState() == SENT_SOMETHING)
          sendLabel.setText("Current sending status: sent a packet");
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
    return myEnergyConsumption;
  }

  public Collection<Element> getConfigXML() {
    return null;
  }
  
  public void setConfigXML(Collection<Element> configXML) {
  }

}
