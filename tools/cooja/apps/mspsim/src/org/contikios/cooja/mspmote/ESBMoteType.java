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
 */

package org.contikios.cooja.mspmote;

import java.awt.Container;
import java.awt.Image;
import java.awt.MediaTracker;
import java.awt.Toolkit;
import java.io.File;
import java.net.URL;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;

import org.apache.log4j.Logger;

import org.contikios.cooja.AbstractionLevelDescription;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.MoteType;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.dialogs.CompileContiki;
import org.contikios.cooja.dialogs.MessageList;
import org.contikios.cooja.dialogs.MessageListText;
import org.contikios.cooja.dialogs.MessageListUI;
import org.contikios.cooja.dialogs.MessageContainer;
import org.contikios.cooja.interfaces.IPAddress;
import org.contikios.cooja.interfaces.Mote2MoteRelations;
import org.contikios.cooja.interfaces.MoteAttributes;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.RimeAddress;
import org.contikios.cooja.mspmote.interfaces.ESBButton;
import org.contikios.cooja.mspmote.interfaces.ESBLED;
import org.contikios.cooja.mspmote.interfaces.MspClock;
import org.contikios.cooja.mspmote.interfaces.MspMoteID;
import org.contikios.cooja.mspmote.interfaces.MspSerial;
import org.contikios.cooja.mspmote.interfaces.TR1001Radio;

@ClassDescription("ESB mote")
@AbstractionLevelDescription("Emulated level")
public class ESBMoteType extends MspMoteType {
  private static Logger logger = Logger.getLogger(ESBMoteType.class);

  public Icon getMoteTypeIcon() {
    Toolkit toolkit = Toolkit.getDefaultToolkit();
    URL imageURL = this.getClass().getClassLoader().getResource("images/esb.jpg");
    Image image = toolkit.getImage(imageURL);
    MediaTracker tracker = new MediaTracker(Cooja.getTopParentContainer());
    tracker.addImage(image, 1);
    try {
      tracker.waitForAll();
    } catch (InterruptedException ex) {
    }
    if (image.getHeight(Cooja.getTopParentContainer()) > 0 && image.getWidth(Cooja.getTopParentContainer()) > 0) {
      image = image.getScaledInstance((100*image.getWidth(Cooja.getTopParentContainer())/image.getHeight(Cooja.getTopParentContainer())), 100, Image.SCALE_DEFAULT);
      return new ImageIcon(image);
    }

    return null;
  }

  protected MspMote createMote(Simulation simulation) {
    return new ESBMote(this, simulation);
  }

  public boolean configureAndInit(Container parentContainer, Simulation simulation, boolean visAvailable)
  throws MoteTypeCreationException {

    /* SPECIAL CASE: Cooja started in applet.
     * Use preconfigured Contiki firmware */
    if (Cooja.isVisualizedInApplet()) {
      String firmware = Cooja.getExternalToolsSetting("ESB_FIRMWARE", "");
      if (!firmware.equals("")) {
        setContikiFirmwareFile(new File(firmware));
        JOptionPane.showMessageDialog(Cooja.getTopParentContainer(),
            "Creating mote type from precompiled firmware: " + firmware,
            "Compiled firmware file available", JOptionPane.INFORMATION_MESSAGE);
      } else {
        JOptionPane.showMessageDialog(Cooja.getTopParentContainer(),
            "No precompiled firmware found",
            "Compiled firmware file not available", JOptionPane.ERROR_MESSAGE);
        return false;
      }
    }

    /* If visualized, show compile dialog and let user configure */
    if (visAvailable) {

      /* Create unique identifier */
      if (getIdentifier() == null) {
        int counter = 0;
        boolean identifierOK = false;
        while (!identifierOK) {
          identifierOK = true;

          counter++;
          setIdentifier("esb" + counter);

          for (MoteType existingMoteType : simulation.getMoteTypes()) {
            if (existingMoteType == this) {
              continue;
            }
            if (existingMoteType.getIdentifier().equals(getIdentifier())) {
              identifierOK = false;
              break;
            }
          }
        }
      }

      /* Create initial description */
      if (getDescription() == null) {
        setDescription("ESB Mote Type #" + getIdentifier());
      }

      return MspCompileDialog.showDialog(parentContainer, simulation, this, "esb");
    }

    /* Not visualized: Compile Contiki immediately */
    if (getIdentifier() == null) {
      throw new MoteTypeCreationException("No identifier");
    }

    final MessageList compilationOutput = visAvailable ? new MessageListUI() : new MessageListText();

    if (getCompileCommands() != null) {
      /* Handle multiple compilation commands one by one */
      String[] arr = getCompileCommands().split("\n");
      for (String cmd: arr) {
        if (cmd.trim().isEmpty()) {
          continue;
        }

        try {
          CompileContiki.compile(
              cmd,
              null,
              null /* Do not observe output firmware file */,
              getContikiSourceFile().getParentFile(),
              null,
              null,
              compilationOutput,
              true
          );
        } catch (Exception e) {
          MoteTypeCreationException newException =
            new MoteTypeCreationException("Mote type creation failed: " + e.getMessage());
          newException = (MoteTypeCreationException) newException.initCause(e);
          newException.setCompilationOutput(compilationOutput);

          /* Print last 10 compilation errors to console */
          MessageContainer[] messages = compilationOutput.getMessages();
          for (int i=messages.length-10; i < messages.length; i++) {
            if (i < 0) {
              continue;
            }
            logger.fatal(">> " + messages[i]);
          }

          logger.fatal("Compilation error: " + e.getMessage());
          throw newException;
        }
      }
    }

    if (getContikiFirmwareFile() == null ||
        !getContikiFirmwareFile().exists()) {
      throw new MoteTypeCreationException("Contiki firmware file does not exist: " + getContikiFirmwareFile());
    }
    return true;
  }

  public Class<? extends MoteInterface>[] getDefaultMoteInterfaceClasses() {
	  return getAllMoteInterfaceClasses();
  }
  public Class<? extends MoteInterface>[] getAllMoteInterfaceClasses() {
    return new Class[] {
        Position.class,
        RimeAddress.class,
        IPAddress.class,
        MspSerial.class,
        MspClock.class,
        ESBLED.class,
        ESBButton.class,
        MspMoteID.class,
        TR1001Radio.class,
        Mote2MoteRelations.class,
        MoteAttributes.class
    };
  }

  public File getExpectedFirmwareFile(File source) {
    File parentDir = source.getParentFile();
    String sourceNoExtension = source.getName().substring(0, source.getName().length()-2);

    return new File(parentDir, sourceNoExtension + ".esb");
  }

  protected String getTargetName() {
  	return "esb";
  }

}
