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
 * $Id: MspMoteType.java,v 1.37 2010/10/04 22:33:58 nifi Exp $
 */

package se.sics.cooja.mspmote;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;

import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.ClassDescription;
import se.sics.cooja.GUI;
import se.sics.cooja.Mote;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.ProjectConfig;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.mspmote.interfaces.MspSerial;
import se.sics.cooja.util.ArrayUtils;
import se.sics.mspsim.util.DebugInfo;
import se.sics.mspsim.util.ELF;

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

  protected Simulation simulation;

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
    /* TODO Move to emulated layer */
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
      File file = simulation.getGUI().createPortablePath(fileSource);
      element.setText(file.getPath().replaceAll("\\\\", "/"));
      element.setAttribute("EXPORT", "discard");
      config.add(element);
      element = new Element("commands");
      element.setText(compileCommands);
      element.setAttribute("EXPORT", "discard");
      config.add(element);
    }

    // Firmware file
    element = new Element("firmware");
    File file = simulation.getGUI().createPortablePath(fileFirmware);
    element.setText(file.getPath().replaceAll("\\\\", "/"));
    element.setAttribute("EXPORT", "copy");
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
    this.simulation = simulation;

    ArrayList<Class<? extends MoteInterface>> intfClassList = new ArrayList<Class<? extends MoteInterface>>();
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("identifier")) {
        identifier = element.getText();
      } else if (name.equals("description")) {
        description = element.getText();
      } else if (name.equals("source")) {
        fileSource = new File(element.getText());
        if (!fileSource.exists()) {
          fileSource = simulation.getGUI().restorePortablePath(fileSource);
        }
      } else if (name.equals("command")) {
        /* Backwards compatibility: command is now commands */
        logger.warn("Old simulation config detected: old version only supports a single compile command");
        compileCommands = element.getText();
      } else if (name.equals("commands")) {
        compileCommands = element.getText();
      } else if (name.equals("firmware")) {
        fileFirmware = new File(element.getText());
        if (!fileFirmware.exists()) {
          fileFirmware = simulation.getGUI().restorePortablePath(fileFirmware);
        }
      } else if (name.equals("elf")) {
        /* Backwards compatibility: elf is now firmware */
        logger.warn("Old simulation config detected: firmware specified as elf");
        fileFirmware = new File(element.getText());
      } else if (name.equals("moteinterface")) {
        String intfClass = element.getText().trim();

        /* Backwards compatibility: MspIPAddress -> IPAddress */
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.MspIPAddress")) {
          logger.warn("Old simulation config detected: IP address interface was moved");
          intfClass = IPAddress.class.getName();
        }
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.ESBLog")) {
          logger.warn("Old simulation config detected: ESBLog was replaced by MspSerial");
          intfClass = MspSerial.class.getName();
        }
        if (intfClass.equals("se.sics.cooja.mspmote.interfaces.SkySerial")) {
          logger.warn("Old simulation config detected: SkySerial was replaced by MspSerial");
          intfClass = MspSerial.class.getName();
        }
        
        Class<? extends MoteInterface> moteInterfaceClass =
          simulation.getGUI().tryLoadClass(this, MoteInterface.class, intfClass);

        if (moteInterfaceClass == null) {
          logger.warn("Can't find mote interface class: " + intfClass);
        } else {
          intfClassList.add(moteInterfaceClass);
        }
      } else {
        logger.fatal("Unrecognized entry in loaded configuration: " + name);
        throw new MoteTypeCreationException(
            "Unrecognized entry in loaded configuration: " + name);
      }
    }

    Class<? extends MoteInterface>[] intfClasses = intfClassList.toArray(new Class[0]);

    if (intfClasses.length == 0) {
      /* Backwards compatibility: No interfaces specifed */
      logger.warn("Old simulation config detected: no mote interfaces specified, assuming all.");
      intfClasses = getAllMoteInterfaceClasses();
    }
    setMoteInterfaceClasses(intfClasses);

    if (fileFirmware == null) {
      if (fileSource == null) {
        throw new MoteTypeCreationException("Neither source or firmware specified");
      }

      /* Backwards compatibility: Generate expected firmware file name from source */
      logger.warn("Old simulation config detected: no firmware file specified, generating expected");
      fileFirmware = getExpectedFirmwareFile(fileSource);
    }

    return configureAndInit(GUI.getTopParentContainer(), simulation, visAvailable);
  }

  public abstract Class<? extends MoteInterface>[] getAllMoteInterfaceClasses();
  public abstract File getExpectedFirmwareFile(File source);

  private static ELF loadELF(URL url) throws Exception {
    byte[] data = ArrayUtils.readFromStream(url.openStream());
    ELF elf = new ELF(data);
    elf.readAll();
    return elf;
  }

  private static ELF loadELF(String filepath) throws IOException {
    return ELF.readELF(filepath);
  }
  
  private ELF elf; /* cached */
  public ELF getELF() throws IOException {
    if (elf == null) {
      if (GUI.isVisualizedInApplet()) {
        logger.warn("ELF loading in applet not implemented");
      }
      elf = loadELF(getContikiFirmwareFile().getPath());
    }
    return elf;
  }
  
  private Hashtable<File, Hashtable<Integer, Integer>> debuggingInfo = null; /* cached */
  public Hashtable<File, Hashtable<Integer, Integer>> getFirmwareDebugInfo() 
  throws IOException {
    if (debuggingInfo == null) {
      debuggingInfo = getFirmwareDebugInfo(getELF());
    }
    return debuggingInfo;
  }

  public static Hashtable<File, Hashtable<Integer, Integer>> getFirmwareDebugInfo(ELF elf) {
    Hashtable<File, Hashtable<Integer, Integer>> fileToLineHash =
      new Hashtable<File, Hashtable<Integer, Integer>>();

    if (elf.getDebug() == null) {
      // No debug information is available
      return fileToLineHash;
    }

    /* Fetch all executable addresses */
    ArrayList<Integer> addresses = elf.getDebug().getExecutableAddresses();

    for (int address: addresses) {
      DebugInfo info = elf.getDebugInfo(address);

      if (info != null && info.getPath() != null && info.getFile() != null && info.getLine() >= 0) {

        /* Nasty Cygwin-Windows fix */
        String path = info.getPath();
        if (path.contains("/cygdrive/")) {
          int index = path.indexOf("/cygdrive/");
          char driveCharacter = path.charAt(index+10);

          path = path.replace("/cygdrive/" + driveCharacter + "/", driveCharacter + ":/");
        }

        File file = new File(path, info.getFile());
        try {
          file = file.getCanonicalFile();
        } catch (IOException e) {
        } catch (java.security.AccessControlException e) {
        }

        Hashtable<Integer, Integer> lineToAddrHash = fileToLineHash.get(file);
        if (lineToAddrHash == null) {
          lineToAddrHash = new Hashtable<Integer, Integer>();
          fileToLineHash.put(file, lineToAddrHash);
        }

        lineToAddrHash.put(info.getLine(), address);
      }
    }

    return fileToLineHash;
  }

}
