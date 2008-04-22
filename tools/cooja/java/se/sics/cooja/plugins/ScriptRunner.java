/*
 * Copyright (c) 2008, Swedish Institute of Computer Science.
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
 * $Id: ScriptRunner.java,v 1.1 2008/04/22 10:12:16 fros4943 Exp $
 */

package se.sics.cooja.plugins;

import java.awt.BorderLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.*;
import javax.swing.*;
import org.apache.log4j.Logger;
import org.jdom.Element;

import se.sics.cooja.*;

@ClassDescription("Script Runner (Log)")
@PluginType(PluginType.COOJA_PLUGIN)
public class ScriptRunner extends VisPlugin {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ScriptRunner.class);

  private JTextArea scriptTextArea;
  private JTextArea logTextArea;

  private GUI gui;

  private LogScriptEngine scriptTester = null;

  private JButton toggleButton;

  public ScriptRunner(GUI gui) {
    super("Script Runner (Log)", gui);
    this.gui = gui;

    scriptTextArea = new JTextArea(8,50);
    scriptTextArea.setMargin(new Insets(5,5,5,5));
    scriptTextArea.setEditable(true);
    scriptTextArea.setCursor(null);
    scriptTextArea.setText("log.log('ID=' + id + ': ' + msg);");

    logTextArea = new JTextArea(8,50);
    logTextArea.setMargin(new Insets(5,5,5,5));
    logTextArea.setEditable(false);
    logTextArea.setCursor(null);

    toggleButton = new JButton("Activate");
    toggleButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ev) {
        if (toggleButton.getText().equals("Activate")) {
          scriptTester = new LogScriptEngine(ScriptRunner.this.gui, scriptTextArea.getText());
          scriptTester.activateScript();
          scriptTester.setScriptLogObserver(new Observer() {
            public void update(Observable obs, Object obj) {
              logTextArea.append((String) obj);
            }
          });
          toggleButton.setText("Deactivate");
          scriptTextArea.setEnabled(false);

        } else {
          if (scriptTester != null) {
            scriptTester.deactiveScript();
            scriptTester = null;
          }
          toggleButton.setText("Activate");
          scriptTextArea.setEnabled(true);
        }
      }
    });

    JPanel centerPanel = new JPanel(new BorderLayout());
    centerPanel.add(BorderLayout.CENTER, new JScrollPane(scriptTextArea));
    centerPanel.add(BorderLayout.SOUTH, new JScrollPane(logTextArea));

    JPanel buttonPanel = new JPanel(new BorderLayout());
    buttonPanel.add(BorderLayout.EAST, toggleButton);

    getContentPane().add(BorderLayout.CENTER, centerPanel);
    getContentPane().add(BorderLayout.SOUTH, buttonPanel);

    pack();

    try {
      setSelected(true);
    } catch (java.beans.PropertyVetoException e) {
      // Could not select
    }
  }

  public void closePlugin() {
    if (scriptTester != null) {
      scriptTester.deactiveScript();
      scriptTester.setScriptLogObserver(null);
    }
  }

  public Collection<Element> getConfigXML() {
    return null;
  }

  public boolean setConfigXML(Collection<Element> configXML, boolean visAvailable) {
    return true;
  }

}
