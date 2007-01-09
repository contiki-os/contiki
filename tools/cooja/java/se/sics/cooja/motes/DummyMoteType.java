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
 * $Id: DummyMoteType.java,v 1.2 2007/01/09 10:01:14 fros4943 Exp $
 */

package se.sics.cooja.motes;

import java.util.*;

import javax.swing.*;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

@ClassDescription("Dummy Mote Type")
public class DummyMoteType implements MoteType {
  private static Logger logger = Logger.getLogger(DummyMoteType.class);

  // Mote type specific data
  private String identifier = null;
  private String description = null;

  public DummyMoteType() {
  }

  public DummyMoteType(String identifier) {
    this.identifier = identifier;
    description = "Dummy Mote Type #" + identifier;
  }

  public Mote generateMote(Simulation simulation) {
    return new DummyMote(this, simulation);
  }
  
  public boolean configureAndInit(JFrame parentFrame, Simulation simulation, boolean visAvailable) {
 
    if (identifier == null) {
      // Create unique identifier
      int counter = 0;
      boolean identifierOK = false;
      while (!identifierOK) {
        counter++;
        identifier = "dummy" + counter;
        identifierOK = true;

        // Check if identifier is already used by some other type
        for (MoteType existingMoteType : simulation.getMoteTypes()) {
          if (existingMoteType != this
              && existingMoteType.getIdentifier().equals(identifier)) {
            identifierOK = false;
            break;
          }
        }
      }
      
      if (description == null) {
        // Create description
        description = "Dummy Mote Type #" + counter;
      }

    }

    if (description == null) {
      // Create description
      description = "Dummy Mote Type #" + identifier;
    }

    return true;
  }

  public String getIdentifier() {
    return identifier;
  }

  public void setIdentifier(String identifier) {
    this.identifier = identifier;
  }

  public String getDescription() {
    return description;
  }

  public void setDescription(String description) {
    this.description = description;
  }

  public JPanel getTypeVisualizer() {
    return null;
  }

  public PlatformConfig getConfig() {
    return null;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Identifier
    element = new Element("identifier");
    element.setText(getIdentifier());
    config.add(element);

    // Description
    element = new Element("description");
    element.setText(getDescription());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Simulation simulation, Collection<Element> configXML, boolean visAvailable) {
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("identifier")) {
        identifier = element.getText();
      } else if (name.equals("description")) {
        description = element.getText();
      } else {
        logger.fatal("Unrecognized entry in loaded configuration: " + name);
      }
    }

    boolean createdOK = configureAndInit(GUI.frame, simulation, visAvailable);
    return createdOK;
  }

}
