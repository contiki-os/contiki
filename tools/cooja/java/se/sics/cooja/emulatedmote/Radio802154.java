/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: Radio802154.java,v 1.2 2010/02/05 09:07:58 fros4943 Exp $
 */
package se.sics.cooja.emulatedmote;

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

/**
 * 802.15.4 radio class for COOJA.
 *
 * @author Joakim Eriksson
 */

public abstract class Radio802154 extends Radio implements CustomDataRadio {

    private final static boolean DEBUG = false;
    
    private static Logger logger = Logger.getLogger(Radio802154.class);

    protected long lastEventTime = 0;

    protected RadioEvent lastEvent = RadioEvent.UNKNOWN;

    protected boolean isInterfered = false;

    private boolean isTransmitting = false;

    protected boolean isReceiving = false;
    //    private boolean hasFailedReception = false;

    private boolean radioOn = true;

    private RadioByte lastOutgoingByte = null;

    private RadioByte lastIncomingByte = null;

    private RadioPacket lastOutgoingPacket = null;

    private RadioPacket lastIncomingPacket = null;

    //    private int mode;
    protected Mote mote;
     
    public Radio802154(Mote mote) {
        this.mote = mote;
    }

    int len = 0;
    int expLen = 0;
    byte[] buffer = new byte[127 + 15];
    protected void handleTransmit(byte val) {
        if (len == 0) {
            lastEventTime = mote.getSimulation().getSimulationTime();
            lastEvent = RadioEvent.TRANSMISSION_STARTED;
            if (DEBUG) logger.debug("----- 802.15.4 TRANSMISSION STARTED -----");
            setChanged();
            notifyObservers();
        }
        /* send this byte to all nodes */
        lastOutgoingByte = new RadioByte(val);
        lastEventTime = mote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.CUSTOM_DATA_TRANSMITTED;
        setChanged();
        notifyObservers();

        buffer[len++] = val;

        //System.out.println("## 802.15.4: " + (val&0xff) + " transmitted...");

        if (len == 6) {
            //System.out.println("## CC2420 Packet of length: " + val + " expected...");
            expLen = val + 6;
        }

        if (len == expLen) {
            if (DEBUG) logger.debug("----- 802.15.4 CUSTOM DATA TRANSMITTED -----");

            lastOutgoingPacket = Radio802154PacketConverter.fromCC2420ToCooja(buffer);
            lastEventTime = mote.getSimulation().getSimulationTime();
            lastEvent = RadioEvent.PACKET_TRANSMITTED;
            if (DEBUG) logger.debug("----- 802.15.4 PACKET TRANSMITTED -----");
            setChanged();
            notifyObservers();

            //          System.out.println("## CC2420 Transmission finished...");

            lastEventTime = mote.getSimulation().getSimulationTime();
            /*logger.debug("----- SKY TRANSMISSION FINISHED -----");*/
            lastEvent = RadioEvent.TRANSMISSION_FINISHED;
            setChanged();
            notifyObservers();
            len = 0;
        }
    }

    /* Packet radio support */
    public RadioPacket getLastPacketTransmitted() {
        return lastOutgoingPacket;
    }

    public RadioPacket getLastPacketReceived() {
        return lastIncomingPacket;
    }

    public void setReceivedPacket(RadioPacket packet) {
    }

    /* Custom data radio support */
    public Object getLastCustomDataTransmitted() {
        return lastOutgoingByte;
    }

    public Object getLastCustomDataReceived() {
        return lastIncomingByte;
    }

    public void receiveCustomData(Object data) {
        if (data instanceof RadioByte) {
            lastIncomingByte = (RadioByte) data;
            handleReceive(lastIncomingByte.getPacketData()[0]);
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

    protected abstract void handleReceive(byte b);

    protected abstract void handleEndOfReception();

    public abstract int getChannel();

    public abstract int getFrequency();

    public abstract boolean isRadioOn();

    public abstract double getCurrentOutputPower();
    
    public abstract int getCurrentOutputPowerIndicator();

    public abstract int getOutputPowerIndicatorMax();

    public abstract double getCurrentSignalStrength();

    public abstract void setCurrentSignalStrength(double signalStrength);

    /* need to add a few more methods later??? */
    public void signalReceptionStart() {
        isReceiving = true;

        //      cc2420.setCCA(true);
        //      hasFailedReception = mode == CC2420.MODE_TXRX_OFF;
        /* TODO cc2420.setSFD(true); */

        lastEventTime = mote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.RECEPTION_STARTED;
        if (DEBUG) logger.debug("----- 802.15.4 RECEPTION STARTED -----");
        setChanged();
        notifyObservers();
    }

    public void signalReceptionEnd() {
        /* Deliver packet data */
        isReceiving = false;
        //      hasFailedReception = false;
        isInterfered = false;
        //      cc2420.setCCA(false);

        /* tell the receiver that the packet is ended */
        handleEndOfReception();

        lastEventTime = mote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.RECEPTION_FINISHED;
        if (DEBUG) logger.debug("----- 802.15.4 RECEPTION FINISHED -----");
       // Exception e = new IllegalStateException("Why finished?");
       // e.printStackTrace();
        setChanged();
        notifyObservers();
    }

    public RadioEvent getLastEvent() {
        return lastEvent;
    }

    public void interfereAnyReception() {
        isInterfered = true;
        isReceiving = false;
        //      hasFailedReception = false;
        lastIncomingPacket = null;

        //cc2420.setCCA(true);

        /* is this ok ?? */
        handleEndOfReception();
        //recv.nextByte(false, (byte)0);

        lastEventTime = mote.getSimulation().getSimulationTime();
        lastEvent = RadioEvent.RECEPTION_INTERFERED;
        /*logger.debug("----- SKY RECEPTION INTERFERED -----");*/
        setChanged();
        notifyObservers();
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
}
