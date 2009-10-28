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
 * $Id: ProjectDirectoriesDialog.java,v 1.10 2009/10/28 12:03:48 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GraphicsEnvironment;
import java.awt.List;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Vector;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;

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

  private GUI gui;

  private File[] projects = null;
  private List projectsList = new List();
  
  /**
   * Allows editing the project directories list by adding new,
   * reordering or removing project directories.
   *
   * @param parent Parent container
   * @param currentProjects Current project directories
   * @return The new project directories, or null if dialog was aborted
   */
  public static File[] showDialog(Container parent, GUI gui, File[] currentProjects) {
    if (GUI.isVisualizedInApplet()) {
      return null;
    }

    ProjectDirectoriesDialog dialog = new ProjectDirectoriesDialog((Window) parent, currentProjects);

    dialog.gui = gui;
    dialog.setLocationRelativeTo(parent);

    dialog.setVisible(true);

    return dialog.projects;
  }

  private ProjectDirectoriesDialog(Container parent, File[] projects) {
    super(
        parent instanceof Dialog?(Dialog)parent:
          parent instanceof Window?(Window)parent:
            (Frame)parent, "Manage Project Directories", ModalityType.APPLICATION_MODAL);

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
        ProjectDirectoriesDialog.this.projects = null;
        dispose();
      }
    });
    buttonPane.add(button);

    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    button = new JButton("Set default");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        Object[] options = { "Ok", "Cancel" };

        String newDefaultProjectDirs = "";
        for (String directory : projectsList.getItems()) {
          if (newDefaultProjectDirs != "") {
            newDefaultProjectDirs += ";";
          }

          newDefaultProjectDirs += gui.createPortablePath(new File(directory)).getPath();
        }
        newDefaultProjectDirs = newDefaultProjectDirs.replace('\\', '/');

        String question = "External tools setting DEFAULT_PROJECTDIRS will change from:\n"
          + GUI.getExternalToolsSetting("DEFAULT_PROJECTDIRS")
          + "\n to:\n"
          + newDefaultProjectDirs;
        String title = "Change external tools settings?";
        int answer = JOptionPane.showOptionDialog(ProjectDirectoriesDialog.this, question, title,
            JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE, null,
            options, options[0]);

        if (answer != JOptionPane.YES_OPTION) {
          return;
        }

        GUI.setExternalToolsSetting("DEFAULT_PROJECTDIRS", newDefaultProjectDirs);
      }
    });
    buttonPane.add(button);

    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    button = new JButton("OK");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        ArrayList<File> newProjects = new ArrayList<File>();
        for (String directory : projectsList.getItems()) {
          newProjects.add(new File(directory));
        }
        ProjectDirectoriesDialog.this.projects = newProjects.toArray(new File[0]);
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

    listPane2.add(projectsList);

    listPane.add(listPane2);

    smallPane = new JPanel();
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.Y_AXIS));

    button = new JButton("Move up");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        int selectedIndex = projectsList.getSelectedIndex();
        if (selectedIndex <= 0) {
          return;
        }

        File file = new File(projectsList.getItem(selectedIndex));

        removeProjectDir(selectedIndex);
        addProjectDir(file, selectedIndex - 1);
        projectsList.select(selectedIndex - 1);
      }
    });
    smallPane.add(button);

    button = new JButton("Move down");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        int selectedIndex = projectsList.getSelectedIndex();
        if (selectedIndex < 0) {
          return;
        }
        if (selectedIndex >= projectsList.getItemCount() - 1) {
          return;
        }

        File file = new File(projectsList.getItem(selectedIndex));
        removeProjectDir(selectedIndex);
        addProjectDir(file, selectedIndex + 1);
        projectsList.select(selectedIndex + 1);
      }
    });
    smallPane.add(button);

    smallPane.add(Box.createRigidArea(new Dimension(10, 10)));

    button = new JButton("Remove");
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (projectsList.getSelectedIndex() < 0) {
          return;
        }

        removeProjectDir(projectsList.getSelectedIndex());
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

        // Add the project directory configurations
        for (String projectDir : projectsList.getItems()) {
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

    button = new JButton("Enter path manually");
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
        fc.setCurrentDirectory(new java.io.File(GUI.getExternalToolsSetting("PATH_CONTIKI")));
        fc.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        fc.setDialogTitle("Select project directory");

        if (fc.showOpenDialog(ProjectDirectoriesDialog.this) == JFileChooser.APPROVE_OPTION) {
          File dir = fc.getSelectedFile();
          String selectedPath = null;
          try {
            selectedPath = dir.getCanonicalPath().replace('\\', '/');
            String contikiPath =
              new File(GUI.getExternalToolsSetting("PATH_CONTIKI", null)).
              getCanonicalPath().replace('\\', '/');
            if (contikiPath != null && selectedPath.startsWith(contikiPath)) {
              selectedPath = selectedPath.replaceFirst(
                  contikiPath, GUI.getExternalToolsSetting("PATH_CONTIKI"));
            }
            addProjectDir(new File(selectedPath));
          } catch (IOException ex) {
            logger.fatal("Error in path extraction: " + ex);
          }

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

    for (File projectDir : projects) {
      addProjectDir(projectDir);
    }
    pack();
  }

  private void addProjectDir(File projectDir) {
    addProjectDir(projectDir, projectsList.getItemCount());
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

    projectsList.add(projectDir.getPath(), index);
  }

  private void removeProjectDir(int index) {
    projectsList.remove(index);
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

    /* Respect screen size */
    Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds();
    if (maxSize != null && (getSize().width > maxSize.width)) {
      setSize(maxSize.width, getSize().height);
    }
    if (maxSize != null && (getSize().height > maxSize.height)) {
      setSize(getSize().width, maxSize.height);
    }

  }

}
