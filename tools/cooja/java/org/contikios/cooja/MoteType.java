/*
 * Copyright (c) 2009, Swedish Institute of Computer Science. All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 2. Redistributions in
 * binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution. 3. Neither the name of the
 * Institute nor the names of its contributors may be used to endorse or promote
 * products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.contikios.cooja;

import java.awt.Container;
import java.io.File;
import java.util.Collection;

import javax.swing.JComponent;

import org.jdom.Element;

import org.contikios.cooja.contikimote.ContikiMoteType;
import org.contikios.cooja.dialogs.MessageList;
import org.contikios.cooja.dialogs.MessageListUI;

/**
 * The mote type defines properties common for several motes. These properties
 * may differ between different implementations, but typically includes how a
 * mote is initialized, which hardware peripherals each mote has
 * etc. All simulated motes belongs to one mote type.
 *
 * A mote type may also hold the connection to an underlying simulation
 * framework, such as a compiled Contiki system.
 *
 * @see ContikiMoteType
 * @author Fredrik Osterlind
 */
public interface MoteType {

  /**
   * Returns the mote type description.
   *
   * @return Description
   */
  public String getDescription();

  /**
   * Sets the mote type description.
   *
   * @param description
   *          New description
   */
  public void setDescription(String description);

  /**
   * Returns the mote type identifier.
   *
   * @return Mote type identifier
   */
  public String getIdentifier();

  /**
   * Sets the mote type identifier.
   *
   * @param identifier
   *          New identifier
   */
  public void setIdentifier(String identifier);

  /**
   * Main Contiki source file of mote type.
   * May be null.
   *
   * @return Contiki main process source file.
   * @see #setContikiSourceFile(File)
   */
  public File getContikiSourceFile();

  /**
   * @param file Contiki main process source file.
   * @see #getContikiSourceFile()
   */
  public void setContikiSourceFile(File file);

  /**
   * Compiled Contiki firmware file or library.
   * May be null.
   *
   * @return Contiki firmware file or library.
   * @see #setContikiFirmwareFile(File)
   */
  public File getContikiFirmwareFile();

  /**
   * @param file Contiki firmware file or library.
   * @see #getContikiFirmwareFile()
   */
  public void setContikiFirmwareFile(File file);

  /**
   * Commands used to build the Contiki firmware from the Contiki source.
   * May be null.
   *
   * @return Compile commands used to build firmware
   * @see #setCompileCommands(String)
   * @see #getContikiFirmwareFile()
   * @see #getContikiSourceFile()
   */
  public String getCompileCommands();

  /**
   * @param commands Compile commands
   * @see #getCompileCommands()
   */
  public void setCompileCommands(String commands);

  /**
   * @return Mote interface classes of mote type.
   * @see #setMoteInterfaceClasses(Class[])
   */
  public Class<? extends MoteInterface>[] getMoteInterfaceClasses();

  /**
   * Sets mote interface Java classes of mote type.
   *
   * @param classes Mote interface classes
   */
  public void setMoteInterfaceClasses(Class<? extends MoteInterface>[] classes);

  /**
   * Returns a panel with mote type specific data.
   * May be null.
   *
   * @return Mote type visualizer
   */
  public JComponent getTypeVisualizer();

  /**
   * Returns this mote type's project configuration.
   *
   * @return Project configuration
   */
  public ProjectConfig getConfig();

  /**
   * Generates a mote of this mote type.
   *
   * @param simulation
   *          Simulation that will contain mote
   * @return New mote
   */
  public Mote generateMote(Simulation simulation);

  /**
   * This method configures and initializes a mote type ready to be used. It is
   * called from the simulator when a new mote type is created. It may simply
   * confirm that all settings are valid and return true, or display a dialog
   * allowing a user to manually configure the mote type.
   *
   * This method need normally only be run once per mote type!
   *
   * @param parentContainer
   *          Parent container. May be null if not visualized.
   * @param simulation
   *          Simulation holding (or that should hold) mote type
   * @param visAvailable
   *          True if this method is allowed to show a visualizer
   * @return True if mote type has valid settings and is ready to be used
   */
  public boolean configureAndInit(Container parentContainer, Simulation simulation,
      boolean visAvailable) throws MoteTypeCreationException;

  /**
   * Returns XML elements representing the current config of this mote type.
   * This is fetched by the simulator for example when saving a simulation
   * configuration file. For example a Contiki base directory may be saved.
   *
   * @see #setConfigXML(Simulation, Collection, boolean)
   * @param simulation
   *          Current simulation
   * @return XML elements representing the current mote type's config
   */
  public Collection<Element> getConfigXML(Simulation simulation);

  /**
   * Sets the current mote type config depending on the given XML elements.
   * Observe that this method is responsible for restoring the configuration
   * depending on the given arguments. This may include recompiling and loading
   * libraries.
   *
   * @see #getConfigXML()
   * @param simulation
   *          Simulation that will hold the mote type
   * @param configXML
   *          Config XML elements
   * @param visAvailable
   *          True if this method is allowed to show a visualizer
   * @return True if config was set successfully, false otherwise
   */
  public boolean setConfigXML(
      Simulation simulation, Collection<Element> configXML, boolean visAvailable)
  throws MoteTypeCreationException;

  public static class MoteTypeCreationException extends Exception {
    private static final long serialVersionUID = 7625450894307392953L;

    private MessageList compilationOutput;

    public MoteTypeCreationException(String message) {
      super(message);
    }
    public MoteTypeCreationException(String message, Throwable cause) {
      super(message, cause);
    }
    public boolean hasCompilationOutput() {
      return compilationOutput != null;
    }
    public void setCompilationOutput(MessageList compilationOutput) {
      this.compilationOutput = compilationOutput;
    }
    public MessageList getCompilationOutput() {
      return compilationOutput;
    }
  }

}
