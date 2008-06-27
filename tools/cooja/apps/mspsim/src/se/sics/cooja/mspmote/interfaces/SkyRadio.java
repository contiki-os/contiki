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
 * $Id: SkyRadio.java,v 1.5 2008/06/27 14:11:52 nifi Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.CustomDataRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.mspmote.SkyMote;
import se.sics.mspsim.chip.CC2420;
import se.sics.mspsim.chip.PacketListener;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.OperatingModeListener;

/**
 * CC2420 to COOJA wrapper.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("CC2420")
public class SkyRadio extends Radio implements CustomDataRadio {
  private static Logger logger = Logger.getLogger(SkyRadio.class);

  private int lastEventTime = 0;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private SkyMote myMote;

  private CC2420 cc2420;

  private boolean isInterfered = false;

  private boolean isTransmitting = false;

  private boolean isReceiving = false;
//  private boolean hasFailedReception = false;

  private boolean radioOn = true;

  private CC2420RadioPacket lastOutgoingCC2420Packet = null;

  private RadioPacket lastIncomingCC2420Packet = null;

  private RadioPacket lastOutgoingPacket = null;

  private RadioPacket lastIncomingPacket = null;

//  private int mode;

  //TODO: HW on/off

  public SkyRadio(SkyMote mote) {
    this.myMote = mote;
    this.cc2420 = mote.skyNode.radio;

    cc2420.setPacketListener(new PacketListener() {
      public void transmissionStarted() {
        lastEventTime = myMote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.TRANSMISSION_STARTED;
        /*logger.debug("----- SKY TRANSMISSION STARTED -----");*/
        setChanged();
        notifyObservers();
      }

      public void transmissionEnded(byte[] outgoingCC2420Data) {
        lastOutgoingCC2420Packet = new CC2420RadioPacket(outgoingCC2420Data);

        lastEventTime = myMote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
        /*logger.debug("----- SKY CUSTOM DATA TRANSMITTED -----");*/
        setChanged();
        notifyObservers();

        lastOutgoingPacket = CC2420RadioPacketConverter.fromCC2420ToCooja(lastOutgoingCC2420Packet);

        lastEventTime = myMote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.PACKET_TRANSMITTED;
        /*logger.debug("----- SKY PACKET TRANSMITTED -----");*/
        setChanged();
        notifyObservers();

        lastEventTime = myMote.getSimulation().getSimulationTime();
        /*logger.debug("----- SKY TRANSMISSION FINISHED -----");*/
        lastEvent = RadioEvent.TRANSMISSION_FINISHED;
        setChanged();
        notifyObservers();
      }
    });
//    cc2420.addOperatingModeListener(new OperatingModeListener() {
//
//      public void modeChanged(Chip source, int mode) {
//        SkyRadio.this.mode = mode;        
//      }
//      
//    });
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return lastOutgoingPacket;
  }

  public RadioPacket getLastPacketReceived() {
    return lastIncomingPacket;
  }

  public void setReceivedPacket(RadioPacket packet) {
    lastIncomingPacket = packet;
    lastIncomingCC2420Packet = CC2420RadioPacketConverter.fromCoojaToCC24240(packet);
  }

  /* Custom data radio support */
  public Object getLastCustomDataTransmitted() {
    return lastOutgoingCC2420Packet;
  }

  public Object getLastCustomDataReceived() {
    return lastIncomingCC2420Packet;
  }

  public void receiveCustomData(Object data) {
    if (data instanceof CC2420RadioPacket) {
      lastIncomingCC2420Packet = (CC2420RadioPacket) data;
    }
  }

  /* General radio support */
  public boolean isTransmitting() {
    return isTransmitting;
  }

  public boolean isReceiving() {
    return isReceiving;
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public int getChannel() {
    return cc2420.getActiveChannel();
  }

  public int getFrequency() {
    return cc2420.getActiveFrequency();
  }

  public void signalReceptionStart() {
    cc2420.setCCA(true);
//    hasFailedReception = mode == CC2420.MODE_TXRX_OFF;
    isReceiving = true;
    /* TODO cc2420.setSFD(true); */

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    /*logger.debug("----- SKY RECEPTION STARTED -----");*/
    setChanged();
    notifyObservers();
  }

  public void signalReceptionEnd() {
    /* Deliver packet data */
    if (isReceiving && !isInterfered && lastIncomingCC2420Packet != null /* && !hasFailedReception */) {
      byte[] packetData = lastIncomingCC2420Packet.getPacketData();
      cc2420.setIncomingPacket(packetData);
    }

    isReceiving = false;
//    hasFailedReception = false;
    isInterfered = false;
    cc2420.setCCA(false);

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    /*logger.debug("----- SKY RECEPTION FINISHED -----");*/
    setChanged();
    notifyObservers();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    isInterfered = true;
    isReceiving = false;
//    hasFailedReception = false;
    lastIncomingPacket = null;
    lastIncomingCC2420Packet = null;
    cc2420.setCCA(true);

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_INTERFERED;
    /*logger.debug("----- SKY RECEPTION INTERFERED -----");*/
    setChanged();
    notifyObservers();
  }

  public double getCurrentOutputPower() {
    return cc2420.getOutputPower();
  }

  public int getCurrentOutputPowerIndicator() {
    return cc2420.getOutputPowerIndicator();
  }

  public int getOutputPowerIndicatorMax() {
    return 31;
  }

  public double getCurrentSignalStrength() {
    return cc2420.getRSSI();
  }

  public void setCurrentSignalStrength(double signalStrength) {
    cc2420.setRSSI((int) signalStrength);
  }

  public double energyConsumptionPerTick() {
    return 0;
  }

  public void doActionsBeforeTick() {
  }

  public void doActionsAfterTick() {
  }

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel wrapperPanel = new JPanel(new BorderLayout());
    JPanel panel = new JPanel(new GridLayout(5, 2));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("");
    final JLabel powerLabel = new JLabel("");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update");

    panel.add(new JLabel("STATE:"));
    panel.add(statusLabel);

    panel.add(new JLabel("LAST EVENT:"));
    panel.add(lastEventLabel);

    panel.add(new JLabel("CHANNEL:"));
    panel.add(channelLabel);

    panel.add(new JLabel("OUTPUT POWER:"));
    panel.add(powerLabel);

    panel.add(new JLabel("SIGNAL STRENGTH:"));
    JPanel smallPanel = new JPanel(new GridLayout(1, 2));
    smallPanel.add(ssLabel);
    smallPanel.add(updateButton);
    panel.add(smallPanel);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        channelLabel.setText(getChannel() + " (freq=" + getFrequency() + " MHz)");
        powerLabel.setText(getCurrentOutputPower() + " dBm (indicator=" + getCurrentOutputPowerIndicator() + "/" + getOutputPowerIndicatorMax() + ")");
        ssLabel.setText(getCurrentSignalStrength() + " dBm");
      }
    });

    Observer observer;
    this.addObserver(observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("transmitting");
        } else if (isReceiving()) {
          statusLabel.setText("receiving");
        } else if (radioOn /* mode != CC2420.MODE_TXRX_OFF */) {
          statusLabel.setText("listening for traffic");
        } else {
          statusLabel.setText("HW off");
        }

        lastEventLabel.setText(lastEvent + " @ time=" + lastEventTime);

        channelLabel.setText(getChannel() + " (freq=" + getFrequency() + " MHz)");
        powerLabel.setText(getCurrentOutputPower() + " dBm (indicator=" + getCurrentOutputPowerIndicator() + "/" + getOutputPowerIndicatorMax() + ")");
        ssLabel.setText(getCurrentSignalStrength() + " dBm");
      }
    });

    observer.update(null, null);

    wrapperPanel.add(BorderLayout.NORTH, panel);

    // Saving observer reference for releaseInterfaceVisualizer
    wrapperPanel.putClientProperty("intf_obs", observer);
    return wrapperPanel;
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
    Observer observer = (Observer) panel.getClientProperty("intf_obs");
    if (observer == null) {
      logger.fatal("Error when releasing panel, observer is null");
      return;
    }

    this.deleteObserver(observer);
  }

  public Mote getMote() {
    return myMote;
  }

  public Position getPosition() {
    return myMote.getInterfaces().getPosition();
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }
}
