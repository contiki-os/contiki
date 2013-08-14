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
 */

package se.sics.cooja.avrmote.interfaces;

import org.apache.log4j.Logger;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.avrmote.MicaZMote;
import se.sics.cooja.emulatedmote.Radio802154;
import se.sics.cooja.interfaces.CustomDataRadio;
import avrora.sim.FiniteStateMachine;
import avrora.sim.FiniteStateMachine.Probe;
import avrora.sim.platform.MicaZ;
import avrora.sim.radio.CC2420Radio;
import avrora.sim.radio.Medium;

/**
 * CC2420 to COOJA wrapper.
 *
 * @author Joakim Eriksson
 */
@ClassDescription("CC2420")
public class MicaZRadio extends Radio802154 {
  private static Logger logger = Logger.getLogger(MicaZRadio.class);

  private MicaZ micaz;
  private CC2420Radio cc2420;

//  private int mode;
  Medium.Transmitter trans;
  CC2420Radio.Receiver recv;
  FiniteStateMachine fsm;
  
  public MicaZRadio(Mote mote) {
    super(mote);
    micaz = ((MicaZMote)mote).getMicaZ();
    cc2420 = (CC2420Radio) micaz.getDevice("radio");
   
    trans = cc2420.getTransmitter();
    fsm = cc2420.getFiniteStateMachine();
    recv = (CC2420Radio.Receiver) cc2420.getReceiver();
    trans.insertProbe(new Medium.Probe.Empty() {
        public void fireBeforeTransmit(Medium.Transmitter t, byte val) {
            handleTransmit(val);
        }
    });
    fsm.insertProbe(new Probe() {
        public void fireBeforeTransition(int arg0, int arg1) {
        }
        public void fireAfterTransition(int arg0, int arg1) {
            //System.out.println("CC2420 - MicaZ FSM: " + arg0 + " " + arg1);
            RadioEvent re = null;
            if (arg1 >= 3) {
                re = RadioEvent.HW_ON;
            } else {
                if (arg0 > 3 && arg1 == 2) {
                    /* likely that radio dips into 2 before going back to 3 */
                } else {
                    re = RadioEvent.HW_OFF;
                }
            }
            if (re != null) {
                lastEvent = re;
                lastEventTime = MicaZRadio.this.mote.getSimulation().getSimulationTime();
                setChanged();
                notifyObservers();
            }
        }
    });
    
    
  }

  public int getChannel() {
//    cc2420.updateActiveFrequency();
//    return cc2420.getActiveChannel();
    return 0;
  }

  public int getFrequency() {
//    cc2420.updateActiveFrequency();
      return (int) cc2420.getFrequency();
  }

  public boolean isRadioOn() {
      FiniteStateMachine fsm = cc2420.getFiniteStateMachine();
      /* based on reading the source code it seems that the fsm state = 3 means on */
      //System.out.println("COOJA: cc2420 FSM: " + fsm.getCurrentState());
      return fsm.getCurrentState() >= 3;
  }

  public void signalReceptionStart() {
//    cc2420.setCCA(true);
//    hasFailedReception = mode == CC2420.MODE_TXRX_OFF;
      super.signalReceptionStart();
  }

  public double getCurrentOutputPower() {
    return 1.1;//cc2420.getOutputPower();
  }

  public int getCurrentOutputPowerIndicator() {
    return 31; //cc2420.getOutputPowerIndicator();
  }

  public int getOutputPowerIndicatorMax() {
    return 31;
  }

  public double getCurrentSignalStrength() {
    return 1;//cc2420.getRSSI();
  }

  public void setCurrentSignalStrength(double signalStrength) {
    //cc2420.setRSSI((int) signalStrength);
  }

  protected void handleEndOfReception() {
      /* tell the receiver that the packet is ended */
      recv.nextByte(false, (byte)0);
  }

  protected void handleReceive(byte b) {
      //System.out.println("MicaZ: Received: " + (b &0xff));
      recv.nextByte(true, (byte)b);
  }
  
  public boolean canReceiveFrom(CustomDataRadio radio) {
    if (radio.getClass().equals(this.getClass())) {
      return true;
    }
    return false;
  }
}
