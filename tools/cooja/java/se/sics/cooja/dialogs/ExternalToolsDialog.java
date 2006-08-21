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
 * $Id: ExternalToolsDialog.java,v 1.1 2006/08/21 12:13:01 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;

/**
 * A dialog for viewing/editing external tools settings.
 * Allows user to change paths and arguments to compilers, linkers etc.
 * 
 * @author Fredrik Osterlind
 */
public class ExternalToolsDialog extends JDialog {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ExternalToolsDialog.class);

  private ExternalToolsEventHandler myEventHandler = new ExternalToolsEventHandler();

  private final static int LABEL_WIDTH = 220;
  private final static int LABEL_HEIGHT = 15;

  private ExternalToolsDialog myDialog;

  private JTextField textFields[];

  /**
   * Creates a dialog for viewing/editing external tools settings.
   * 
   * @param parentFrame
   *          Parent frame for dialog
   */
  public static void showDialog(Frame parentFrame) {
    ExternalToolsDialog myDialog = new ExternalToolsDialog(parentFrame);
    myDialog.setLocationRelativeTo(parentFrame);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
  }

  private ExternalToolsDialog(Frame frame) {
    super(frame, "Edit Settings", true);

    myDialog = this;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;
    JButton button;
    JTextField textField;

    // BOTTOM BUTTON PART
    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    buttonPane.add(Box.createHorizontalGlue());

    button = new JButton("Cancel");
    button.setActionCommand("cancel");
    button.addActionListener(myEventHandler);
    buttonPane.add(button);

    button = new JButton("Reset");
    button.setActionCommand("reset");
    button.addActionListener(myEventHandler);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    button = new JButton("OK (Saves)");
    button.setActionCommand("ok");
    button.addActionListener(myEventHandler);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    // MAIN PART
    textFields = new JTextField[GUI.getExternalToolsSettingsCount()];
    for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
      // Add text fields for every changable property
      smallPane = new JPanel();
      smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
      smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
      label = new JLabel(GUI.getExternalToolsSettingName(i));
      label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

      textField = new JTextField(35);
      textField.setText("");
      textField.addFocusListener(myEventHandler);
      textFields[i] = textField;

      smallPane.add(label);
      smallPane.add(Box.createHorizontalStrut(10));
      smallPane.add(textField);

      mainPane.add(smallPane);
      mainPane.add(Box.createRigidArea(new Dimension(0, 5)));
    }

    // Set actual used values into all text fields
    updateTextFields();

    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    Container contentPane = getContentPane();
    contentPane.add(mainPane, BorderLayout.NORTH);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

    pack();
  }

  private void updateTextFields() {
    for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
      textFields[i].setText(GUI.getExternalToolsSetting(GUI.getExternalToolsSettingName(i), ""));
    }
  }

  private class ExternalToolsEventHandler
      implements
        ActionListener,
        FocusListener {
    public void focusGained(FocusEvent e) {
      // NOP
    }
    public void focusLost(FocusEvent e) {
      // NOP
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("reset")) {
        GUI.loadExternalToolsDefaultSettings();
        updateTextFields();
      } else if (e.getActionCommand().equals("ok")) {
        for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
          GUI.setExternalToolsSetting(GUI.getExternalToolsSettingName(i), textFields[i].getText()
              .trim());
        }
        GUI.saveExternalToolsUserSettings();
        myDialog.dispose();
      } else if (e.getActionCommand().equals("cancel")) {
        myDialog.dispose();
      } else
        logger.debug("Unhandled command: " + e.getActionCommand());
    }
  }

}
