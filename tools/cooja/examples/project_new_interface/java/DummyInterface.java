/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import org.contikios.cooja.*;
import org.contikios.cooja.contikimote.ContikiMoteInterface;
import org.contikios.cooja.interfaces.PolledAfterAllTicks;
import org.contikios.cooja.interfaces.PolledBeforeAllTicks;

/**
 * An example of how to implement new mote interfaces.
 *
 * Contiki variables:
 * <ul>
 * <li>char simDummyVar
 * </ul>
 * <p>
 *
 * Core interface:
 * <ul>
 * <li>dummy_interface
 * </ul>
 * <p>
 *
 * This observable never changes.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Dummy Interface")
public class DummyInterface extends MoteInterface implements ContikiMoteInterface, PolledBeforeAllTicks, PolledAfterAllTicks {
  private static Logger logger = Logger.getLogger(DummyInterface.class);

  private Mote mote;
  private SectionMoteMemory memory;

  public DummyInterface(Mote mote) {
    this.mote = mote;
    memory = (SectionMoteMemory) mote.getMemory();
  }

  public static String[] getCoreInterfaceDependencies() {
    // I need the corresponding C dummy interface (in dummy_intf.c)
    return new String[] { "dummy_interface" };
  }

  public void doActionsBeforeTick() {
    logger.debug("Java-part of dummy interface acts BEFORE mote tick: " + memory.getByteValueOf("simDummyVar"));
  }

  public void doActionsAfterTick() {
    byte dummyVal = memory.getByteValueOf("simDummyVar");
    dummyVal++;
    memory.setByteValueOf("simDummyVar", dummyVal);

    logger.debug("Java-part of dummy interface acts AFTER mote tick: " + memory.getByteValueOf("simDummyVar"));
  }

  public JPanel getInterfaceVisualizer() {
    return null; // No visualizer exists
  }

  public void releaseInterfaceVisualizer(JPanel panel) {
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }

}
