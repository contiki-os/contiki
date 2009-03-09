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
import se.sics.cooja.*;
import se.sics.cooja.interfaces.Position;

/**
 * Simple application-level mote that periodically transmits dummy radio packets
 * on a configurable radio channel, interfering all surrounding radio communication.
 *
 * @see DisturberMote
 * @author Fredrik Osterlind, Thiemo Voigt
 */
@ClassDescription("Disturber Mote Type")
@AbstractionLevelDescription("Application level")
public class DisturberMoteType extends AbstractApplicationMoteType {
  private static Logger logger = Logger.getLogger(DisturberMoteType.class);

  public DisturberMoteType() {
    super();
  }

  public DisturberMoteType(String identifier) {
    super(identifier);
    setDescription("Disturber Mote Type #" + getIdentifier());
  }

  public Mote generateMote(Simulation simulation) {
    return new DisturberMote(this, simulation);
  }

  public boolean configureAndInit(Container parentContainer, Simulation simulation, boolean visAvailable) {
    super.configureAndInit(parentContainer, simulation, visAvailable);
    setDescription("Disturber Mote Type #" + getIdentifier());

    Class<? extends MoteInterface>[] moteInterfaces = new Class[2];
    moteInterfaces[0] = Position.class;
    moteInterfaces[1] = DisturberRadio.class;
    setMoteInterfaceClasses(moteInterfaces);

    return true;
  }
}
