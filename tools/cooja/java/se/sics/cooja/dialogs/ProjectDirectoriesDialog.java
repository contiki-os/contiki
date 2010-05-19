/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 * $Id: ProjectDirectoriesDialog.java,v 1.15 2010/05/19 17:52:23 fros4943 Exp $
 */

package se.sics.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.io.File;
import java.io.FileFilter;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Enumeration;

import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTree;
import javax.swing.ListSelectionModel;
import javax.swing.UIManager;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;

import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.ProjectConfig;

/**
 * This dialog allows a user to manage COOJA projects: extensions to COOJA that 
 * provide new functionality such as radio mediums, plugins, and mote types.
 *
 * @author Fredrik Osterlind
 */
public class ProjectDirectoriesDialog extends JDialog {
  private static final long serialVersionUID = 1896348946753376556L;
  private static Logger logger = Logger.getLogger(ProjectDirectoriesDialog.class);

  private GUI gui;

  private JTable table = null;
  private DirectoryTreePanel treePanel = null;

  private ArrayList<File> currentProjects = new ArrayList<File>();
  private File[] finalProjects = null;

  /**
   * Shows a blocking configuration dialog.
   * Returns a list of new COOJA project directories, or null if canceled by the user. 
   *  
   * @param parent Parent container
   * @param gui COOJA
   * @param currentProjects Current project configuration
   * @return New COOJA projects, or null
   */
  public static File[] showDialog(Container parent, GUI gui, File[] currentProjects) {
    if (GUI.isVisualizedInApplet()) {
      return null;
    }

    ProjectDirectoriesDialog dialog = new ProjectDirectoriesDialog((Window) parent, currentProjects);
    dialog.gui = gui;
    dialog.setLocationRelativeTo(parent);
    dialog.setVisible(true);
    return dialog.finalProjects;
  }

  private ProjectDirectoriesDialog(Container parent, File[] projects) {
    super(
        parent instanceof Dialog?(Dialog)parent:
          parent instanceof Window?(Window)parent:
            (Frame)parent, "COOJA projects", ModalityType.APPLICATION_MODAL);

    table = new JTable(new AbstractTableModel() {
      private static final long serialVersionUID = 591599455927509191L;
      public int getColumnCount() {
        return 2;
      }
      public int getRowCount() {
        return currentProjects.size();
      }
      public Object getValueAt(int rowIndex, int columnIndex) {
        if (columnIndex == 0) {
          return rowIndex+1;
        }

        if (!currentProjects.get(rowIndex).exists()) {
          return currentProjects.get(rowIndex) + "  (directory not found)";
        }
        if (!new File(currentProjects.get(rowIndex), GUI.PROJECT_CONFIG_FILENAME).exists()) {
          return currentProjects.get(rowIndex) + "  (no " + GUI.PROJECT_CONFIG_FILENAME + " found)";
        }

        return currentProjects.get(rowIndex);
      }
    });
    table.setTableHeader(null);
    table.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
    table.getSelectionModel().addListSelectionListener(new ListSelectionListener() {
      public void valueChanged(ListSelectionEvent e) {
        if (table.getSelectedRow() < 0) {
          return;
        }
        selectTreeProject(currentProjects.get(table.getSelectedRow()));
      }
    });
    table.getColumnModel().getColumn(0).setPreferredWidth(30);
    table.getColumnModel().getColumn(0).setMaxWidth(30);
    table.getColumnModel().getColumn(1).setCellRenderer(new DefaultTableCellRenderer() {
      private static final long serialVersionUID = 7224219223448831880L;
      public Component getTableCellRendererComponent(JTable table,
          Object value, boolean isSelected, boolean hasFocus, int row,
          int column) {
        if (!new File(currentProjects.get(row), GUI.PROJECT_CONFIG_FILENAME).exists()) {
          setBackground(Color.RED);
        } else {
          setBackground(Color.WHITE);
        }

        return super.getTableCellRendererComponent(table, value, isSelected, hasFocus,
            row, column);
      }
    });

    /* Add current projects */
    for (File projectDir : projects) {
      addProjectDir(projectDir);
    }

    Box mainPane = Box.createVerticalBox();
    Box buttonPane = Box.createHorizontalBox();
    JPanel smallPane;
    JButton button;

    /* Lower buttons */
    {
      buttonPane.setBorder(BorderFactory.createEmptyBorder(0,3,3,3));
      buttonPane.add(Box.createHorizontalGlue());

      button = new JButton("View merged config");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          ProjectConfig config;
          try {
            config = new ProjectConfig(true);
          } catch (FileNotFoundException ex) {
            logger.fatal("Could not find default project config file: " + GUI.PROJECT_DEFAULT_CONFIG_FILENAME);
            return;
          } catch (IOException ex) {
            logger.fatal("Error when reading default project config file: " + GUI.PROJECT_DEFAULT_CONFIG_FILENAME);
            return;
          }

          /* Merge configs */
          for (File project : getProjects()) {
            try {
              config.appendProjectDir(project);
            } catch (Exception ex) {
              logger.fatal("Error when merging configurations: " + ex);
              return;
            }
          }

          ConfigViewer.showDialog(ProjectDirectoriesDialog.this, config);
        }
      });
      buttonPane.add(button);
      buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

      button = new JButton("Cancel");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          ProjectDirectoriesDialog.this.finalProjects = null;
          dispose();
        }
      });
      buttonPane.add(button);

      buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

      button = new JButton("Save as default");
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          Object[] options = { "Ok", "Cancel" };

          String newDefaultProjectDirs = "";
          for (File f: currentProjects) {
            if (newDefaultProjectDirs != "") {
              newDefaultProjectDirs += ";";
            }

            newDefaultProjectDirs += gui.createPortablePath(f).getPath();
          }
          newDefaultProjectDirs = newDefaultProjectDirs.replace('\\', '/');

          String question = "External tools setting DEFAULT_PROJECTDIRS will change from:\n"
            + GUI.getExternalToolsSetting("DEFAULT_PROJECTDIRS", "").replace(';', '\n')
            + "\n\n to:\n\n"
            + newDefaultProjectDirs.replace(';', '\n');
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
          ProjectDirectoriesDialog.this.finalProjects = currentProjects.toArray(new File[0]);
          dispose();
        }
      });
      buttonPane.add(button);
      this.getRootPane().setDefaultButton(button);
    }

    /* Center: Tree and list*/
    {
      JPanel listPane = new JPanel(new BorderLayout());

      listPane.add(BorderLayout.WEST, treePanel = new DirectoryTreePanel(this));
      listPane.add(BorderLayout.CENTER, new JScrollPane(table));

      smallPane = new JPanel(new BorderLayout());
      Icon icon = UIManager.getLookAndFeelDefaults().getIcon("Table.ascendingSortIcon");
      if (icon == null) {
        button = new JButton("Up");
      } else {
        button = new JButton(icon);
      }
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          int selectedIndex = table.getSelectedRow();
          if (selectedIndex <= 0) {
            return;
          }
          File file = currentProjects.get(selectedIndex);
          removeProjectDir(file);
          addProjectDir(file, selectedIndex - 1);
          table.getSelectionModel().setSelectionInterval(selectedIndex - 1, selectedIndex - 1);
        }
      });
      smallPane.add(BorderLayout.NORTH, button);
      icon = UIManager.getLookAndFeelDefaults().getIcon("Table.descendingSortIcon");
      if (icon == null) {
        button = new JButton("Down");
      } else {
        button = new JButton(icon);
      }
      button.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          int selectedIndex = table.getSelectedRow();
          if (selectedIndex < 0) {
            return;
          }
          if (selectedIndex >= currentProjects.size() - 1) {
            return;
          }
          File file = currentProjects.get(selectedIndex);
          removeProjectDir(file);
          addProjectDir(file, selectedIndex + 1);
          table.getSelectionModel().setSelectionInterval(selectedIndex + 1, selectedIndex + 1);
        }
      });
      smallPane.add(BorderLayout.SOUTH, button);
      listPane.add(BorderLayout.EAST, smallPane);
      mainPane.setBackground(Color.WHITE);
      mainPane.add(listPane);
    }

    JPanel topPanel = new JPanel(new BorderLayout());
    topPanel.add(BorderLayout.CENTER, new JLabel(" A COOJA project depends on a cooja.config file, and extends COOJA with radio mediums, mote types, plugins etc."));
    topPanel.setBackground(Color.WHITE);
    getContentPane().add(BorderLayout.NORTH, topPanel);
    getContentPane().add(BorderLayout.CENTER, mainPane);
    getContentPane().add(BorderLayout.SOUTH, buttonPane);
    setSize(700, 500);
  }

  public File[] getProjects() {
    return currentProjects.toArray(new File[0]);
  }
  protected void addProjectDir(File projectDir) {
    currentProjects.add(projectDir);
    ((AbstractTableModel)table.getModel()).fireTableDataChanged();
  }
  protected void addProjectDir(File projectDir, int index) {
    currentProjects.add(index, projectDir);
    ((AbstractTableModel)table.getModel()).fireTableDataChanged();
  }
  protected void removeProjectDir(int index) {
    currentProjects.remove(index);
    ((AbstractTableModel)table.getModel()).fireTableDataChanged();
  }
  protected void removeProjectDir(File dir) {
    currentProjects.remove(dir);
    ((AbstractTableModel)table.getModel()).fireTableDataChanged();
  }
  private int getProjectListIndex(File dir) {
    return currentProjects.indexOf(dir);
  }
  public void selectListProject(File dir) {
    int i = getProjectListIndex(dir);
    if (i >= 0) {
      table.getSelectionModel().setSelectionInterval(i, i);
    }
  }
  public void selectTreeProject(File dir) {
    treePanel.selectProject(dir);
  }
}

/**
 * Shows a directory tree, and allows for selecting directories with a cooja.config file.  
 * 
 * @author Fredrik Osterlind
 */
class DirectoryTreePanel extends JPanel {
  private static final long serialVersionUID = -6852893350326771136L;
  private static Logger logger = Logger.getLogger(DirectoryTreePanel.class);

  private ProjectDirectoriesDialog parent;
  private JTree tree;
  private DefaultMutableTreeNode treeRoot;
  public DirectoryTreePanel(ProjectDirectoriesDialog parent) {
    super(new BorderLayout());
    this.parent = parent;

    /* Build directory tree */
    treeRoot = new DefaultMutableTreeNode("My Computer");
    tree = new JTree(treeRoot);
    tree.setRootVisible(false);
    tree.setShowsRootHandles(true);
    tree.expandRow(0);
    tree.setCellRenderer(new DefaultTreeCellRenderer() {
      private static final long serialVersionUID = 280434957859560569L;
      private Icon unselectedIcon = new CheckboxIcon(null);
      private Icon selectedIcon = new CheckboxIcon(new Color(0, 255, 0, 128));
      private Icon errorIcon = new CheckboxIcon(new Color(255, 0, 0, 128));
      private Font boldFont = null;
      private Font normalFont = null;
      public Component getTreeCellRendererComponent(JTree tree,
          Object value, boolean sel, boolean expanded, boolean leaf,
          int row, boolean hasFocus) {
        super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf,
            row, hasFocus);
        if (value instanceof DefaultMutableTreeNode) {
          value = ((DefaultMutableTreeNode) value).getUserObject();
        }
        if (!(value instanceof ProjectDirectory)) {
          return this;
        }
        ProjectDirectory td = (ProjectDirectory) value;

        if (boldFont == null) {
          normalFont = getFont();
          boldFont = getFont().deriveFont( Font.BOLD );
        }

        /* Style */
        setFont(normalFont);
        if (td.isProject()) {
          if (td.containsConfig()) {
            setIcon(selectedIcon);
          } else {
            /* Error: no cooja.config */
            setIcon(errorIcon);
            setFont(boldFont);
          }
        } else if (td.containsConfig()) {
          setIcon(unselectedIcon);
        } else if (td.subtreeContainsProject()) {
          setFont(boldFont);
        }

        return this;
      }
      class CheckboxIcon implements Icon {
        Icon icon;
        Color color;
        public CheckboxIcon(Color color) {
          this.icon = (Icon) UIManager.get("CheckBox.icon");
          this.color = color;
        }
        public int getIconHeight() {
          if (icon == null) {
            return 18;
          }
          return icon.getIconHeight();
        }
        public int getIconWidth() {
          if (icon == null) {
            return 18;
          }
          return icon.getIconWidth();
        }
        public void paintIcon(Component c, Graphics g, int x, int y) {
          if (icon != null) {
            try {
              icon.paintIcon(c, g, x, y);
            } catch (Exception e) {
              icon = null;
            }
          }
          if (icon == null) {
            g.setColor(Color.WHITE);
            g.fillRect(x+1, y+1, 16, 16);
            g.setColor(Color.BLACK);
            g.drawRect(x+1, y+1, 16, 16);
          }
          if (color != null) {
            g.setColor(color);
            g.fillRect(x, y, getIconWidth(), getIconHeight());
          }
        }
      }
    });
    tree.setModel(new COOJAProjectTreeModel(treeRoot));
    tree.addMouseListener(new MouseAdapter() {
      public void mousePressed(MouseEvent e) {
        TreePath selPath = tree.getPathForLocation(e.getX(), e.getY());
        if (selPath == null) {
          return;
        }
        if (e.getClickCount() != 1) {
          return;
        }
        Object o = selPath.getLastPathComponent();
        if (!(o instanceof DefaultMutableTreeNode)) {
          return;
        }
        if (!(((DefaultMutableTreeNode) o).getUserObject() instanceof ProjectDirectory)) {
          return;
        }
        ProjectDirectory pd = (ProjectDirectory) ((DefaultMutableTreeNode) o).getUserObject();
        Rectangle r = tree.getPathBounds(selPath);
        int delta = e.getX() - r.x;
        if (delta > 18 /* XXX Icon width */) {
          return;
        }

        if (pd.isProject()) {
          /* Remove project */
          DirectoryTreePanel.this.parent.removeProjectDir(pd.dir);
          DirectoryTreePanel.this.parent.repaint();
        } else if (pd.containsConfig()) {
          /* Add project */
          DirectoryTreePanel.this.parent.addProjectDir(pd.dir);
          DirectoryTreePanel.this.parent.repaint();
        }
      }
    });
    tree.addTreeSelectionListener(new TreeSelectionListener() {
      public void valueChanged(TreeSelectionEvent e) {
        TreePath selPath = e.getPath();
        if (selPath == null) {
          return;
        }
        Object o = selPath.getLastPathComponent();
        if (!(o instanceof DefaultMutableTreeNode)) {
          return;
        }
        if (!(((DefaultMutableTreeNode) o).getUserObject() instanceof ProjectDirectory)) {
          return;
        }
        ProjectDirectory pd = (ProjectDirectory) ((DefaultMutableTreeNode) o).getUserObject();
        if (pd.isProject()) {
          DirectoryTreePanel.this.parent.selectListProject(pd.dir);
        }
      }
    });

    /* Try expand current COOJA projects */
    for (File projectDir: parent.getProjects()) {
      if (!projectDir.exists()) {
        logger.fatal("Project directory not found: " + projectDir);
        continue;
      }
      try {
        String projectCanonical = projectDir.getCanonicalPath();
        TreePath tp = new TreePath(tree.getModel().getRoot());
        tp = buildTreePath(projectCanonical, treeRoot, tp, tree);
        /*logger.info("Expanding: " + tp);*/
        if (tp != null) {
          tree.expandPath(tp.getParentPath());
        }
      } catch (IOException ex) {
        logger.warn("Error when expanding projects: " + ex.getMessage());
      }
    }
    add(BorderLayout.CENTER, new JScrollPane(tree));
  }

  public void selectProject(File dir) {
    /* Expand view */
    try {
      String projectCanonical = dir.getCanonicalPath();
      TreePath tp = new TreePath(tree.getModel().getRoot());
      tp = buildTreePath(projectCanonical, treeRoot, tp, tree);
      /*logger.info("Expanding: " + tp);*/
      if (tp != null) {
        tree.setSelectionPath(tp);
        tree.scrollPathToVisible(tp);
      }
    } catch (IOException ex) {
      logger.warn("Error when expanding projects: " + ex.getMessage());
    }
  }

  private static TreePath buildTreePath(String projectCanonical, DefaultMutableTreeNode parent, TreePath tp, JTree tree)
  throws IOException {
    /* Force filesystem listing */
    tree.getModel().getChildCount(parent);

    for (int i=0; i < tree.getModel().getChildCount(parent); i++) {
      DefaultMutableTreeNode child = (DefaultMutableTreeNode) tree.getModel().getChild(parent, i);
      Object userObject = child.getUserObject();
      if (!(userObject instanceof ProjectDirectory)) {
        logger.fatal("Bad tree element: " + userObject.getClass());
        continue;
      }
      ProjectDirectory td = (ProjectDirectory) userObject;
      String treeCanonical = td.dir.getCanonicalPath();

      if (projectCanonical.startsWith(treeCanonical)) {
        tp = tp.pathByAddingChild(child);
        if (projectCanonical.equals(treeCanonical)) {
          return tp;
        }

        return buildTreePath(projectCanonical, child, tp, tree);
      }
    }
    return null;
  }

  private class ProjectDirectory {
    File dir = null;
    File[] subdirs = null;

    public ProjectDirectory(File file) {
      this.dir = file;
    }

    boolean isProject() {
      for (File project: parent.getProjects()) {
        if (project.equals(dir)) {
          return true;
        }
      }
      return false;
    }
    boolean containsConfig() {
      return new File(dir, GUI.PROJECT_CONFIG_FILENAME).exists();
    }
    boolean subtreeContainsProject() {
      try {
        String dirCanonical = dir.getCanonicalPath();
        for (File project: parent.getProjects()) {
          if (!project.exists()) {
            continue;
          }
          String projectCanonical = project.getCanonicalPath();
          if (projectCanonical.startsWith(dirCanonical)) {
            return true;
          }
        }
      } catch (IOException ex) {
      }
      return false;
    }
    public String toString() {
      if (dir.getName() == null || dir.getName().equals("")) {
        return dir.getAbsolutePath();
      }
      return dir.getName();
    }
  }
  private class COOJAProjectTreeModel extends DefaultTreeModel {
    private static final long serialVersionUID = -4673855124090194313L;

    private DefaultMutableTreeNode computerNode;

    public COOJAProjectTreeModel(DefaultMutableTreeNode computerNode) {
      super(computerNode);
      this.computerNode = computerNode;

      /* List roots */
      File[] devices = File.listRoots();
      if (devices == null) {
        logger.fatal("Could not list filesystem");
        return;
      }
      for (File device: devices) {
        DefaultMutableTreeNode deviceNode = new DefaultMutableTreeNode(new ProjectDirectory(device));
        computerNode.add(deviceNode);
      }
    }
    public Object getRoot() {
      return computerNode.getUserObject();
    }
    public boolean isLeaf(Object node) {  
      if ((node instanceof DefaultMutableTreeNode)) {
        node = ((DefaultMutableTreeNode)node).getUserObject();
      }
      if (!(node instanceof ProjectDirectory)) {
        /* Computer node */
        return false;
      }
      ProjectDirectory td = ((ProjectDirectory)node);

      return td.dir.isFile();
    }
    public int getChildCount(Object parent) {
      if ((parent instanceof DefaultMutableTreeNode)) {
        parent = ((DefaultMutableTreeNode)parent).getUserObject();
      }
      if (!(parent instanceof ProjectDirectory)) {
        /* Computer node */
        return computerNode.getChildCount();
      }
      ProjectDirectory td = ((ProjectDirectory)parent);

      File[] children;
      if (td.subdirs != null) {
        children = td.subdirs;
      } else {
        children = td.dir.listFiles(DIRECTORIES);
        td.subdirs = children;
      }
      if (children == null) {
        return 0;
      }
      return children.length;
    }
    public Object getChild(Object parent, int index) {
      if ((parent instanceof DefaultMutableTreeNode)) {
        parent = ((DefaultMutableTreeNode)parent).getUserObject();
      }
      if (!(parent instanceof ProjectDirectory)) {
        /* Computer node */
        return computerNode.getChildAt(index);
      }
      ProjectDirectory td = ((ProjectDirectory)parent);

      File[] children;
      if (td.subdirs != null) {
        children = td.subdirs;
      } else {
        children = td.dir.listFiles(DIRECTORIES);
        td.subdirs = children;
      }
      if ((children == null) || (index >= children.length)) {
        return null;
      }
      return new DefaultMutableTreeNode(new ProjectDirectory(children[index]));
    }
    public int getIndexOfChild(Object parent, Object child) {
      if ((parent instanceof DefaultMutableTreeNode)) {
        parent = ((DefaultMutableTreeNode)parent).getUserObject();
      }
      if (!(parent instanceof ProjectDirectory)) {
        /* Computer node */
        for(int i=0; i < computerNode.getChildCount(); i++) {
          if (computerNode.getChildAt(i).equals(child)) {
            return i;
          }
        }
      }
      ProjectDirectory td = ((ProjectDirectory)parent);

      File[] children;
      if (td.subdirs != null) {
        children = td.subdirs;
      } else {
        children = td.dir.listFiles(DIRECTORIES);
        td.subdirs = children;
      }
      if (children == null) {
        return -1;
      }
      if (child instanceof DefaultMutableTreeNode) {
        child = ((DefaultMutableTreeNode)child).getUserObject();
      }
      File subDir = ((ProjectDirectory)child).dir;
      for(int i = 0; i < children.length; i++) {
        if (subDir.equals(children[i])) {
          return i;
        }
      }
      return -1;
    }

    public void valueForPathChanged(TreePath path, Object newvalue) {}
    public void addTreeModelListener(TreeModelListener l) {}
    public void removeTreeModelListener(TreeModelListener l) {}

    private final FileFilter DIRECTORIES = new FileFilter() {
      public boolean accept(File file) {
        if (file.isDirectory()) {
          return true;
        }
        return false;
      }
    };
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
