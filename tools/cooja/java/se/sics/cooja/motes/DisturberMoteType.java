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
 *
 */

package se.sics.cooja.motes;

import java.awt.Container;

import org.apache.log4j.Logger;

import se.sics.cooja.AbstractionLevelDescription;
import se.sics.cooja.COOJARadioPacket;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteTimeEvent;
import se.sics.cooja.MoteType;
import se.sics.cooja.RadioPacket;
import se.sics.cooja.Simulation;
import se.sics.cooja.MoteType.MoteTypeCreationException;
import se.sics.cooja.interfaces.ApplicationRadio;
import se.sics.cooja.interfaces.Radio.RadioEvent;

/**
 * Simple application-level mote that periodically transmits dummy radio packets
 * on all radio channels (-1), interfering all surrounding radio communication.
 * 
 * This mote type also implements the mote functionality ("mote software"),
 * and can be used as an example of implementing application-level mote.
 *
 * @see DisturberMote
 * @author Fredrik Osterlind, Thiemo Voigt
 */
@ClassDescription("Disturber mote")
@AbstractionLevelDescription("Application level")
public class DisturberMoteType extends AbstractApplicationMoteType {
  private static Logger logger = Logger.getLogger(DisturberMoteType.class);

  public DisturberMoteType() {
    super();
  }

  public DisturberMoteType(String identifier) {
    super(identifier);
    setDescription("Disturber Mote Type #" + identifier);
  }

  public boolean configureAndInit(Container parentContainer,
      Simulation simulation, boolean visAvailable) 
  throws MoteTypeCreationException {
    if (!super.configureAndInit(parentContainer, simulation, visAvailable)) {
      return false;
    }
    setDescription("Disturber Mote Type #" + getIdentifier());
    return true;
  }
  
  public Mote generateMote(Simulation simulation) {
    return new DisturberMote(this, simulation);
  }

  public static class DisturberMote extends AbstractApplicationMote {
    private ApplicationRadio radio = null;
    
    private final RadioPacket radioPacket = new COOJARadioPacket(new byte[] {
        0x01, 0x02, 0x03, 0x04, 0x05
    });
    private final static long DELAY = Simulation.MILLISECOND/5;
    private final static long DURATION = 10*Simulation.MILLISECOND;
    
    public DisturberMote() {
      super();
    }
    public DisturberMote(MoteType moteType, Simulation simulation) {
      super(moteType, simulation);
    }
    
    public void execute(long time) {
      if (radio == null) {
        radio = (ApplicationRadio) getInterfaces().getRadio();
      }
      
      /* Start sending interfering traffic */
      /*logger.info("Sending radio packet on channel: " + radio.getChannel());*/
      radio.startTransmittingPacket(radioPacket, DURATION);
    }

    public void receivedPacket(RadioPacket p) {
      /* Ignore */
    }
    public void sentPacket(RadioPacket p) {
      /* Send another packet after a small pause */
      getSimulation().scheduleEvent(new MoteTimeEvent(this, 0) {
        public void execute(long t) {
          /*logger.info("Sending another radio packet on channel: " + radio.getChannel());*/
          radio.startTransmittingPacket(radioPacket, DURATION);
        }
      }, getSimulation().getSimulationTime() + DELAY);
    }

    public String toString() {
      return "Disturber " + getID();
    }
}
}
