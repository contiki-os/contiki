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
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.ArrayDeque;
import java.util.Collection;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.COOJARadioPacket;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.interfaces.CustomDataRadio;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.Radio;
import se.sics.cooja.mspmote.MspMote;
import se.sics.cooja.mspmote.MspMoteTimeEvent;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.USART;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;

/**
 * TR1001 radio interface on ESB platform.
 * Assumes Contiki driver specifics such as preambles, synchbytes, GCR coding, CRC16.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("TR1001 Radio")
public class TR1001Radio extends Radio implements USARTListener, CustomDataRadio {
  private static Logger logger = Logger.getLogger(TR1001Radio.class);

  /**
   * Cross-level:
   * Delay used when feeding packet data to radio chip (us).
   * 416us corresponds to 19200 bit/s with encoding.
   */
  public static final long DELAY_BETWEEN_BYTES = 416;

  /* The data used when transmission is interfered */
  private static final Byte CORRUPTED_DATA = (byte) 0xff;

  private MspMote mote;

  private boolean isTransmitting = false;
  private boolean isReceiving = false;
  private boolean isInterfered = false;

  private RadioEvent lastEvent = RadioEvent.UNKNOWN;
  private long lastEventTime = 0;

  private USART radioUSART = null;

  private RadioPacket receivedPacket = null;
  private RadioPacket sentPacket = null;

  private byte receivedByte, sentByte;

  private TR1001RadioPacketConverter tr1001PacketConverter = null;

  private boolean radioOn = true; /* TODO MSPSim: Not implemented */
  private double signalStrength = 0;  /* TODO MSPSim: Not implemented */
  
  
  /**
   * Creates an interface to the TR1001 radio at mote.
   * 
   * @param mote Mote
   */
  public TR1001Radio(Mote mote) {
    this.mote = (MspMote) mote;

    /* Start listening to CPU's USART */
    IOUnit usart = this.mote.getCPU().getIOUnit("USART 0");
    if (usart != null && usart instanceof USART) {
      radioUSART = (USART) usart;
      radioUSART.addUSARTListener(this);
    } else {
      throw new RuntimeException("Bad TR1001 IO: " + usart);
    }
  }

  /* Packet radio support */
  public RadioPacket getLastPacketTransmitted() {
    return sentPacket;
  }

  public RadioPacket getLastPacketReceived() {
    return receivedPacket;
  }

  public void setReceivedPacket(RadioPacket packet) {
    receivedPacket = packet;

    /* Convert to TR1001 packet data */
    byte[] arr = TR1001RadioPacketConverter.fromCoojaToTR1001(packet);
    final ArrayDeque<Byte> data = new ArrayDeque<Byte>();
    for (Byte b : arr) {
      data.addLast(b);
    }

    /* Feed incoming bytes to radio "slowly" via time events */
    TimeEvent receiveCrosslevelDataEvent = new MspMoteTimeEvent(mote, 0) {
      public void execute(long t) {
        super.execute(t);
        
        /* Stop receiving data when buffer is empty */
        if (data.isEmpty()) {
          return;
        }

        byte b = data.pop();
        if (isInterfered) {
          radioUSART.byteReceived(0xFF); /* Corrupted data */
        } else {
          radioUSART.byteReceived(b);
        }
        mote.requestImmediateWakeup();

        mote.getSimulation().scheduleEvent(this, t + DELAY_BETWEEN_BYTES);
      }
    };
    receiveCrosslevelDataEvent.execute(mote.getSimulation().getSimulationTime());
  }

  /* Custom data radio support */
  public Object getLastCustomDataTransmitted() {
    return sentByte;
  }

  public Object getLastCustomDataReceived() {
    return receivedByte;
  }

  public void receiveCustomData(Object data) {
    if (!(data instanceof Byte)) {
      logger.fatal("Received bad custom data: " + data);
      return;
    }

    receivedByte = isInterfered ? CORRUPTED_DATA : (Byte) data;

    final byte finalByte = receivedByte;
    mote.getSimulation().scheduleEvent(new MspMoteTimeEvent(mote, 0) {
      public void execute(long t) {
        super.execute(t);

        if (radioUSART.isReceiveFlagCleared()) {
          /*logger.info("----- TR1001 RECEIVED BYTE -----");*/
          radioUSART.byteReceived(finalByte);
        } else {
          logger.warn(mote.getSimulation().getSimulationTime() + ": ----- TR1001 RECEIVED BYTE DROPPED -----");
        }
        mote.requestImmediateWakeup();
      }
    }, mote.getSimulation().getSimulationTime());
  }

  /* USART listener support */
  public void dataReceived(USARTSource source, int data) {
    if (!isTransmitting()) {
      /* New transmission discovered */
      /*logger.info("----- NEW TR1001 TRANSMISSION DETECTED -----");*/
      tr1001PacketConverter = new TR1001RadioPacketConverter();
      
      lastEvent = RadioEvent.TRANSMISSION_STARTED;
      lastEventTime = mote.getSimulation().getSimulationTime();
      isTransmitting = true;
      this.setChanged();
      this.notifyObservers();

      /* Timeout transmission after some time */
      if (timeoutTransmission.isScheduled()) {
        logger.warn("Timeout TX event already scheduled");
        timeoutTransmission.remove();
      }
      mote.getSimulation().scheduleEvent(
          timeoutTransmission,
          mote.getSimulation().getSimulationTime() + 40*Simulation.MILLISECOND
      );
    }

    /* Deliver custom data byte */
    lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
    sentByte = (byte) data;
    this.setChanged();
    this.notifyObservers();

    /* Detect full packet */
    boolean finished = tr1001PacketConverter.fromTR1001ToCoojaAccumulated(sentByte);
    if (finished) {
      timeoutTransmission.remove();
      
      /* Transmission finished - deliver packet immediately */
      if (tr1001PacketConverter.accumulatedConversionIsOk()) {
        /* Deliver packet */
        /* logger.info("----- TR1001 DELIVERED PACKET -----"); */
        sentPacket = tr1001PacketConverter.getAccumulatedConvertedCoojaPacket();
        lastEvent = RadioEvent.PACKET_TRANSMITTED;
        this.setChanged();
        this.notifyObservers();
      }

      /* Finish transmission */
      isTransmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      TR1001Radio.this.setChanged();
      TR1001Radio.this.notifyObservers();
      /* logger.info("----- TR1001 TRANSMISSION ENDED -----"); */
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
    /* TODO Implement */
    return -1;
  }

  public void signalReceptionStart() {
    lastEvent = RadioEvent.RECEPTION_STARTED;
    isReceiving = true;
    isInterfered = false;
    this.setChanged();
    this.notifyObservers();
  }

  public void signalReceptionEnd() {
    isInterfered = false;
    isReceiving = false;
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
      receivedPacket = null;

      lastEventTime = mote.getSimulation().getSimulationTime();
      lastEvent = RadioEvent.RECEPTION_INTERFERED;
      /*logger.info("----- TR1001 RECEPTION INTERFERED -----");*/
      this.setChanged();
      this.notifyObservers();
    }
  }

  public double getCurrentOutputPower() {
    // TODO Implement method
    return 0;
  }

  public int getOutputPowerIndicatorMax() {
    return 100;
  }

  public int getCurrentOutputPowerIndicator() {
    // TODO Implement output power indicator
    return 100;
  }

  public double getCurrentSignalStrength() {
    return signalStrength;
  }

  public void setCurrentSignalStrength(double signalStrength) {
    this.signalStrength = signalStrength;
  }

  public Position getPosition() {
    return mote.getInterfaces().getPosition();
  }

  private TimeEvent timeoutTransmission = new MoteTimeEvent(mote, 0) {
    public void execute(long t) {
      if (!isTransmitting()) {
        /* Nothing to do */
        return;
      }
      
      logger.warn("TR1001 transmission timed out, delivering empty packet");
      
      /* XXX Timeout: We may need to deliver an empty radio packet here */
      sentPacket = new COOJARadioPacket(new byte[0]);
      lastEvent = RadioEvent.PACKET_TRANSMITTED;
      TR1001Radio.this.setChanged();
      TR1001Radio.this.notifyObservers();
      
      isTransmitting = false;
      lastEvent = RadioEvent.TRANSMISSION_FINISHED;
      TR1001Radio.this.setChanged();
      TR1001Radio.this.notifyObservers();
    }
  };

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

  public Mote getMote() {
    return mote;
  }

  public boolean isRadioOn() {
    /* TODO Implement me */
    return true;
  }
}
