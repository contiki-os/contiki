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
 * $Id: ProjectDirectoriesDialog.java,v 1.5 2008/02/12 15:06:09 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Vector;
import javax.swing.*;

import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.ProjectConfig;

/**
 * This dialog allows a user to manage the project directory configurations. Project
 * directories can be added, removed or reordered. The resulting
 * configuration can also be viewed.
 *
 * This dialog reads from the external project configuration files in each project
 * directory, as well as from any specified default configuration files.
 *
 * @author Fredrik Osterlind
 */
public class ProjectDirectoriesDialog extends JDialog {

  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(ProjectDirectoriesDialog.class);

  private List changableProjectsList = new List();
  private List fixedProjectsList = null;
  private Vector<File> changableProjects = null;

  private ProjectDirectoriesDialog myDialog;

  /**
   * Allows user to alter the given project directories list by adding new,
   * reordering or removing project directories. Only the changable project directories
   * can be altered.
   *
   * @param parentContainer
   *          Parent container
   * @param changableProjects
   *          Changeable project directories
   * @param fixedProjects
   *          Fixed project directory
   * @return Null if dialog aborted, else the new CHANGEABLE project directory list.
   */
  public static Vector<File> showDialog(Container parentContainer,
      Vector<File> changableProjects, Vector<File> fixedProjects) {

    ProjectDirectoriesDialog myDialog = null;
    if (parentContainer instanceof Window) {
      myDialog = new ProjectDirectoriesDialog((Window) parentContainer, changableProjects, fixedProjects);
    } else if (parentContainer instanceof Dialog) {
      myDialog = new ProjectDirectoriesDialog((Dialog) parentContainer, changableProjects, fixedProjects);
    } else if (parentContainer instanceof Frame) {
      myDialog = new ProjectDirectoriesDialog((Frame) parentContainer, changableProjects, fixedProjects);
    } else {
      logger.fatal("Unknown parent container type: " + parentContainer);
      return null;
    }
    myDialog.setLocationRelativeTo(parentContainer);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }

    return myDialog.changableProjects;
  }

  /**
   * Allows user to alter the given project directories list by adding new,
   * reordering or removing project directories. Only the changable project directories
   * may be altered.
   *
   * @param parentDialog
   *          Parent dialog
   * @param changableProjects
   *          Changeable project directories
   * @param fixedProjects
   *          Fixed project directory
   * @return Null if dialog aborted, else the new CHANGEABLE project directory list.
   */
  public static Vector<File> showDialog(Dialog parentDialog,
      Vector<File> changableProjects, Vector<File> fixedProjects) {
    ProjectDirectoriesDialog myDialog = new ProjectDirectoriesDialog(parentDialog,
        changableProjects, fixedProjects);
    myDialog.setLocationRelativeTo(parentDialog);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }

    return myDialog.changableProjects;
  }

  private ProjectDirectoriesDialog(Frame frame, Vector<File> changableProjects,
      Vector<File> fixedProjects) {
    super(frame, "Manage Project Directories", ModalityType.APPLICATION_MODAL);
    setupDialog(changableProjects, fixedProjects);
  }

  private ProjectDirectoriesDialog(Dialog dialog, Vector<File> changableProjects,
      Vector<File> fixedProjects) {
    super(dialog, "Manage Project Directories", ModalityType.APPLICATION_MODAL);
    setupDialog(changableProjects, fixedProjects);
  }

  private ProjectDirectoriesDialog(Window window, Vector<File> changableProjects,
      Vector<File> fixedProjects) {
    super(window, "Manage Project Directories", ModalityType.APPLICATION_MODAL);
    setupDialog(changableProjects, fixedProjects);
  }

  private void setupDialog(Vector<File> changablePlatforms, Vector<File> fixedProjects) {
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
        changableProjects = null;
        dispose();
      }
    });
    buttonPane.add(button);

    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    if (fixedProjects == null) {
      button = new JButton("Set default");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          Object[] options = { "Ok", "Cancel" };

          String newDefaultProjectDirs = "";
          for (String directory : changableProjectsList.getItems()) {
            if (newDefaultProjectDirs != "") {
              newDefaultProjectDirs += ";";
            }

            newDefaultProjectDirs += directory;
          }
          newDefaultProjectDirs = newDefaultProjectDirs.replace('\\', '/');

          String question = "External tools setting DEFAULT_PROJECTDIRS will change from:\n"
            + GUI.getExternalToolsSetting("DEFAULT_PROJECTDIRS")
            + "\n to:\n"
            + newDefaultProjectDirs + "\n\nAre you sure?";
          String title = "Change external tools settings?";
          int answer = JOptionPane.showOptionDialog(myDialog, question, title,
              JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE, null,
              options, options[0]);

          if (answer != JOptionPane.YES_OPTION) {
            return;
          }

          GUI.setExternalToolsSetting("DEFAULT_PROJECTDIRS",
              newDefaultProjectDirs);
        }
      });

      buttonPane.add(button);

      buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    }

    button = new JButton("OK");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        changableProjects = new Vector<File>();
        for (String directory : changableProjectsList.getItems()) {
          File projectDir = new File(directory);
          if (projectDir.exists() && projectDir.isDirectory()) {
            changableProjects.add(projectDir);
          } else {
            logger.fatal("Can't find project directory: " + projectDir);
          }
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

    if (fixedProjects != null) {
      fixedProjectsList = new List();
      fixedProjectsList.setEnabled(false);
      listPane2.add(new JLabel("Fixed:"));
      listPane2.add(fixedProjectsList);
    }

    listPane2.add(new JLabel("Changable:"));
    listPane2.add(changableProjectsList);

    listPane.add(listPane2);

    smallPane = new JPanel();
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.Y_AXIS));

    button = new JButton("Move up");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        int selectedIndex = changableProjectsList.getSelectedIndex();
        if (selectedIndex <= 0) {
          return;
        }

        File file = new File(changableProjectsList.getItem(selectedIndex));

        removeProjectDir(selectedIndex);
        addProjectDir(file, selectedIndex - 1);
        changableProjectsList.select(selectedIndex - 1);
      }
    });
    smallPane.add(button);

    button = new JButton("Move down");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        int selectedIndex = changableProjectsList.getSelectedIndex();
        if (selectedIndex < 0) {
          return;
        }
        if (selectedIndex >= changableProjectsList.getItemCount() - 1) {
          return;
        }

        File file = new File(changableProjectsList.getItem(selectedIndex));
        removeProjectDir(selectedIndex);
        addProjectDir(file, selectedIndex + 1);
        changableProjectsList.select(selectedIndex + 1);
      }
    });
    smallPane.add(button);

    smallPane.add(Box.createRigidArea(new Dimension(10, 10)));

    button = new JButton("Remove");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (changableProjectsList.getSelectedIndex() < 0) {
          return;
        }

        removeProjectDir(changableProjectsList.getSelectedIndex());
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
        ProjectConfig config;
        try {
          // Create default configuration
          config = new ProjectConfig(true);
        } catch (FileNotFoundException ex) {
          logger.fatal("Could not find default project config file: "
              + GUI.PROJECT_DEFAULT_CONFIG_FILENAME);
          return;
        } catch (IOException ex) {
          logger.fatal("Error when reading default project config file: "
              + GUI.PROJECT_DEFAULT_CONFIG_FILENAME);
          return;
        }

        // Add the fixed project configurations
        if (fixedProjectsList != null) {
          for (String projectDir : fixedProjectsList.getItems()) {
            try {
              config.appendProjectDir(new File(projectDir));
            } catch (Exception ex) {
              logger.fatal("Error when merging configurations: " + ex);
              return;
            }
          }
        }

        // Add the project directory configurations
        for (String projectDir : changableProjectsList.getItems()) {
          try {
            config.appendProjectDir(new File(projectDir));
          } catch (Exception ex) {
            logger.fatal("Error when merging configurations: " + ex);
            return;
          }
        }

        // Show merged configuration
        ConfigViewer.showDialog(ProjectDirectoriesDialog.this, config);
      }
    });
    addRemovePane.add(button);

    addRemovePane.add(Box.createRigidArea(new Dimension(10, 0)));

    button = new JButton("Add manually");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ProjectDirectoryInputDialog pathDialog = new ProjectDirectoryInputDialog(ProjectDirectoriesDialog.this);
        pathDialog.pack();

        pathDialog.setLocationRelativeTo(ProjectDirectoriesDialog.this);
        pathDialog.setVisible(true);

        File projectPath = pathDialog.getProjectDirectory();
        if (projectPath != null) {
          addProjectDir(projectPath);
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
        fc.setDialogTitle("Select project directory");

        if (fc.showOpenDialog(myDialog) == JFileChooser.APPROVE_OPTION) {
          addProjectDir(fc.getSelectedFile());
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

    // Add fixed project directories if any
    if (fixedProjects != null) {
      for (File projectDir : fixedProjects) {
        fixedProjectsList.add(projectDir.getPath());
      }
    }

    // Add already existing project directories
    for (File projectDir : changablePlatforms) {
      addProjectDir(projectDir);
    }

    pack();
  }

  private void addProjectDir(File projectDir) {
    addProjectDir(projectDir, changableProjectsList.getItemCount());
  }

  private void addProjectDir(File projectDir, int index) {
    // Check that file exists, is a directory and contains the correct files
    if (!projectDir.exists()) {
      logger.fatal("Can't find project directory: " + projectDir);
      return;
    }
    if (!projectDir.isDirectory()) {
      logger.fatal("Specified path is not a directory: " + projectDir);
      return;
    }

    File projectConfigFile = new File(projectDir.getPath(),
        GUI.PROJECT_CONFIG_FILENAME);
    if (!projectConfigFile.exists()) {

      Object[] options = {"Create",
                          "Cancel"};

      int n = JOptionPane.showOptionDialog(
          this,
          "No " + GUI.PROJECT_CONFIG_FILENAME + " file exists in specified directory!"
          + "\nCreate an empty " + GUI.PROJECT_CONFIG_FILENAME + " file?",
          "Create project directory configuration?",
          JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE,
          null, options, options[1]);

      if (n == JOptionPane.NO_OPTION) {
        return;
      }

      try {
        projectConfigFile.createNewFile();
      } catch (IOException e) {
        logger.fatal("Could not create project directory configuration file: "
            + projectConfigFile);
        return;
      }
    }

    changableProjectsList.add(projectDir.getPath(), index);
  }

  private void removeProjectDir(int index) {
    changableProjectsList.remove(index);
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

  public static void showDialog(Frame parentFrame, ProjectConfig config) {
    ConfigViewer myDialog = new ConfigViewer(parentFrame, config);
    myDialog.setLocationRelativeTo(parentFrame);
    myDialog.setAlwaysOnTop(true);

    Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds();
    if (maxSize != null &&
        (myDialog.getSize().getWidth() > maxSize.getWidth()
            || myDialog.getSize().getHeight() > maxSize.getHeight())) {
      Dimension newSize = new Dimension();
      newSize.height = Math.min((int) maxSize.getHeight(), (int) myDialog.getSize().getHeight());
      newSize.width = Math.min((int) maxSize.getWidth(), (int) myDialog.getSize().getWidth());
      myDialog.setSize(newSize);
    }

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
  }

  public static void showDialog(Dialog parentDialog, ProjectConfig config) {
    ConfigViewer myDialog = new ConfigViewer(parentDialog, config);
    myDialog.setLocationRelativeTo(parentDialog);
    myDialog.setAlwaysOnTop(true);

    if (myDialog != null) {
      myDialog.setVisible(true);
    }
  }

  private ConfigViewer(Dialog dialog, ProjectConfig config) {
    super(dialog, "Current class configuration", true);
    init(config);
  }

  private ConfigViewer(Frame frame, ProjectConfig config) {
    super(frame, "Current class configuration", true);
    init(config);
  }

  private void init(ProjectConfig config) {
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
      if (config.getStringValue(propertyName).equals("")) {
        valuePane.add(new JLabel(" "));
      } else {
        valuePane.add(new JLabel(config.getStringValue(propertyName)));
      }
    }

    // Add components
    Container contentPane = getContentPane();
    contentPane.add(new JScrollPane(mainPane), BorderLayout.CENTER);
    contentPane.add(buttonPane, BorderLayout.SOUTH);

    pack();
  }

}
