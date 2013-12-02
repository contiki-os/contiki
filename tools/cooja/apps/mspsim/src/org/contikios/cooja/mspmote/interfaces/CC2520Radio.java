
package org.contikios.cooja.mspmote.interfaces;

import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Mote;
import org.contikios.cooja.RadioPacket;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.CustomDataRadio;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.Radio;
import org.contikios.cooja.mspmote.MspMote;
import org.contikios.cooja.mspmote.MspMoteTimeEvent;
import org.contikios.cooja.mspmote.interfaces.CC2420RadioPacketConverter;
import se.sics.mspsim.chip.CC2520;
import se.sics.mspsim.chip.ChannelListener;
import se.sics.mspsim.chip.RFListener;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.OperatingModeListener;

/**
 * MSPSim CC2520 radio to COOJA wrapper.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("IEEE CC2520 Radio")
public class CC2520Radio extends Radio implements CustomDataRadio {
  private static Logger logger = Logger.getLogger(CC2520Radio.class);

  /**
   * Cross-level:
   * Inter-byte delay for delivering cross-level packet bytes.
   */
  public static final long DELAY_BETWEEN_BYTES =
    (long) (1000.0*Simulation.MILLISECOND/(250000.0/8.0)); /* us. Corresponds to 250kbit/s */

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;

  private final MspMote mote;
  private final CC2520 radio;

  private boolean isInterfered = false;
  private boolean isTransmitting = false;
  private boolean isReceiving = false;

  private byte lastOutgoingByte;
  private byte lastIncomingByte;

  private RadioPacket lastOutgoingPacket = null;
  private RadioPacket lastIncomingPacket = null;

  public CC2520Radio(Mote m) {
    this.mote = (MspMote)m;
    this.radio = this.mote.getCPU().getChip(CC2520.class);
    if (radio == null) {
      throw new IllegalStateException("Mote is not equipped with an IEEE CC2520 radio");
    }

    radio.addRFListener(new RFListener() {
      int len = 0;
      int expLen = 0;
      byte[] buffer = new byte[127 + 15];
      public void receivedByte(byte data) {
        if (!isTransmitting()) {
          lastEvent = RadioEvent.TRANSMISSION_STARTED;
          isTransmitting = true;
          len = 0;
          /*logger.debug("----- CC2520 TRANSMISSION STARTED -----");*/
          setChanged();
          notifyObservers();
        }

        if (len >= buffer.length) {
          /* Bad size packet, too large */
          logger.debug("Error: bad size: " + len + ", dropping outgoing byte: " + data);
          return;
        }

        /* send this byte to all nodes */
        lastOutgoingByte = data;
        lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
        setChanged();
        notifyObservers();

        buffer[len++] = data;

        if (len == 6) {
//          System.out.println("## CC2520 Packet of length: " + data + " expected...");
          expLen = data + 6;
        }

        if (len == expLen) {
          /*logger.debug("----- CC2520 CUSTOM DATA TRANSMITTED -----");*/
        	len -= 4; /* preamble */
        	len -= 1; /* synch */
        	len -= radio.getFooterLength(); /* footer */
        	final byte[] packetdata = new byte[len];
        	System.arraycopy(buffer, 4+1, packetdata, 0, len);
        	lastOutgoingPacket =  new RadioPacket() {
        		public byte[] getPacketData() {
        			return packetdata;
        		}
        	};

          /*logger.debug("----- CC2520 PACKET TRANSMITTED -----");*/
          setChanged();
          notifyObservers();

          /*logger.debug("----- CC2520 TRANSMISSION FINISHED -----");*/
          isTransmitting = false;
          lastEvent = RadioEvent.TRANSMISSION_FINISHED;
          setChanged();
          notifyObservers();
          len = 0;
        }
      }
    });

    radio.addOperatingModeListener(new OperatingModeListener() {
      public void modeChanged(Chip source, int mode) {
        if (radio.isReadyToReceive()) {
          lastEvent = RadioEvent.HW_ON;
          setChanged();
          notifyObservers();
        } else {
          radioOff();
        }
      }
    });

    radio.addChannelListener(new ChannelListener() {
      public void channelChanged(int channel) {
        /* XXX Currently assumes zero channel switch time */
        lastEvent = RadioEvent.UNKNOWN;
        setChanged();
        notifyObservers();
      }
    });
  }

  private void radioOff() {
    /* Radio was turned off during transmission.
     * May for example happen if watchdog triggers */
    if (isTransmitting()) {
      logger.warn("Turning off radio while transmitting, ending packet prematurely");

      /* Simulate end of packet */
      lastOutgoingPacket = new RadioPacket() {
        public byte[] getPacketData() {
          return new byte[0];
        }
      };

      lastEvent = RadioEvent.PACKET_TRANSMITTED;
      /*logger.debug("----- CC2520 PACKET TRANSMITTED -----");*/
      setChanged();
      notifyObservers();

      /* Register that transmission ended in radio medium */
      /*logger.debug("----- CC2520 TRANSMISSION FINISHED -----");*/
      isTransmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      setChanged();
      notifyObservers();
    }

    lastEvent = RadioEvent.HW_OFF;
    setChanged();
    notifyObservers();
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return lastOutgoingPacket;
  }

  public RadioPacket getLastPacketReceived() {
    return lastIncomingPacket;
  }

  public void setReceivedPacket(RadioPacket packet) {
    logger.fatal("TODO Implement me!");
  }

  /* Custom data radio support */
  public Object getLastCustomDataTransmitted() {
    return lastOutgoingByte;
  }

  public Object getLastCustomDataReceived() {
    return lastIncomingByte;
  }

  public void receiveCustomData(Object data) {
    if (!(data instanceof Byte)) {
      logger.fatal("Bad custom data: " + data);
      return;
    }
    lastIncomingByte = (Byte) data;

    final byte inputByte;
    if (isInterfered()) {
      inputByte = (byte)0xFF;
    } else {
      inputByte = lastIncomingByte;
    }
    mote.getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
      public void execute(long t) {
        super.execute(t);
        radio.receivedByte(inputByte);
        mote.requestImmediateWakeup();
      }
    }, mote.getSimulation().getSimulationTime());

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
    return radio.getActiveChannel();
  }

  public int getFrequency() {
    return radio.getActiveFrequency();
  }

  public void signalReceptionStart() {
    isReceiving = true;

    lastEvent = RadioEvent.RECEPTION_STARTED;
    /*logger.debug("----- CC2520 RECEPTION STARTED -----");*/
    setChanged();
    notifyObservers();
  }

  public void signalReceptionEnd() {
    /* Deliver packet data */
    isReceiving = false;
    isInterfered = false;

    lastEvent = RadioEvent.RECEPTION_FINISHED;
    /*logger.debug("----- CC2520 RECEPTION FINISHED -----");*/
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

    lastEvent = RadioEvent.RECEPTION_INTERFERED;
    /*logger.debug("----- CC2520 RECEPTION INTERFERED -----");*/
    setChanged();
    notifyObservers();
  }

  public double getCurrentOutputPower() {
    return radio.getOutputPower();
  }

  public int getCurrentOutputPowerIndicator() {
	  return 100;
//    return radio.getOutputPowerIndicator();
  }

  public int getOutputPowerIndicatorMax() {
	  return 100;
//    return 31;
  }

  double currentSignalStrength = 0;

  /**
   * Last 8 received signal strengths
   */
  private double[] rssiLast = new double[8];
  private int rssiLastCounter = 0;

  public double getCurrentSignalStrength() {
    return currentSignalStrength;
  }

  public void setCurrentSignalStrength(final double signalStrength) {
    if (signalStrength == currentSignalStrength) {
      return; /* ignored */
    }
    currentSignalStrength = signalStrength;
    if (rssiLastCounter == 0) {
      getMote().getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
        public void execute(long t) {
          super.execute(t);

          /* Update average */
          System.arraycopy(rssiLast, 1, rssiLast, 0, 7);
          rssiLast[7] = currentSignalStrength;
          double avg = 0;
          for (double v: rssiLast) {
            avg += v;
          }
          avg /= rssiLast.length;

          radio.setRSSI((int) avg);

          rssiLastCounter--;
          if (rssiLastCounter > 0) {
            mote.getSimulation().scheduleEvent(this, t+DELAY_BETWEEN_BYTES/2);
          }
        }
      }, mote.getSimulation().getSimulationTime());
    }
    rssiLastCounter = 8;
  }
  
  
  public void setLQI(int lqi){
	  radio.setLQI(lqi);
  }

  public int getLQI(){
	  return radio.getLQI();
  }
  
  
  public Mote getMote() {
    return mote;
  }

  public Position getPosition() {
    return mote.getInterfaces().getPosition();
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public boolean isRadioOn() {
    if (radio.isReadyToReceive()) {
      return true;
    }
    if (radio.getMode() == CC2520.MODE_POWER_OFF) {
      return false;
    }
    if (radio.getMode() == CC2520.MODE_TXRX_OFF) {
      return false;
    }
    return true;
  }
  
  public boolean canReceiveFrom(CustomDataRadio radio) {
    if (radio.getClass().equals(this.getClass())) {
      return true;
    }
    return false;
  }
}
