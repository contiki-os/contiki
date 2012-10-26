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
 * 4. Neither the name of the Institute nor the names of its contributors
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

package se.sics.cooja.dialogs;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.text.NumberFormat;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.InputMap;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDesktopPane;
import javax.swing.JDialog;
import javax.swing.JFormattedTextField;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.KeyStroke;

import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.SimEventCentral;
import se.sics.cooja.Simulation;

public class BufferSettings extends JDialog {
  private static final long serialVersionUID = 7086171115472941104L;
  private static Logger logger = Logger.getLogger(BufferSettings.class);
  private final static Dimension LABEL_SIZE = new Dimension(150, 25);

  private SimEventCentral central;

  public static void showDialog(JDesktopPane parent, Simulation simulation) {
    if (GUI.isVisualizedInApplet()) {
      return;
    }

    BufferSettings dialog = new BufferSettings(simulation);
    dialog.setLocationRelativeTo(parent); 
    dialog.setVisible(true);
  }

  private BufferSettings(Simulation simulation) {
    setTitle("Event buffer settings");
    setResizable(false);
    setModal(true);

    central = simulation.getEventCentral();

    /* Escape key */
    InputMap inputMap = getRootPane().getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
    inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0, false), "dispose");
    getRootPane().getActionMap().put("dispose", disposeAction);

    Box main = Box.createVerticalBox();
    JButton okButton = new JButton(disposeAction);
    getRootPane().setDefaultButton(okButton);

    JFormattedTextField value = addEntry(main, "Log output messages");
    value.setValue(central.getLogOutputBufferSize());
    value.addPropertyChangeListener("value", new PropertyChangeListener() {
      public void propertyChange(PropertyChangeEvent evt) {
        int newVal = ((Number)evt.getNewValue()).intValue();
        if (newVal < 1) {
          newVal = 1;
          ((JFormattedTextField)evt.getSource()).setValue(newVal);
        }
        central.setLogOutputBufferSize(newVal);
      }
    });

    main.add(Box.createVerticalStrut(10));

    Box line = Box.createHorizontalBox();
    line.add(Box.createHorizontalGlue());
    line.add(new JButton(setDefaultAction));
    line.add(okButton);
    main.add(line);

    main.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));
    getContentPane().add(main);
    pack();
  }

  private JFormattedTextField addEntry(JComponent container, String desc) {
    Box box;
    box = Box.createHorizontalBox();
    JLabel label = new JLabel(desc);
    label.setPreferredSize(LABEL_SIZE);
    box.add(label);
    box.add(Box.createHorizontalGlue());
    JFormattedTextField value = new JFormattedTextField(NumberFormat.getIntegerInstance());
    value.setPreferredSize(LABEL_SIZE);
    box.add(value);

    container.add(box);
    return value;
  }

  private Action setDefaultAction = new AbstractAction("Set default") {
    public void actionPerformed(ActionEvent e) {
      Object[] options = { "Ok", "Cancel" };

      String question = "Use current settings as default for future simulations?";
      String title = "Set default?";
      int answer = JOptionPane.showOptionDialog(BufferSettings.this, question, title,
          JOptionPane.DEFAULT_OPTION, JOptionPane.QUESTION_MESSAGE, null,
          options, options[0]);

      if (answer != JOptionPane.YES_OPTION) {
        return;
      }

      GUI.setExternalToolsSetting("BUFFERSIZE_LOGOUTPUT", "" + central.getLogOutputBufferSize());
    }
  };

  private Action disposeAction = new AbstractAction("OK") {
    public void actionPerformed(ActionEvent e) {
      dispose();
    }
  };

}
