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
 * $Id: MantisMoteTypeDialog.java,v 1.5 2007/03/24 00:44:55 fros4943 Exp $
 */

package se.sics.cooja.mantismote;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import org.apache.log4j.Logger;

import se.sics.cooja.*;
import se.sics.cooja.dialogs.MessageList;

/**
 * A dialog for configuring Mantis mote types and compiling KMantis mote type
 * libraries. 
 * 
 * The dialog takes a Mantis mote type as argument and pre-selects the values
 * already set in that mote type before showing the dialog. Any changes made to
 * the settings are written to the mote type if the compilation is successful
 * and the user presses OK.
 * 
 * This dialog uses external tools to scan for sources and compile libraries.
 * 
 * @author Fredrik Osterlind
 */
public class MantisMoteTypeDialog extends JDialog {
  private static final long serialVersionUID = 1L;
  private static Logger logger = Logger.getLogger(MantisMoteTypeDialog.class);

  private MoteTypeEventHandler myEventHandler = new MoteTypeEventHandler();
  private Thread compilationThread;

  /**
   * Suggested mote type identifier prefix
   */
  public static final String ID_PREFIX = "mtype";

  private final static int LABEL_WIDTH = 170;
  private final static int LABEL_HEIGHT = 15;

  private MantisMoteType myMoteType = null;

  private JTextField textID, textOutputFiles, textDescription, textMantisBinary;
  private JButton createButton, compileButton;
  
  private File objFile = null;
  private File workingDir = null;
  private File libFile = null;
  private File srcFile = null;
  
  private Vector<Class<? extends MoteInterface>> moteInterfaceClasses = null;

  private boolean settingsOK = false; // Do all settings seem correct?
  private boolean compilationSucceded = false; // Did compilation succeed?
  private boolean libraryCreatedOK = false; // Was a library created?

  private Vector<MoteType> allOtherTypes = null; // Used to check for conflicting parameters

  private MantisMoteTypeDialog myDialog;

  /**
   * Shows a dialog for configuring a Mantis mote type.
   * 
   * @param parentFrame
   *          Parent frame for dialog
   * @param simulation
   *          Simulation holding (or that will hold) mote type
   * @param moteTypeToConfigure
   *          Mote type to configure
   * @return True if mote type configuration succeded and library is ready to be loaded
   */
  public static boolean showDialog(Frame parentFrame, Simulation simulation,
      MantisMoteType moteTypeToConfigure) {

    final MantisMoteTypeDialog myDialog = new MantisMoteTypeDialog(
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
      }

      myDialog.textID.setText(testIdentifier);
    }

    // Set preset description of mote type
    if (moteTypeToConfigure.getDescription() != null) {
      myDialog.textDescription.setText(moteTypeToConfigure.getDescription());
    } else {
      myDialog.textDescription.setText("mantis type, id=" + myDialog.textID.getText());
    }

    // Set preset object file of mote type
    if (moteTypeToConfigure.getObjectFilename() != null) {
      myDialog.textMantisBinary.setText(moteTypeToConfigure.getObjectFilename());
    }

    // Load all mote interface classes
    String[] moteInterfaces = simulation.getGUI().getProjectConfig().getStringArrayValue(MantisMoteType.class, "MOTE_INTERFACES");
    myDialog.moteInterfaceClasses = new Vector<Class<? extends MoteInterface>>();
    for (String moteInterface : moteInterfaces) {
      try {
        Class<? extends MoteInterface> newMoteInterfaceClass = 
          simulation.getGUI().tryLoadClass(simulation.getGUI(), MoteInterface.class, moteInterface);
        myDialog.moteInterfaceClasses.add(newMoteInterfaceClass);
        /*logger.info("Loaded Mantis mote interface: " + newMoteInterfaceClass);*/
      } catch (Exception e) {
        logger.fatal("Failed to load mote interface, aborting: " + moteInterface + ", " + e.getMessage());
        return false;
      }
    }

    // Set position and focus of dialog
    myDialog.pack();
    myDialog.setLocationRelativeTo(parentFrame);
    myDialog.textDescription.requestFocus();
    myDialog.textDescription.select(0, myDialog.textDescription.getText().length());
    myDialog.pathsWereUpdated();
    myDialog.setVisible(true);

    if (myDialog.myMoteType != null) {
      // Library was compiled and loaded
      return true;
    }
    return false;
  }

  private MantisMoteTypeDialog(Frame frame) {
    super(frame, "Configure Mantis Mote Type", true);

    myDialog = this;

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

    button = new JButton("Compile");
    button.setActionCommand("compile");
    button.addActionListener(myEventHandler);
    compileButton = button;
    this.getRootPane().setDefaultButton(button);
    buttonPane.add(button);
    buttonPane.add(Box.createRigidArea(new Dimension(10, 0)));

    buttonPane.add(Box.createHorizontalGlue());

    button = new JButton("Clean");
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
    textField.setText("");
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
    textField.setText("");
    textField.getDocument().addDocumentListener(myEventHandler);
    textDescription = textField;
    label.setLabelFor(textField);
    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);

    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));

    // Mantis binary
    smallPane = new JPanel();
    smallPane.setAlignmentX(Component.LEFT_ALIGNMENT);
    smallPane.setLayout(new BoxLayout(smallPane, BoxLayout.X_AXIS));
    label = new JLabel("Mantis x86 object");
    label.setPreferredSize(new Dimension(LABEL_WIDTH, LABEL_HEIGHT));

    textField = new JTextField();
    textField.setText("");
    textField.getDocument().addDocumentListener(myEventHandler);
    textMantisBinary = textField;
    label.setLabelFor(textField);

    button = new JButton("Browse");
    button.setActionCommand("browsemantis");
    button.addActionListener(myEventHandler);

    smallPane.add(label);
    smallPane.add(Box.createHorizontalStrut(10));
    smallPane.add(Box.createHorizontalGlue());
    smallPane.add(textField);
    smallPane.add(button);
    mainPane.add(smallPane);

    mainPane.add(Box.createRigidArea(new Dimension(0, 5)));
    mainPane.add(Box.createVerticalGlue());

    // Add everything!
    mainPane.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));

    Container contentPane = getContentPane();
    contentPane.add(mainPane, BorderLayout.NORTH);
    contentPane.add(buttonPane, BorderLayout.SOUTH);
  }

  /**
   * Tries to compile library using current settings.
   */
  public void doCompileCurrentSettings() {
    libraryCreatedOK = false;

    JPanel progressPanel = new JPanel(new BorderLayout());
    final JDialog progressDialog = new JDialog(myDialog, (String) null);
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
        if (progressDialog != null && progressDialog.isDisplayable())
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

    progressDialog.getRootPane().setDefaultButton(button);
    progressDialog.setVisible(true);

    // Generate main mantis source file
    try {
      // Remove old file is existing
      if (srcFile.exists()) {
        srcFile.delete();
      }
      
      if (srcFile.exists()) {
        throw new Exception("could not remove old source file");
      }
      
      generateSourceFile(srcFile);
      
      if (!srcFile.exists()) {
        throw new Exception("source file not created");
      }
    } catch (Exception e) {
      libraryCreatedOK = false;
      progressBar.setBackground(Color.ORANGE);
      if (e.getMessage() != null)
        progressBar.setString("source file generation failed: " + e.getMessage());
      else
        progressBar.setString("source file generation failed");
         
      progressBar.setIndeterminate(false);
      progressBar.setValue(0);
      createButton.setEnabled(libraryCreatedOK);
      return;
    }
    
    // Test compile shared library
    progressBar.setString("..compiling..");

    if (libFile.exists()) {
      libFile.delete();
    }

    compilationThread = new Thread(new Runnable() {
      public void run() {
        compilationSucceded = 
          MantisMoteTypeDialog.compileLibrary(
              libFile,
              objFile,
              srcFile,
              workingDir,
              taskOutput.getInputStream(MessageList.NORMAL),
              taskOutput.getInputStream(MessageList.ERROR));
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
      libraryCreatedOK = false;
    } else {
      libraryCreatedOK = true;
      if (!libFile.exists())
        libraryCreatedOK = false;
    }

    if (libraryCreatedOK) {
      progressBar.setBackground(Color.GREEN);
      progressBar.setString("compilation succeded");
      button.grabFocus();
      myDialog.getRootPane().setDefaultButton(createButton);
    } else {
      progressBar.setBackground(Color.ORANGE);
      progressBar.setString("compilation failed");
      myDialog.getRootPane().setDefaultButton(compileButton);
    }
    progressBar.setIndeterminate(false);
    progressBar.setValue(0);
    createButton.setEnabled(libraryCreatedOK);
  }

  /**
   * Generates new source file by reading default source template and replacing
   * certain field in order to be loadable from given Java class.
   * 
   * @param outputFile Source file to create
   * @throws Exception
   */
  public static void generateSourceFile(File outputFile)
      throws Exception {

    // CHECK JNI CLASS AVAILABILITY
    String libString = CoreComm.getAvailableClassName();
    if (libString == null) {
      logger.fatal("No more libraries can be loaded!");
      throw new Exception("Maximum number of mote types already exist");
    }

    // GENERATE NEW FILE
    BufferedWriter destFile = null;
    BufferedReader sourceFile = null;
    try {
      Reader reader;
      String mainTemplate = GUI
          .getExternalToolsSetting("MANTIS_MAIN_TEMPLATE_FILENAME");
      if ((new File(mainTemplate)).exists()) {
        reader = new FileReader(mainTemplate);
      } else {
        InputStream input = MantisMoteTypeDialog.class
            .getResourceAsStream('/' + mainTemplate);
        if (input == null) {
          throw new FileNotFoundException(mainTemplate + " not found");
        }
        reader = new InputStreamReader(input);
      }

      sourceFile = new BufferedReader(reader);
      
      destFile = new BufferedWriter(new OutputStreamWriter(
          new FileOutputStream(outputFile)));

      // Replace fields in template
      String line;
      while ((line = sourceFile.readLine()) != null) {
        line = line.replaceFirst("\\[CLASS_NAME\\]", libString);
        destFile.write(line + "\n");
      }

      destFile.close();
      sourceFile.close();
    } catch (Exception e) {
      try {
        if (destFile != null)
          destFile.close();
        if (sourceFile != null)
          sourceFile.close();
      } catch (Exception e2) {
      }

      // Forward exception
      throw e;
    }
  }
  
  /**
   * Compiles a mote type shared library using the standard Mantis makefile.
   * 
   * @param libFile Library file to create
   * @param binFile Binary file to link against
   * @param sourceFile Source file to compile
   * @param workingDir Working directory
   * @param outputStream
   *          Output stream from compilation (optional)
   * @param errorStream
   *          Error stream from compilation (optional)
   * @return True if compilation succeeded, false otherwise
   */
  public static boolean compileLibrary(File libFile, File binFile, File sourceFile, File workingDir,
      final PrintStream outputStream, final PrintStream errorStream) {

    // Check needed files
    if (!workingDir.exists()) {
      if (errorStream != null)
        errorStream.println("Bad paths");
      logger.fatal("Working directory does not exist");
      return false;
    }
    if (!workingDir.isDirectory()) {
      if (errorStream != null)
        errorStream.println("Bad paths");
      logger.fatal("Working directory is not a directory");
      return false;
    }

    if (libFile.exists()) {
      if (errorStream != null)
        errorStream.println("Bad output filenames");
      logger.fatal("Library already exists");
      return false;
    }

    if (!sourceFile.exists()) {
      if (errorStream != null)
        errorStream.println("Bad dependency files");
      logger.fatal("Source file not found");
      return false;
    }

    if (!binFile.exists()) {
      if (errorStream != null)
        errorStream.println("Bad dependency files");
      logger.fatal("Link object file not found");
      return false;
    }

    if (CoreComm.hasLibraryFileBeenLoaded(libFile)) {
      if (errorStream != null)
        errorStream.println("Bad output filenames");
      logger.fatal("A library has already been loaded with the same name before");
      return false;
    }

    try {
      // Call make file
      String[] cmd = new String[]{
          GUI.getExternalToolsSetting("PATH_MAKE"),
          libFile.getName()};

      String[] env = new String[]{
          "COOJA_LINKFILE=" + binFile.getName(),
          "COOJA_SOURCE=" + sourceFile.getName(),
          "PATH=" + System.getenv("PATH")};

      Process p = Runtime.getRuntime().exec(cmd, env, workingDir);

      final BufferedReader input = new BufferedReader(new InputStreamReader(p.getInputStream()));
      final BufferedReader err = new BufferedReader(new InputStreamReader(p.getErrorStream()));

      Thread readInput = new Thread(new Runnable() {
        public void run() {
          String readLine;
          try {
            while ((readLine = input.readLine()) != null) {
              if (outputStream != null && readLine != null)
                outputStream.println(readLine);
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
              if (errorStream != null && readLine != null)
                errorStream.println(readLine);
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
      if (p.exitValue() != 0) {
        logger.fatal("Make file returned error: " + p.exitValue());
        return false;
      }
    } catch (Exception e) {
      logger.fatal("Error while compiling library: " + e);
      return false;
    }
    return true;
  }

  private void pathsWereUpdated() {
    updateVisualFields();
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

    // Check that binary exists
    textMantisBinary.setBackground(Color.WHITE);
    textMantisBinary.setToolTipText(null);

    objFile = new File(textMantisBinary.getText());
    workingDir = objFile.getParentFile();
    libFile = new File(workingDir, textID.getText() + ".library");
    srcFile = new File(workingDir, textID.getText() + ".c");
    // TODO Check that file is correct type (.o or something)
    if (objFile == null || !objFile.exists()) {
      textMantisBinary.setBackground(Color.RED);
      textMantisBinary.setToolTipText("Incorrect object file");
      objFile = null;
      libFile = null;
      srcFile = null;
      workingDir = null;
      settingsOK = false;
    }

    // Update output text field
    if (settingsOK) {
      textOutputFiles.setText(libFile.getName() + ", " + srcFile.getName() + ", " + textID.getText() + ".o");
    } else {
      textOutputFiles.setText("");
    }
    
    createButton.setEnabled(libraryCreatedOK = false);
    compileButton.setEnabled(settingsOK);
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
        // TODO Not implemented
        logger.fatal("Clean functionality not implemented");
      } else if (e.getActionCommand().equals("create")) {
        // Create mote type and set related fields
        boolean ret = myMoteType.doInit(libFile, objFile, moteInterfaceClasses);
        myMoteType.setDescription(textDescription.getText());
        myMoteType.setIdentifier(textID.getText());
        if (ret) {
          dispose();
        } else {
          logger.fatal("Mote type creation failed.");
        }
      } else if (e.getActionCommand().equals("compile")) {
        compileButton.requestFocus();
        Thread testSettingsThread = new Thread(new Runnable() {
          public void run() {
            doCompileCurrentSettings();
          }
        }, "test settings thread");
        testSettingsThread.start();
      } else if (e.getActionCommand().equals("browsemantis")) {
        JFileChooser fc = new JFileChooser();
        fc.setCurrentDirectory(new java.io.File("."));
        fc.setFileSelectionMode(JFileChooser.FILES_ONLY);
        fc.setDialogTitle("Mantis binary to link against");

        if (fc.showOpenDialog(myDialog) == JFileChooser.APPROVE_OPTION) {
          textMantisBinary.setText(fc.getSelectedFile().getPath());
        }
        createButton.setEnabled(libraryCreatedOK = false);
        pathsWereUpdated();
      } else
        logger.warn("Unhandled action: " + e.getActionCommand());

      createButton.setEnabled(libraryCreatedOK = false);

    }
  }

}
