/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: ApplicationRadio.java,v 1.14 2010/08/31 07:35:22 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Observable;
import java.util.Observer;

import javax.swing.Box;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.Mote;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;

/**
 * Application radio.
 * 
 * May be used by Java-based mote to implement radio functionality.
 * Supports radio channels and output power functionality.
 * The mote itself should observe the radio for incoming radio packet data.
 *
 * @author Fredrik Osterlind
 */
public class ApplicationRadio extends Radio implements NoiseSourceRadio, DirectionalAntennaRadio {
  private static Logger logger = Logger.getLogger(ApplicationRadio.class);

  private Simulation simulation;
  private Mote mote;

  private RadioPacket packetFromMote = null;
  private RadioPacket packetToMote = null;

  private boolean isTransmitting = false;
  private boolean isReceiving = false;
  private boolean isInterfered = false;

  private long transmissionEndTime = 0;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;
  private long lastEventTime = 0;

  private double signalStrength = -100;
  private int radioChannel = -1;
  private double outputPower = 0; /* typical cc2420 values: -25 <-> 0 dBm */
  private int outputPowerIndicator = 100;

  private int interfered;

  public ApplicationRadio(Mote mote) {
    this.mote = mote;
    this.simulation = mote.getSimulation();
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return packetFromMote;
  }

  public RadioPacket getLastPacketReceived() {
    return packetToMote;
  }

  public void setReceivedPacket(RadioPacket packet) {
    packetToMote = packet;
  }

  /* General radio support */
  public void signalReceptionStart() {
    packetToMote = null;
    if (isInterfered() || isReceiving() || isTransmitting()) {
      interfereAnyReception();
      return;
    }

    isReceiving = true;
    lastEventTime = simulation.getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    //System.out.println("SignalReceptionEnded for node: " + mote.getID() + " intf:" + interfered);
    if (isInterfered() || packetToMote == null) {
      interfered--;
      if (interfered == 0) isInterfered = false;
      if (interfered < 0) {
        isInterfered = false;
        //logger.warn("Interfered got lower than 0!!!");
        interfered = 0;
      }
      packetToMote = null;
      if (interfered > 0) return;
    }

    isReceiving = false;
    lastEventTime = simulation.getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    this.setChanged();
    this.notifyObservers();
  }

  public boolean isTransmitting() {
    return isTransmitting;
  }

  public long getTransmissionEndTime() {
    return transmissionEndTime;
  }

  public boolean isReceiving() {
    return isReceiving;
  }

  public int getChannel() {
    return radioChannel;
  }

  public Position getPosition() {
    return mote.getInterfaces().getPosition();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  /* Note: this must be called exactly as many times as the reception ended */
  public void interfereAnyReception() {
    interfered++;
    if (!isInterfered()) {
      isInterfered = true;

      lastEvent = RadioEvent.RECEPTION_INTERFERED;
      lastEventTime = simulation.getSimulationTime();
      this.setChanged();
      this.notifyObservers();
    }
  }

  public boolean isInterfered() {
    return isInterfered;
  }

  public double getCurrentOutputPower() {
    return outputPower;
  }

  public int getOutputPowerIndicatorMax() {
    return outputPowerIndicator;
  }

  public int getCurrentOutputPowerIndicator() {
    return outputPowerIndicator;
  }

  public double getCurrentSignalStrength() {
    return signalStrength;
  }

  public void setCurrentSignalStrength(double signalStrength) {
    this.signalStrength = signalStrength;
  }

  /* Application radio support */

  /**
   * Start transmitting given packet.
   *
   * @param packet Packet data
   * @param duration Duration to transmit
   */
  public void startTransmittingPacket(final RadioPacket packet, final long duration) {
    Runnable startTransmission = new Runnable() {
      public void run() {
        if (isTransmitting) {
          logger.warn("Already transmitting, aborting new transmission");
          return;
        }

        /* Start transmission */
        isTransmitting = true;
        lastEvent = RadioEvent.TRANSMISSION_STARTED;
        lastEventTime = simulation.getSimulationTime();
        ApplicationRadio.this.setChanged();
        ApplicationRadio.this.notifyObservers();

        /* Deliver data */
        packetFromMote = packet;
        lastEvent = RadioEvent.PACKET_TRANSMITTED;
        ApplicationRadio.this.setChanged();
        ApplicationRadio.this.notifyObservers();

        /*logger.info("Transmission started");*/

        /* Finish transmission */
        simulation.scheduleEvent(new MoteTimeEvent(mote, 0) {
          public void execute(long t) {
            isTransmitting = false;
            lastEvent = RadioEvent.TRANSMISSION_FINISHED;
            lastEventTime = t;
            ApplicationRadio.this.setChanged();
            ApplicationRadio.this.notifyObservers();
            /*logger.info("Transmission finished");*/
          }
        }, simulation.getSimulationTime() + duration);
      }
    };

    if (simulation.isSimulationThread()) {
      startTransmission.run();
    } else {
      simulation.invokeSimulationThread(startTransmission);
    }
  }

  /**
   * @param i New output power indicator
   */
  public void setOutputPowerIndicator(int i) {
    outputPowerIndicator = i;
  }

  /**
   * @param p New output power
   */
  public void setOutputPower(double p) {
    outputPower = p;
  }

  /**
   * @param channel New radio channel
   */
  public void setChannel(int channel) {
    radioChannel = channel;
    lastEvent = RadioEvent.UNKNOWN;
    lastEventTime = simulation.getSimulationTime();
    setChanged();
    notifyObservers();
  }

  public JPanel getInterfaceVisualizer() {
    JPanel panel = new JPanel(new BorderLayout());
    Box box = Box.createVerticalBox();

    final JLabel statusLabel = new JLabel("");
    final JLabel lastEventLabel = new JLabel("");
    final JLabel channelLabel = new JLabel("");
    final JLabel powerLabel = new JLabel("Output power (dBm):");
    final JLabel ssLabel = new JLabel("");
    final JButton updateButton = new JButton("Update SS");

    JComboBox channelMenu = new JComboBox(new String[] {
        "ALL",
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
        "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
        "21", "22", "23", "24", "25", "26", "27", "28", "29", "30"
    });
    channelMenu.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        JComboBox m = (JComboBox) e.getSource();
        String s = (String) m.getSelectedItem();
        if (s == null || s.equals("ALL")) {
          setChannel(-1);
        } else {
          setChannel(Integer.parseInt(s));
        }
      }
    });
    if (getChannel() == -1) {
      channelMenu.setSelectedIndex(0);
    } else {
      channelMenu.setSelectedIndex(getChannel());
    }
    final JFormattedTextField outputPower = new JFormattedTextField(new Double(getCurrentOutputPower()));
    outputPower.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent evt) {
        setOutputPower(((Number)outputPower.getValue()).doubleValue());
      }
    });

    box.add(statusLabel);
    box.add(lastEventLabel);
    box.add(ssLabel);
    box.add(updateButton);
    box.add(channelLabel);
    box.add(channelMenu);
    box.add(powerLabel);
    box.add(outputPower);

    updateButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ssLabel.setText("Signal strength (not auto-updated): "
            + String.format("%1.1f", getCurrentSignalStrength()) + " dBm");
      }
    });

    final Observer observer = new Observer() {
      public void update(Observable obs, Object obj) {
        if (isTransmitting()) {
          statusLabel.setText("Transmitting");
        } else if (isReceiving()) {
          statusLabel.setText("Receiving");
        } else {
          statusLabel.setText("Listening");
        }

        lastEventLabel.setText("Last event (time=" + lastEventTime + "): " + lastEvent);
        ssLabel.setText("Signal strength (not auto-updated): "
            + String.format("%1.1f", getCurrentSignalStrength()) + " dBm");
        if (getChannel() == -1) {
          channelLabel.setText("Current channel: ALL");
        } else {
          channelLabel.setText("Current channel: " + getChannel());
        }
      }
    };
    this.addObserver(observer);

    observer.update(null, null);

    panel.add(BorderLayout.NORTH, box);
    panel.putClientProperty("intf_obs", observer);
    return panel;
  }

  public Collection<Element> getConfigXML() {
    /* TODO Save channel info? */
    /* TODO Save output power? */
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public Mote getMote() {
    return mote;
  }

  private boolean radioOn = true;
  public void setReceiverOn(boolean radioOn) {
    if (this.radioOn == radioOn) {
      return;
    }

    this.radioOn = radioOn;
    lastEvent = radioOn?RadioEvent.HW_ON:RadioEvent.HW_OFF;
    lastEventTime = simulation.getSimulationTime();
    this.setChanged();
    this.notifyObservers();
  }
  public boolean isRadioOn() {
    return radioOn;
  }

  /* Noise source radio support */
  public int getNoiseLevel() {
    return noiseSignal;
  }
  public void addNoiseLevelListener(NoiseLevelListener l) {
    noiseListeners.add(l);
  }
  public void removeNoiseLevelListener(NoiseLevelListener l) {
    noiseListeners.remove(l);
  }

  /* Noise source radio support (app mote API) */
  private int noiseSignal = Integer.MIN_VALUE;
  private ArrayList<NoiseLevelListener> noiseListeners = new ArrayList<NoiseLevelListener>();
  public void setNoiseLevel(int signal) {
    this.noiseSignal = signal;
    for (NoiseLevelListener l: noiseListeners) {
      l.noiseLevelChanged(this, signal);
    }
  }

  public double getDirection() {
    return 0;
  }
  public double getRelativeGain(double radians, double distance) {
    /* Simple sinus-based gain */
    return 5.0*Math.sin(5.0*radians)/(0.01*distance);
  }
  public void addDirectionChangeListener(DirectionChangeListener l) {
  }
  public void removeDirectionChangeListener(DirectionChangeListener l) {
  }

}
