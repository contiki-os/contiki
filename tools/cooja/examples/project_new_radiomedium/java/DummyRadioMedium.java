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
 * $Id: DummyRadioMedium.java,v 1.1 2007/03/23 23:33:54 fros4943 Exp $
 */

import java.util.Collection;
import java.util.Observable;
import java.util.Observer;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.interfaces.*;

/**
 * Dummy radio medium. No data is ever transferred through this medium.
 * It also outputs some dummy debug messages during usage.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Dummy Radio Medium")
public class DummyRadioMedium extends RadioMedium {
  private static Logger logger = Logger.getLogger(GUI.class);
  
  public void registerMote(Mote mote, Simulation sim) {
    // Do nothing
    logger.debug("I'm a dummy. Nothing will be registered by me.");
  }

  public void unregisterMote(Mote mote, Simulation sim) {
    // Do nothing
  }

  public void registerRadioInterface(Radio radio, Simulation sim) {
    // Do nothing
    logger.debug("I'm a dummy. Nothing will be registered by me.");
  }

  public void unregisterRadioInterface(Radio radio, Simulation sim) {
    // Do nothing
  }

  public void addRadioMediumObserver(Observer observer) {
    // Do nothing
    logger.debug("I'm a dummy. I will never change.");
  }

  public Observable getRadioMediumObservable() {
    return new Observable();
  }

  public void deleteRadioMediumObserver(Observer observer) {
    // Do nothing
    logger.debug("I'm a dummy. I will never change.");
  }

  public RadioConnection[] getLastTickConnections() {
    return null;
  }

  public void setConnectionLogger(ConnectionLogger connection) {
    // Do nothing
  }

  public Collection<Element> getConfigXML() {
    return null;
  }
  
  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

}
