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
 */

package se.sics.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.Frame;
import java.awt.GraphicsEnvironment;
import java.awt.Rectangle;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.InputMap;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTabbedPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.filechooser.FileFilter;

import org.apache.log4j.Logger;

import se.sics.cooja.GUI;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.interfaces.MoteID;
import se.sics.cooja.interfaces.Position;

/**
 * Abstract configure mote type dialog used by Contiki-based mote type implementations.
 *
 * The dialog uses tabs for configuring the mote type.
 * Three tabs are provided: compile commands, mote interfaces, and compilation output.
 *
 * In addition, the dialog implementation can provide additional tabs.
 *
 * @see #tabbedPane
 * @see ContikiMoteCompileDialog
 *
 * @author Fredrik Osterlind
 */
public abstract class AbstractCompileDialog extends JDialog {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(AbstractCompileDialog.class);

  protected final static Dimension LABEL_DIMENSION = new Dimension(170, 25);

  private static File lastFile = null;

  public static enum DialogState {
    NO_SELECTION,
    SELECTED_SOURCE, AWAITING_COMPILATION, IS_COMPILING, COMPILED_FIRMWARE,
    SELECTED_FIRMWARE,
  }

  protected Simulation simulation;
  protected GUI gui;
  protected MoteType moteType;

  protected JTabbedPane tabbedPane;
  protected Box moteIntfBox;

  protected JTextField contikiField;
  private JTextField descriptionField;
  private JTextArea commandsArea;
  private JButton cleanButton;
  private JButton compileButton;
  private JButton createButton;

  private Component currentCompilationOutput = null;
  private Process currentCompilationProcess = null;

  /* Accessible at Contiki compilation success */
  public File contikiSource = null;
  public File contikiFirmware = null;

  public AbstractCompileDialog(Container parent, Simulation simulation, final MoteType moteType) {
    super(
        parent instanceof Dialog?(Dialog)parent:
          parent instanceof Window?(Window)parent:
            (Frame)parent, "Create Mote Type: Compile Contiki", ModalityType.APPLICATION_MODAL);

    this.simulation = simulation;
    this.gui = simulation.getGUI();
    this.moteType = moteType;

    JPanel mainPanel = new JPanel(new BorderLayout());
    JLabel label;

    /* Top: Contiki source */
    JPanel topPanel = new JPanel();
    topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));
    topPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    Box descriptionBox = Box.createHorizontalBox();
    label = new JLabel("Description:");
    label.setPreferredSize(LABEL_DIMENSION);
    descriptionBox.add(label);
    descriptionField = new JTextField(40);
    descriptionField.setText("[enter mote type description]");
    descriptionBox.add(descriptionField);

    topPanel.add(descriptionBox);

    JPanel sourcePanel = new JPanel();
    sourcePanel.setLayout(new BoxLayout(sourcePanel, BoxLayout.X_AXIS));
    label = new JLabel("Contiki process / Firmware:");
    label.setPreferredSize(LABEL_DIMENSION);
    sourcePanel.add(label);
    contikiField = new JTextField(40);
    final Runnable selectedContikiFile = new Runnable() {
      public void run() {
        setContikiSelection(new File(contikiField.getText()));
      }
    };
    DocumentListener contikiFieldListener = new DocumentListener() {
      public void changedUpdate(DocumentEvent e) {
        selectedContikiFile.run();
      }
      public void insertUpdate(DocumentEvent e) {
        selectedContikiFile.run();
      }
      public void removeUpdate(DocumentEvent e) {
        selectedContikiFile.run();
      }
    };
    sourcePanel.add(contikiField);
    JButton browseButton = new JButton("Browse");
    browseButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        JFileChooser fc = new JFileChooser();

        File fp = new File(contikiField.getText());
        if (fp.exists() && fp.isFile()) {
            lastFile = fp;
        }
        if (lastFile == null) {
          String path = GUI.getExternalToolsSetting("COMPILE_LAST_FILE", null);
          if (path != null) {
            lastFile = new File(path);
            lastFile = gui.restorePortablePath(lastFile);
          }
        }

        /* Last file/directory */
        if (lastFile != null) {
          if (lastFile.isDirectory()) {
            fc.setCurrentDirectory(lastFile);
          } else if (lastFile.isFile() && lastFile.exists()) {
            fc.setCurrentDirectory(lastFile.getParentFile());
            fc.setSelectedFile(lastFile);
          } else if (lastFile.isFile() && !lastFile.exists()) {
            fc.setCurrentDirectory(lastFile.getParentFile());
          }
        } else {
          File helloworldSourceFile =
            new java.io.File(
                GUI.getExternalToolsSetting("PATH_CONTIKI"), "examples/hello-world/hello-world.c");
          try {
            helloworldSourceFile = helloworldSourceFile.getCanonicalFile();
            fc.setCurrentDirectory(helloworldSourceFile.getParentFile());
            fc.setSelectedFile(helloworldSourceFile);
          } catch (IOException e1) {
          }
        }

        fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
        fc.addChoosableFileFilter(new FileFilter() {
          public boolean accept(File f) {
            if (f.isDirectory()) {
              return true;
            }

            String filename = f.getName();
            if (filename == null) {
              return false;
            }

            if (filename.endsWith(".c")) {
              return true;
            }

            if (canLoadFirmware(f)) {
              return true;
            }

            return false;
          }

          public String getDescription() {
            return "Contiki process source or Precompiled firmware";
          }
        });
        fc.setDialogTitle("Select Contiki process source");

        if (fc.showOpenDialog(AbstractCompileDialog.this) == JFileChooser.APPROVE_OPTION) {
          contikiField.setText(fc.getSelectedFile().getAbsolutePath());
        }
      }
    });
    sourcePanel.add(browseButton);

    topPanel.add(sourcePanel);

    Action compileAction = new AbstractAction("Compile") {
    	private static final long serialVersionUID = 1L;
    	public void actionPerformed(ActionEvent e) {
    		if (!compileButton.isEnabled()) {
    			return;
    		}
    		try {
    			setDialogState(DialogState.AWAITING_COMPILATION);
    			compileContiki();
    		} catch (Exception e1) {
    			logger.fatal("Error while compiling Contiki: " + e1.getMessage());
    		}
    	}
    };
    cleanButton = new JButton("Clean");
    cleanButton.setToolTipText("make clean TARGET=" + getTargetName());
    cleanButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				try {
					currentCompilationProcess = CompileContiki.compile(
							"make clean TARGET=" + getTargetName(),
							compilationEnvironment,
							null /* Do not observe output firmware file */,
							new File(contikiField.getText()).getParentFile(),
							null,
							null,
							null,
							true
					);
				} catch (Exception e1) {
					logger.fatal("Clean failed: " + e1.getMessage(), e1);
				}
			}
		});

    compileButton = new JButton(compileAction);
    getRootPane().setDefaultButton(compileButton);


    createButton = new JButton("Create");
    createButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
      	/* Write mote type settings (generic) */
      	moteType.setDescription(descriptionField.getText());
      	moteType.setContikiSourceFile(contikiSource);
      	moteType.setContikiFirmwareFile(contikiFirmware);
      	moteType.setMoteInterfaceClasses(getSelectedMoteInterfaceClasses());
      	moteType.setCompileCommands(getCompileCommands());

      	/* Write mote type settings (mote type specific) */
      	writeSettingsToMoteType();

      	AbstractCompileDialog.this.dispose();
      }
    });

    Box buttonPanel = Box.createHorizontalBox();
    buttonPanel.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));
    buttonPanel.add(Box.createHorizontalGlue());
    buttonPanel.add(cleanButton);
    buttonPanel.add(Box.createHorizontalStrut(5));
    buttonPanel.add(compileButton);
    buttonPanel.add(Box.createHorizontalStrut(5));
    buttonPanel.add(createButton);
    buttonPanel.setAlignmentX(LEFT_ALIGNMENT);

    topPanel.add(buttonPanel);

    /* Center: Tabs showing configuration, compilation output, ... */
    tabbedPane = new JTabbedPane();
    addCompileCommandTab(tabbedPane);
    addMoteInterfacesTab(tabbedPane);

    /* Build panel */
    mainPanel.add(BorderLayout.NORTH, topPanel);
    mainPanel.add(BorderLayout.CENTER, tabbedPane);
    mainPanel.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
    setContentPane(mainPanel);

    setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        abortAnyCompilation();
        contikiSource = null;
        contikiFirmware = null;
        AbstractCompileDialog.this.dispose();
      }
    });

    /* Restore old configuration if mote type is already configured */
    boolean restoredDialogState = false;
    if (moteType != null) {
      /* Restore description */
      if (moteType.getDescription() != null) {
        descriptionField.setText(moteType.getDescription());
      }

      /* Restore Contiki source or firmware */
      if (moteType.getContikiSourceFile() != null) {
        contikiField.setText(moteType.getContikiSourceFile().getAbsolutePath());
        setDialogState(DialogState.SELECTED_SOURCE);
        restoredDialogState = true;
      } else if (moteType.getContikiFirmwareFile() != null) {
        contikiField.setText(moteType.getContikiFirmwareFile().getAbsolutePath());
        setDialogState(DialogState.SELECTED_FIRMWARE);
        restoredDialogState = true;
      }

      /* Restore mote interface classes */
      for (Component c : moteIntfBox.getComponents()) {
        if (!(c instanceof JCheckBox)) {
          continue;
        }
        ((JCheckBox) c).setSelected(false);
      }
      if (moteType.getMoteInterfaceClasses() != null) {
        for (Class<? extends MoteInterface> intfClass: getDefaultMoteInterfaces()) {
          addMoteInterface(intfClass, false);
        }
        for (Class<? extends MoteInterface> intf: moteType.getMoteInterfaceClasses()) {
          addMoteInterface(intf, true);
        }
      } else {
        /* Select default mote interfaces */
        for (Class<? extends MoteInterface> intfClass: getDefaultMoteInterfaces()) {
          addMoteInterface(intfClass, true);
        }
      }

      /* Restore compile commands */
      if (moteType.getCompileCommands() != null) {
      	setCompileCommands(moteType.getCompileCommands());
      	setDialogState(DialogState.AWAITING_COMPILATION);
        restoredDialogState = true;
      }
    }
    if (!restoredDialogState) {
      setDialogState(DialogState.NO_SELECTION);
    }

    descriptionField.requestFocus();
    descriptionField.select(0, descriptionField.getText().length());

    /* Add listener only after restoring old config */
    contikiField.getDocument().addDocumentListener(contikiFieldListener);

    /* Final touches: respect window size, focus on description etc */
    Rectangle maxSize = GraphicsEnvironment.getLocalGraphicsEnvironment().getMaximumWindowBounds();
    if (maxSize != null &&
        (getSize().getWidth() > maxSize.getWidth() || getSize().getHeight() > maxSize.getHeight())) {
      Dimension newSize = new Dimension();
      newSize.height = Math.min((int) maxSize.getHeight(), (int) getSize().getHeight());
      newSize.width = Math.min((int) maxSize.getWidth(), (int) getSize().getWidth());
      /*logger.info("Resizing dialog: " + myDialog.getSize() + " -> " + newSize);*/
      setSize(newSize);
    }

    /* Recompile at Ctrl+R */
    InputMap inputMap = getRootPane().getInputMap(JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
    inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_R, KeyEvent.CTRL_DOWN_MASK, false), "recompile");
    getRootPane().getActionMap().put("recompile", compileAction);

    pack();
    setLocationRelativeTo(parent);
  }

  /**
   * @return Mote type was created without errors
   */
  public boolean createdOK() {
    if (contikiFirmware == null) {
      return false;
    }
    if (!contikiFirmware.exists()) {
      return false;
    }
    return true;
  }

  /**
   * Called when user is closing the dialog.
   * Should immediately write mote type specific settings
   * to the current mote type.
   *
   * Note that generic mote type settings are already written,
   * such as description, Contiki source, compile commands, and mote interfaces.
   */
  public abstract void writeSettingsToMoteType();

  public abstract boolean canLoadFirmware(File file);

  protected String[] compilationEnvironment = null; /* Default environment: inherit from current process */
  public void compileContiki() throws Exception {
    final MessageList taskOutput = new MessageList();

    if (contikiFirmware.exists()) {
      contikiFirmware.delete();
    }

    /* Handle multiple compilation commands one by one */
    final ArrayList<String> commands = new ArrayList<String>();
    String[] arr = getCompileCommands().split("\n");
    for (String cmd: arr) {
      if (cmd.trim().isEmpty()) {
        continue;
      }

      commands.add(cmd);
    }
    if (commands.isEmpty()) {
      throw new Exception("No compile commands specified");
    }

    SwingUtilities.invokeLater(new Runnable() {
			public void run() {
		    setDialogState(DialogState.IS_COMPILING);
			}
		});
    createNewCompilationTab(taskOutput);

    /* Add abort compilation menu item */
    final JMenuItem abortMenuItem = new JMenuItem("Abort compilation");
    abortMenuItem.setEnabled(true);
    abortMenuItem.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        abortAnyCompilation();
      }
    });
    taskOutput.addPopupMenuItem(abortMenuItem, true);

    /* Called when last command has finished (success only) */
    final Action compilationSuccessAction = new AbstractAction() {
			private static final long serialVersionUID = -3815068126197234346L;
			public void actionPerformed(ActionEvent e) {
        abortMenuItem.setEnabled(false);

        /* Make sure firmware exists */
        if (!contikiFirmware.exists()) {
          logger.fatal("Contiki firmware does not exist: " + contikiFirmware.getAbsolutePath());
          setDialogState(DialogState.AWAITING_COMPILATION);
          return;
        }
        setDialogState(DialogState.COMPILED_FIRMWARE);
      }
    };

    /* Called immediately if any command fails */
    final Action compilationFailureAction = new AbstractAction() {
			private static final long serialVersionUID = -800799353242963993L;
			public void actionPerformed(ActionEvent e) {
        abortMenuItem.setEnabled(false);
        setDialogState(DialogState.AWAITING_COMPILATION);
      }
    };

    /* Called once per command */
    final Action nextCommandAction = new AbstractAction() {
			private static final long serialVersionUID = -4525372566302330762L;
			public void actionPerformed(ActionEvent e) {
        Action nextSuccessAction;
        if (commands.size() == 1) {
          nextSuccessAction = compilationSuccessAction;
        } else {
          nextSuccessAction = this;
        }

        String command = commands.remove(0);
        if (command.trim().isEmpty()) {
          nextSuccessAction.actionPerformed(null);
        } else {
          try {
            currentCompilationProcess = CompileContiki.compile(
                command,
                compilationEnvironment,
                null /* Do not observe output firmware file */,
                new File(contikiField.getText()).getParentFile(),
                nextSuccessAction,
                compilationFailureAction,
                taskOutput,
                false
            );
          } catch (Exception ex) {
            logger.fatal("Exception when compiling: " + ex.getMessage());
            ex.printStackTrace();
            compilationFailureAction.actionPerformed(null);
          }
        }
      }
    };
    nextCommandAction.actionPerformed(null); /* Recursive calls for all commands */
  }

  private void setContikiSelection(File file) {
    if (!file.exists()) {
      setDialogState(DialogState.NO_SELECTION);
      return;
    }

    lastFile = file;
    GUI.setExternalToolsSetting("COMPILE_LAST_FILE", gui.createPortablePath(lastFile).getPath());

    if (file.getName().endsWith(".c")) {
      setDialogState(DialogState.SELECTED_SOURCE);
      return;
    }

    if (canLoadFirmware(file)) {
      setDialogState(DialogState.SELECTED_FIRMWARE);
      return;
    }

    setDialogState(DialogState.SELECTED_SOURCE);
  }

  /**
   * @see DialogState
   * @param dialogState New dialog state
   */
  protected void setDialogState(DialogState dialogState) {
    File sourceFile = new File(contikiField.getText());
    compileButton.setText("Compile");
    getRootPane().setDefaultButton(compileButton);

    switch (dialogState) {
    case NO_SELECTION:
    	cleanButton.setEnabled(false);
    	compileButton.setEnabled(false);
    	createButton.setEnabled(false);
    	commandsArea.setEnabled(false);
      break;

    case SELECTED_SOURCE:
      if (!sourceFile.exists()) {
        logger.warn("Could not find Contiki source: " + sourceFile.getAbsolutePath());
        setDialogState(DialogState.NO_SELECTION);
        return;
      }
      if (!sourceFile.getName().endsWith(".c")) {
        setDialogState(DialogState.NO_SELECTION);
        return;
      }

    	cleanButton.setEnabled(true);
    	compileButton.setEnabled(true);
    	createButton.setEnabled(false);
    	commandsArea.setEnabled(true);
      setCompileCommands(getDefaultCompileCommands(sourceFile));
      contikiFirmware = getExpectedFirmwareFile(sourceFile);
      contikiSource = sourceFile;
      setDialogState(DialogState.AWAITING_COMPILATION);
      break;

    case AWAITING_COMPILATION:
      if (!sourceFile.exists()) {
        logger.warn("Could not find Contiki source: " + sourceFile.getAbsolutePath());
        setDialogState(DialogState.NO_SELECTION);
        return;
      }
      if (!sourceFile.getName().endsWith(".c")) {
        setDialogState(DialogState.NO_SELECTION);
        return;
      }

      cleanButton.setEnabled(true);
      compileButton.setEnabled(true);
      createButton.setEnabled(false);
      commandsArea.setEnabled(true);
      break;

    case IS_COMPILING:
    	cleanButton.setEnabled(false);
    	compileButton.setEnabled(false);
    	compileButton.setText("Compiling");
    	createButton.setEnabled(false);
    	commandsArea.setEnabled(false);
      break;

    case COMPILED_FIRMWARE:
    	cleanButton.setEnabled(true);
    	compileButton.setEnabled(true);
    	createButton.setEnabled(true);
    	commandsArea.setEnabled(true);
      getRootPane().setDefaultButton(createButton);
      break;

    case SELECTED_FIRMWARE:
      contikiSource = null;
      contikiFirmware = new File(contikiField.getText());
      if (!contikiFirmware.exists()) {
        setDialogState(DialogState.NO_SELECTION);
        return;
      }
      if (!canLoadFirmware(contikiFirmware)) {
        setDialogState(DialogState.NO_SELECTION);
        return;
      }

    	cleanButton.setEnabled(true);
    	compileButton.setEnabled(false);
    	createButton.setEnabled(true);
    	commandsArea.setEnabled(false);
    	setCompileCommands("");
      getRootPane().setDefaultButton(createButton);
      break;

    default:
      break;
    }
  }

  private void addCompileCommandTab(JTabbedPane parent) {
    commandsArea = new JTextArea(10, 1);
    commandsArea.getDocument().addDocumentListener(new DocumentListener() {
      public void changedUpdate(DocumentEvent e) {
        setDialogState(DialogState.AWAITING_COMPILATION);
      }
      public void insertUpdate(DocumentEvent e) {
        setDialogState(DialogState.AWAITING_COMPILATION);
      }
      public void removeUpdate(DocumentEvent e) {
        setDialogState(DialogState.AWAITING_COMPILATION);
      }
    });
    parent.addTab("Compile commands", null, new JScrollPane(commandsArea), "Manually alter Contiki compilation commands");
  }

  private void addMoteInterfacesTab(JTabbedPane parent) {
    moteIntfBox = Box.createVerticalBox();
    JPanel panel = new JPanel(new BorderLayout());
    JLabel label = new JLabel("Cooja interacts with simulated motes via mote interfaces. These settings normally do not need to be changed.");
    Box b = Box.createHorizontalBox();
    b.add(new JButton(defaultAction));
    b.add(label);
    panel.add(BorderLayout.NORTH, b);
    panel.add(BorderLayout.CENTER, new JScrollPane(moteIntfBox));
    parent.addTab("Mote interfaces", null, panel, "Mote interfaces");
  }

  private Action defaultAction = new AbstractAction("Use default") {
    private static final long serialVersionUID = 2874355910493988933L;

    public void actionPerformed(ActionEvent e) {
      /* Unselect all */
      for (Component c : moteIntfBox.getComponents()) {
        if (!(c instanceof JCheckBox)) {
          continue;
        }
        ((JCheckBox) c).setSelected(false);
      }

      /* Select default */
      for (Class<? extends MoteInterface> moteIntf : getDefaultMoteInterfaces()) {
        addMoteInterface(moteIntf, true);
      }
    }
  };

  public abstract Class<? extends MoteInterface>[] getDefaultMoteInterfaces();

  /**
   * @return Currently selected mote interface classes
   */
  public Class<? extends MoteInterface>[] getSelectedMoteInterfaceClasses() {
    ArrayList<Class<? extends MoteInterface>> selected = new ArrayList<Class<? extends MoteInterface>>();

    for (Component c : moteIntfBox.getComponents()) {
      if (!(c instanceof JCheckBox)) {
        continue;
      }

      if (!((JCheckBox) c).isSelected()) {
        continue;
      }

      Class<? extends MoteInterface> intfClass =
        (Class<? extends MoteInterface>)
        ((JCheckBox) c).getClientProperty("class");

      if (intfClass == null) {
        continue;
      }

      selected.add(intfClass);
    }

    Class<? extends MoteInterface>[] arr = new Class[selected.size()];
    return selected.toArray(arr);
  }

  /**
   * Adds given mote interface to mote interface list, represented by a checkbox.
   * If mote interface already exists, this method call is ignored.
   *
   * @param intfClass Mote interface class
   * @param selected If true, interface will initially be selected
   */
  public void addMoteInterface(Class<? extends MoteInterface> intfClass, boolean selected) {
    /* If mote interface was already added  */
    for (Component c : moteIntfBox.getComponents()) {
      if (!(c instanceof JCheckBox)) {
        continue;
      }

      Class<? extends MoteInterface> existingClass =
        (Class<? extends MoteInterface>)
        ((JCheckBox) c).getClientProperty("class");

      if (existingClass == null) {
        continue;
      }

      if (existingClass == intfClass) {
        ((JCheckBox) c).setSelected(selected);
        return;
      }
    }

    /* Create new mote interface checkbox */
    JCheckBox intfCheckBox = new JCheckBox(GUI.getDescriptionOf(intfClass));
    intfCheckBox.setSelected(selected);
    intfCheckBox.putClientProperty("class", intfClass);
    intfCheckBox.setAlignmentX(Component.LEFT_ALIGNMENT);
    intfCheckBox.setToolTipText(intfClass.getName());
    intfCheckBox.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (contikiSource == null &&
            contikiFirmware != null) {
          setDialogState(DialogState.SELECTED_FIRMWARE);
        } else if (contikiSource != null){
          setDialogState(DialogState.AWAITING_COMPILATION);
        } else {
          setDialogState(DialogState.SELECTED_SOURCE);
        }
      }
    });

    /* Always select position and ID interface */
    if (intfClass == Position.class ||
        intfClass == MoteID.class) {
      intfCheckBox.setEnabled(false);
      intfCheckBox.setSelected(true);
    }

    moteIntfBox.add(intfCheckBox);
  }

  /**
   * @param commands User configured compile commands
   */
  public void setCompileCommands(String commands) {
    /* TODO Merge from String[] */
    commandsArea.setText(commands);
  }

  /**
   * @return User configured compile commands
   */
  public String getCompileCommands() {
    /* TODO Split into String[] */
    return commandsArea.getText();
  }

  /**
   * @param source Contiki source
   * @return Suggested compile commands for compiling source
   */
  public abstract String getDefaultCompileCommands(File source);

  /**
   * @param source Contiki source
   * @return Expected Contiki firmware compiled from source
   */
  public abstract File getExpectedFirmwareFile(File source);

  private void abortAnyCompilation() {
    if (currentCompilationProcess == null) {
      return;
    }
    currentCompilationProcess.destroy();
    currentCompilationProcess = null;
  }

  private boolean createNewCompilationTab(MessageList output) {
    abortAnyCompilation();
    tabbedPane.remove(currentCompilationOutput);

    JScrollPane scrollOutput = new JScrollPane(output);
    tabbedPane.addTab("Compilation output", null, scrollOutput, "Shows Contiki compilation output");

    tabbedPane.setSelectedComponent(scrollOutput);
    currentCompilationOutput = scrollOutput;
    return true;
  }

  protected abstract String getTargetName();
}
