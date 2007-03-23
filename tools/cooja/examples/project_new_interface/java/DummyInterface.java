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
 * $Id: DummyInterface.java,v 1.1 2007/03/23 23:33:54 fros4943 Exp $
 */

import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

/**
 * This is an example of how to implement new simulation interfaces.
 * 
 * It needs read/write access to the following core variables:
 * <ul>
 * <li>char simDummyVar
 * </ul>
 * <p>
 * Dependency core interfaces are:
 * <ul>
 * <li>dummy_interface
 * </ul>
 * <p>
 * This observable never changes.
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Dummy Interface")
public class DummyInterface extends MoteInterface {
  private static Logger logger = Logger.getLogger(DummyInterface.class);

  public DummyInterface(Mote mote) {
  }

  public static String[] getCoreInterfaceDependencies() {
    // I need the corresponding C dummy interface (in dummy_intf.c)
    return new String[] { "dummy_interface" };
  }

  public void doActionsBeforeTick() {
    logger.debug("Simulation (Java) dummy interface acts BEFORE mote tick");
  }

  public void doActionsAfterTick() {
    logger.debug("Simulation (Java) dummy interface acts AFTER mote tick");
  }

  public JPanel getInterfaceVisualizer() {
    return null; // No visualizer exists
  }
  
  public void releaseInterfaceVisualizer(JPanel panel) {
  }
  
  public double energyConsumptionPerTick() {
    return 0.0; // I never require any energy
  }

  public Collection<Element> getConfigXML() {
    return null;
  }
  
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }
  
}
