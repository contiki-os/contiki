package se.sics.cooja.dialogs;

import java.beans.*;
import java.io.File;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.event.*;
import org.apache.log4j.Logger;

class ProjectDirectoryInputDialog extends JDialog implements ActionListener, PropertyChangeListener {
  private static Logger logger = Logger.getLogger(ProjectDirectoryInputDialog.class);

  private File projectDirectoryPath = null;
  private JTextField textField;

  private JOptionPane optionPane;

  private String buttonAdd = "Add";
  private String buttonCancel = "Cancel";

  public ProjectDirectoryInputDialog(Window window) {
    super(window, ModalityType.APPLICATION_MODAL);
    setupDialog();
  }
  public ProjectDirectoryInputDialog(Dialog dialog) {
    super(dialog, ModalityType.APPLICATION_MODAL);
    setupDialog();
  }
  public ProjectDirectoryInputDialog(Frame frame) {
    super(frame, ModalityType.APPLICATION_MODAL);
    setupDialog();
  }

  public void setupDialog() {
    setTitle("Enter path");

    textField = new JTextField(10);
    textField.getDocument().addDocumentListener(new DocumentListener() {
      public void insertUpdate(DocumentEvent e) {
        pathChanged();
      }
      public void removeUpdate(DocumentEvent e) {
        pathChanged();
      }
      public void changedUpdate(DocumentEvent e) {
        pathChanged();
      }
    });

    Object[] objects = {"Enter path to project directory", textField};
    Object[] options = {buttonAdd, buttonCancel};

    optionPane = new JOptionPane(objects,
        JOptionPane.QUESTION_MESSAGE,
        JOptionPane.YES_NO_OPTION,
        null,
        options,
        options[0]);

    setContentPane(optionPane);

    setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent we) {
        optionPane.setValue(new Integer(
            JOptionPane.CLOSED_OPTION));
      }
    });

    addComponentListener(new ComponentAdapter() {
      public void componentShown(ComponentEvent ce) {
        textField.requestFocusInWindow();
      }
    });

    textField.addActionListener(this);
    optionPane.addPropertyChangeListener(this);

    SwingUtilities.invokeLater(new Runnable() {
      public void run() {
        pathChanged();
      }
    });
  }

  public File getProjectDirectory() {
    return projectDirectoryPath;
  }

  private void pathChanged() {
    String newPath = textField.getText();
    File projectDir = new File(newPath);
    if (!projectDir.exists()) {
      textField.setBackground(Color.RED);
      textField.setToolTipText("Directory does not exist");
      return;
    }

    if (!projectDir.isDirectory()) {
      textField.setBackground(Color.RED);
      textField.setToolTipText("Not a directory");
      return;
    }

//    if (!new File(projectDir, GUI.PROJECT_CONFIG_FILENAME).exists()) {
//      textField.setBackground(Color.LIGHT_GRAY);
//      textField.setToolTipText("No configuration file found");
//      return;
//    }

    textField.setBackground(Color.WHITE);
    textField.setToolTipText("");
  }

  public void actionPerformed(ActionEvent e) {
    optionPane.setValue(buttonAdd);
  }

  public void propertyChange(PropertyChangeEvent e) {
    String prop = e.getPropertyName();
    if (isVisible() && (e.getSource() == optionPane)
        && (JOptionPane.VALUE_PROPERTY.equals(prop) ||
            JOptionPane.INPUT_VALUE_PROPERTY.equals(prop))) {
      Object value = optionPane.getValue();

      if (value == JOptionPane.UNINITIALIZED_VALUE) {
        return;
      }

      optionPane.setValue(
          JOptionPane.UNINITIALIZED_VALUE);

      if (buttonAdd.equals(value)) {
        projectDirectoryPath = new File(textField.getText());
        close();
      } else {
        projectDirectoryPath = null;
        close();
      }
    }
  }

  public void close() {
    textField.setText(null);
    setVisible(false);
  }
}
