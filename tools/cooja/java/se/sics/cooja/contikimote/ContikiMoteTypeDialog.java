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
 * $Id: ContikiMoteTypeDialog.java,v 1.4 2006/08/22 15:28:18 fros4943 Exp $
 */

package se.sics.cooja.contikimote;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.*;
import java.util.regex.*;
import javax.swing.*;
import javax.swing.event.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.dialogs.UserPlatformsDialog;

/**
 * A dialog for configuring Contiki mote types and compiling Contiki mote type
 * libraries. Allows user to change mote type specific data such as
 * descriptions, which processes should be started at mote initialization and
 * which interfaces the type should support.
 * 
 * The dialog takes a Contiki mote type as argument and pre-selects the values
 * already set in that mote type before showing the dialog. Any changes made to
 * the settings are written to the mote type if the compilation is successful
 * and the user presses OK.
 * 
 * This dialog uses external tools to scan for sources and compile libraries.
 * 
 * @author Fredrik Osterlind
 */
public class ContikiMoteTypeDialog extends JDialog {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ContikiMoteTypeDialog.class);

  private MoteTypeEventHandler myEventHandler = new MoteTypeEventHandler();
  private Thread compilationThread;

  /**
   * Suggested mote type identifier prefix
   */
  public static final String ID_PREFIX = "mtype";

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private ContikiMoteType myMoteType = null;

  private JTextField textID, textOutputFiles, textDescription, textContikiDir,
      textCoreDir, textUserPlatforms;
  private JButton createButton, testButton, rescanButton;

  private JPanel processPanel; // Holds process checkboxes
  private JPanel sensorPanel; // Holds sensor checkboxes
  private JPanel moteInterfacePanel; // Holds mote interface checkboxes
  private JPanel coreInterfacePanel; // Holds core interface checkboxes

  private JPanel entireSensorPane; // Holds entire sensor pane (can be hidden)
  private JPanel entireCoreInterfacePane; // Holds entire core interface pane
  // (can be hidden)

  private boolean settingsOK = false; // Do all settings seem correct?
  private boolean compilationSucceded = false; // Did compilation succeed?
  private boolean libraryCreatedOK = false; // Was a library created?

  private PlatformConfig newMoteTypeConfig = null; // Mote type platform config
  private Vector<File> moteTypeUserPlatforms = new Vector<File>(); // Mote type
  // user
  // platforms

  private Vector<MoteType> allOtherTypes = null; // Used to check for
  // conflicting parameters

  private Frame myParentFrame;
  private ContikiMoteTypeDialog myDialog;

  /**
   * Shows a dialog for configuring a Contiki mote type and compiling the shared
   * library it uses.
   * 
   * @param parentFrame
   *          Parent frame for dialog
   * @param simulation
   *          Simulation holding (or that will hold) mote type
   * @param moteTypeToConfigure
   *          Mote type to configure
   * @return True if compilation succeded and library is ready to be loaded
   */
  public static boolean showDialog(Frame parentFrame, Simulation simulation,
      ContikiMoteType moteTypeToConfigure) {

    final ContikiMoteTypeDialog myDialog = new ContikiMoteTypeDialog(
        parentFrame);

    myDialog.myMoteType = moteTypeToConfigure;
    myDialog.allOtherTypes = simulation.getMoteTypes();

    // Set identifier of mote type
    if (moteTypeToConfigure.getIdentifier() != null) {
      // Identifier already preset, assuming recompilation of mote type library
      // Use preset identifier (read-only)
      myDialog.textID.setText(moteTypeToConfigure.getIdentifier());
      myDialog.textID.setEditable(false);
      myDialog.textID.setEnabled(false);

      // Change title to indicate this is a recompilation
      myDialog.setTitle("Recompile Mote Type");
    } else {
      // Suggest new identifier
      int counter = 0;
      String testIdentifier = "";
      boolean identifierOK = false;
      while (!identifierOK) {
        counter++;
        testIdentifier = ID_PREFIX + counter;
        identifierOK = true;

        // Check if identifier is already used by some other type
        for (MoteType existingMoteType : myDialog.allOtherTypes) {
          if (existingMoteType != myDialog.myMoteType
              && existingMoteType.getIdentifier().equals(testIdentifier)) {
            identifierOK = false;
            break;
          }
        }

        // Check if library file with given identifier has already been loaded
        if (identifierOK
            && CoreComm.hasLibraryFileBeenLoaded(new File(
                ContikiMoteType.tempOutputDirectory.getPath()
                    + File.separatorChar + testIdentifier
                    + ContikiMoteType.librarySuffix))) {
          identifierOK = false;
        }
      }

      myDialog.textID.setText(testIdentifier);

    }

    // Set preset description of mote type
    if (moteTypeToConfigure.getDescription() != null) {
      myDialog.textDescription.setText(moteTypeToConfigure.getDescription());
    }

    // Set preset Contiki base directory of mote type
    if (moteTypeToConfigure.getContikiBaseDir() != null) {
      myDialog.textContikiDir.setText(moteTypeToConfigure.getContikiBaseDir());
    }

    // Set preset Contiki core directory of mote type
    if (moteTypeToConfigure.getContikiCoreDir() != null) {
      myDialog.textCoreDir.setText(moteTypeToConfigure.getContikiCoreDir());
    }

    // Set preset user platform directories of mote type
    if (moteTypeToConfigure.getUserPlatformDirs() != null) {
      myDialog.moteTypeUserPlatforms = moteTypeToConfigure
          .getUserPlatformDirs();
      String userPlatformText = null;
      for (File userPlatform : myDialog.moteTypeUserPlatforms) {
        if (userPlatformText == null)
          userPlatformText = "'" + userPlatform.getPath() + "'";
        else
          userPlatformText += ", '" + userPlatform.getPath() + "'";
      }
      myDialog.textUserPlatforms.setText(userPlatformText);
    }

    // Scan directories for processes, sensors and core interfaces
    // TODO Really do this without starting a separate thread?
    myDialog.updateVisualFields();
    myDialog.rescanDirectories();

    // Select preset processes of mote type
    if (moteTypeToConfigure.getProcesses() != null) {
      for (String presetProcess : moteTypeToConfigure.getProcesses()) {
        // Try to find process in current list
        boolean foundAndSelectedProcess = false;
        for (Component processCheckBox : myDialog.processPanel.getComponents()) {
          if (presetProcess.equals(((JCheckBox) processCheckBox).getText())) {
            ((JCheckBox) processCheckBox).setSelected(true);
            foundAndSelectedProcess = true;
            break;
          }
        }

        // Warn if not found
        if (!foundAndSelectedProcess) {
          logger.warn("Process was not found in current environment: "
              + presetProcess);
        }
      }
    }

    // Select preset sensors
    if (moteTypeToConfigure.getSensors() != null) {
      // Deselect all sensors already automatically selected
      for (Component coreInterfaceCheckBox : myDialog.sensorPanel
          .getComponents()) {
        ((JCheckBox) coreInterfaceCheckBox).setSelected(false);
      }

      for (String presetSensor : moteTypeToConfigure.getSensors()) {
        // Try to find sensor in current list
        boolean foundAndSelectedSensor = false;
        for (Component sensorCheckBox : myDialog.sensorPanel.getComponents()) {
          if (presetSensor.equals(((JCheckBox) sensorCheckBox).getText())) {
            ((JCheckBox) sensorCheckBox).setSelected(true);
            foundAndSelectedSensor = true;
            break;
          }
        }

        // Warn if not found
        if (!foundAndSelectedSensor) {
          logger.warn("Sensor was not found in current environment: "
              + presetSensor);
        }
      }
    }

    // Select preset core interfaces
    if (moteTypeToConfigure.getCoreInterfaces() != null) {
      // Deselect all core interfaces already automatically selected
      for (Component coreInterfaceCheckBox : myDialog.coreInterfacePanel
          .getComponents()) {
        ((JCheckBox) coreInterfaceCheckBox).setSelected(false);
      }

      for (String presetCoreInterface : moteTypeToConfigure.getCoreInterfaces()) {
        // Try to find core interface in current list
        boolean foundAndSelectedCoreInterface = false;
        for (Component coreInterfaceCheckBox : myDialog.coreInterfacePanel
            .getComponents()) {
          if (presetCoreInterface.equals(((JCheckBox) coreInterfaceCheckBox)
              .getText())) {
            ((JCheckBox) coreInterfaceCheckBox).setSelected(true);
            foundAndSelectedCoreInterface = true;
            break;
          }
        }

        // Warn if not found
        if (!foundAndSelectedCoreInterface) {
          logger.warn("Core interface was not found in current environment: "
              + presetCoreInterface);
        }
      }
    }

    // Select preset mote interfaces
    if (moteTypeToConfigure.getMoteInterfaces() != null) {
      // Deselect all mote interfaces already automatically selected
      for (Component moteInterfaceCheckBox : myDialog.moteInterfacePanel
          .getComponents()) {
        ((JCheckBox) moteInterfaceCheckBox).setSelected(false);
      }

      for (Class presetMoteInterface : moteTypeToConfigure.getMoteInterfaces()) {
        // Try to find mote interface in current list
        boolean foundAndSelectedMoteInterface = false;
        for (Component moteInterfaceCheckBox : myDialog.moteInterfacePanel
            .getComponents()) {
          Class moteInterfaceClass = (Class) ((JCheckBox) moteInterfaceCheckBox)
              .getClientProperty("class");

          if (presetMoteInterface == moteInterfaceClass) {
            ((JCheckBox) moteInterfaceCheckBox).setSelected(true);
            foundAndSelectedMoteInterface = true;
            break;
          }
        }

        // Warn if not found
        if (!foundAndSelectedMoteInterface) {
          logger.warn("Mote interface was not found in current environment: "
              + presetMoteInterface);
        }
      }
    }

    // Set position and focus of dialog
    myDialog.pack();
    myDialog.setLocationRelativeTo(parentFrame);
    myDialog.textDescription.requestFocus();
    myDialog.textDescription.select(0, myDialog.textDescription.getText()
        .length());
    myDialog.setVisible(true);

    if (myDialog.myMoteType != null) {
      // Library was compiled and loaded
      return true;
    }
    return false;
  }

  private ContikiMoteTypeDialog(Frame frame) {
    super(frame, "Add Mote Type", true);

    myDialog = this;
    myParentFrame = frame;

    JLabel label;
    JPanel mainPane = new JPanel();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;
    JTextField textField;
    JButton button;

    // BOTTOM BUTTON PART
    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    button = new JButton("Test current settings");
    button.setActionCommand("testsettings");
    button.addActionListener(myEventHandler);
    testButton = button;
    this.getRootPane().setDefaultButton(button);
    buttonPane.add(button);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    buttonPane.add(Box.createHorizontalGlue());

    button = new JButton("Clean intermediate files");
    button.setActionCommand("clean");
    button.addActionListener(myEventHandler);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    button = new JButton("Cancel");
    button.setActionCommand("cancel");
    button.addActionListener(myEventHandler);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    button = new JButton("Create");
    button.setEnabled(libraryCreatedOK);
    button.setActionCommand("create");
    button.addActionListener(myEventHandler);
    createButton = button;
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));
    buttonPane.add(button);

    // MAIN PART

    // Identifier
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Identifier");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();

    textField.setText("");

    textField.getDocument().addDocumentListener(myEventHandler);
    textID = textField;
    label.setLabelFor(textField);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Output filenames
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Output files");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + textID.getText() + ContikiMoteType.mapSuffix
        + ", " + ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + textID.getText() + ContikiMoteType.librarySuffix
        + ", " + ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + textID.getText() + ContikiMoteType.dependSuffix);
    textField.setEnabled(false);
    textOutputFiles = textField;
    label.setLabelFor(textField);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Description
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Description");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();
    textField.setBackground(Color.GREEN);
    textField.setText("[enter description here]");
    textField.getDocument().addDocumentListener(myEventHandler);
    textDescription = textField;
    label.setLabelFor(textField);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Contiki dir
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Contiki 2.x OS path");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText(GUI.getExternalToolsSetting("PATH_CONTIKI"));
    textField.getDocument().addDocumentListener(myEventHandler);
    textContikiDir = textField;
    label.setLabelFor(textField);

    button = new JButton("Browse");
    button.setActionCommand("browsecontiki");
    button.addActionListener(myEventHandler);

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);
    smallPane.add(button);
    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // COOJA core platform dir
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Core platform path");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText(textContikiDir.getText()
        + GUI.getExternalToolsSetting("PATH_COOJA_CORE_RELATIVE"));
    textField.setEditable(false);
    textCoreDir = textField;
    label.setLabelFor(textField);

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // COOJA user platform dir
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Mote type user platforms");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText("");
    textField.setEditable(false);
    textUserPlatforms = textField;
    label.setLabelFor(textField);

    button = new JButton("Manage");
    button.setActionCommand("manageuserplatforms");
    button.addActionListener(myEventHandler);

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);
    smallPane.add(button);
    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Separator
    mainPane.add(new JSeparator());
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Rescan button
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Scan after entering above information");

    rescanButton = new JButton("Scan now");
    rescanButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        rescanDirectories();
      }
    });

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(rescanButton);
    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Separator
    mainPane.add(new JSeparator());
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Processes
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Processes");
    label.setAlignmentX(Component.LEFT_ALIGNMENT);
    label.setAlignmentY(Component.TOP_ALIGNMENT);
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    JPanel processHolder = new JPanel(new BorderLayout());
    processHolder.setAlignmentX(Component.LEFT_ALIGNMENT);
    processHolder.setAlignmentY(Component.TOP_ALIGNMENT);

    button = new JButton("Add process name");
    button.setActionCommand("addprocess");
    button.addActionListener(myEventHandler);
    processHolder.add(BorderLayout.SOUTH, button);

    processPanel = new JPanel();

    processPanel.setLayout(new BoxLayout(processPanel, BoxLayout.Y_AXIS));

    JScrollPane tempPane = new JScrollPane(processPanel);
    tempPane.setPreferredSize(new Dimension(300, 200));
    processHolder.add(BorderLayout.WEST, tempPane);

    label.setLabelFor(processPanel);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(processHolder);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Mote interfaces
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Mote Interfaces");
    label.setAlignmentX(Component.LEFT_ALIGNMENT);
    label.setAlignmentY(Component.TOP_ALIGNMENT);
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    JPanel moteInterfaceHolder = new JPanel(new BorderLayout());
    moteInterfaceHolder.setAlignmentX(Component.LEFT_ALIGNMENT);
    moteInterfaceHolder.setAlignmentY(Component.TOP_ALIGNMENT);

    moteInterfacePanel = new JPanel();

    moteInterfacePanel.setLayout(new BoxLayout(moteInterfacePanel,
        BoxLayout.Y_AXIS));

    tempPane = new JScrollPane(moteInterfacePanel);
    tempPane.setPreferredSize(new Dimension(300, 200));
    moteInterfaceHolder.add(BorderLayout.WEST, tempPane);

    label.setLabelFor(moteInterfacePanel);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(moteInterfaceHolder);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Separator with show advanced checkbox
    JCheckBox showAdvancedCheckBox = new JCheckBox("Show advanced settings");
    showAdvancedCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (((JCheckBox) e.getSource()).isSelected()) {
          if (entireCoreInterfacePane != null)
            entireCoreInterfacePane.setVisible(true);
          if (entireSensorPane != null)
            entireSensorPane.setVisible(true);
        } else {
          if (entireCoreInterfacePane != null)
            entireCoreInterfacePane.setVisible(false);
          if (entireSensorPane != null)
            entireSensorPane.setVisible(false);
        }
      }
    });
    mainPane.add(new JSeparator());
    mainPane.add(showAdvancedCheckBox);
    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Core sensors
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Sensors");
    label.setAlignmentX(Component.LEFT_ALIGNMENT);
    label.setAlignmentY(Component.TOP_ALIGNMENT);
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    JPanel sensorHolder = new JPanel(new BorderLayout());
    sensorHolder.setAlignmentX(Component.LEFT_ALIGNMENT);
    sensorHolder.setAlignmentY(Component.TOP_ALIGNMENT);

    // button = new JButton(GUI.lang.getString("motetype_scansens"));
    // button.setActionCommand("scansensors");
    // button.addActionListener(myEventHandler);
    // sensorHolder.add(BorderLayout.NORTH, button);

    button = new JButton("Add sensor name");
    button.setActionCommand("addsensor");
    button.addActionListener(myEventHandler);
    sensorHolder.add(BorderLayout.SOUTH, button);

    sensorPanel = new JPanel();

    sensorPanel.setLayout(new BoxLayout(sensorPanel, BoxLayout.Y_AXIS));

    JScrollPane tempPane2 = new JScrollPane(sensorPanel);
    tempPane2.setPreferredSize(new Dimension(300, 200));
    sensorHolder.add(BorderLayout.WEST, tempPane2);

    label.setLabelFor(sensorPanel);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(sensorHolder);

    mainPane.add(smallPane);
    entireSensorPane = smallPane;
    entireSensorPane.setVisible(false);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Core interfaces
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Core Interfaces");
    label.setAlignmentX(Component.LEFT_ALIGNMENT);
    label.setAlignmentY(Component.TOP_ALIGNMENT);
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    JPanel interfaceHolder = new JPanel(new BorderLayout());
    interfaceHolder.setAlignmentX(Component.LEFT_ALIGNMENT);
    interfaceHolder.setAlignmentY(Component.TOP_ALIGNMENT);

    button = new JButton("Add interface name");
    button.setActionCommand("addinterface");
    button.addActionListener(myEventHandler);
    interfaceHolder.add(BorderLayout.SOUTH, button);

    coreInterfacePanel = new JPanel();

    coreInterfacePanel.setLayout(new BoxLayout(coreInterfacePanel,
        BoxLayout.Y_AXIS));

    JScrollPane tempPane3 = new JScrollPane(coreInterfacePanel);
    tempPane3.setPreferredSize(new Dimension(300, 200));
    interfaceHolder.add(BorderLayout.WEST, tempPane3);

    label.setLabelFor(coreInterfacePanel);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(interfaceHolder);

    mainPane.add(smallPane);
    entireCoreInterfacePane = smallPane;
    entireCoreInterfacePane.setVisible(false);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Add everything!
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    Container contentPane = getContentPane();
    JScrollPane mainScrollPane = new JScrollPane(mainPane);
//     mainScrollPane.setPreferredSize(new Dimension(
//         mainPane.getPreferredSize().width + 50, 500));
    contentPane.add(mainScrollPane, BorderLayout.CENTER);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

//     pack();
  }

  /**
   * Checks which core interfaces are needed by the currently selected mote
   * interfaces, and selects only them.
   */
  private void recheckInterfaceDependencies() {

    // Unselect all core interfaces
    for (Component checkBox : coreInterfacePanel.getComponents()) {
      ((JCheckBox) checkBox).setSelected(false);
    }

    // Loop through all mote interfaces
    for (Component checkBox : moteInterfacePanel.getComponents()) {
      JCheckBox moteIntfCheckBox = (JCheckBox) checkBox;

      // If the mote interface is selected, select needed core interfaces
      if (moteIntfCheckBox.isSelected()) {
        String[] neededCoreInterfaces = null;

        // Get needed core interfaces (if any)
        try {
          Class moteInterfaceClass = (Class) moteIntfCheckBox
              .getClientProperty("class");
          if (ContikiMoteInterface.class.isAssignableFrom(moteInterfaceClass)) {
            Method m = moteInterfaceClass.getDeclaredMethod(
                "getCoreInterfaceDependencies", (Class[]) null);
            neededCoreInterfaces = (String[]) m.invoke(null, (Object[]) null);
          }
        } catch (NoSuchMethodException e) {
          logger.warn("Can't read core interface dependencies of "
              + moteIntfCheckBox.getText() + ", assuming no core dependencies");
        } catch (InvocationTargetException e) {
          logger.warn("Can't read core interface dependencies of "
              + moteIntfCheckBox.getText() + ": " + e);
        } catch (IllegalAccessException e) {
          logger.warn("Can't read core interface dependencies of "
              + moteIntfCheckBox.getText() + ": " + e);
        }

        // If needed core interfaces found, select them
        if (neededCoreInterfaces != null) {
          // Loop through all needed core interfaces
          for (String neededCoreInterface : neededCoreInterfaces) {
            int coreInterfacePosition = -1;

            // Check that the core interface actually exists
            for (int j = 0; j < coreInterfacePanel.getComponentCount(); j++) {
              JCheckBox coreCheckBox = (JCheckBox) coreInterfacePanel
                  .getComponent(j);

              if (coreCheckBox.getText().equals(neededCoreInterface)) {
                coreInterfacePosition = j;
                coreCheckBox.setSelected(true);
                break;
              }
            }

            // Was the core interface found?
            if (coreInterfacePosition < 0) {
              logger.warn("Warning! " + moteIntfCheckBox.getText()
                  + " needs non-existing core interface " + neededCoreInterface
                  + " (rescan?)");
            }
          }
        }

      }
    }
  }

  /**
   * Tries to compile library using current settings.
   */
  public void doTestSettings() {
    libraryCreatedOK = false;

    JPanel progressPanel = new JPanel(new BorderLayout());
    final JDialog progressDialog = new JDialog(myDialog, null);
    JProgressBar progressBar;
    JButton button;
    final MessageList taskOutput;
    progressDialog.setLocationRelativeTo(myDialog);

    progressBar = new JProgressBar(0, 100);
    progressBar.setValue(0);
    progressBar.setStringPainted(true);
    progressBar.setIndeterminate(true);

    taskOutput = new MessageList();

    button = new JButton("Close/Abort");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (compilationThread != null && compilationThread.isAlive()) {
          compilationThread.interrupt();
        }
        progressDialog.dispose();
      }
    });

    progressPanel.add(BorderLayout.CENTER, new JScrollPane(taskOutput));
    progressPanel.add(BorderLayout.NORTH, progressBar);
    progressPanel.add(BorderLayout.SOUTH, button);
    progressPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));
    progressPanel.setVisible(true);

    progressDialog.getContentPane().add(progressPanel);
    progressDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
    progressDialog.pack();

    progressDialog.setVisible(true);

    // Create temp output directory if not already exists
    if (!ContikiMoteType.tempOutputDirectory.exists())
      ContikiMoteType.tempOutputDirectory.mkdir();

    // Generate main contiki source file
    String errorMessage = generateSourceFile();
    if (errorMessage != null) {
      libraryCreatedOK = false;
      progressBar.setBackground(Color.ORANGE);
      progressBar.setString("Maximum number of mote types already exist");
      progressBar.setIndeterminate(false);
      progressBar.setValue(0);
      createButton.setEnabled(libraryCreatedOK);
      return;
    }

    // Test compile shared library
    progressBar.setString("..compiling..");
    final File contikiDir = new File(textContikiDir.getText());
    final String identifier = textID.getText();
    File libFile = new File(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + identifier + ContikiMoteType.librarySuffix);
    File mapFile = new File(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + identifier + ContikiMoteType.mapSuffix);
    File depFile = new File(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + identifier + ContikiMoteType.dependSuffix);

    if (libFile.exists()) {
      libFile.delete();
    }

    if (depFile.exists()) {
      depFile.delete();
    }

    if (mapFile.exists()) {
      mapFile.delete();
    }

    compilationThread = new Thread(new Runnable() {
      public void run() {
        // Add all user platform directories
        Vector<File> filesToCompile = (Vector<File>) GUI.currentGUI
            .getUserPlatforms().clone();
        filesToCompile.addAll(moteTypeUserPlatforms);

        // Add source files from platform configs
        String[] projectSourceFiles = newMoteTypeConfig.getStringArrayValue(
            ContikiMoteType.class, "C_SOURCES");
        for (String projectSourceFile : projectSourceFiles) {
          if (!projectSourceFile.equals("")) {
            File file = new File(projectSourceFile);
            if (file.getParent() != null) {
              // Find which user platform added this file
              File userPlatform = newMoteTypeConfig.getUserPlatformDefining(
                  ContikiMoteType.class, "C_SOURCES", projectSourceFile);
              if (userPlatform != null) {
                // We found a user platform - Add directory
                filesToCompile.add(new File(userPlatform.getPath(),
                    file.getParent()));
              }
            }
            filesToCompile.add(new File(file.getName()));
          }
        }

        // Add selected process source files
        for (Component checkBox : processPanel.getComponents()) {
          if (((JCheckBox) checkBox).isSelected()) {
            String processName = ((JCheckBox) checkBox).getToolTipText();
            if (processName != null) {
              filesToCompile.add(new File(processName));
            }
          }
        }

        compilationSucceded = ContikiMoteTypeDialog.compileLibrary(identifier,
            contikiDir, filesToCompile, taskOutput);
      }
    }, "compilation thread");
    compilationThread.start();

    while (compilationThread.isAlive()) {
      try {
        Thread.sleep(100);
      } catch (InterruptedException e) {
        // NOP
      }
    }

    if (!compilationSucceded) {
      if (libFile.exists()) {
        libFile.delete();
      }
      if (depFile.exists()) {
        depFile.delete();
      }
      if (mapFile.exists()) {
        mapFile.delete();
      }
      libraryCreatedOK = false;
    } else {
      libraryCreatedOK = true;
      if (!libFile.exists() || !depFile.exists() || !mapFile.exists())
        libraryCreatedOK = false;
    }

    if (libraryCreatedOK) {
      progressBar.setBackground(Color.GREEN);
      progressBar.setString("compilation succeded");
    } else {
      progressBar.setBackground(Color.ORANGE);
      progressBar.setString("compilation failed");
    }
    progressBar.setIndeterminate(false);
    progressBar.setValue(0);
    createButton.setEnabled(libraryCreatedOK);
  }

  /**
   * @return Error message or null if source file generation ok
   */
  private String generateSourceFile() {

    // SENSORS
    String sensorString = "";
    String externSensorDefs = "";
    for (Component checkBox : sensorPanel.getComponents()) {
      if (((JCheckBox) checkBox).isSelected()) {
        if (!sensorString.equals(""))
          sensorString = sensorString.concat(", ");
        sensorString = sensorString.concat("&"
            + ((JCheckBox) checkBox).getText());
        externSensorDefs = externSensorDefs
            .concat("extern const struct sensors_sensor "
                + ((JCheckBox) checkBox).getText() + ";\n");
      }
    }

    if (!sensorString.equals(""))
      sensorString = "SENSORS(" + sensorString + ");";
    else
      sensorString = "SENSORS(NULL);";

    // CORE INTERFACES
    String externInterfaceDefs = "";
    String interfaceString = "";
    for (Component checkBox : coreInterfacePanel.getComponents()) {
      if (((JCheckBox) checkBox).isSelected()) {
        if (!interfaceString.equals(""))
          interfaceString = interfaceString.concat(", ");
        interfaceString = interfaceString.concat("&"
            + ((JCheckBox) checkBox).getText());
        externInterfaceDefs = externInterfaceDefs.concat("SIM_INTERFACE_NAME("
            + ((JCheckBox) checkBox).getText() + ");\n");
      }
    }

    if (!interfaceString.equals(""))
      interfaceString = "SIM_INTERFACES(" + interfaceString + ");";
    else
      interfaceString = "SIM_INTERFACES(NULL);";

    // PROCESSES
    String userProcessString = "";
    String externProcessDefs = "";
    for (Component checkBox : processPanel.getComponents()) {
      if (((JCheckBox) checkBox).isSelected()) {
        if (!userProcessString.equals(""))
          userProcessString = userProcessString.concat(", ");
        userProcessString = userProcessString.concat("&"
            + ((JCheckBox) checkBox).getText());
        externProcessDefs = externProcessDefs.concat("PROCESS_NAME("
            + ((JCheckBox) checkBox).getText() + ");\n");
      }
    }

    String coreProcessString = "";
    String processArray[] = GUI.getExternalToolsSetting(
        "CONTIKI_STANDARD_PROCESSES").split(";");
    for (String processString : processArray) {
      if (!coreProcessString.equals(""))
        coreProcessString = coreProcessString.concat(", ");
      coreProcessString = coreProcessString + "&" + processString;
    }

    if (userProcessString.equals("")) {
      logger
          .warn("No application processes specified! Sure you don't want any?");
    }

    String processString;
    if (!coreProcessString.equals(""))
      processString = "PROCINIT(" + coreProcessString + ");";
    else
      processString = "PROCINIT(NULL);";

    if (!userProcessString.equals(""))
      processString = processString + "\nAUTOSTART_PROCESSES("
          + userProcessString + ");";
    else
      processString = processString.concat("\nAUTOSTART_PROCESSES(NULL);");

    // JNI CLASS NAME
    String libString = CoreComm.getAvailableClassName();
    if (libString == null) {
      logger.fatal("No more libraries can be loaded!");
      return "Maximum number of mote types already exist";
    }

    // Create new source file and replace special fields
    try {
      Reader reader;
      String mainTemplate = GUI
          .getExternalToolsSetting("CONTIKI_MAIN_TEMPLATE_FILENAME");
      if ((new File(mainTemplate)).exists()) {
        reader = new FileReader(mainTemplate);
      } else {
        InputStream input = ContikiMoteTypeDialog.class
            .getResourceAsStream('/' + mainTemplate);
        if (input == null) {
          throw new FileNotFoundException(mainTemplate + " not found");
        }
        reader = new InputStreamReader(input);
      }

      BufferedReader sourceFile = new BufferedReader(reader);
      BufferedWriter destFile = new BufferedWriter(new OutputStreamWriter(
          new FileOutputStream(ContikiMoteType.tempOutputDirectory.getPath()
              + File.separatorChar + textID.getText() + ".c")));
      String line;
      while ((line = sourceFile.readLine()) != null) {
        line = line
            .replaceFirst("\\[PROCESS_DEFINITIONS\\]", externProcessDefs);
        line = line.replaceFirst("\\[PROCESS_ARRAY\\]", processString);

        line = line.replaceFirst("\\[SENSOR_DEFINITIONS\\]", externSensorDefs);
        line = line.replaceFirst("\\[SENSOR_ARRAY\\]", sensorString);

        line = line.replaceFirst("\\[INTERFACE_DEFINITIONS\\]",
            externInterfaceDefs);
        line = line.replaceFirst("\\[INTERFACE_ARRAY\\]", interfaceString);

        line = line.replaceFirst("\\[CLASS_NAME\\]", libString);
        destFile.write(line + "\n");
      }

      destFile.close();
      sourceFile.close();
    } catch (Exception e) {
      logger.debug("Exception " + e);
      return "Exception " + e;
    }

    return null;
  }

  /**
   * Compiles a mote type shared library using the standard Contiki makefile.
   * 
   * @param identifier
   *          Mote type identifier
   * @param contikiDir
   *          Contiki base directory
   * @param sourceFiles
   *          Source files and directories to include in compilation
   * @param appender
   *          Component to append process output to (optional)
   * @return True if compilation succeded, false otherwise
   */
  public static boolean compileLibrary(String identifier, File contikiDir,
      Vector<File> sourceFiles, final MessageList appender) {

    File libFile = new File(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + identifier + ContikiMoteType.librarySuffix);
    File mapFile = new File(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + identifier + ContikiMoteType.mapSuffix);
    File depFile = new File(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + identifier + ContikiMoteType.dependSuffix);

    // Recheck that contiki path exists
    if (!contikiDir.exists()) {
      appender.addMessage("Bad Contiki OS path", MessageList.ERROR);
      logger.fatal("Contiki path does not exist");
      return false;
    }
    if (!contikiDir.isDirectory()) {
      appender.addMessage("Bad Contiki OS path", MessageList.ERROR);
      logger.fatal("Contiki path is not a directory");
      return false;
    }

    if (libFile.exists()) {
      appender.addMessage("Bad output filenames", MessageList.ERROR);
      logger.fatal("Could not overwrite already existing library");
      return false;
    }

    if (CoreComm.hasLibraryFileBeenLoaded(libFile)) {
      appender.addMessage("Bad output filenames", MessageList.ERROR);
      logger
          .fatal("A library has already been loaded with the same name before");
      return false;
    }

    if (depFile.exists()) {
      appender.addMessage("Bad output filenames", MessageList.ERROR);
      logger.fatal("Could not overwrite already existing dependency file");
      return false;
    }

    if (mapFile.exists()) {
      appender.addMessage("Bad output filenames", MessageList.ERROR);
      logger.fatal("Could not overwrite already existing map file");
      return false;
    }

    try {
      // Let Contiki 2.x regular make file compile
      String[] cmd = new String[]{
          GUI.getExternalToolsSetting("PATH_MAKE"),
          libFile.getPath().replace(File.separatorChar, '/'),
          "-f",
          contikiDir.getPath().replace(File.separatorChar, '/')
              + "/Makefile.include"};

      String sourceDirs = System.getProperty("PROJECTDIRS", "");
      String sourceFileNames = "";
      String ccFlags = GUI.getExternalToolsSetting("COMPILER_ARGS", "");

      for (File sourceFile : sourceFiles) {
        if (sourceFile.isDirectory()) {
          // Add directory to search path
          sourceDirs += " "
              + sourceFile.getPath().replace(File.separatorChar, '/');
          ccFlags += " -I" + sourceFile.getPath().replace(File.separatorChar, '/');
        } else if (sourceFile.isFile()) {
          // Add both file name and directory
          if (sourceFile.getParent() != null) {
            sourceDirs += " " +
            sourceFile.getParent().replace(File.separatorChar, '/');
          }
          sourceFileNames += " " + sourceFile.getName();
        } else {
          // Add filename and hope Contiki knows where to find it...
          sourceFileNames += " " + sourceFile.getName();
        }
      }

      logger.info("Project dirs: " + sourceDirs);
      logger.info("Project sources: " + sourceFileNames);
      logger.info("Compiler flags: " + ccFlags);
      
      String[] env = new String[]{
          "CONTIKI=" + contikiDir.getPath().replace(File.separatorChar, '/'),
          "TARGET=cooja", "TYPEID=" + identifier,
          "LD_ARGS_1=" + GUI.getExternalToolsSetting("LINKER_ARGS_1", ""),
          "LD_ARGS_2=" + GUI.getExternalToolsSetting("LINKER_ARGS_2", ""),
          "EXTRA_CC_ARGS=" + ccFlags,
          "CC=" + GUI.getExternalToolsSetting("PATH_C_COMPILER"),
          "LD=" + GUI.getExternalToolsSetting("PATH_LINKER"), "COMPILE_MAIN=1",
          "PROJECTDIRS=" + sourceDirs,
          "PROJECT_SOURCEFILES=" + sourceFileNames,
          "PATH=" + System.getenv("PATH")};

      Process p = Runtime.getRuntime().exec(cmd, env, null);

      final BufferedReader input = new BufferedReader(new InputStreamReader(p
          .getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(p
          .getErrorStream()));

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = input.readLine()) != null) {
              if (appender != null && readLine != null) {
                appender.addMessage(readLine);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read input stream thread");

      Thread readError = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = err.readLine()) != null) {
              if (appender != null && readLine != null) {
                appender.addMessage(readLine, MessageList.ERROR);
              }
            }
          } catch (IOException e) {
            logger.warn("Error while reading from process");
          }
        }
      }, "read input stream thread");

      readInput.start();
      readError.start();

      while (readInput.isAlive() || readError.isAlive()) {
        Thread.sleep(100);
      }

      input.close();
      err.close();

      p.waitFor();
      if (p.exitValue() != 0)
        return false;
    } catch (Exception e) {
      logger.fatal("Error while compiling library: " + e);
      return false;
    }
    return true;
  }

  /**
   * Scans a directory for sourcefiles which defines a Contiki process.
   * 
   * @param rootDirectory
   *          Top directory to search in
   * @return Process definitions found under rootDirectory, {sourcefile,
   *         processname}
   */
  public static Vector<String[]> scanForProcesses(File rootDirectory) {
    if (!rootDirectory.isDirectory()) {
      logger.fatal("Not a directory: " + rootDirectory);
      return null;
    }

    if (!rootDirectory.exists()) {
      logger.fatal("Does not exist: " + rootDirectory);
      return null;
    }

    Vector<String[]> processes = new Vector<String[]>();

    // Scan in rootDirectory
    try {
      String line;
      String cmdString = GUI.getExternalToolsSetting("CMD_GREP_PROCESSES")
          + " '" + rootDirectory.getPath().replace(File.separatorChar, '/')
          + "'/*.[ch]";
      Pattern pattern = Pattern.compile(GUI
          .getExternalToolsSetting("REGEXP_PARSE_PROCESSES"));

      String[] cmd = new String[3];
      cmd[0] = GUI.getExternalToolsSetting("PATH_SHELL");
      cmd[1] = "-c";
      cmd[2] = cmdString;

      Process p = Runtime.getRuntime().exec(cmd);
      BufferedReader input = new BufferedReader(new InputStreamReader(p
          .getInputStream()));
      while ((line = input.readLine()) != null) {
        Matcher matcher = pattern.matcher(line);
        if (matcher.find()) {
          processes.add(new String[]{matcher.group(1), matcher.group(2)});
        }
      }
      input.close();

      BufferedReader err = new BufferedReader(new InputStreamReader(p
          .getErrorStream()));
      if (err.ready())
        logger.warn("Error occured during scan:");
      while ((line = err.readLine()) != null) {
        logger.warn(line);
      }
      err.close();
    } catch (IOException err) {
      logger.fatal("Error while scanning for processes: " + err);
      err.printStackTrace();
    } catch (Exception err) {
      logger.fatal("Error while scanning for processes: " + err);
      err.printStackTrace();
    }
    return processes;
  }

  /**
   * Scans a directory and all subdirectories for sourcefiles which defines a
   * Contiki sensor.
   * 
   * @param rootDirectory
   *          Top directory to search in
   * @return Sensor definitions found under rootDirectory, {sourcefile,
   *         sensorname}
   */
  public static Vector<String[]> scanForSensors(File rootDirectory) {
    if (!rootDirectory.isDirectory()) {
      logger.fatal("Not a directory: " + rootDirectory);
      return null;
    }

    if (!rootDirectory.exists()) {
      logger.fatal("Does not exist: " + rootDirectory);
      return null;
    }

    Vector<String[]> sensors = new Vector<String[]>();

    // Scan in rootDirectory
    try {
      String line;
      String cmdString = GUI.getExternalToolsSetting("CMD_GREP_SENSORS") + " '"
          + rootDirectory.getPath().replace(File.separatorChar, '/') + "'";
      Pattern pattern = Pattern.compile(GUI
          .getExternalToolsSetting("REGEXP_PARSE_SENSORS"));

      String[] cmd = new String[3];
      cmd[0] = GUI.getExternalToolsSetting("PATH_SHELL");
      cmd[1] = "-c";
      cmd[2] = cmdString;

      Process p = Runtime.getRuntime().exec(cmd);
      BufferedReader input = new BufferedReader(new InputStreamReader(p
          .getInputStream()));
      while ((line = input.readLine()) != null) {
        Matcher matcher = pattern.matcher(line);
        if (matcher.find()) {
          sensors.add(new String[]{matcher.group(1), matcher.group(2)});
        }
      }
      input.close();

      BufferedReader err = new BufferedReader(new InputStreamReader(p
          .getErrorStream()));
      if (err.ready())
        logger.warn("Error occured during scan:");
      while ((line = err.readLine()) != null) {
        logger.warn(line);
      }
      err.close();
    } catch (IOException err) {
      logger.fatal("Error while scanning for sensors: " + err);
      err.printStackTrace();
    } catch (Exception err) {
      logger.fatal("Error while scanning for sensors: " + err);
      err.printStackTrace();
    }
    return sensors;
  }

  /**
   * Scans a directory and all subdirectories for sourcefiles which defines a
   * COOJA core interface.
   * 
   * @param rootDirectory
   *          Top directory to search in
   * @return Core interface definitions found under rootDirectory, {sourcefile,
   *         interfacename}
   */
  public static Vector<String[]> scanForInterfaces(File rootDirectory) {
    if (!rootDirectory.isDirectory()) {
      logger.fatal("Not a directory: " + rootDirectory);
      return null;
    }

    if (!rootDirectory.exists()) {
      logger.fatal("Does not exist: " + rootDirectory);
      return null;
    }

    Vector<String[]> interfaces = new Vector<String[]>();

    // Scan in rootDirectory
    try {
      String line;
      String cmdString = GUI.getExternalToolsSetting("CMD_GREP_INTERFACES")
          + " '" + rootDirectory.getPath().replace(File.separatorChar, '/')
          + "'";
      Pattern pattern = Pattern.compile(GUI
          .getExternalToolsSetting("REGEXP_PARSE_INTERFACES"));

      String[] cmd = new String[3];
      cmd[0] = GUI.getExternalToolsSetting("PATH_SHELL");
      cmd[1] = "-c";
      cmd[2] = cmdString;

      Process p = Runtime.getRuntime().exec(cmd);
      BufferedReader input = new BufferedReader(new InputStreamReader(p
          .getInputStream()));
      while ((line = input.readLine()) != null) {
        Matcher matcher = pattern.matcher(line);
        if (matcher.find()) {
          interfaces.add(new String[]{matcher.group(1), matcher.group(2)});
        }
      }
      input.close();

      BufferedReader err = new BufferedReader(new InputStreamReader(p
          .getErrorStream()));
      if (err.ready())
        logger.warn("Error occured during scan:");
      while ((line = err.readLine()) != null) {
        logger.warn(line);
      }
      err.close();
    } catch (IOException err) {
      logger.fatal("Error while scanning for interfaces: " + err);
      err.printStackTrace();
    } catch (Exception err) {
      logger.fatal("Error while scanning for interfaces: " + err);
      err.printStackTrace();
    }
    return interfaces;
  }

  private void autoSelectDependencyProcesses(String processName,
      String sourceFilename, boolean confirmSelection) {

    /*
     * Try to autofind and select dependency processes (via
     * AUTOSTART_PROCESSES()) If this procedure fails, nothing will be reported
     * to user
     */

    // Open source code file
    FileReader reader = null;
    try {
      reader = new FileReader(textCoreDir.getText() + File.separatorChar
          + sourceFilename);
    } catch (Exception ex) {
    }
    for (File userPlatform : GUI.currentGUI.getUserPlatforms()) {
      if (reader == null) {
        try {
          reader = new FileReader(userPlatform.getPath() + File.separatorChar
              + sourceFilename);
        } catch (Exception ex) {
        }
      }
    }
    for (File userPlatform : moteTypeUserPlatforms) {
      if (reader == null) {
        try {
          reader = new FileReader(userPlatform.getPath() + File.separatorChar
              + sourceFilename);
        } catch (Exception ex) {
        }
      }
    }
    if (reader == null) {
      // Could not locate source file
      return;
    }

    // Find which processes were set to autostart
    BufferedReader sourceFile = new BufferedReader(reader);
    String line;
    try {
      String autostartExpression = "^AUTOSTART_PROCESSES([^$]*)$";

      Pattern pattern = Pattern.compile(autostartExpression);

      while ((line = sourceFile.readLine()) != null) {
        Matcher matcher = pattern.matcher(line);
        if (matcher.find()) {
          // Parse autostart processes
          String[] allProcesses = matcher.group(1).split(",");
          for (String autostartProcess : allProcesses) {
            autostartProcess = autostartProcess.replaceAll("[&; \\(\\)]", "");

            // Does this process already exist?
            boolean processAlreadyExists = false;
            for (Component checkBox : processPanel.getComponents()) {
              String existingProcess = ((JCheckBox) checkBox).getText();
              if (existingProcess.equals(autostartProcess)) {
                processAlreadyExists = true;
              }
            }

            if (!processAlreadyExists) {

              if (confirmSelection) {
                Object[] options = {"Create", "Cancel"};

                String question = "The process " + processName
                    + " depends on the following process: " + autostartProcess
                    + "\nDo you want to add this as well?";
                String title = "Add dependency process?";
                int answer = JOptionPane.showOptionDialog(myDialog, question,
                    title, JOptionPane.DEFAULT_OPTION,
                    JOptionPane.QUESTION_MESSAGE, null, options, options[0]);

                if (answer == JOptionPane.YES_OPTION) {
                  JCheckBox processCheckBox = new JCheckBox(autostartProcess,
                      true);
                  processCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);

                  processCheckBox.setActionCommand("process_clicked: "
                      + autostartProcess);
                  processCheckBox.addActionListener(myEventHandler);

                  processPanel.add(processCheckBox);
                }
              } else {
                JCheckBox processCheckBox = new JCheckBox(autostartProcess,
                    true);
                processCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);

                processCheckBox.setActionCommand("process_clicked: "
                    + autostartProcess);
                processCheckBox.addActionListener(myEventHandler);

                processPanel.add(processCheckBox);
              }
            }
          }

        }
      }
    } catch (Exception ex) {
      return;
    }
  }

  private void pathsWereUpdated() {
    updateVisualFields();

    // Remove all prevously scanned entries
    coreInterfacePanel.removeAll();
    moteInterfacePanel.removeAll();
    processPanel.removeAll();
    sensorPanel.removeAll();
    coreInterfacePanel.revalidate();
    coreInterfacePanel.repaint();
    moteInterfacePanel.revalidate();
    moteInterfacePanel.repaint();
    processPanel.revalidate();
    processPanel.repaint();
    sensorPanel.revalidate();
    sensorPanel.repaint();
    createButton.setEnabled(libraryCreatedOK = false);

    settingsOK = false;
    testButton.setEnabled(settingsOK);
  }

  private void updateVisualFields() {
    settingsOK = true;

    // Check for non-unique identifier
    textID.setBackground(Color.WHITE);
    textID.setToolTipText(null);

    for (MoteType otherType : allOtherTypes) {
      if (otherType != myMoteType
          && otherType.getIdentifier().equalsIgnoreCase(textID.getText())) {
        textID.setBackground(Color.RED);
        textID.setToolTipText("Conflicting name - must be unique");
        settingsOK = false;
        break;
      }
    }

    // Check for non-unique description
    textDescription.setBackground(Color.WHITE);
    textDescription.setToolTipText(null);

    for (MoteType otherType : allOtherTypes) {
      if (otherType != myMoteType
          && otherType.getDescription().equals(textDescription.getText())) {
        textDescription.setBackground(Color.RED);
        textDescription.setToolTipText("Conflicting name - must be unique");
        settingsOK = false;
        break;
      }
    }

    // Warn if spaces in Contiki path
    textContikiDir.setBackground(Color.WHITE);
    textContikiDir.setToolTipText(null);
    if (textContikiDir.getText().contains(" ")) {
      textContikiDir.setBackground(Color.ORANGE);
      textContikiDir
          .setToolTipText("Compilation may not work correctly with spaced paths");
    }
    textCoreDir.setText(textContikiDir.getText()
        + GUI.getExternalToolsSetting("PATH_COOJA_CORE_RELATIVE"));
    textCoreDir.setBackground(Color.WHITE);
    textCoreDir.setToolTipText(null);
    if (textCoreDir.getText().contains(" ")) {
      textCoreDir.setBackground(Color.ORANGE);
      textCoreDir
          .setToolTipText("Compilation may not work correctly with spaced paths");
    }

    // Warn if spaces in a user platform path
    textUserPlatforms.setBackground(Color.WHITE);
    textUserPlatforms.setToolTipText(null);
    for (File userPlatform : moteTypeUserPlatforms) {
      if (userPlatform.getPath().contains(" ")) {
        textUserPlatforms.setBackground(Color.ORANGE);
        textUserPlatforms
            .setToolTipText("Compilation may not work correctly with spaced paths");
      }
    }

    // Update output text field
    textOutputFiles.setText(ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + textID.getText() + ContikiMoteType.mapSuffix
        + ", " + ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + textID.getText() + ContikiMoteType.librarySuffix
        + ", " + ContikiMoteType.tempOutputDirectory.getPath()
        + File.separatorChar + textID.getText() + ContikiMoteType.dependSuffix);

    createButton.setEnabled(libraryCreatedOK = false);
    testButton.setEnabled(settingsOK);
  }

  /**
   * Scans Contiki base + (optional) user platform for Contiki processes,
   * sensors and core interfaces. The new mote type config is recreated every
   * time this method is run. If a user platform is specified, it looks for a
   * special class config file there, and appends it to the new mote type
   * config. By reading that config all available mote interfaces are parsed -
   * which will all be selected initially. This method also selects the core
   * interfaces needed by the mote interfaces.
   * 
   * Finally any pre-specified processes (via shortcut start) will be selected.
   */
  private void rescanDirectories() {

    boolean pathErrorFound = false;

    // Check that Contiki path is correct
    if (!new File(myDialog.textContikiDir.getText()).isDirectory()) {
      // Contiki path specified does not exist
      textContikiDir.setBackground(Color.RED);
      textContikiDir.setToolTipText("Incorrect path");
      pathErrorFound = true;
    }

    // Check that cooja main platform path is correct
    if (!new File(myDialog.textCoreDir.getText()).isDirectory()) {
      // Cooja main platform specified does not exist
      textContikiDir.setBackground(Color.RED);
      textContikiDir.setToolTipText("Incorrect path");
      textCoreDir.setBackground(Color.RED);
      textCoreDir.setToolTipText("Incorrect path");
      pathErrorFound = true;
    }

    // Check that all user platforms are valid
    for (File userPlatform : moteTypeUserPlatforms) {
      File userPlatformConfig = new File(userPlatform.getPath()
          + File.separatorChar + GUI.PLATFORM_CONFIG_FILENAME);
      if (!userPlatformConfig.exists()) {
        textUserPlatforms.setBackground(Color.RED);
        textUserPlatforms.setToolTipText("Invalid user platform: "
            + userPlatform);
        pathErrorFound = true;
      }
    }

    if (pathErrorFound) {
      // Remove all prevously scanned entries
      coreInterfacePanel.removeAll();
      processPanel.removeAll();
      sensorPanel.removeAll();
      coreInterfacePanel.revalidate();
      coreInterfacePanel.repaint();
      processPanel.revalidate();
      processPanel.repaint();
      sensorPanel.revalidate();
      sensorPanel.repaint();
      testButton.setEnabled(settingsOK = false);
      createButton.setEnabled(libraryCreatedOK = false);
      return;
    }

    // Scan for mote interfaces (+ recreate node type class configuration)
    myEventHandler.actionPerformed(new ActionEvent(myDialog.createButton,
        ActionEvent.ACTION_PERFORMED, "scanmoteinterfaces"));

    // Scan for processes
    myEventHandler.actionPerformed(new ActionEvent(myDialog.createButton,
        ActionEvent.ACTION_PERFORMED, "scanprocesses"));

    // Scan for sensors
    myDialog.myEventHandler.actionPerformed(new ActionEvent(
        myDialog.createButton, ActionEvent.ACTION_PERFORMED, "scansensors"));

    // Scan for core interfaces
    myDialog.myEventHandler.actionPerformed(new ActionEvent(
        myDialog.createButton, ActionEvent.ACTION_PERFORMED,
        "scancoreinterfaces"));

    // Recheck dependencies between selected mote interfaces and available
    // core interfaces
    myDialog.myEventHandler.actionPerformed(new ActionEvent(
        myDialog.createButton, ActionEvent.ACTION_PERFORMED,
        "recheck_interface_dependencies"));

    // Check if COOJA was started with a recommended process (select process)
    if (System.getProperty("QUICKSTART_APP") != null) {

      // Pre-select recommended process
      String wantedSourceFilename = System.getProperty("QUICKSTART_APP");

      for (Component checkBox : myDialog.processPanel.getComponents()) {
        JCheckBox processCheckBox = (JCheckBox) checkBox;
        String processFilename = processCheckBox.getToolTipText();

        if (processFilename.startsWith(wantedSourceFilename)) {
          // We found the recommended process, select it
          processCheckBox.setSelected(true);
          myDialog.autoSelectDependencyProcesses(processCheckBox.getText(),
              processFilename, false);
          break;
        }
      }
    }

    settingsOK = true;
    testButton.setEnabled(settingsOK);
  }

  private class MoteTypeEventHandler
      implements
        ActionListener,
        DocumentListener {
    public void insertUpdate(DocumentEvent e) {
      if (myDialog.isVisible())
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            pathsWereUpdated();
          }
        });
    }
    public void removeUpdate(DocumentEvent e) {
      if (myDialog.isVisible())
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            pathsWereUpdated();
          }
        });
    }
    public void changedUpdate(DocumentEvent e) {
      if (myDialog.isVisible())
        javax.swing.SwingUtilities.invokeLater(new Runnable() {
          public void run() {
            pathsWereUpdated();
          }
        });
    }
    public void actionPerformed(ActionEvent e) {
      if (e.getActionCommand().equals("cancel")) {
        // Cancel creation of mote type
        myMoteType = null;
        dispose();
      } else if (e.getActionCommand().equals("clean")) {
        // Delete any created intermediate files
        File objectDir = new File("obj_cooja");
        if (objectDir.exists() && objectDir.isDirectory()) {
          File[] objectFiles = objectDir.listFiles();
          for (File objectFile : objectFiles)
            objectFile.delete();

          objectDir.delete();
        }
      } else if (e.getActionCommand().equals("create")) {
        // Create mote type and set various data
        myMoteType.doInit(textID.getText());
        myMoteType.setDescription(textDescription.getText());
        myMoteType.setContikiBaseDir(textContikiDir.getText());
        myMoteType.setContikiCoreDir(textCoreDir.getText());
        myMoteType.setUserPlatformDirs(moteTypeUserPlatforms);
        myMoteType.setConfig(newMoteTypeConfig);

        // Set startup processes
        Vector<String> processes = new Vector<String>();
        for (Component checkBox : processPanel.getComponents()) {
          if (((JCheckBox) checkBox).isSelected()) {
            processes.add(((JCheckBox) checkBox).getText());
          }
        }
        myMoteType.setProcesses(processes);

        // Set registered sensors
        Vector<String> sensors = new Vector<String>();
        for (Component checkBox : sensorPanel.getComponents()) {
          if (((JCheckBox) checkBox).isSelected()) {
            sensors.add(((JCheckBox) checkBox).getText());
          }
        }
        myMoteType.setSensors(sensors);

        // Set registered core interfaces
        Vector<String> coreInterfaces = new Vector<String>();
        for (Component checkBox : coreInterfacePanel.getComponents()) {
          if (((JCheckBox) checkBox).isSelected()) {
            coreInterfaces.add(((JCheckBox) checkBox).getText());
          }
        }
        myMoteType.setCoreInterfaces(coreInterfaces);

        // Set registered mote interfaces
        Vector<Class<? extends MoteInterface>> moteInterfaces = new Vector<Class<? extends MoteInterface>>();
        for (Component checkBox : moteInterfacePanel.getComponents()) {
          JCheckBox interfaceCheckBox = (JCheckBox) checkBox;
          if (interfaceCheckBox.isSelected()) {
            moteInterfaces
                .add((Class<? extends MoteInterface>) interfaceCheckBox
                    .getClientProperty("class"));
          }
        }
        myMoteType.setMoteInterfaces(moteInterfaces);

        dispose();
      } else if (e.getActionCommand().equals("testsettings")) {
        testButton.requestFocus();
        Thread testSettingsThread = new Thread(new Runnable() {
          public void run() {
            doTestSettings();
          }
        }, "test settings thread");
        testSettingsThread.start();
      } else if (e.getActionCommand().equals("browsecontiki")) {
        JFileChooser fc = new JFileChooser();
        fc.setCurrentDirectory(new java.io.File("."));
        fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        fc.setDialogTitle("Contiki OS base directory");

        if (fc.showOpenDialog(myDialog) == JFileChooser.APPROVE_OPTION) {
          textContikiDir.setText(fc.getSelectedFile().getPath());
        }
        createButton.setEnabled(libraryCreatedOK = false);
        pathsWereUpdated();
      } else if (e.getActionCommand().equals("manageuserplatforms")) {
        Vector<File> newPlatforms = UserPlatformsDialog.showDialog(
            myParentFrame, moteTypeUserPlatforms, GUI.currentGUI
                .getUserPlatforms());
        if (newPlatforms != null) {
          moteTypeUserPlatforms = newPlatforms;
          String userPlatformText = null;
          for (File userPlatform : newPlatforms) {
            if (userPlatformText == null)
              userPlatformText = "'" + userPlatform.getPath() + "'";
            else
              userPlatformText += " + '" + userPlatform.getPath() + "'";
          }
          textUserPlatforms.setText(userPlatformText);

          createButton.setEnabled(libraryCreatedOK = false);
          pathsWereUpdated();
        }
      } else if (e.getActionCommand().equals("scanprocesses")) {
        // Clear process panel
        processPanel.removeAll();
        Vector<String[]> processes = new Vector<String[]>();

        // Scan core platform for processes
        processes.addAll(ContikiMoteTypeDialog.scanForProcesses(new File(
            textCoreDir.getText())));

        // If user platforms exists, scan those too
        for (File userPlatform : GUI.currentGUI.getUserPlatforms()) {
          processes
              .addAll(ContikiMoteTypeDialog.scanForProcesses(userPlatform));
        }
        if (moteTypeUserPlatforms != null) {
          for (File userPlatform : moteTypeUserPlatforms) {
            processes.addAll(ContikiMoteTypeDialog
                .scanForProcesses(userPlatform));
          }
        }

        if (processes != null) {
          for (String[] processInfo : processes) {
            JCheckBox processCheckBox = new JCheckBox(processInfo[1], false);
            processCheckBox.setToolTipText(processInfo[0]);
            processCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);

            processCheckBox.setActionCommand("process_clicked: "
                + processInfo[1]);
            processCheckBox.addActionListener(myEventHandler);

            processPanel.add(processCheckBox);
          }
        } else {
          logger.warn("No processes found during scan");
          testButton.setEnabled(settingsOK = false);
        }

	processPanel.revalidate();
        processPanel.repaint();
        createButton.setEnabled(libraryCreatedOK = false);

      } else if (e.getActionCommand().equals("scansensors")) {
        // Clear sensor panel
        sensorPanel.removeAll();
        Vector<String[]> sensors = new Vector<String[]>();

        // Scan core platform for sensors
        sensors.addAll(ContikiMoteTypeDialog.scanForSensors(new File(
            textCoreDir.getText())));

        // If user platforms exists, scan those too
        for (File userPlatform : GUI.currentGUI.getUserPlatforms()) {
          sensors.addAll(ContikiMoteTypeDialog.scanForSensors(userPlatform));
        }
        if (moteTypeUserPlatforms != null) {
          for (File userPlatform : moteTypeUserPlatforms) {
            sensors.addAll(ContikiMoteTypeDialog.scanForSensors(userPlatform));
          }
        }

        if (sensors != null) {
          for (String[] sensorInfo : sensors) {
            JCheckBox sensorCheckBox = new JCheckBox(sensorInfo[1], true);
            sensorCheckBox.setToolTipText(sensorInfo[0]);
            sensorCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);

            sensorPanel.add(sensorCheckBox);
          }
        } else {
          logger.warn("No sensors found during scan");
          testButton.setEnabled(settingsOK = false);
        }

	sensorPanel.revalidate();
        sensorPanel.repaint();
        createButton.setEnabled(libraryCreatedOK = false);
      } else if (e.getActionCommand().equals("scancoreinterfaces")) {
        // Clear core interface panel
        coreInterfacePanel.removeAll();
        Vector<String[]> interfaces = new Vector<String[]>();

        // Scan core platform for core interfaces
        interfaces.addAll(ContikiMoteTypeDialog.scanForInterfaces(new File(
            textCoreDir.getText())));

        // If user platforms exists, scan those too
        for (File userPlatform : GUI.currentGUI.getUserPlatforms()) {
          interfaces.addAll(ContikiMoteTypeDialog
              .scanForInterfaces(userPlatform));
        }
        if (moteTypeUserPlatforms != null) {
          for (File userPlatform : moteTypeUserPlatforms) {
            interfaces.addAll(ContikiMoteTypeDialog
                .scanForInterfaces(userPlatform));
          }
        }

        if (interfaces != null) {
          for (String[] interfaceInfo : interfaces) {
            JCheckBox interfaceCheckBox = new JCheckBox(interfaceInfo[1], false);
            interfaceCheckBox.setToolTipText(interfaceInfo[0]);
            interfaceCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
            coreInterfacePanel.add(interfaceCheckBox);
          }
        } else {
          logger.warn("No core interfaces found during scan");
          testButton.setEnabled(settingsOK = false);
        }
        recheckInterfaceDependencies();

	coreInterfacePanel.revalidate();
        coreInterfacePanel.repaint();
        createButton.setEnabled(libraryCreatedOK = false);

      } else if (e.getActionCommand().equals("scanmoteinterfaces")) {
        // Clear core interface panel
        moteInterfacePanel.removeAll();

        // Clone general simulator config
        newMoteTypeConfig = GUI.currentGUI.getPlatformConfig().clone();

        // Merge with all user platform configs (if any)
        for (File userPlatform : moteTypeUserPlatforms) {
          try {
            newMoteTypeConfig.appendUserPlatform(userPlatform);
          } catch (Exception ex) {
            logger.fatal("Error when parsing user platform config: " + ex);
            return;
          }
        }

        // Get all mote interfaces available from config
        String[] moteInterfaces = newMoteTypeConfig.getStringArrayValue(
            ContikiMoteType.class, "MOTE_INTERFACES");
        Vector<Class<? extends MoteInterface>> moteIntfClasses = new Vector<Class<? extends MoteInterface>>();

	ClassLoader classLoader =
	  GUI.currentGUI.createUserPlatformClassLoader(moteTypeUserPlatforms);

        // Find and load the mote interface classes
        for (String moteInterface : moteInterfaces) {
	  try {
	    Class<? extends MoteInterface> newMoteInterfaceClass =
	      classLoader.loadClass(moteInterface).
	      asSubclass(MoteInterface.class);
            moteIntfClasses.add(newMoteInterfaceClass);
            // logger.info("Loaded mote interface: " + newMoteInterfaceClass);
	  } catch (Exception ce) {
            logger.warn("Failed to load mote interface: " + moteInterface);
	  }
        }

        // Create and add checkboxes for all mote interfaces
        if (moteIntfClasses.size() > 0) {
          for (Class<? extends MoteInterface> moteIntfClass : moteIntfClasses) {
            JCheckBox interfaceCheckBox = new JCheckBox(GUI
                .getDescriptionOf(moteIntfClass), true);
            interfaceCheckBox.putClientProperty("class", moteIntfClass);
            interfaceCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);

            interfaceCheckBox
                .setActionCommand("recheck_interface_dependencies");
            interfaceCheckBox.addActionListener(myEventHandler);

            moteInterfacePanel.add(interfaceCheckBox);
          }
        } else {
          logger
              .warn("Error occured during parsing of mote interfaces (no found)");
          testButton.setEnabled(settingsOK = false);
        }

	moteInterfacePanel.revalidate();
        moteInterfacePanel.repaint();
        createButton.setEnabled(libraryCreatedOK = false);
      } else if (e.getActionCommand().equals("addprocess")) {
        String newProcessName = JOptionPane.showInputDialog(myDialog,
            "Enter process name");
        if (newProcessName != null) {
          JCheckBox processCheckBox = new JCheckBox(newProcessName, false);
          processCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
          processCheckBox.setSelected(true);

          processCheckBox
              .setActionCommand("process_clicked: " + newProcessName);
          processCheckBox.addActionListener(myEventHandler);

          processPanel.add(processCheckBox);
	  processPanel.revalidate();
	  processPanel.repaint();
        }
      } else if (e.getActionCommand().equals("addsensor")) {
        String newSensorName = JOptionPane.showInputDialog(myDialog,
            "Enter sensor name");
        if (newSensorName != null) {
          JCheckBox sensorCheckBox = new JCheckBox(newSensorName, false);
          sensorCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
          sensorCheckBox.setSelected(true);
          sensorPanel.add(sensorCheckBox);
	  sensorPanel.revalidate();
	  sensorPanel.repaint();
        }
      } else if (e.getActionCommand().equals("addinterface")) {
        String newInterfaceName = JOptionPane.showInputDialog(myDialog,
            "Enter interface name");
        if (newInterfaceName != null) {
          JCheckBox interfaceCheckBox = new JCheckBox(newInterfaceName, false);
          interfaceCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
          interfaceCheckBox.setSelected(true);
          coreInterfacePanel.add(interfaceCheckBox);
	  coreInterfacePanel.revalidate();
	  coreInterfacePanel.repaint();
        }
      } else if (e.getActionCommand().equals("recheck_interface_dependencies")) {
        recheckInterfaceDependencies();
      } else if (e.getActionCommand().startsWith("process_clicked")) {

        boolean processWasSelected = false;
        String sourceFilename = null;
        String processName = e.getActionCommand().split(": ")[1].trim();

        // Was the process selected or unselected?
        for (Component checkBox : processPanel.getComponents()) {
          JCheckBox processCheckbox = (JCheckBox) checkBox;
          if (processCheckbox.isSelected()
              && processCheckbox.getText().equals(processName)) {
            processWasSelected = true;
            sourceFilename = ((JCheckBox) checkBox).getToolTipText();
            break;
          }
        }

        if (!processWasSelected || sourceFilename == null)
          return;

        autoSelectDependencyProcesses(processName, sourceFilename, true);

      } else
        logger.warn("Unhandled action: " + e.getActionCommand());

      createButton.setEnabled(libraryCreatedOK = false);

    }
  }

}
