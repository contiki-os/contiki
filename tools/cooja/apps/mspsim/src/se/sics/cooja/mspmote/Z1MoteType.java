/*
 * Copyright (c) 2012, Swedish Institute of Computer Science.
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

package se.sics.cooja.mspmote;
import se.sics.cooja.AbstractionLevelDescription;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.interfaces.Mote2MoteRelations;
import se.sics.cooja.interfaces.MoteAttributes;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.RimeAddress;
import se.sics.cooja.mspmote.interfaces.Msp802154Radio;
import se.sics.cooja.mspmote.interfaces.MspButton;
import se.sics.cooja.mspmote.interfaces.MspClock;
import se.sics.cooja.mspmote.interfaces.MspDebugOutput;
import se.sics.cooja.mspmote.interfaces.MspDefaultSerial;
import se.sics.cooja.mspmote.interfaces.MspLED;
import se.sics.cooja.mspmote.interfaces.MspMoteID;

@ClassDescription("Z1 mote")
@AbstractionLevelDescription("Emulated level")
public class Z1MoteType extends AbstractMspMoteType {

    @Override
    public String getMoteType() {
        return "z1";
    }

    @Override
    public String getMoteName() {
        return "Z1";
    }

    @Override
    protected String getMoteImage() {
        return "images/z1.jpg";
    }

    @Override
    protected MspMote createMote(Simulation simulation) {
        return new Z1Mote(this, simulation);
    }

    public Class<? extends MoteInterface>[] getDefaultMoteInterfaceClasses() {
  	  return getAllMoteInterfaceClasses();
    }

    public Class<? extends MoteInterface>[] getAllMoteInterfaceClasses() {
        @SuppressWarnings("unchecked")
        Class<? extends MoteInterface>[] list = createMoteInterfaceList(
                Position.class,
                RimeAddress.class,
                IPAddress.class,
                Mote2MoteRelations.class,
                MoteAttributes.class,
                MspClock.class,
                MspMoteID.class,
                MspButton.class,
//                SkyFlash.class,
                Msp802154Radio.class,
                MspDefaultSerial.class,
                MspLED.class,
                MspDebugOutput.class /* EXPERIMENTAL: Enable me for COOJA_DEBUG(..) */
        );
        return list;
    }

}
