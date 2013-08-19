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
 */

package se.sics.cooja.mspmote;

import java.awt.Container;
import java.io.File;

import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;

import se.sics.cooja.GUI;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.Simulation;
import se.sics.cooja.dialogs.AbstractCompileDialog;

public class MspCompileDialog extends AbstractCompileDialog {
  private static final long serialVersionUID = -7273193946433145019L;
  private final String target;

  public static boolean showDialog(
      Container parent,
      Simulation simulation,
      MspMoteType moteType,
      String target) {

    final AbstractCompileDialog dialog = new MspCompileDialog(parent, simulation, moteType, target);

    /* Show dialog and wait for user */
    dialog.setVisible(true); /* BLOCKS */
    if (!dialog.createdOK()) {
      return false;
    }

    /* Assume that if a firmware exists, compilation was ok */
    return true;
  }

  private MspCompileDialog(Container parent, Simulation simulation, MspMoteType moteType, String target) {
    super(parent, simulation, moteType);
    this.target = target;
    setTitle("Create Mote Type: Compile Contiki for " + target);
    addCompilationTipsTab(tabbedPane);
  }

  public Class<? extends MoteInterface>[] getAllMoteInterfaces() {
	  return ((MspMoteType)moteType).getAllMoteInterfaceClasses();
  }
  public Class<? extends MoteInterface>[] getDefaultMoteInterfaces() {
	  return ((MspMoteType)moteType).getDefaultMoteInterfaceClasses();
  }

  private void addCompilationTipsTab(JTabbedPane parent) {
    JTextArea textArea = new JTextArea();
    textArea.setEditable(false);
    textArea.append("# Without low-power radio:\n" +
    		"DEFINES=NETSTACK_MAC=nullmac_driver,NETSTACK_RDC=nullrdc_noframer_driver,CC2420_CONF_AUTOACK=0\n" +
    		"# (remember to \"make clean\" after changing compilation flags)"
    );

    parent.addTab("Tips", null, new JScrollPane(textArea), "Compilation tips");
  }

  public boolean canLoadFirmware(File file) {
    if (file.getName().endsWith("." + target)) {
      return true;
    }
    if (file.getName().equals("main.exe")) {
      return true;
    }
    return false;
  }

  public String getDefaultCompileCommands(File source) {
    /* TODO Split into String[] */
    return
    GUI.getExternalToolsSetting("PATH_MAKE") + " " +
    getExpectedFirmwareFile(source).getName() + " TARGET=" + target;
  }

  public File getExpectedFirmwareFile(File source) {
    return ((MspMoteType)moteType).getExpectedFirmwareFile(source);
  }

  public void writeSettingsToMoteType() {
    /* Nothing to do */
  }

  protected String getTargetName() {
  	/* Override me */
  	return target;
  }

}
