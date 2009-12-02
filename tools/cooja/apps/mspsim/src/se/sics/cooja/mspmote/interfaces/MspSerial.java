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
 * $Id: MspSerial.java,v 1.1 2009/12/02 17:12:32 fros4943 Exp $
 */

package se.sics.cooja.mspmote.interfaces;

import java.util.Vector;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.Simulation;
import se.sics.cooja.TimeEvent;
import se.sics.cooja.dialogs.SerialUI;
import se.sics.cooja.interfaces.SerialPort;
import se.sics.cooja.mspmote.MspMote;
import se.sics.mspsim.core.IOUnit;
import se.sics.mspsim.core.USART;
import se.sics.mspsim.core.USARTListener;

/**
 * @author Fredrik Osterlind
 */
@ClassDescription("Serial port")
public class MspSerial extends SerialUI implements SerialPort {
  private static final long DELAY_INCOMING_DATA = 69; /* 115200 bit/s */
  
  private static Logger logger = Logger.getLogger(MspSerial.class);

  private Simulation simulation;
  private MspMote mote;
  private USART usart;
  
  private Vector<Byte> incomingData = new Vector<Byte>();
 
  public MspSerial(Mote mote) {
    this.mote = (MspMote) mote;
    this.simulation = mote.getSimulation();

    /* Listen to port writes */
    IOUnit ioUnit = this.mote.getCPU().getIOUnit("USART 1");
    if (ioUnit instanceof USART) {
      usart = (USART) ioUnit;
      usart.setUSARTListener(new USARTListener() {
        public void dataReceived(USART arg0, int arg1) {
          MspSerial.this.dataReceived(arg1);
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
    if (writeDataEvent.isScheduled()) {
      return;
    }

    /* Simulation thread: schedule immediately */
    if (simulation.isSimulationThread()) {
      simulation.scheduleEvent(writeDataEvent, simulation.getSimulationTime());
      return;
    }
    
    /* Non-simulation thread: poll */
    simulation.invokeSimulationThread(new Runnable() {
      public void run() {
        if (writeDataEvent.isScheduled()) {
          return;
        }
        simulation.scheduleEvent(writeDataEvent, simulation.getSimulationTime());
      }
    });
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
    mote.requestImmediateWakeup();
  }

  private TimeEvent writeDataEvent = new MoteTimeEvent(mote, 0) {
    public void execute(long t) {
      tryWriteNextByte();
      if (!incomingData.isEmpty()) {
        simulation.scheduleEvent(this, t+DELAY_INCOMING_DATA);
      }
    }
  };

  public Mote getMote() {
    return mote;
  }
}
