/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
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
 * $Id: MspMoteType.java,v 1.23 2009/03/03 13:52:35 fros4943 Exp $
 */

package se.sics.cooja.mspmote;

import java.awt.*;
import java.awt.Dialog.ModalityType;
import java.awt.event.*;
import java.io.*;
import java.util.Collection;
import java.util.Vector;
import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.filechooser.FileFilter;
import org.apache.log4j.Logger;
import org.jdom.Element;
import se.sics.cooja.*;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.dialogs.MessageList.MessageContainer;

@ClassDescription("Msp Mote Type")
public abstract class MspMoteType implements MoteType {
  private static Logger logger = Logger.getLogger(MspMoteType.class);

  protected static String getTargetFileExtension(String target) {
    return "." + target;
  }

  /* Convenience: Preselecting last used directory  */
  protected static File lastParentDirectory = null;

  private String identifier = null;
  private String description = null;

  /* If source file is defined, (re)compilation is performed */
  private File fileFirmware = null;
  private File fileSource = null;
  private String compileCommand = null;

  public String getIdentifier() {
    return identifier;
  }

  public void setIdentifier(String identifier) {
    this.identifier = identifier;
  }

  public String getDescription() {
    return description;
  }

  public void setDescription(String description) {
    this.description = description;
  }

  /**
   * Set ELF file.
   *
   * @param file
   *          ELF file
   */
  public void setELFFile(File file) {
    this.fileFirmware = file;
  }

  /**
   * Set compile command.
   *
   * @param command
   *          Compile command
   */
  public void setCompileCommand(String command) {
    this.compileCommand = command;
  }

  /**
   * @return ELF file
   */
  public File getELFFile() {
    return fileFirmware;
  }

  /**
   * @return Compile command
   */
  public String getCompileCommand() {
    return compileCommand;
  }

  /**
   * Set source file
   *
   * @param file Source file
   */
  public void setSourceFile(File file) {
    fileSource = file;
  }

  /**
   * @return Source file
   */
  public File getSourceFile() {
    return fileSource;
  }

  public final Mote generateMote(Simulation simulation) {
    MspMote mote = createMote(simulation);
    mote.initMote();
    return mote;
  }

  protected abstract MspMote createMote(Simulation simulation);

  /**
   * Configures and initialized Msp mote types.
   *
   * @param parentContainer Graphical parent container
   * @param simulation Current simulation
   * @param visAvailable Enable graphical interfaces and user input
   * @param target Contiki target platform name
   * @param targetNice Nicer representation of target
   * @return True is successful
   * @throws MoteTypeCreationException Mote type creation failed
   */
  protected boolean configureAndInitMspType(Container parentContainer, Simulation simulation,
      boolean visAvailable, final String target, final String targetNice)
  throws MoteTypeCreationException {
    boolean compileFromSource = false;

    if (getIdentifier() == null && !visAvailable) {
      throw new MoteTypeCreationException("No identifier");
    }

    /* Generate unique identifier */
    if (getIdentifier() == null) {
      int counter = 0;
      boolean identifierOK = false;
      while (!identifierOK) {
        counter++;
        setIdentifier(target + counter);
        identifierOK = true;

        // Check if identifier is already used by some other type
        for (MoteType existingMoteType : simulation.getMoteTypes()) {
          if (existingMoteType != this
              && existingMoteType.getIdentifier().equals(getIdentifier())) {
            identifierOK = false;
            break;
          }
        }
      }

      if (getDescription() == null) {
        setDescription(targetNice + " Mote Type #" + counter);
      }

      /* Let user choose whether to compile or load existing binaries */
      Object[] options = { "Compile", "Existing" };

      String question = targetNice + " mote type loads a firmware (ELF).\n\n"
          + "To compile a Contiki application from source: 'Compile'\n"
          + "To use a pre-compiled existing firmware: 'Existing'.\n";
      String title = "Compile or load existing " + targetNice + " firmware";

      if (GUI.isVisualizedInApplet()) {
        compileFromSource = false;
      } else {
        int answer = JOptionPane.showOptionDialog(GUI.getTopParentContainer(),
            question, title, JOptionPane.YES_NO_OPTION,
            JOptionPane.QUESTION_MESSAGE, null, options, options[0]);

        if (answer != JOptionPane.YES_OPTION && answer != JOptionPane.NO_OPTION) {
          return false;
        }
        compileFromSource = answer == JOptionPane.YES_OPTION;
      }
    }

    /* Description */
    if (getDescription() == null) {
      setDescription(targetNice + " Mote Type #" + getIdentifier());
    }

    if (getSourceFile() != null) {
      compileFromSource = true;
    }

    if (compileFromSource) {
      MspELFCompiler compiler = new MspELFCompiler(target);
      compiler.setCompileCommand(compileCommand);

      if (visAvailable) {
        boolean success = compiler.showDialog(GUI.getTopParentContainer(), this);
        if (success) {
          setSourceFile(compiler.getSourceFile());
          setELFFile(compiler.getOutputFile());
          setCompileCommand(compiler.getLastCompileCommand());
          return true;
        } else {
          return false;
        }
      } else {
        MessageList compilationOutput = new MessageList();
        try {

          /* Automatically clean if not visualized */
          if (!GUI.isVisualized()) {
            compiler.cleanTempFiles(compilationOutput, getSourceFile().getParentFile(), true);
          }

          compiler.compileFirmware(getSourceFile(), null, null, compilationOutput, true);

        } catch (Exception e) {
          MoteTypeCreationException newException = new MoteTypeCreationException(
              "Mote type creation failed: " + e.getMessage());
          newException = (MoteTypeCreationException) newException.initCause(e);
          newException.setCompilationOutput(compilationOutput);

          /* Print last 5 compilation errors */
          MessageContainer[] messages = compilationOutput.getMessages();
          for (int i=messages.length-5; i < messages.length; i++) {
            if (i < 0) {
              continue;
            }
            logger.fatal(">> " + messages[i]);
          }

          logger.fatal("Compilation error: " + e.getMessage());
          throw newException;
        }

        setSourceFile(compiler.getSourceFile());
        setELFFile(compiler.getOutputFile());
        setCompileCommand(compiler.getLastCompileCommand());

        return true;
      }
    }

    if (GUI.isVisualizedInApplet()) {
      return true;
    }

    // Check dependency files
    if (getELFFile() == null || !getELFFile().exists()) {
      if (!visAvailable) {
        throw new MoteTypeCreationException("ELF file does not exist: " + getELFFile());
      }

      JFileChooser fc = new JFileChooser();

      // Select previous directory
      if (lastParentDirectory != null) {
        fc.setCurrentDirectory(lastParentDirectory);
      } else {
        fc.setCurrentDirectory(new java.io.File(GUI.getExternalToolsSetting("PATH_CONTIKI")));
      }
      fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
      fc.addChoosableFileFilter(new FileFilter() {
        public boolean accept(File f) {
          String filename = f.getName();
          if (f.isDirectory()) {
            return true;
          }

          if (filename != null) {
            if (filename.endsWith(getTargetFileExtension(target)) || filename.endsWith(".firmware")) {
              return true;
            }
          }
          return false;
        }

        public String getDescription() {
          return "Firmware files";
        }
      });
      fc.addChoosableFileFilter(new FileFilter() {
        public boolean accept(File f) {
          String filename = f.getName();
          if (f.isDirectory()) {
            return true;
          }

          if (filename != null) {
            if (filename.endsWith(getTargetFileExtension(target))) {
              return true;
            }
          }
          return false;
        }

        public String getDescription() {
          return targetNice + " firmware files";
        }
      });
      fc.setDialogTitle("Select firmware file");

      if (fc.showOpenDialog(parentContainer) == JFileChooser.APPROVE_OPTION) {
        File selectedFile = fc.getSelectedFile();

        if (!selectedFile.exists()) {
          logger.fatal("Selected file \"" + selectedFile + "\" does not exist");
          return false;
        }

//        if (!selectedFile.getName().endsWith(firmwareFileExtension)) {
//          logger.fatal("Selected file \"" + selectedFile + "\" does not end with " + firmwareFileExtension);
//          return false;
//        }

        setELFFile(fc.getSelectedFile());
      } else {
        return false;
      }
    }
    return true;
  }

  protected static class MspELFCompiler {
    private static int LABEL_WIDTH = 170;

    private static int LABEL_HEIGHT = 15;

    private JButton cancelButton = new JButton("Cancel");

    private JButton cleanButton = new JButton("Clean");

    private JButton compileButton = new JButton("Compile");

    private JButton createButton = new JButton("Create");

    private JTextField sourceTextField = new JTextField();

    private JTextField compileCommandTextField = new JTextField();

    // private JFormattedTextField nodeIDTextField;
    // private NumberFormat integerFormat = NumberFormat.getIntegerInstance();

    private String lastCompileCommand = null;

    private File sourceFile = null;

    private File ELFFile = null;

    private String target;

    private JDialog myDialog;

    private String customizedCompileCommand = null;

    private Process compileProcess;

    static enum DialogState {
      NO_SOURCE, SELECTED_SOURCE, IS_COMPILING, COMPILED_SOURCE
    }

    public MspELFCompiler(String target) {
      this.target = target;
    }

    private String getCompileCommand(String filename) {
      if (customizedCompileCommand != null) {
        return customizedCompileCommand;
      }
      return GUI.getExternalToolsSetting("PATH_MAKE") + " " + filename + getTargetFileExtension(target) + " TARGET=" + target;
    }

    private void setCompileCommand(String command) {
      if (command == null || command.isEmpty()) {
        customizedCompileCommand = null;
        return;
      }
      customizedCompileCommand = command;
    }

    public void cleanTempFiles(final MessageList taskOutput, final File parentDir, boolean synch) {
      Thread t = new Thread(new Runnable() {
        public void run() {
          try {
            compileFirmware(
                "make clean TARGET=" + target,
                null,
                null,
                parentDir,
                null,
                null,
                taskOutput,
                true);
          } catch (Exception e2) {
          }
        }
      });
      t.start();
      if (synch) {
        try {
          t.join();
        } catch (InterruptedException e) {
          e.printStackTrace();
        }
      }
    }

    /**
     * @return Compiler output
     */
    public File getOutputFile() {
      return ELFFile;
    }

    public File getSourceFile() {
      return sourceFile;
    }

    public String getLastCompileCommand() {
      return lastCompileCommand;
    }

    private void updateDialog(DialogState dialogState) {
      switch (dialogState) {
      case NO_SOURCE:
        compileButton.setEnabled(false);
        createButton.setEnabled(false);
        compileCommandTextField.setText("");
        break;

      case IS_COMPILING:
        compileButton.setEnabled(false);
        createButton.setEnabled(false);
        break;

      case SELECTED_SOURCE:
        File sourceFile = new File(sourceTextField.getText());
        if (!sourceFile.exists()) {
          updateDialog(DialogState.NO_SOURCE);
          break;
        }

        File parentDirectory = sourceFile.getParentFile();
        if (!parentDirectory.exists()) {
          updateDialog(DialogState.NO_SOURCE);
          break;
        }

        if (!sourceFile.getName().endsWith(".c")) {
          updateDialog(DialogState.NO_SOURCE);
          break;
        }

        String name = sourceFile.getName().substring(0,
            sourceFile.getName().length() - 2);

        compileButton.setEnabled(true);
        createButton.setEnabled(false);
        compileCommandTextField.setText(getCompileCommand(name));
        compileButton.requestFocusInWindow();
        break;

      case COMPILED_SOURCE:
        compileButton.setEnabled(true);
        createButton.setEnabled(true);

        createButton.requestFocusInWindow();
        myDialog.getRootPane().setDefaultButton(createButton);
        break;

      default:
        break;
      }
    }

    protected void compileFirmware(final File sourceFile,
        final Action successAction, final Action failAction,
        final MessageList compilationOutput, boolean synchronous) throws Exception {
      final File parentDirectory = sourceFile.getParentFile();

      if (!sourceFile.getName().endsWith(".c")) {
        logger.fatal("Source file does not end with '.c'");
        return;
      }

      final String filenameNoExtension = sourceFile.getName().substring(0,
          sourceFile.getName().length() - 2);

      final String command = getCompileCommand(filenameNoExtension);
      logger.info("-- Compiling MSP430 Firmware --");

      compileFirmware(command, sourceFile, filenameNoExtension + getTargetFileExtension(target),
          parentDirectory,
          successAction, failAction,
          compilationOutput, synchronous);
    }

    protected void compileFirmware(
        final String command, final File sourceFile,
        final String firmware, final File parentDirectory,
        final Action successAction, final Action failAction,
        final MessageList compilationOutput, boolean synchronous) throws Exception {

      if (compilationOutput != null) {
        compilationOutput.clearMessages();
      }

      try {
        logger.info("Compilation command: " + command);
        compileProcess = Runtime.getRuntime().exec(
            new String[] {"bash", "-c", command},
            null,
            parentDirectory
        );

        final BufferedReader processNormal = new BufferedReader(
            new InputStreamReader(compileProcess.getInputStream()));
        final BufferedReader processError = new BufferedReader(
            new InputStreamReader(compileProcess.getErrorStream()));

        final File ELFFile = new File(parentDirectory, firmware);
        if (firmware != null) {
          if (ELFFile.exists()) {
            ELFFile.delete();
            if (ELFFile.exists()) {
              if (compilationOutput != null) {
                compilationOutput.addMessage("Error when deleting old " + ELFFile.getName(), MessageList.ERROR);
              }
              if (failAction != null) {
                failAction.actionPerformed(null);
              }
              throw new MoteTypeCreationException("Error when deleting old "
                  + ELFFile.getName());
            }
          }
        }

        Thread readInput = new Thread(new Runnable() {
          public void run() {
            try {
              String readLine;
              while ((readLine = processNormal.readLine()) != null) {
                if (compilationOutput != null) {
                  compilationOutput.addMessage(readLine, MessageList.NORMAL);
                }
              }
            } catch (IOException e) {
              logger.warn("Error while reading from process");
            }
          }
        }, "read input stream thread");

        Thread readError = new Thread(new Runnable() {
          public void run() {
            try {
              String readLine;
              while ((readLine = processError.readLine()) != null) {
                if (compilationOutput != null) {
                  compilationOutput.addMessage(readLine, MessageList.ERROR);
                }
              }
            } catch (IOException e) {
              logger.warn("Error while reading from process");
            }
          }
        }, "read input stream thread");

        final MoteTypeCreationException syncException =
          new MoteTypeCreationException("");
        Thread handleCompilationResultThread = new Thread(new Runnable() {
          public void run() {

            /* Wait for compilation to end */
            try {
              compileProcess.waitFor();
            } catch (Exception e) {
              if (compilationOutput != null) {
                compilationOutput.addMessage(e.getMessage(), MessageList.ERROR);
              }
              syncException.setCompilationOutput(new MessageList());
              syncException.fillInStackTrace();
              return;
            }

            /* Check return value */
            if (compileProcess.exitValue() != 0) {
              if (compilationOutput != null) {
                compilationOutput.addMessage("Process returned error code " + compileProcess.exitValue(), MessageList.ERROR);
              }
              if (failAction != null) {
                failAction.actionPerformed(null);
              }
              syncException.setCompilationOutput(new MessageList());
              syncException.fillInStackTrace();
              return;
            }

            if (firmware == null) {
              return;
            }

            if (!ELFFile.exists()) {
              if (compilationOutput != null) {
                compilationOutput.addMessage("Can't locate output file " + ELFFile, MessageList.ERROR);
              }
              if (failAction != null) {
                failAction.actionPerformed(null);
              }
              syncException.setCompilationOutput(new MessageList());
              syncException.fillInStackTrace();
              return;
            }

            if (compilationOutput != null) {
              compilationOutput.addMessage("", MessageList.NORMAL);
              compilationOutput.addMessage("Compilation succeded", MessageList.NORMAL);
            }
            MspELFCompiler.this.lastCompileCommand = command;
            MspELFCompiler.this.sourceFile = sourceFile;
            MspELFCompiler.this.ELFFile = ELFFile;
            if (successAction != null) {
              successAction.actionPerformed(null);
            }
          }
        }, "handle compilation results");

        readInput.start();
        readError.start();
        handleCompilationResultThread.start();

        if (synchronous) {
          try {
            handleCompilationResultThread.join();
          } catch (Exception e) {
            throw (MoteTypeCreationException) new MoteTypeCreationException(
                "Compilation error: " + e.getMessage()).initCause(e);
          }

          /* Detect error manually */
          if (syncException.hasCompilationOutput()) {
            throw (MoteTypeCreationException) new MoteTypeCreationException(
                "Bad return value").initCause(syncException);
          }
        }

        else {
        }
      } catch (IOException ex) {
        if (failAction != null) {
          failAction.actionPerformed(null);
        }
        throw (MoteTypeCreationException) new MoteTypeCreationException(
            "Compilation error: " + ex.getMessage()).initCause(ex);
      }
    }

    public boolean showDialog(Container parentContainer, final MspMoteType moteType) {

      if (parentContainer instanceof Window) {
        myDialog = new JDialog((Window)parentContainer, "Compile firmware file", ModalityType.APPLICATION_MODAL);
      } else if (parentContainer instanceof Dialog) {
        myDialog = new JDialog((Dialog)parentContainer, "Compile firmware file", ModalityType.APPLICATION_MODAL);
      } else if (parentContainer instanceof Frame) {
        myDialog = new JDialog((Frame)parentContainer, "Compile firmware file", ModalityType.APPLICATION_MODAL);
      } else {
        logger.fatal("Unknown parent container type: " + parentContainer);
        return false;
      }

      final MessageList taskOutput = new MessageList();

      // BOTTOM BUTTON PART
      Box buttonBox = Box.createHorizontalBox();
      buttonBox.setBorder(BorderFactory.createEmptyBorder(0, 10, 10, 10));

      buttonBox.add(Box.createHorizontalGlue());

      cancelButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          sourceFile = null;
          ELFFile = null;

          if (compileProcess != null) {
            compileProcess.destroy();
          }
          myDialog.dispose();
        }
      });

      cleanButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          File parentDir = new File(sourceTextField.getText()).getParentFile();
          cleanTempFiles(taskOutput, parentDir, false);
        }
      });

      compileButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          final File selectedSourceFile = new File(sourceTextField.getText());

          /* Strip .c file extension */
          if (!selectedSourceFile.getName().endsWith(".c")) {
            logger.fatal("Source file does not end with '.c'");
            return;
          }

          final String filenameNoExtension = selectedSourceFile.getName()
              .substring(0, selectedSourceFile.getName().length() - 2);

          final Action successAction = new AbstractAction() {
            public void actionPerformed(ActionEvent e) {
              updateDialog(DialogState.COMPILED_SOURCE);
              File parentFile = selectedSourceFile.getParentFile();

              sourceFile = selectedSourceFile;
              ELFFile = new File(parentFile, filenameNoExtension + getTargetFileExtension(target));
            }
          };
          final Action failAction = new AbstractAction() {
            public void actionPerformed(ActionEvent e) {
              updateDialog(DialogState.SELECTED_SOURCE);
            }
          };

          updateDialog(DialogState.IS_COMPILING);
          try {
            new Thread(new Runnable() {
              public void run() {
                try {
                  compileFirmware(selectedSourceFile,
                      successAction, failAction,
                      taskOutput, false);
                } catch (Exception e) {
                  e.printStackTrace();
                }
              }
            }).start();
          } catch (Exception e2) {
          }
        }
      });

      createButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          myDialog.dispose();
        }
      });

      buttonBox.add(cancelButton);
      buttonBox.add(Box.createHorizontalStrut(5));
      buttonBox.add(cleanButton);
      buttonBox.add(Box.createHorizontalStrut(5));
      buttonBox.add(compileButton);
      buttonBox.add(Box.createHorizontalStrut(5));
      buttonBox.add(createButton);

      // MAIN DIALOG CONTENTS
      Box horizBox;
      JLabel label;
      Box vertBox = Box.createVerticalBox();

      // Source
      horizBox = Box.createHorizontalBox();
      horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE, LABEL_HEIGHT));
      horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
      label = new JLabel("Contiki process sourcefile");
      label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

      sourceTextField.setText("");
      if (moteType.getSourceFile() != null) {
        sourceTextField.setText(moteType.getSourceFile().getAbsolutePath());
      }
      sourceTextField.setColumns(25);

      sourceTextField.getDocument().addDocumentListener(new DocumentListener() {
        public void insertUpdate(DocumentEvent e) {
          updateDialog(DialogState.SELECTED_SOURCE);
        }
        public void changedUpdate(DocumentEvent e) {
          updateDialog(DialogState.SELECTED_SOURCE);
        }
        public void removeUpdate(DocumentEvent e) {
          updateDialog(DialogState.SELECTED_SOURCE);
        }
      });

      JButton browseButton = new JButton("Browse");
      browseButton.addActionListener(new ActionListener() {
        public void actionPerformed(ActionEvent e) {
          updateDialog(DialogState.NO_SOURCE);

          JFileChooser fc = new JFileChooser();
          if (lastParentDirectory != null) {
            fc.setCurrentDirectory(lastParentDirectory);
          } else {
            fc.setCurrentDirectory(new java.io.File(GUI
                .getExternalToolsSetting("PATH_CONTIKI")));
          }

          fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
          fc.addChoosableFileFilter(new FileFilter() {
            public boolean accept(File f) {
              if (f.isDirectory()) {
                return true;
              }

              String filename = f.getName();
              if (filename != null) {
                if (filename.endsWith(".c")) {
                  return true;
                }
              }
              return false;
            }

            public String getDescription() {
              return "Contiki process source";
            }
          });
          fc.setDialogTitle("Select Contiki process source");

          if (fc.showOpenDialog(myDialog) == JFileChooser.APPROVE_OPTION) {
            lastParentDirectory = null;
            sourceTextField.setText("");

            File selectedFile = fc.getSelectedFile();
            if (!selectedFile.exists()) {
              return;
            }

            if (!selectedFile.getName().endsWith(".c")) {
              return;
            }

            lastParentDirectory = fc.getSelectedFile().getParentFile();
            sourceTextField.setText(fc.getSelectedFile().getAbsolutePath());
            updateDialog(DialogState.SELECTED_SOURCE);
          }
        }
      });

      horizBox.add(label);
      horizBox.add(Box.createHorizontalStrut(10));
      horizBox.add(sourceTextField);
      horizBox.add(browseButton);

      vertBox.add(horizBox);
      vertBox.add(Box.createRigidArea(new Dimension(0, 5)));

      // Node ID
      /*
       * horizBox = Box.createHorizontalBox(); horizBox.setMaximumSize(new
       * Dimension(Integer.MAX_VALUE,LABEL_HEIGHT));
       * horizBox.setAlignmentX(Component.LEFT_ALIGNMENT); label = new
       * JLabel("Node ID (0=EEPROM)"); label.setPreferredSize(new
       * Dimension(LABEL_WIDTH,LABEL_HEIGHT));
       *
       * nodeIDTextField = new JFormattedTextField(integerFormat);
       * nodeIDTextField.setValue(new Integer(0));
       * nodeIDTextField.setColumns(25);
       * nodeIDTextField.addPropertyChangeListener("value", new
       * PropertyChangeListener() { public void
       * propertyChange(PropertyChangeEvent e) {
       * updateDialog(DialogState.SELECTED_SOURCE); } });
       *
       * horizBox.add(label); horizBox.add(Box.createHorizontalStrut(150));
       * horizBox.add(nodeIDTextField);
       *
       * vertBox.add(horizBox); vertBox.add(Box.createRigidArea(new
       * Dimension(0,5)));
       */

      // Compile command
      horizBox = Box.createHorizontalBox();
      horizBox.setMaximumSize(new Dimension(Integer.MAX_VALUE, LABEL_HEIGHT));
      horizBox.setAlignmentX(Component.LEFT_ALIGNMENT);
      label = new JLabel("Compile command");
      label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

      compileCommandTextField.setText("");
      compileCommandTextField.setColumns(25);
      compileCommandTextField.setEditable(true);

      compileCommandTextField.getDocument().addDocumentListener(new DocumentListener() {
        public void insertUpdate(DocumentEvent e) {
          setCompileCommand(compileCommandTextField.getText());
        }
        public void changedUpdate(DocumentEvent e) {
          setCompileCommand(compileCommandTextField.getText());
        }
        public void removeUpdate(DocumentEvent e) {
          setCompileCommand(compileCommandTextField.getText());
        }
      });

      horizBox.add(label);
      horizBox.add(Box.createHorizontalStrut(10));
      horizBox.add(compileCommandTextField);

      vertBox.add(horizBox);
      vertBox.add(Box.createRigidArea(new Dimension(0, 5)));

      vertBox.add(Box.createRigidArea(new Dimension(0, 5)));
      vertBox.add(new JLabel("Compilation output:"));
      vertBox.add(Box.createRigidArea(new Dimension(0, 5)));

      vertBox.add(new JScrollPane(taskOutput));
      vertBox.add(Box.createRigidArea(new Dimension(0, 5)));

      vertBox.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      Container contentPane = myDialog.getContentPane();
      contentPane.add(vertBox, BorderLayout.CENTER);
      contentPane.add(buttonBox, BorderLayout.SOUTH);

      myDialog.pack();
      myDialog.setLocationRelativeTo(parentContainer);
      myDialog.getRootPane().setDefaultButton(compileButton);

      // Dispose on escape key
      InputMap inputMap = myDialog.getRootPane().getInputMap(
          JComponent.WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);
      inputMap.put(KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0, false),
          "dispose");
      AbstractAction cancelAction = new AbstractAction() {
        public void actionPerformed(ActionEvent e) {
          cancelButton.doClick();
        }
      };
      myDialog.getRootPane().getActionMap().put("dispose", cancelAction);

      myDialog.setDefaultCloseOperation(JDialog.DO_NOTHING_ON_CLOSE);
      myDialog.addWindowListener(new WindowListener() {
        public void windowDeactivated(WindowEvent e) {
        }

        public void windowIconified(WindowEvent e) {
        }

        public void windowDeiconified(WindowEvent e) {
        }

        public void windowOpened(WindowEvent e) {
        }

        public void windowClosed(WindowEvent e) {
        }

        public void windowActivated(WindowEvent e) {
        }

        public void windowClosing(WindowEvent e) {
          cancelButton.doClick();
        }
      });

      updateDialog(DialogState.NO_SOURCE);
      if (moteType.getSourceFile() != null) {
        updateDialog(DialogState.SELECTED_SOURCE);
        if (customizedCompileCommand != null && !customizedCompileCommand.equals("")) {
          compileCommandTextField.setText(customizedCompileCommand);
        }
        compileButton.requestFocus();
      }

      myDialog.setVisible(true);

      return sourceFile != null;
    }

  }

  public JPanel getTypeVisualizer() {
    JPanel panel = new JPanel();
    JLabel label = new JLabel();
    JPanel smallPane;

    panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));

    // Identifier
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Identifier");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(getIdentifier());
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Description
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Description");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(getDescription());
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // ELF Hex file
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("ELF file");
    smallPane.add(BorderLayout.WEST, label);
    label = new JLabel(getELFFile().getName());
    label.setToolTipText(getELFFile().getPath());
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Source file
    smallPane = new JPanel(new BorderLayout());
    label = new JLabel("Source file");
    smallPane.add(BorderLayout.WEST, label);
    if (getSourceFile() != null) {
      label = new JLabel(getSourceFile().getName());
      label.setToolTipText(getSourceFile().getPath());
    } else {
      label = new JLabel("[not specified]");
    }
    smallPane.add(BorderLayout.EAST, label);
    panel.add(smallPane);

    // Icon (if available)
    if (!GUI.isVisualizedInApplet()) {
      Icon moteTypeIcon = getMoteTypeIcon();
      if (moteTypeIcon != null) {
        smallPane = new JPanel(new BorderLayout());
        label = new JLabel(moteTypeIcon);
        smallPane.add(BorderLayout.CENTER, label);
        panel.add(smallPane);
      }
    } else {
      smallPane = new JPanel(new BorderLayout());
      label = new JLabel("No icon available in applet mode");
      smallPane.add(BorderLayout.CENTER, label);
      panel.add(smallPane);
    }

    panel.add(Box.createRigidArea(new Dimension(0, 5)));
    return panel;
  }

  public abstract Icon getMoteTypeIcon();

  public ProjectConfig getConfig() {
    logger.warn("Msp mote type project config not implemented");
    return null;
  }

  public Collection<Element> getConfigXML() {
    Vector<Element> config = new Vector<Element>();

    Element element;

    // Identifier
    element = new Element("identifier");
    element.setText(getIdentifier());
    config.add(element);

    // Description
    element = new Element("description");
    element.setText(getDescription());
    config.add(element);

    // Source file
    if (fileSource != null) {
      element = new Element("source");
      fileSource = GUI.stripAbsoluteContikiPath(fileSource);
      element.setText(fileSource.getPath().replaceAll("\\\\", "/"));
      config.add(element);
      element = new Element("command");
      element.setText(compileCommand);
      config.add(element);
    } else {
      // ELF file
      element = new Element("elf");
      fileFirmware = GUI.stripAbsoluteContikiPath(fileFirmware);
      element.setText(fileFirmware.getPath().replaceAll("\\\\", "/"));
      config.add(element);
    }

    return config;
  }

  public boolean setConfigXML(Simulation simulation,
      Collection<Element> configXML, boolean visAvailable)
      throws MoteTypeCreationException {
    for (Element element : configXML) {
      String name = element.getName();

      if (name.equals("identifier")) {
        identifier = element.getText();
      } else if (name.equals("description")) {
        description = element.getText();
      } else if (name.equals("source")) {
        fileSource = new File(element.getText());
      } else if (name.equals("command")) {
        compileCommand = element.getText();
      } else if (name.equals("elf")) {
        fileFirmware = new File(element.getText());
      } else {
        logger.fatal("Unrecognized entry in loaded configuration: " + name);
        throw new MoteTypeCreationException(
            "Unrecognized entry in loaded configuration: " + name);
      }
    }

    return configureAndInit(GUI.getTopParentContainer(), simulation, visAvailable);
  }

}
