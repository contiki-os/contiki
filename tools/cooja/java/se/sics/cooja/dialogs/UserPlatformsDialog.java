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
 * $Id: UserPlatformsDialog.java,v 1.5 2006/09/06 12:26:33 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.*;
import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.PlatformConfig;

/**
 * This dialog allows a user to manage the user platforms configuration. User
 * platforms can be added, removed or reordered. The resulting platform
 * configuration can also be viewed.
 * 
 * This dialog reads from the external platform configuration files in each user
 * platform, as well as from any specified default configuration files.
 * 
 * @author Fredrik Osterlind
 */
public class UserPlatformsDialog extends JDialog {

  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(UserPlatformsDialog.class);

  private List changablePlatformsList = new List();
  private List fixedPlatformsList = null;
  private Vector<File> fixedUserPlatforms = null;
  private Vector<File> changableUserPlatforms = null;

  private UserPlatformsDialog myDialog;
  private Frame myParentFrame = null;
  private Dialog myParentDialog = null;

  /**
   * Allows user to alter the given user platforms list by adding new,
   * reordering or removing user platforms. Only the changable user platforms
   * may be changed,
   * 
   * @param parentFrame
   *          Parent frame
   * @param changablePlatforms
   *          Changeable user platforms
   * @param fixedPlatforms
   *          Fixed user platform
   * @return Null if dialog aborted, else the new CHANGEABLE user platform list.
   */
  public static Vector<File> showDialog(Frame parentFrame,
      Vector<File> changablePlatforms, Vector<File> fixedPlatforms) {
    UserPlatformsDialog myDialog = new UserPlatformsDialog(parentFrame,
        changablePlatforms, fixedPlatforms);
    myDialog.setLocationRelativeTo(parentFrame);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }

    return myDialog.changableUserPlatforms;
  }

  /**
   * Allows user to alter the given user platforms list by adding new,
   * reordering or removing user platforms. Only the changable user platforms
   * may be changed,
   * 
   * @param parentDialog
   *          Parent dialog
   * @param changablePlatforms
   *          Changeable user platforms
   * @param fixedPlatforms
   *          Fixed user platform
   * @return Null if dialog aborted, else the new CHANGEABLE user platform list.
   */
  public static Vector<File> showDialog(Dialog parentDialog,
      Vector<File> changablePlatforms, Vector<File> fixedPlatforms) {
    UserPlatformsDialog myDialog = new UserPlatformsDialog(parentDialog,
        changablePlatforms, fixedPlatforms);
    myDialog.setLocationRelativeTo(parentDialog);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }

    return myDialog.changableUserPlatforms;
  }

  private UserPlatformsDialog(Frame frame, Vector<File> changablePlatforms,
      Vector<File> fixedPlatforms) {
    super(frame, "Manage User Platforms", true);
    myParentFrame = frame;
    init(changablePlatforms, fixedPlatforms);
  }

  private UserPlatformsDialog(Dialog dialog, Vector<File> changablePlatforms,
      Vector<File> fixedPlatforms) {
    super(dialog, "Manage User Platforms", true);
    myParentDialog = dialog;
    init(changablePlatforms, fixedPlatforms);
  }

  private void init(Vector<File> changablePlatforms, Vector<File> fixedPlatforms) {
    myDialog = this;

    JPanel mainPane = new JPanel();
    mainPane.setLayout(new BoxLayout(mainPane, BoxLayout.Y_AXIS));
    JPanel smallPane;
    JButton button;

    // BOTTOM BUTTON PART
    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    buttonPane.add(Box.createHorizontalGlue());

    button = new JButton("Cancel");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        changableUserPlatforms = null;
        dispose();
      }
    });
    buttonPane.add(button);

    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    button = new JButton("OK");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        changableUserPlatforms = new Vector<File>();
        for (String directory : changablePlatformsList.getItems()) {
          File userPlatform = new File(directory);
          if (userPlatform.exists() && userPlatform.isDirectory())
            changableUserPlatforms.add(userPlatform);
          else
            logger.fatal("Can't find user platform: " + userPlatform);
        }
        dispose();
      }
    });
    buttonPane.add(button);
    this.getRootPane().setDefaultButton(button);

    // LIST PART
    JPanel listPane = new JPanel();
    listPane.setLayout(new BoxLayout(listPane, BoxLayout.X_AXIS));
    listPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    JPanel listPane2 = new JPanel();
    listPane2.setLayout(new BoxLayout(listPane2, BoxLayout.Y_AXIS));

    if (fixedPlatforms != null) {
      fixedPlatformsList = new List();
      fixedPlatformsList.setEnabled(false);
      listPane2.add(new JLabel("Fixed:"));
      listPane2.add(fixedPlatformsList);
    }

    listPane2.add(new JLabel("Changable:"));
    listPane2.add(changablePlatformsList);

    listPane.add(listPane2);

    smallPane = new JPanel();
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.Y_AXIS));

    button = new JButton("Move up");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        int selectedIndex = changablePlatformsList.getSelectedIndex();
        if (selectedIndex <= 0)
          return;

        File file = new File(changablePlatformsList.getItem(selectedIndex));

        removeUserPlatform(selectedIndex);
        addUserPlatform(file, selectedIndex - 1);
        changablePlatformsList.select(selectedIndex - 1);
      }
    });
    smallPane.add(button);

    button = new JButton("Move down");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        int selectedIndex = changablePlatformsList.getSelectedIndex();
        if (selectedIndex < 0)
          return;
        if (selectedIndex >= changablePlatformsList.getItemCount() - 1)
          return;

        File file = new File(changablePlatformsList.getItem(selectedIndex));
        removeUserPlatform(selectedIndex);
        addUserPlatform(file, selectedIndex + 1);
        changablePlatformsList.select(selectedIndex + 1);
      }
    });
    smallPane.add(button);

    smallPane.add(Box.createRigidArea(new Dimension(10, 10)));

    button = new JButton("Remove");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (changablePlatformsList.getSelectedIndex() < 0)
          return;

        removeUserPlatform(changablePlatformsList.getSelectedIndex());
      }
    });
    smallPane.add(button);

    listPane.add(smallPane);

    // ADD/REMOVE PART
    JPanel addRemovePane = new JPanel();
    addRemovePane.setLayout(new BoxLayout(addRemovePane, BoxLayout.X_AXIS));
    addRemovePane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    button = new JButton("View resulting config");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        PlatformConfig config;
        try {
          // Create default configuration
          config = new PlatformConfig(true);
        } catch (FileNotFoundException ex) {
          logger.fatal("Could not find default platform config file: "
              + GUI.PLATFORM_DEFAULT_CONFIG_FILENAME);
          return;
        } catch (IOException ex) {
          logger.fatal("Error when reading default platform config file: "
              + GUI.PLATFORM_DEFAULT_CONFIG_FILENAME);
          return;
        }

        // Add the fixed platform configurations
        if (fixedPlatformsList != null) {
          for (String userPlatform : fixedPlatformsList.getItems()) {
            try {
              config.appendUserPlatform(new File(userPlatform));
            } catch (Exception ex) {
              logger.fatal("Error when merging configurations: " + ex);
              return;
            }
          }
        }

        // Add the user platform configurations
        for (String userPlatform : changablePlatformsList.getItems()) {
          try {
            config.appendUserPlatform(new File(userPlatform));
          } catch (Exception ex) {
            logger.fatal("Error when merging configurations: " + ex);
            return;
          }
        }

        // Show merged configuration
        if (myParentFrame != null)
          ConfigViewer.showDialog(myParentFrame, config);
        else
          ConfigViewer.showDialog(myParentDialog, config);
      }
    });
    addRemovePane.add(button);

    addRemovePane.add(Box.createRigidArea(new Dimension(10, 0)));

    button = new JButton("Add manually");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        String newUserPlatformPath = JOptionPane.showInputDialog(myDialog,
            "Enter path to user platform", "Enter path",
            JOptionPane.QUESTION_MESSAGE);
        if (newUserPlatformPath != null) {
          addUserPlatform(new File(newUserPlatformPath));
        }
      }
    });
    addRemovePane.add(button);

    addRemovePane.add(Box.createRigidArea(new Dimension(10, 0)));

    button = new JButton("Browse");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        JFileChooser fc = new JFileChooser();
        fc.setCurrentDirectory(new java.io.File("."));
        fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        fc.setDialogTitle("Select user platform");

        if (fc.showOpenDialog(myDialog) == JFileChooser.APPROVE_OPTION) {
          addUserPlatform(fc.getSelectedFile());
        }
      }
    });
    addRemovePane.add(button);

    // Add components
    Container contentPane = getContentPane();
    mainPane.add(listPane);
    mainPane.add(addRemovePane);
    contentPane.add(mainPane, BorderLayout.CENTER);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

    // Add fixed user platforms if any
    if (fixedPlatforms != null) {
      for (File userPlatform : fixedPlatforms) {
        fixedPlatformsList.add(userPlatform.getPath());
      }
    }

    // Add already existing user platforms
    for (File userPlatform : changablePlatforms) {
      addUserPlatform(userPlatform);
    }

    pack();
  }

  private void addUserPlatform(File userPlatform) {
    addUserPlatform(userPlatform, changablePlatformsList.getItemCount());
  }

  private void addUserPlatform(File userPlatform, int index) {
    // Check that file exists, is a directory and contains the correct files
    if (!userPlatform.exists()) {
      logger.fatal("Can't find user platform: " + userPlatform);
      return;
    }
    if (!userPlatform.isDirectory()) {
      logger.fatal("User platform is not a directory: " + userPlatform);
      return;
    }

    File userPlatformConfigFile = new File(userPlatform.getPath(),
        GUI.PLATFORM_CONFIG_FILENAME);
    if (!userPlatformConfigFile.exists()) {
      logger.fatal("User platform has no configuration file: "
          + userPlatformConfigFile);
      return;
    }

    changablePlatformsList.add(userPlatform.getPath(), index);
  }

  private void removeUserPlatform(int index) {
    changablePlatformsList.remove(index);
  }

}

/**
 * Modal frame that shows all keys with their respective values of a given class
 * configuration.
 * 
 * @author Fredrik Osterlind
 */
class ConfigViewer extends JDialog {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ConfigViewer.class);

  public static void showDialog(Frame parentFrame, PlatformConfig config) {
    ConfigViewer myDialog = new ConfigViewer(parentFrame, config);
    myDialog.setLocationRelativeTo(parentFrame);
    myDialog.setAlwaysOnTop(true);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
  }

  public static void showDialog(Dialog parentDialog, PlatformConfig config) {
    ConfigViewer myDialog = new ConfigViewer(parentDialog, config);
    myDialog.setLocationRelativeTo(parentDialog);
    myDialog.setAlwaysOnTop(true);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
  }

  private ConfigViewer(Dialog dialog, PlatformConfig config) {
    super(dialog, "Current class configuration", true);
    init(config);
  }

  private ConfigViewer(Frame frame, PlatformConfig config) {
    super(frame, "Current class configuration", true);
    init(config);
  }

  private void init(PlatformConfig config) {
    JPanel mainPane = new JPanel(new BorderLayout());
    JLabel label;
    JButton button;

    // BOTTOM BUTTON PART
    JPanel buttonPane = new JPanel();
    buttonPane.setLayout(new BoxLayout(buttonPane, BoxLayout.X_AXIS));
    buttonPane.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

    buttonPane.add(Box.createHorizontalGlue());

    button = new JButton("Close");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        dispose();
      }
    });
    buttonPane.add(button);

    // LIST PART
    JPanel keyPane = new JPanel();
    keyPane.setLayout(new BoxLayout(keyPane, BoxLayout.Y_AXIS));
    mainPane.add(keyPane, BorderLayout.WEST);

    JPanel valuePane = new JPanel();
    valuePane.setLayout(new BoxLayout(valuePane, BoxLayout.Y_AXIS));
    mainPane.add(valuePane, BorderLayout.CENTER);

    label = new JLabel("KEY");
    label.setForeground(Color.RED);
    keyPane.add(label);
    label = new JLabel("VALUE");
    label.setForeground(Color.RED);
    valuePane.add(label);

    Enumeration<String> allPropertyNames = config.getPropertyNames();
    while (allPropertyNames.hasMoreElements()) {
      String propertyName = allPropertyNames.nextElement();

      keyPane.add(new JLabel(propertyName));
      if (config.getStringValue(propertyName).equals(""))
        valuePane.add(new JLabel(" "));
      else
        valuePane.add(new JLabel(config.getStringValue(propertyName)));
    }

    // Add components
    Container contentPane = getContentPane();
    contentPane.add(new JScrollPane(mainPane), BorderLayout.CENTER);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

    pack();
  }

}
