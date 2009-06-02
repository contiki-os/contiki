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
 * $Id: SkySerial.java,v 1.16 2009/06/02 09:34:59 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.*;

import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.TimeEvent;
import se.sics.mspsim.core.*;
import se.sics.cooja.dialogs.SerialUI;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.mspmote.SkyMote;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Serial port")
public class SkySerial extends SerialUI implements SerialPort {
  private static final long DELAY_INCOMING_DATA = 69; /* Corresponds to 115200 bit/s */
  
  private static Logger logger = Logger.getLogger(SkySerial.class);

  private SkyMote mote;
  private USART usart;
  
  private Vector<Byte> incomingData = new Vector<Byte>();

  public SkySerial(Mote mote) {
    this.mote = (SkyMote) mote;

    /* Listen to port writes */
    IOUnit ioUnit = this.mote.getCPU().getIOUnit("USART 1");
    if (ioUnit instanceof USART) {
      usart = (USART) ioUnit;
      usart.setUSARTListener(new USARTListener() {
        public void dataReceived(USART arg0, int arg1) {
          SkySerial.this.dataReceived(arg1);
        }
        public void stateChanged(int state) {
          if (state == USARTListener.RXFLAG_CLEARED) {
            tryWriteNextByte();
          }
        }
      });
    }
  }

  public void writeByte(byte b) {
    incomingData.add(b);
    mote.getSimulation().scheduleEvent(writeDataEvent, mote.getSimulation().getSimulationTime());
  }

  public void writeString(String s) {
    for (int i=0; i < s.length(); i++) {
      writeByte((byte) s.charAt(i));
    }
    writeByte((byte) 10);
  }

  public void writeArray(byte[] s) {
    for (byte element : s) {
      writeByte(element);
    }
  }

  private void tryWriteNextByte() {
    byte b;

    synchronized (incomingData) {
      if (!usart.isReceiveFlagCleared()) {
        return;
      }
      if (incomingData.isEmpty()) {
        return;
      }

      /* Write byte to serial port */
      b = incomingData.remove(0);
    }
    usart.byteReceived(b);
  }

  private TimeEvent writeDataEvent = new TimeEvent(0) {
    public void execute(long t) {
      tryWriteNextByte();
      if (!incomingData.isEmpty()) {
        mote.getSimulation().scheduleEvent(this, t+DELAY_INCOMING_DATA);
      }
    }
  };

  public Mote getMote() {
    return mote;
  }

}
