/*
 * Copyright (c) 2006, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: DummyMote.java,v 1.8 2009/03/09 15:38:10 fros4943 Exp $
 */

package se.sics.cooja.motes;

import java.util.Random;

import org.apache.log4j.Logger;
import se.sics.cooja.*;
import se.sics.cooja.interfaces.Position;

public class DummyMote extends AbstractApplicationMote {

  private static Logger logger = Logger.getLogger(DummyMote.class);
  private Random random = new Random(); /* XXX Not using Cooja main random generator */

  public DummyMote() {
    super();
  }

  public DummyMote(MoteType moteType, Simulation simulation) {
    super(moteType, simulation);
  }

  public boolean tick(long simTime) {

    /* Dummy task: move randomly */
    if (random.nextDouble() > 0.9) {
      Position pos = getInterfaces().getPosition();
      pos.setCoordinates(
          pos.getXCoordinate() + random.nextDouble() - 0.5,
          pos.getYCoordinate() + random.nextDouble() - 0.5,
          pos.getZCoordinate() + random.nextDouble() - 0.5
      );
    }

    return false;
  }

  public String toString() {
    return "Dummy Mote";
  }
}
