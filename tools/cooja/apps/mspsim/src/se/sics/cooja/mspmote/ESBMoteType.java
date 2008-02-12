/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: ESBMoteType.java,v 1.3 2008/02/12 15:12:38 fros4943 Exp $
 */

package se.sics.cooja.mspmote;

import java.awt.Container;
import java.awt.Image;
import java.awt.MediaTracker;
import java.awt.Toolkit;
import java.net.URL;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;

@ClassDescription("ESB Mote Type")
@AbstractionLevelDescription("Emulated level")
public class ESBMoteType extends MspMoteType {
  private static Logger logger = Logger.getLogger(ESBMoteType.class);

  public static final String target = "esb";
  public static final String targetNice = "ESB";

  public ESBMoteType() {
  }

  public ESBMoteType(String identifier) {
    setIdentifier(identifier);
    setDescription(targetNice + " Mote Type #" + identifier);
  }

  public Icon getMoteTypeIcon() {
    Toolkit toolkit = Toolkit.getDefaultToolkit();
    URL imageURL = this.getClass().getClassLoader().getResource("images/esb.jpg");
    Image image = toolkit.getImage(imageURL);
    MediaTracker tracker = new MediaTracker(GUI.getTopParentContainer());
    tracker.addImage(image, 1);
    try {
      tracker.waitForAll();
    } catch (InterruptedException ex) {
    }
    if (image.getHeight(GUI.getTopParentContainer()) > 0 && image.getWidth(GUI.getTopParentContainer()) > 0) {
      image = image.getScaledInstance((100*image.getWidth(GUI.getTopParentContainer())/image.getHeight(GUI.getTopParentContainer())), 100, Image.SCALE_DEFAULT);
      return new ImageIcon(image);
    }

    return null;
  }

  public Mote generateMote(Simulation simulation) {
    return new ESBMote(this, simulation);
  }

  public boolean configureAndInit(Container parentContainer, Simulation simulation,
      boolean visAvailable) throws MoteTypeCreationException {
    return configureAndInitMspType(parentContainer, simulation, visAvailable, target, targetNice);
  }

}
