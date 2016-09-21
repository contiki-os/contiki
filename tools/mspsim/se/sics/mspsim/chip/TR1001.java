/**
 * Copyright (c) 2008-2012, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
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
 * This file is part of MSPSim.
 *
 * -----------------------------------------------------------------
 * TR1001
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 11 mar 2008
 */

package se.sics.mspsim.chip;
import se.sics.mspsim.core.Chip;
import se.sics.mspsim.core.MSP430Core;
import se.sics.mspsim.core.USART;
import se.sics.mspsim.core.USARTListener;
import se.sics.mspsim.core.USARTSource;

/**
 *
 */
public class TR1001 extends Chip implements RFListener, RFSource {

  public static final int MODE_TXRX_OFF = 0x00;
  public static final int MODE_RX_ON = 0x01;
  public static final int MODE_TXRX_ON = 0x02;
  public static final int MODE_MAX = MODE_TXRX_ON;
  private static final String[] MODE_NAMES = new String[] {
    "off", "listen", "transmit"
  };
  private final USART usart;
  private RFListener rfListener;

  public TR1001(MSP430Core cpu, USART usart) {
    super("TR1001", "Radio", cpu);
    this.usart = usart;
    setModeNames(MODE_NAMES);
    setMode(MODE_TXRX_OFF);
    usart.addUSARTListener(new USARTListener() {

      public void dataReceived(USARTSource source, int data) {
        RFListener listener = rfListener;
        if (getMode() != MODE_TXRX_ON) {
          // Radio is turned off during transmission
          if (DEBUG) {
            log(" ----- TR1001 OFF DURING TRANSMISSION -----");
          }
        } else if (listener != null) {
          listener.receivedByte((byte) (data & 0xff));
        }
      }
    });
  }

  public void setMode(int mode) {
    super.setMode(mode);
  }

  @Override
  public int getModeMax() {
    return MODE_MAX;
  }

  public String info() {
      return "Radio State: " + getModeName(getMode());
  }

  @Override
  public synchronized void addRFListener(RFListener rf) {
    rfListener = RFListener.Proxy.INSTANCE.add(rfListener, rf);
  }

  @Override
  public synchronized void removeRFListener(RFListener rf) {
    rfListener = RFListener.Proxy.INSTANCE.remove(rfListener, rf);
  }

  /* Receive a byte from the radio medium
   * @see se.sics.mspsim.chip.RFListener#receivedByte(byte)
   */
  public void receivedByte(byte data) {
    if (getMode() == MODE_TXRX_OFF) {
      // Radio turned off
      if (DEBUG) {
        log(" ----- TR1001 OFF DURING RECEPTION -----");
      }

    } else if (usart.isReceiveFlagCleared()) {
      /* logger.info("----- TR1001 RECEIVED BYTE -----"); */
      usart.byteReceived(data);

    } else {
      if (DEBUG) {
        log(" ----- TR1001 RECEIVED BYTE TOO EARLY -----");
      }
    }
  }

  public int getConfiguration(int parameter) {
      return 0;
  }
}
