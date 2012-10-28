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
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.io.File;
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
   * @param parentContainer
   *          Parent container for dialog
   */
  public static void showDialog(Container parentContainer) {
    if (GUI.isVisualizedInApplet()) {
      return;
    }

    ExternalToolsDialog myDialog = null;
    if (parentContainer instanceof Window) {
      myDialog = new ExternalToolsDialog((Window) parentContainer);
    } else if (parentContainer instanceof Dialog) {
      myDialog = new ExternalToolsDialog((Dialog) parentContainer);
    } else if (parentContainer instanceof Frame) {
      myDialog = new ExternalToolsDialog((Frame) parentContainer);
    } else {
      logger.fatal("Unknown parent container type: " + parentContainer);
      return;
    }
    myDialog.setLocationRelativeTo(parentContainer);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
  }

  private ExternalToolsDialog(Dialog dialog) {
    super(dialog, "Edit Settings", ModalityType.APPLICATION_MODAL);
    setupDialog();
  }
  private ExternalToolsDialog(Window window) {
    super(window, "Edit Settings", ModalityType.APPLICATION_MODAL);
    setupDialog();
  }
  private ExternalToolsDialog(Frame frame) {
    super(frame, "Edit Settings", ModalityType.APPLICATION_MODAL);
    setupDialog();
  }

  private void setupDialog() {
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

    button = new JButton("Save");
    button.setActionCommand("ok");
    button.addActionListener(myEventHandler);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    // Some explanations
    Box explanations = Box.createVerticalBox();
    explanations.add(new JLabel("Special variables used in COMPILER_ARGS, LINK_COMMAND_[12] and AR_COMMAND_[12]:"));
    String javaHome = System.getenv().get("JAVA_HOME");
    if (javaHome != null) {
      javaHome = javaHome.replace(File.separatorChar, '/');
      JLabel javaHomeLabel = new JLabel("  $(JAVA_HOME) maps to the environment Java home: " + javaHome);
      explanations.add(javaHomeLabel);
    } else {
      javaHome = "[null]";
      JLabel javaHomeLabel = new JLabel("  $(JAVA_HOME) maps to the environment Java home: " + javaHome);
      javaHomeLabel.setForeground(Color.RED);
      explanations.add(javaHomeLabel);
    }
    explanations.add(new JLabel("  $(LIBFILE) maps to the current library file being created (\"mtype1.library\")"));
    explanations.add(new JLabel("  $(MAPFILE) maps to the current map file being created (\"mtype1.map\")"));
    explanations.add(new JLabel("  $(ARFILE) maps to the current archive file being created (\"mtype1.a\")"));
    explanations.setBorder(BorderFactory.createEmptyBorder(5, 5, 5, 5));

    // MAIN PART
    textFields = new JTextField[GUI.getExternalToolsSettingsCount()];
    for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
      // Add text fields for every changeable property
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
    compareWithDefaults();

    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    Container contentPane = getContentPane();
    JScrollPane scrollPane = new JScrollPane(mainPane);
    scrollPane.setPreferredSize(new Dimension(700, 500));
    contentPane.add(explanations, BorderLayout.NORTH);
    contentPane.add(scrollPane, BorderLayout.CENTER);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

    pack();
  }

  private void updateTextFields() {
    for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
      textFields[i].setText(GUI.getExternalToolsSetting(GUI.getExternalToolsSettingName(i), ""));
    }
  }

  private void compareWithDefaults() {
    for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
      String currentValue = textFields[i].getText();

      // Compare with default value
      String defaultValue = GUI.getExternalToolsDefaultSetting(GUI.getExternalToolsSettingName(i), "");
      if (currentValue.equals(defaultValue)) {
        textFields[i].setBackground(Color.WHITE);
        textFields[i].setToolTipText("");
      } else {
        textFields[i].setBackground(Color.LIGHT_GRAY);
        textFields[i].setToolTipText("Default value: " + defaultValue);
      }
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
      compareWithDefaults();
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("reset")) {
        GUI.loadExternalToolsDefaultSettings();
        updateTextFields();
        compareWithDefaults();
      } else if (e.getActionCommand().equals("ok")) {
        for (int i = 0; i < GUI.getExternalToolsSettingsCount(); i++) {
          GUI.setExternalToolsSetting(GUI.getExternalToolsSettingName(i), textFields[i].getText()
              .trim());
        }
        GUI.saveExternalToolsUserSettings();
        if (myDialog != null && myDialog.isDisplayable()) {
          myDialog.dispose();
        }
      } else if (e.getActionCommand().equals("cancel")) {
        if (myDialog != null && myDialog.isDisplayable()) {
          myDialog.dispose();
        }
      } else {
        logger.debug("Unhandled command: " + e.getActionCommand());
      }
    }
  }

}
