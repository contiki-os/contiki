/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 * $Id: SkyCompileDialog.java,v 1.4 2009/08/27 12:25:12 nvt-se Exp $
 */

package se.sics.cooja.mspmote;
import java.awt.Container;
import java.io.File;
import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.dialogs.AbstractCompileDialog;
import se.sics.mspsim.util.ELF;

public class SkyCompileDialog extends AbstractCompileDialog {
  private static Logger logger = Logger.getLogger(SkyCompileDialog.class);

  public static boolean showDialog(
      Container parent,
      Simulation simulation,
      MoteType moteType) {

    final AbstractCompileDialog dialog = new SkyCompileDialog(parent, simulation, moteType);

    /* Show dialog and wait for user */
    dialog.setVisible(true); /* BLOCKS */
    if (!dialog.createdOK()) {
      return false;
    }

    /* Assume that if a firmware exists, compilation was ok */
    return true;
  }

  private SkyCompileDialog(Container parent, Simulation simulation, MoteType moteType) {
    super(parent, simulation, moteType);

    /* Add all available Sky mote interfaces
     * Selected by default unless interfaces already configured */
    boolean selected = true;
    if (moteIntfBox.getComponentCount() > 0) {
      selected = false;
    }

    for (Class<? extends MoteInterface> intfClass: ((SkyMoteType)moteType).getAllMoteInterfaceClasses()) {
      addMoteInterface(intfClass, selected);
    }
  }

  public boolean canLoadFirmware(File file) {
    if (ELF.isELF(file)) {
      return true;
    }
    return false;
  }

  public String getDefaultCompileCommands(File source) {
    /* TODO Split into String[] */
    return
    /*"make clean TARGET=sky\n" + */
    GUI.getExternalToolsSetting("PATH_MAKE") + " " + getExpectedFirmwareFile(source).getName() + " TARGET=sky";
  }

  public File getExpectedFirmwareFile(File source) {
    return ((SkyMoteType)moteType).getExpectedFirmwareFile(source);
  }

  public void writeSettingsToMoteType() {
    /* Nothing to do */
  }
}
