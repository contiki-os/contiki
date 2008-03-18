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
 * $Id: DisturberRadio.java,v 1.6 2008/03/18 12:54:39 fros4943 Exp $
 */

package se.sics.cooja.motes;

import java.text.NumberFormat;
import java.util.*;
import javax.swing.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

/**
 * This radio transmits data packet over and over again on a configurable channel.
 *
 * @author Fredrik Osterlind, Thiemo Voigt
 */
public class DisturberRadio extends Radio {
  private Mote myMote;

  private static Logger logger = Logger.getLogger(DisturberRadio.class);

  private RadioPacket packetFromMote = new COOJARadioPacket(new byte[] { 1, 2, 3, 4, 5 });

  private boolean transmitting = false;

  private int distChannel = -1; // channel mote is disturbing

  private int transEndTime = 0;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private int lastEventTime = 0;

  public static int TRANSMISSION_INTERVAL = 100;
  public static int TRANSMISSION_DURATION = 98;

  /**
   * Creates an interface to the radio at mote.
   *
   * @param mote
   *          Radio's mote.
   * @see Mote
   * @see se.sics.cooja.MoteInterfaceHandler
   */
  public DisturberRadio(Mote mote) {
    this.myMote = mote;
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return packetFromMote;
  }

  public RadioPacket getLastPacketReceived() {
    return null;
  }

  public void setReceivedPacket(RadioPacket packet) {
  }


  /* General radio support */
  public void signalReceptionStart() {
  }

  public void signalReceptionEnd() {
  }

  public boolean isTransmitting() {
    return transmitting;
  }

  public int getTransmissionEndTime() {
    return transEndTime;
  }

  public boolean isReceiving() {
    return false;
  }

  public int getChannel() {
    return distChannel;
  }

  public Position getPosition() {
    return myMote.getInterfaces().getPosition();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
  }

  public boolean isInterfered() {
    return false;
  }

  public double getCurrentOutputPower() {
    // TODO Implement method
    logger.warn("Not implemeted, always returning 1.5 dBm");
    return 1.5;
  }

  public int getOutputPowerIndicatorMax() {
    return 100;
  }

  public int getCurrentOutputPowerIndicator() {
    return 100;
  }

  public double getCurrentSignalStrength() {
    return 2.0;
  }

  public void setCurrentSignalStrength(double signalStrength) {
  }

  public void doActionsBeforeTick() {
    int currentTime = myMote.getSimulation().getSimulationTime();

    if (!transmitting && currentTime % TRANSMISSION_INTERVAL == 0) {
      transmitting = true;
      lastEvent = RadioEvent.TRANSMISSION_STARTED;
      lastEventTime = currentTime;
      transEndTime = currentTime + TRANSMISSION_DURATION;
      this.setChanged();
      this.notifyObservers();
    } else if (transmitting && currentTime >= transEndTime) {
      transmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      lastEventTime = currentTime;
      this.setChanged();
      this.notifyObservers();
    }
  }

  public void doActionsAfterTick() {
  }

  public JPanel getInterfaceVisualizer() {
    // Location
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("");
    final JFormattedTextField channelPicker = new JFormattedTextField(NumberFormat.getIntegerInstance());

    panel.add(statusLabel);
    panel.add(lastEventLabel);
    panel.add(Box.createVerticalStrut(3));
    panel.add(channelLabel);
    panel.add(channelPicker);
    panel.add(Box.createVerticalGlue());

    channelPicker.setValue(distChannel);
    channelPicker.setColumns(3);
    channelPicker.setText(Integer.toString(distChannel));

    final Observer observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("Transmitting now!");
        } else {
          statusLabel.setText("Disturber resting...");
        }

        channelLabel.setText("Channel: " + getChannel());

        lastEventLabel.setText("Last event (time=" + lastEventTime + "): " + lastEvent);
      }
    };
    this.addObserver(observer);

    channelPicker.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent e) {
        distChannel = ((Number) channelPicker.getValue()).intValue();
        if (observer != null) {
          observer.update(null, null);
        }
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
    return 0;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();
    Element element;

    // We need to save the mote type identifier
    element = new Element("channel");
    element.setText(Integer.toString(distChannel));
    config.add(element);

    return config;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("channel")) {
        distChannel = Integer.parseInt(element.getText());
      } else {
        logger.fatal("Read unknown configuration: " + name);
      }
    }
  }

  public Mote getMote() {
    return myMote;
  }
}
