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
 * $Id: MspMoteType.java,v 1.27 2009/03/11 08:42:07 fros4943 Exp $
 */

package se.sics.cooja.mspmote;

import java.awt.*;
import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Vector;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.*;

/**
 * MSP430-based mote types emulated in MSPSim.
 *
 * @see SkyMoteType
 * @see ESBMoteType
 *
 * @author Fredrik Osterlind, Joakim Eriksson, Niclas Finne
 */
@ClassDescription("Msp Mote Type")
public abstract class MspMoteType implements MoteType {
  private static Logger logger = Logger.getLogger(MspMoteType.class);

  private String identifier = null;
  private String description = null;

  /* If source file is defined, the firmware is recompiled when loading simulations */
  private File fileSource = null;
  private String compileCommands = null;
  private File fileFirmware = null;

  private Class<? extends MoteInterface>[] moteInterfaceClasses = null;

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

  public String getCompileCommands() {
    return compileCommands;
  }

  public void setCompileCommands(String commands) {
    this.compileCommands = commands;
  }

  public File getContikiSourceFile() {
    return fileSource;
  }

  public File getContikiFirmwareFile() {
    return fileFirmware;
  }

  public void setContikiSourceFile(File file) {
    fileSource = file;
  }

  public void setContikiFirmwareFile(File file) {
    this.fileFirmware = file;
  }

  public Class<? extends MoteInterface>[] getMoteInterfaceClasses() {
    return moteInterfaceClasses;
  }

  public void setMoteInterfaceClasses(Class<? extends MoteInterface>[] classes) {
    moteInterfaceClasses = classes;
  }

  public final Mote generateMote(Simulation simulation) {
    MspMote mote = createMote(simulation);
    mote.initMote();
    return mote;
  }

  protected abstract MspMote createMote(Simulation simulation);

  public JPanel getTypeVisualizer() {
    JPanel panel = new JPanel();
    JLabel label = new JLabel();
    JPanel smallPane;

    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    // Identifier
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Identifier");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(getIdentifier());
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Description
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Description");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(getDescription());
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    /* Contiki source */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Contiki source");
    smallPane.add(BorderLayout.WEST, label);
    if (getContikiSourceFile() != null) {
      label = new JLabel(getContikiSourceFile().getName());
      label.setToolTipText(getContikiSourceFile().getPath());
    } else {
      label = new JLabel("[not specified]");
    }
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    /* Contiki firmware */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Contiki firmware");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(getContikiFirmwareFile().getName());
    label.setToolTipText(getContikiFirmwareFile().getPath());
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    /* Compile commands */
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Compile commands");
    smallPane.add(BorderLayout.WEST, label);
    JTextArea textArea = new JTextArea(getCompileCommands());
    textArea.setEditable(false);
    textArea.setBorder(BorderFactory.createEmptyBorder());
    smallPane.add(BorderLayout.EAST, textArea);
    panel.add(smallPane);

    /* Icon (if available) */
    if (!GUI.isVisualizedInApplet()) {
      Icon moteTypeIcon = getMoteTypeIcon();
      if (moteTypeIcon != null) {
        smallPane = new JPanel(new BorderLayout());
        label = new JLabel(moteTypeIcon);
        smallPane.add(BorderLayout.CENTER, label);
        panel.add(smallPane);
      }
    }

    panel.add(Box.createRigidArea(new Dimension(0, 5)));
    return panel;
  }

  public abstract Icon getMoteTypeIcon();

  public ProjectConfig getConfig() {
    logger.warn("Msp mote type project config not implemented");
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

    // Source file
    if (fileSource != null) {
      element = new Element("source");
      fileSource = GUI.stripAbsoluteContikiPath(fileSource);
      element.setText(fileSource.getPath().replaceAll("\\\\", "/"));
      config.add(element);
      element = new Element("commands");
      element.setText(compileCommands);
      config.add(element);
    }

    // Firmware file
    element = new Element("firmware");
    fileFirmware = GUI.stripAbsoluteContikiPath(fileFirmware);
    element.setText(fileFirmware.getPath().replaceAll("\\\\", "/"));
    config.add(element);

    // Mote interfaces
    for (Class moteInterface : getMoteInterfaceClasses()) {
      element = new Element("moteinterface");
      element.setText(moteInterface.getName());
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable)
      throws MoteTypeCreationException {
    ArrayList<Class<? extends MoteInterface>> intfClassList = new ArrayList<Class<? extends MoteInterface>>();
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("identifier")) {
        identifier = element.getText();
      } else if (name.equals("description")) {
        description = element.getText();
      } else if (name.equals("source")) {
        fileSource = new File(element.getText());
      } else if (name.equals("commands")) {
        compileCommands = element.getText();
      } else if (name.equals("firmware")) {
        fileFirmware = new File(element.getText());
      } else if (name.equals("elf")) {
        /* Backwards compatibility: elf is now firmware */
        fileFirmware = new File(element.getText());
      } else if (name.equals("moteinterface")) {
        Class<? extends MoteInterface> moteInterfaceClass =
          simulation.getGUI().tryLoadClass(this, MoteInterface.class, element.getText().trim());

        if (moteInterfaceClass == null) {
          logger.warn("Can't find mote interface class: " + element.getText());
        } else {
          intfClassList.add(moteInterfaceClass);
        }
      } else {
        logger.fatal("Unrecognized entry in loaded configuration: " + name);
        throw new MoteTypeCreationException(
            "Unrecognized entry in loaded configuration: " + name);
      }
    }

    Class<? extends MoteInterface>[] intfClasses = new Class[intfClassList.size()];
    intfClassList.toArray(intfClasses);
    setMoteInterfaceClasses(intfClasses);

    return configureAndInit(GUI.getTopParentContainer(), simulation, visAvailable);
  }
}
