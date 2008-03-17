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
import se.sics.cooja.interfaces.PacketRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.mspmote.SkyMote;
import se.sics.mspsim.chip.CC2420;
import se.sics.mspsim.chip.PacketListener;

@ClassDescription("CC2420")
public class SkyRadio extends Radio implements PacketRadio {
  private static Logger logger = Logger.getLogger(SkyRadio.class);

  private int lastEventTime = 0;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private SkyMote myMote;

  private CC2420 cc2420;

  private boolean isInterfered = false;

  private boolean isTransmitting = false;

  private boolean isReceiving = false;

  private boolean radioOn = true;

  private byte[] lastOutgoingPacket = null;

  private byte[] lastIncomingPacket = null;

  //TODO: HW on/off

  public SkyRadio(SkyMote mote) {
    this.myMote = mote;
    this.cc2420 = mote.skyNode.radio;

    cc2420.setPacketListener(new PacketListener() {
      public void transmissionStarted() {
        lastEventTime = myMote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.TRANSMISSION_STARTED;
        setChanged();
        notifyObservers();
      }

      public void transmissionEnded(int[] receivedData) {
        lastOutgoingPacket = new byte[receivedData.length];
        for (int i=0; i < receivedData.length; i++) {
          lastOutgoingPacket[i] = (byte) receivedData[i];
        }
        lastEventTime = myMote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.PACKET_TRANSMITTED;
        setChanged();
        notifyObservers();

        lastEventTime = myMote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.TRANSMISSION_FINISHED;
        setChanged();
        notifyObservers();
      }
    });
  }

  /* Packet radio support */
  public byte[] getLastPacketTransmitted() {
    return lastOutgoingPacket;
  }

  public byte[] getLastPacketReceived() {
    return lastIncomingPacket;
  }

  public void setReceivedPacket(byte[] data) {
    lastIncomingPacket = data;
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
    isReceiving = true;
    cc2420.setCCA(true);
    /* TODO cc2420.setSFD(true); */

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_STARTED;
    setChanged();
    notifyObservers();
  }

  public void signalReceptionEnd() {
    /* Deliver packet data */
    if (isReceiving && !isInterfered && lastIncomingPacket != null) {
      int[] incomingDataInts = new int[lastIncomingPacket.length];
      for (int i=0; i < lastIncomingPacket.length; i++) {
        incomingDataInts[i] = lastIncomingPacket[i];
      }

      cc2420.setIncomingPacket(incomingDataInts);
    }

    isReceiving = false;
    isInterfered = false;
    cc2420.setCCA(false);

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_FINISHED;
    setChanged();
    notifyObservers();
  }

  public RadioEvent getLastEvent() {
    return lastEvent;
  }

  public void interfereAnyReception() {
    isInterfered = true;
    isReceiving = false;
    lastIncomingPacket = null;
    cc2420.setCCA(true);
    logger.info("[interfered]: CCA true");

    lastEventTime = myMote.getSimulation().getSimulationTime();
    lastEvent = RadioEvent.RECEPTION_INTERFERED;
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
        } else if (radioOn) {
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

    // Saving observer reference for releaseInterfaceVisualizer
    panel.putClientProperty("intf_obs", observer);

    wrapperPanel.add(BorderLayout.NORTH, panel);
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
