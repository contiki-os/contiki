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
 *
 */

package se.sics.cooja.motes;

import java.awt.Container;
import org.apache.log4j.Logger;
import se.sics.cooja.*;
import se.sics.cooja.interfaces.Position;

@ClassDescription("Dummy Mote Type")
@AbstractionLevelDescription("Application level")
public class DummyMoteType extends AbstractApplicationMoteType {
  private static Logger logger = Logger.getLogger(DummyMoteType.class);

  public DummyMoteType() {
    super();
  }

  public DummyMoteType(String identifier) {
    super(identifier);
    setDescription("Dummy Mote Type #" + getIdentifier());
  }

  public Mote generateMote(Simulation simulation) {
    return new DummyMote(this, simulation);
  }

  public boolean configureAndInit(Container parentContainer, Simulation simulation, boolean visAvailable) {
    super.configureAndInit(parentContainer, simulation, visAvailable);
    setDescription("Dummy Mote Type #" + getIdentifier());
    return true;
  }
}
