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
 * $Id: Battery.java,v 1.12 2010/02/03 15:49:24 fros4943 Exp $
 */

package se.sics.cooja.interfaces;

import java.util.Collection;

import javax.swing.JPanel;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;

/**
 * A Battery represents the energy source for a mote.
 * The previous battery mote interface implementation was removed, 
 * awaiting a new more powerful design connected to Contiki's power profiler.
 * The current code does not monitor the energy consumption of simulated motes.
 * 
 * @see MoteInterface
 *
 * @author Fredrik Osterlind
 */
@ClassDescription("Battery")
public class Battery extends MoteInterface {
  private static Logger logger = Logger.getLogger(Battery.class);

  /**
   * @param mote Mote
   */
  public Battery(Mote mote) {
  }

  public Collection<Element> getConfigXML() {
    return null;
  }
  public void setConfigXML(Collection<Element> configXML, boolean visAvailable) {
  }
  public JPanel getInterfaceVisualizer() {
    return null;
  }
  public void releaseInterfaceVisualizer(JPanel panel) {
  }

}
