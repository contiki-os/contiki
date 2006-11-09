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
 * $Id: MantisMoteType.java,v 1.1 2006/11/09 19:31:15 fros4943 Exp $
 */

package se.sics.cooja.mantismote;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.io.File;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;
import se.sics.cooja.contikimote.ContikiMoteType;

/**
 * The Mantis mote type holds the native library used to communicate with an
 * underlying Mantis system. All communication with that system should always
 * pass through this mote type.
 * <p>
 * All core communication with the Mantis mote should be via this class. When a
 * mote type is created it allocates a CoreComm to be used with this type.
 * <p>
 * When a new mote type is created an initialization function is run on the
 * Mantis system in order to create the initial memory. When a new mote is
 * created the createInitialMemory() method should be called to get this initial
 * memory for the mote.
 * 
 * @author Fredrik Osterlind
 */
@ClassDescription("Mantis Mote Type")
public class MantisMoteType implements MoteType {
  private static Logger logger = Logger.getLogger(MantisMoteType.class);
  private Simulation mySimulation = null;

  // Mote type specific information
  private String myIdentifier = null;
  private String myDescription = null;
  private String myObjectFilename = null;
  private SectionMoteMemory myInitialMemory = null;
  private Vector<Class<? extends MoteInterface>> moteInterfaceClasses = null;

  // Core communication variables
  private String libraryClassName = null;
  private int offsetRelToAbs = 0;
  private CoreComm myCoreComm = null;

  // Variable name to address mappings
  private Properties varAddresses = new Properties();

  /**
   * Creates a new uninitialized Mantis mote type. This mote type's doInit
   * method must be called and succeed before it can be used.
   */
  public MantisMoteType() {
  }

  /**
   * Creates a new initialized Mantis mote type. The given library file is
   * loaded by the first available CoreComm. Each mote generated from this mote
   * type will have the interfaces specified in the given mote interface class
   * list.
   * 
   * @param libFile
   *          Library file to load
   * @param objFile
   *          Object file
   * @param moteInterfaceClasses
   *          List of mote interfaces
   */
  public MantisMoteType(File libFile, File objFile,
      Vector<Class<? extends MoteInterface>> moteInterfaceClasses) {
    if (!doInit(libFile, objFile, moteInterfaceClasses))
      logger.fatal("Mantis mote type creation failed!");
  }
 
  /**
   * This is an mote type initialization method and should normally never be
   * called by any other part than the mote type constructor. It is called from
   * the constructor with an identifier argument, but not from the standard
   * constructor. This method may be called from the simulator when loading
   * configuration files, and the libraries must be recompiled.
   * 
   * This method allocates a core communicator, loads the Mantis library file,
   * creates variable name to address mappings and finally creates the Mantis
   * mote initial memory.
   * 
   * @param libFile Library file
   * @param objFile Object file
   * @param moteInterfaceClasses Mote interface classes
   * @return True if initialization ok, false otherwise
   */
  protected boolean doInit(File libFile, File objFile,
        Vector<Class<? extends MoteInterface>> moteInterfaceClasses) {
    myObjectFilename = objFile.getAbsolutePath();
    myIdentifier = libFile.getName();
    myDescription = libFile.getAbsolutePath();
    
    // Allocate core communicator class
    libraryClassName = CoreComm.getAvailableClassName();
    myCoreComm = CoreComm.createCoreComm(libraryClassName, libFile);
    
    // Parse variable name to addresses mappings using nm
    varAddresses.clear();
    Vector<String> nmData = ContikiMoteType.loadNmData(libFile);
    if (nmData == null || !ContikiMoteType.parseNmData(nmData, varAddresses)) {
      logger.fatal("Nm response parsing failed");
      return false;
    }

    // TODO Bug. Both sections sizes must be > 0!

    // Parse section offsets and sizes using objdump
    Vector<String> objdumpData = ContikiMoteType.loadObjdumpData(libFile);
    int relDataSectionAddr = -1;
    int dataSectionSize = -1;
    int relBssSectionAddr = -1;
    int bssSectionSize = -1;
    String dataRegExp = "^[ \t]*[0-9]*[ \t]*.data[ \t]*([0-9A-Fa-f]*)[ \t]*[0-9A-Fa-f]*[ \t]*([0-9A-Fa-f]*)[ \t]*[0-9A-Fa-f]*[ \t]*";
    String bssRegExp = "^[ \t]*[0-9]*[ \t]*.bss[ \t]*([0-9A-Fa-f]*)[ \t]*[0-9A-Fa-f]*[ \t]*([0-9A-Fa-f]*)[ \t]*[0-9A-Fa-f]*[ \t]*";
    Pattern dataPattern = Pattern.compile(dataRegExp);
    Pattern bssPattern = Pattern.compile(bssRegExp);
    Matcher matcher;
    for (String objdumpLine: objdumpData) {
      matcher = dataPattern.matcher(objdumpLine);
      if (matcher.find()) {
        String size = matcher.group(1);
        String offset = matcher.group(2);
        dataSectionSize = Integer.parseInt(size, 16);
        relDataSectionAddr = Integer.parseInt(offset, 16);
      }
      matcher = bssPattern.matcher(objdumpLine);
      if (matcher.find()) {
        String size = matcher.group(1);
        String offset = matcher.group(2);
        bssSectionSize = Integer.parseInt(size, 16);
        relBssSectionAddr = Integer.parseInt(offset, 16);
      }
    }

    if (relDataSectionAddr == -1) {
      logger.fatal("Data section address parsing failed");
      return false;
    }
    if (dataSectionSize == -1) {
      logger.fatal("Data section size parsing failed");
      return false;
    }
    if (relBssSectionAddr == -1) {
      logger.fatal("BSS section address parsing failed");
      return false;
    }
    if (bssSectionSize == -1) {
      logger.fatal("BSS section size parsing failed");
      return false;
    }
    
    // Get offset between relative and absolute addresses
    offsetRelToAbs = myCoreComm.getReferenceAbsAddr() - (Integer) varAddresses.get("referenceVar");

    // Read initial memory from Mantis system
    byte[] initialDataSection = new byte[dataSectionSize];
    myCoreComm.getMemory(relDataSectionAddr + offsetRelToAbs, dataSectionSize, initialDataSection);
    byte[] initialBssSection = new byte[bssSectionSize];
    myCoreComm.getMemory(relBssSectionAddr + offsetRelToAbs, bssSectionSize, initialBssSection);

    // Store initial memory for later use
    myInitialMemory = new SectionMoteMemory(varAddresses);
    myInitialMemory.setMemorySegment(relDataSectionAddr, initialDataSection);
    myInitialMemory.setMemorySegment(relBssSectionAddr, initialBssSection);
    
    this.moteInterfaceClasses = moteInterfaceClasses;
    
    return true;
  }
  
  /**
   * Creates and returns a copy of this mote type's initial memory (just after
   * the init function has been run). When a new mote is created it should get
   * it's memory from here.
   * 
   * @return Initial memory of a mote type
   */
  public SectionMoteMemory createInitialMemory() {
    return myInitialMemory.clone();
  }

  /**
   * Ticks the currently loaded mote. This should not be used directly, but
   * rather via MantisMote.tick().
   */
  public void tick() {
    myCoreComm.tick();
  }

  /**
   * Copy core memory to given memory. This should not be used directly, but
   * instead via MantisMote.getMemory().
   * 
   * @param mem
   *          Memory to set
   */
  public void getCoreMemory(SectionMoteMemory mem) {
    for (int i = 0; i < mem.getNumberOfSections(); i++) {
      int startAddr = mem.getStartAddrOfSection(i);
      int size = mem.getSizeOfSection(i);
      byte[] data = mem.getDataOfSection(i);
      
      getCoreMemory(startAddr + offsetRelToAbs,
          size, data);
    }
  }

  /**
   * Copy given memory to the Mantis system. This should not be used directly,
   * but instead via MantisMote.setMemory().
   * 
   * @param mem
   *          New memory
   */
  public void setCoreMemory(SectionMoteMemory mem) {
    for (int i = 0; i < mem.getNumberOfSections(); i++) {
      setCoreMemory(mem.getStartAddrOfSection(i) + offsetRelToAbs, mem
          .getSizeOfSection(i), mem.getDataOfSection(i));
    }
  }

  private void getCoreMemory(int start, int length, byte[] data) {
    myCoreComm.getMemory(start, length, data);
  }

  private void setCoreMemory(int start, int length, byte[] mem) {
    myCoreComm.setMemory(start, length, mem);
  }


  /**
   * Returns all mote interfaces of this mote type
   * 
   * @return All mote interfaces
   */
  public Vector<Class<? extends MoteInterface>> getMoteInterfaces() {
    return moteInterfaceClasses;
  }

  public String getDescription() {
    return myDescription;
  }

  public void setDescription(String description) {
    myDescription = description;
  }

  public String getIdentifier() {
    return myIdentifier;
  }

  public void setIdentifier(String identifier) {    
    myIdentifier = identifier;
  }

  public String getObjectFilename() {
    return myObjectFilename;
  }

  public void setObjectFilename(String objectFilename) {
    myObjectFilename = objectFilename;
  }

  public JPanel getTypeVisualizer() {
    JPanel panel = new JPanel();
    JLabel label = new JLabel();
    JPanel smallPane;

    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    // Identifier
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Identifier");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(myIdentifier);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Description
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Description");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(myDescription);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Object file
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Object file");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(myObjectFilename);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Library class name
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("JNI Class");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(libraryClassName);
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    panel.add(Box.createRigidArea(new Dimension(0, 5)));
    return panel;
  }

  public PlatformConfig getConfig() {
    logger.debug("MantisMoteType::getConfig");
    return null;
  }

  public Mote generateMote(Simulation simulation) {
    return new MantisMote(this, mySimulation);
  }

  public boolean configureAndInit(JFrame parentFrame, Simulation simulation) {
    return MantisMoteTypeDialog.showDialog(parentFrame, simulation, this);
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

    // Object file
    element = new Element("objectfile");
    element.setText(getObjectFilename());
    config.add(element);

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML) {
    mySimulation = simulation;

    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("identifier")) {
        myIdentifier = element.getText();
      } else if (name.equals("description")) {
        myDescription = element.getText();
      } else if (name.equals("objectfile")) {
        myObjectFilename = element.getText();
      } else {
        logger.fatal("Unrecognized entry in loaded configuration: " + name);
      }
    }

    boolean createdOK = configureAndInit(GUI.frame, simulation);
    return createdOK;
  }

}
