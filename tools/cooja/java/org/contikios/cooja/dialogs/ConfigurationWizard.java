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
 *
 */

package org.contikios.cooja.dialogs;

import java.awt.BorderLayout;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintStream;
import java.lang.reflect.Constructor;
import java.util.HashMap;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JProgressBar;
import javax.swing.JScrollPane;

import org.contikios.cooja.CoreComm;
import org.contikios.cooja.Cooja;
import org.contikios.cooja.MoteType.MoteTypeCreationException;
import org.contikios.cooja.mote.memory.SectionMoteMemory;
import org.contikios.cooja.contikimote.ContikiMoteType;
import org.contikios.cooja.contikimote.ContikiMoteType.SectionParser;
import org.contikios.cooja.mote.memory.MemoryInterface.Symbol;
import org.contikios.cooja.mote.memory.VarMemory;

/* TODO Test common section */
/* TODO Test readonly section */

public class ConfigurationWizard extends JDialog {
  private static final long serialVersionUID = 1L;

  private static final String COMPILER_ARGS_suggestions[] = new String[] {
    "",
    "Windows cygwin:",
    "-mno-cygwin -Wall -I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/win32' -fno-builtin-printf",
    "-mno-cygwin -Wall -I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/win32'",
    "-Wall -D_JNI_IMPLEMENTATION_ -I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/win32'",
    "-mno-cygwin -I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/win32'",

    "Linux:",
    "-I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/linux' -fno-builtin-printf -fPIC",

    "Mac OS X:",
    "-Wall -I/System/Library/Frameworks/JavaVM.framework/Versions/CurrentJDK/Headers -dynamiclib -fno-common"
  };

  private static final String LINK_COMMAND_1_suggestions[] = new String[] {
    "",
    "Windows cygwin:",
    "gcc -mno-cygwin -shared -Wl,-Map=$(MAPFILE) -Wl,--add-stdcall-alias -o $(LIBFILE)",
    "gcc -shared -Wl,-Map=$(MAPFILE) -o $(LIBFILE)",
    "",
    "Linux:",
    "gcc -I'$(JAVA_HOME)/include' -I'$(JAVA_HOME)/include/linux' -shared -Wl,-Map=$(MAPFILE) -o $(LIBFILE)",
    "ld -Map=$(MAPFILE) -shared --add-stdcall-alias /usr/lib/mingw/dllcrt2.o -o $(LIBFILE)",
    "gcc -shared -Wl,-Map=$(MAPFILE) -Wall -D_JNI_IMPLEMENTATION_ -Wl,--kill-at -o $(LIBFILE)",
    "",
    "Mac OS X:",
    "gcc -dynamiclib -fno-common -o $(LIBFILE)"
  };

  private static final String LINK_COMMAND_2_suggestions[] = new String[] {
    "",
    "-L/usr/lib/mingw -lmsvcrt",
    "-L/usr/lib/mingw -lmingw32 -lmingwex -lmsvcrt",
    "-framework JavaVM"
  };

  private static final String PARSE_WITH_COMMAND_suggestions[] = new String[] {
    "true",
    "false"
  };

  private static final String AR_COMMAND_1_suggestions[] = new String[] {
    "ar rcf $(ARFILE)",
    "ar rc $(ARFILE)"
  };

  private static final String PARSE_COMMAND_suggestions[] = new String[] {
    "nm -C $(LIBFILE)",
    "nm -a $(LIBFILE)",
    "nmandsize $(LIBFILE)"
  };

  private static final String OPTION_RUN_TEST = "Run test";
  private static final String OPTION_NEXT_TEST = "Next test";
  private static final String OPTION_CLOSE_WIZARD = "Close wizard";

  private static final String testTemplate = "test_template.c";

  private static int testCounter = 0;

  private static String cLibraryName;
  private static File cLibrarySourceFile;
  private static File cLibraryFile;
  private static String javaLibraryName;
  private static CoreComm javaLibrary;
  private static HashMap<String, Symbol> addresses;
  private static int relDataSectionAddr;
  private static int dataSectionSize;
  private static int relBssSectionAddr;
  private static int bssSectionSize;

  private static MessageListUI output;
  private static JDialog progressDialog;
  private static JButton button;
  private static JProgressBar progressBar;

  public static boolean startWizard(Container parentContainer, Cooja gui) {
    if (Cooja.isVisualizedInApplet()) {
      return false;
    }

    /* Initial info message */
    if (!showWizardInfo(parentContainer, gui)) {
      return false;
    }

    /* Test 1 - Compile and link C source */
    if (!doCompileCTest((JFrame)parentContainer, gui)) {
      return false;
    }

    /* Test 2 - Load Java library */
    if (!doLoadLibraryTest((JFrame)parentContainer, gui)) {
      return false;
    }

    /* Test 3 - Address parsing */
    if (!doAddressParsingTest((JFrame)parentContainer, gui)) {
      return false;
    }

    /* Test 4 - Memory replacement */
    if (!doMemoryReplacementTest((JFrame)parentContainer, gui)) {
      return false;
    }

    return true;
  }

  public static boolean showWizardInfo(Container parent, Cooja gui) {
    String options[] = {"Start tests", OPTION_CLOSE_WIZARD};
    int value = JOptionPane.showOptionDialog(parent,
        "This wizard configures and tests your toolchain for simulation of Cooja motes.\n" +
        "Throughout the wizard, Contiki libraries are compiled and loaded while allowing you to \n" +
        "alter external tools settings such as compiler arguments.\n" +
        "\n" +
        "Changes made in this wizard are reflected in menu Settings, External tools paths.\n" +
        "\n" +
        "NOTE: You do not need to complete this wizard for emulated motes, such as Sky motes.\n",
        "Cooja mote configuration wizard",
        JOptionPane.YES_NO_OPTION, JOptionPane.INFORMATION_MESSAGE,
        null, options, options[0]);

    if (value != JOptionPane.YES_OPTION) {
      return false;
    }

    return true;
  }

  public static boolean doCompileCTest(JFrame parent, Cooja gui) {
    final String testDescription = "Step 1/4 - Compile and link Contiki library";
    String value = OPTION_RUN_TEST;
    while (value.equals(OPTION_RUN_TEST)) {
      value = showStepDialog(
          parent,
          "Generates, compiles and links a Cooja/Contiki stub.\n",
          testDescription,
          new String[] {
              "COMPILER_ARGS", "LINK_COMMAND_1", "LINK_COMMAND_2", "AR_COMMAND_1", "AR_COMMAND_2"
          },
          true
      );

      if (value == null) {
        return false;
      }

      if (value.equals(OPTION_NEXT_TEST)) {
        return true;
      }

      if (!value.equals(OPTION_RUN_TEST)) {
        return false;
      }

      prepareShowTestProgress(parent, testDescription);

      /* Start test */
      new Thread(new Runnable() {
        public void run() {
          testCounter++;
          PrintStream normalStream = new PrintStream(output.getInputStream(MessageList.NORMAL));
          PrintStream errorStream = new PrintStream(output.getInputStream(MessageList.ERROR));
          boolean success = performCompileCTest(output, normalStream, errorStream);
          normalStream.close();
          errorStream.close();

          if (success) {
            output.addMessage("### Test OK", MessageList.NORMAL);
          } else {
            output.addMessage("### Test failed", MessageList.ERROR);
          }
          button.setEnabled(true);
          progressBar.setIndeterminate(false);
        }
      }).start();

      showTestProgress(parent);
    }

    return false;
  }

  public static boolean doLoadLibraryTest(JFrame parent, Cooja gui) {
    final String testDescription = "Step 2/4 - Load Contiki library in Java";
    String value = OPTION_RUN_TEST;
    while (value.equals(OPTION_RUN_TEST)) {
      value = showStepDialog(
          parent,
          "Generates, compiles and loads a CoreComm Java class.\n" +
          "A CoreComm instance loads the Contiki library created in step 2.\n",
          testDescription,
          new String[] {
              "PATH_JAVAC"
          },
          true
      );

      if (value == null) {
        return false;
      }

      if (value.equals(OPTION_NEXT_TEST)) {
        return true;
      }

      if (!value.equals(OPTION_RUN_TEST)) {
        return false;
      }

      prepareShowTestProgress(parent, testDescription);

      /* Start test */
      new Thread(new Runnable() {
        public void run() {
          testCounter++;
          PrintStream normalStream = new PrintStream(output.getInputStream(MessageList.NORMAL));
          PrintStream errorStream = new PrintStream(output.getInputStream(MessageList.ERROR));
          boolean success = performLoadTest(output, normalStream, errorStream);
          normalStream.close();
          errorStream.close();

          if (success) {
            output.addMessage("### Test OK", MessageList.NORMAL);
          } else {
            output.addMessage("### Test failed", MessageList.ERROR);
          }
          button.setEnabled(true);
          progressBar.setIndeterminate(false);
        }
      }).start();

      showTestProgress(parent);
    }

    return false;
  }

  public static boolean doAddressParsingTest(JFrame parent, Cooja gui) {
    final String testDescription = "Step 3/4 - Library memory addresses";
    String value = OPTION_RUN_TEST;
    while (value.equals(OPTION_RUN_TEST)) {
      value = showStepDialog(
          parent,
          "Tests parsing memory addresses of a loaded Contiki library.\n" +
          "Addresses can be parsed using either a map file, or with the nm tool.\n" +
          "Both approaches are tested.\n",
          testDescription,
          new String[] {
              "PARSE_WITH_COMMAND", "PARSE_COMMAND"
          },
          true
      );

      if (value == null) {
        return false;
      }

      if (value.equals(OPTION_NEXT_TEST)) {
        return true;
      }

      if (!value.equals(OPTION_RUN_TEST)) {
        return false;
      }

      prepareShowTestProgress(parent, testDescription);

      /* Start test */
      new Thread(new Runnable() {
        public void run() {
          testCounter++;
          PrintStream normalStream = new PrintStream(output.getInputStream(MessageList.NORMAL));
          PrintStream errorStream = new PrintStream(output.getInputStream(MessageList.ERROR));
          boolean success = performAddressTest(output, normalStream, errorStream);
          normalStream.close();
          errorStream.close();

          if (success) {
            output.addMessage("### Test OK", MessageList.NORMAL);
          } else {
            output.addMessage("### Test failed", MessageList.ERROR);
          }
          button.setEnabled(true);
          progressBar.setIndeterminate(false);
        }
      }).start();

      showTestProgress(parent);
    }

    return false;
  }

  public static boolean doMemoryReplacementTest(JFrame parent, Cooja gui) {
    final String testDescription = "Step 4/4 - Memory replacement test";
    String value = OPTION_RUN_TEST;
    while (value.equals(OPTION_RUN_TEST)) {
      value = showStepDialog(
          parent,
          "Tests copying memory sections between Contiki and Cooja.\n" +
          "Variable values are both altered in Contiki and in Cooja.\n" +
          "\n" +
          "This is the final test!\n",
          testDescription,
          null,
          false
      );

      if (value == null) {
        return false;
      }

      if (value.equals(OPTION_NEXT_TEST)) {
        return true;
      }

      if (!value.equals(OPTION_RUN_TEST)) {
        return false;
      }

      prepareShowTestProgress(parent, testDescription);

      /* Start test */
      new Thread(new Runnable() {
        public void run() {
          testCounter++;
          PrintStream normalStream = new PrintStream(output.getInputStream(MessageList.NORMAL));
          PrintStream errorStream = new PrintStream(output.getInputStream(MessageList.ERROR));
          boolean success = performMemoryReplacementTest(output, normalStream, errorStream);
          normalStream.close();
          errorStream.close();

          if (success) {
            output.addMessage("### Test OK", MessageList.NORMAL);
          } else {
            output.addMessage("### Test failed", MessageList.ERROR);
          }
          button.setEnabled(true);
          progressBar.setIndeterminate(false);
        }
      }).start();

      showTestProgress(parent);
    }

    return false;
  }

  private static void prepareShowTestProgress(JFrame parent, String desc) {
    output = new MessageListUI();
    output.addPopupMenuItem(null, true);
    progressDialog = new JDialog(parent, desc);
    button = new JButton("Close");
    progressBar = new JProgressBar(0, 100);
  }

  private static void showTestProgress(JFrame parent) {
    progressBar.setValue(0);
    progressBar.setStringPainted(false);
    progressBar.setIndeterminate(true);
    JPanel progressPanel = new JPanel(new BorderLayout());
    button.setEnabled(false);
    button.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        if (progressDialog.isDisplayable()) {
          progressDialog.dispose();
        }
      }
    });
    progressPanel.add(BorderLayout.CENTER, new JScrollPane(output));
    progressPanel.add(BorderLayout.NORTH, progressBar);
    progressPanel.add(BorderLayout.SOUTH, button);
    progressDialog.getContentPane().add(progressPanel);
    progressDialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
    progressDialog.setSize(700, 500);
    progressDialog.setModal(true);
    progressDialog.setLocationRelativeTo(parent);
    progressDialog.getRootPane().setDefaultButton(button);
    progressDialog.setVisible(true);
  }

  private static JPanel createConfigureComboBox(final String name) {
    JPanel panel = new JPanel();
    panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
    JComboBox combo = new JComboBox(new Object[] { Cooja.getExternalToolsSetting(name, "") });

    JLabel label = new JLabel(name);
    label.setToolTipText(name);
    label.setPreferredSize(new Dimension(120, label.getPreferredSize().height));

    /* Suggest external tools settings */
    String[] suggestions = null;
    if (name.equals("COMPILER_ARGS")) {
      suggestions = COMPILER_ARGS_suggestions;
    } else if (name.equals("LINK_COMMAND_1")) {
      suggestions = LINK_COMMAND_1_suggestions;
    } else if (name.equals("LINK_COMMAND_2")) {
      suggestions = LINK_COMMAND_2_suggestions;
    } else if (name.equals("PARSE_WITH_COMMAND")) {
      suggestions = PARSE_WITH_COMMAND_suggestions;
    } else if (name.equals("AR_COMMAND_1")) {
      suggestions = AR_COMMAND_1_suggestions;
    } else if (name.equals("PARSE_COMMAND")) {
      suggestions = PARSE_COMMAND_suggestions;
    }
    if (suggestions != null) {
      for (String suggestion: suggestions) {
        combo.addItem(suggestion);
      }
    }
    combo.addItemListener(new ItemListener() {
      public void itemStateChanged(ItemEvent e) {
        Cooja.setExternalToolsSetting(name, (String) e.getItem());
      }
    });
    combo.setEditable(true);
    combo.setPreferredSize(new Dimension(300, combo.getPreferredSize().height));

    panel.add(label);
    panel.add(Box.createHorizontalStrut(10));
    panel.add(combo);
    return panel;
  }

  private static String showStepDialog(JFrame parent, String desc, String title, String[] settings, boolean hasNext) {

    /* Create configurable settings panels (label+combobox) */
    JPanel settingsPanels[] = null;
    if (settings != null && settings.length > 0) {
      settingsPanels = new JPanel[settings.length];
      for (int i=0; i < settings.length; i++) {
        settingsPanels[i] = createConfigureComboBox(settings[i]);
      }
    }

    /* Create dialog message */
    Object descAndSettings[] = {
        desc + "\n",
        settingsPanels
    };
    JOptionPane optionPane = new JOptionPane();
    optionPane.setMessage(descAndSettings);
    optionPane.setMessageType(JOptionPane.INFORMATION_MESSAGE);

    /* Create dialog options */
    String options[];
    if (hasNext) {
      options = new String[] {OPTION_RUN_TEST, OPTION_NEXT_TEST, OPTION_CLOSE_WIZARD};
    } else {
      options = new String[] {OPTION_RUN_TEST, OPTION_CLOSE_WIZARD};
    }
    optionPane.setOptions(options);
    optionPane.setInitialValue(options[0]);

    /* Show dialog */
    JDialog dialog = optionPane.createDialog(parent, title);
    dialog.setResizable(true);
    dialog.setVisible(true);
    return (String) optionPane.getValue();
  }

  public static boolean performCompileCTest(MessageListUI testOutput, PrintStream normalStream, PrintStream errorStream) {
    javaLibraryName = "LibTest" + testCounter;
    cLibraryName = "libtest" + testCounter;
    cLibrarySourceFile = new File(ContikiMoteType.tempOutputDirectory, cLibraryName + ".c");
    cLibraryFile = new File(ContikiMoteType.tempOutputDirectory, cLibraryName + ContikiMoteType.librarySuffix);

    testOutput.addMessage("### Reading C library template source: " + testTemplate, MessageList.NORMAL);
    BufferedReader templateReader = null;
    try {
      if ((new File(testTemplate)).exists()) {
        templateReader = new BufferedReader(new FileReader(testTemplate));
      } else {
        InputStream input = ConfigurationWizard.class.getResourceAsStream('/' + testTemplate);
        if (input == null) {
          throw new FileNotFoundException("File not found: " + testTemplate);
        }
        templateReader = new BufferedReader(new InputStreamReader(input));
      }
    } catch (FileNotFoundException e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Generating C library source: " + cLibrarySourceFile.getName());
    if (!ContikiMoteType.tempOutputDirectory.exists()) {
      ContikiMoteType.tempOutputDirectory.mkdir();
    }
    if (cLibrarySourceFile.exists()) {
      cLibrarySourceFile.delete();
    }
    try {
      cLibrarySourceFile.createNewFile();
    } catch (IOException e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }
    BufferedWriter cLibraryWriter = null;
    try {
      cLibraryWriter = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(cLibrarySourceFile)));
      String line;
      while ((line = templateReader.readLine()) != null) {
        line = line.replaceFirst("\\[CLASS_NAME\\]", javaLibraryName);
        cLibraryWriter.write(line + "\n");
      }
      cLibraryWriter.close();
      templateReader.close();
    } catch (IOException e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }

    /* Prepare compiler environment */
    testOutput.addMessage("### Preparing compiler environment");
    String[][] env;
    try {
      env = CompileContiki.createCompilationEnvironment(
          cLibraryName,
          new File(cLibraryName + ".c"),
          new File(cLibraryName + ContikiMoteType.mapSuffix),
          new File(cLibraryName + ContikiMoteType.librarySuffix),
          new File(cLibraryName + ContikiMoteType.dependSuffix),
          javaLibraryName
      );
    } catch (Exception e) {
      testOutput.addMessage("### Error: Compiler environment failed", MessageList.ERROR);
      return false;
    }
    String[] envOneDimension = new String[env.length];
    for (int i=0; i < env.length; i++) {
      envOneDimension[i] = env[i][0] + "=" + env[i][1];
    }

    testOutput.addMessage("### Compiling C library source: " + cLibrarySourceFile.getName());
    try {
      String contikiPath = Cooja.getExternalToolsSetting("PATH_CONTIKI").replaceAll("\\\\", "/");
      CompileContiki.compile(
          "make " +
          "-f " + contikiPath + "/Makefile.include " +
          "CONTIKI=" + contikiPath + " " +
          "obj_cooja/" + cLibraryName + ".cooja " +
          "TARGET=cooja CONTIKI_APP_OBJ=",
          envOneDimension,
          null,
          new File("."),
          null,
          null,
          testOutput,
          true
      );
    } catch (Exception e) {
      testOutput.addMessage("### Error: Compilation failed", MessageList.ERROR);
      return false;
    }

    return true;
  }

  public static boolean performLoadTest(MessageListUI testOutput, PrintStream normalStream, PrintStream errorStream) {
    MessageListUI dummy = new MessageListUI();
    PrintStream dummyStream = dummy.getInputStream(MessageList.NORMAL);
    if (!performCompileCTest(dummy, dummyStream, errorStream)) {
      return false;
    }
    dummyStream.close();

    testOutput.addMessage("### Generating Java library source: org/contikios/cooja/corecomm/" + javaLibraryName + ".java");
    try {
      CoreComm.generateLibSourceFile(javaLibraryName);
    } catch (MoteTypeCreationException e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Compiling Java library source: org/contikios/cooja/corecomm/" + javaLibraryName + ".java");
    try {
      CoreComm.compileSourceFile(javaLibraryName);
    } catch (MoteTypeCreationException e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Loading Java library class: org/contikios/cooja/corecomm/" + javaLibraryName);
    Class<? extends CoreComm> javaLibraryClass = null;
    try {
      javaLibraryClass = (Class<? extends CoreComm>) CoreComm.loadClassFile(javaLibraryName);
    } catch (MoteTypeCreationException e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Creating Java library: org/contikios/cooja/corecomm/" + javaLibraryName + " with argument: " + cLibraryFile);
    javaLibrary = null;
    try {
      Constructor<? extends CoreComm> constr = javaLibraryClass.getConstructor(new Class[] { File.class });
      javaLibrary = constr.newInstance(new Object[] { cLibraryFile });
    } catch (Exception e) {
      e.printStackTrace(errorStream);
      testOutput.addMessage("### Error: " + e.getMessage(), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Calling native Contiki stub functions");
    javaLibrary.setReferenceAddress(0);
    javaLibrary.getMemory(0, 0, new byte[]{});
    javaLibrary.setMemory(0, 0, new byte[]{});
    return true;
  }

  public static boolean performAddressTest(MessageListUI testOutput, PrintStream normalStream, PrintStream errorStream) {
    MessageListUI dummy = new MessageListUI();
    PrintStream dummyStream = dummy.getInputStream(MessageList.NORMAL);
    if (!performCompileCTest(dummy, dummyStream, errorStream)) {
      return false;
    }
    if (!performLoadTest(dummy, dummyStream, errorStream)) {
      return false;
    }
    dummyStream.close();

    boolean successMap = false;
    boolean successCommand = false;

    successMap = performMapAddressTest(testOutput, normalStream, errorStream);
    testOutput.addMessage("");
    successCommand = performCommandAddressTest(testOutput, normalStream, errorStream);

    boolean parseWithCommand = Boolean.parseBoolean(Cooja.getExternalToolsSetting("PARSE_WITH_COMMAND", "false"));

    if (successMap && successCommand) {
      testOutput.addMessage("### Both map file and command based address parsing succeeded");
      return true;
    }
    if (parseWithCommand) {
      if (successCommand) {
        testOutput.addMessage("### Command based address parsing succeeded");
        return true;
      }
      if (successMap) {
        testOutput.addMessage("Map file based parsing succeded: use PARSE_WITH_COMMAND=false", MessageList.ERROR);
        return false;
      }
    }
    if (!parseWithCommand) {
      if (successMap) {
        testOutput.addMessage("### Map file based address parsing succeeded");
        return true;
      }
      if (successMap) {
        testOutput.addMessage("Command based parsing succeded: use PARSE_WITH_COMMAND=true", MessageList.ERROR);
        return false;
      }
    }
    return false;
  }

  private static boolean performMapAddressTest(MessageListUI testOutput, PrintStream normalStream, PrintStream errorStream) {
    testOutput.addMessage("### Testing map file based address parsing");

    File mapFile = new File(ContikiMoteType.tempOutputDirectory, cLibraryName + ContikiMoteType.mapSuffix);

    testOutput.addMessage("### Reading map file: " + mapFile);
    if (!mapFile.exists()) {
      testOutput.addMessage("### Error: Map file not found", MessageList.ERROR);
      return false;
    }
    String[] mapData = ContikiMoteType.loadMapFile(mapFile);
    if (mapData == null) {
      testOutput.addMessage("### Error: Map file could not be read", MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Parsing map file data for addresses");
    addresses = new HashMap<String, Symbol>();
//    boolean parseOK = ContikiMoteType.parseMapFileData(mapData, addresses);
//    if (!parseOK) {
//      testOutput.addMessage("### Error: Failed parsing map file data", MessageList.ERROR);
//      return false;
//    }

    testOutput.addMessage("### Validating section addresses");
    SectionParser dataSecParser = new ContikiMoteType.MapSectionParser(
            mapData,
            Cooja.getExternalToolsSetting("MAPFILE_DATA_START"),
            Cooja.getExternalToolsSetting("MAPFILE_DATA_SIZE"));
    SectionParser bssSecParser = new ContikiMoteType.MapSectionParser(
            mapData,
            Cooja.getExternalToolsSetting("MAPFILE_BSS_START"),
            Cooja.getExternalToolsSetting("MAPFILE_BSS_SIZE"));
    dataSecParser.parse(0);
    bssSecParser.parse(0);
    relDataSectionAddr = dataSecParser.getStartAddr();
    dataSectionSize = dataSecParser.getSize();
    relBssSectionAddr = bssSecParser.getStartAddr();
    bssSectionSize = bssSecParser.getSize();
    testOutput.addMessage("Data section address: 0x" + Integer.toHexString(relDataSectionAddr));
    testOutput.addMessage("Data section size: 0x" + Integer.toHexString(dataSectionSize));
    testOutput.addMessage("BSS section address: 0x" + Integer.toHexString(relBssSectionAddr));
    testOutput.addMessage("BSS section size: 0x" + Integer.toHexString(bssSectionSize));
    if (relDataSectionAddr < 0) {
      testOutput.addMessage("Data section address < 0: 0x" + Integer.toHexString(relDataSectionAddr), MessageList.ERROR);
      return false;
    }
    if (relBssSectionAddr < 0) {
      testOutput.addMessage("BSS section address < 0: 0x" + Integer.toHexString(relBssSectionAddr), MessageList.ERROR);
      return false;
    }
    if (dataSectionSize <= 0) {
      testOutput.addMessage("Data section size <= 0: 0x" + Integer.toHexString(dataSectionSize), MessageList.ERROR);
      return false;
    }
    if (bssSectionSize <= 0) {
      testOutput.addMessage("BSS section size <= 0: 0x" + Integer.toHexString(bssSectionSize), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Looking for known Contiki variables");
    String varName;
    varName = "var1";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "var2";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "arr1";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "arr2";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "uvar1";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "uvar2";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }

    return true;
  }

  private static boolean performCommandAddressTest(MessageListUI testOutput, PrintStream normalStream, PrintStream errorStream) {
    testOutput.addMessage("### Testing command based address parsing");

    testOutput.addMessage("### Executing command");
    String[] commandData = ContikiMoteType.loadCommandData(cLibraryFile);
    if (commandData == null) {
      testOutput.addMessage("### Error: Could not execute command", MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Parsing command output for addresses");
    addresses = new HashMap<String, Symbol>();
//    boolean parseOK = ContikiMoteType.parseCommandData(commandData, addresses);
//    if (!parseOK) {
//      testOutput.addMessage("### Error: Failed parsing command output", MessageList.ERROR);
//      return false;
//    }

    testOutput.addMessage("### Validating section addresses");
    SectionParser dataSecParser = new ContikiMoteType.CommandSectionParser(
            commandData,
            Cooja.getExternalToolsSetting("COMMAND_DATA_START"),
            Cooja.getExternalToolsSetting("COMMAND_DATA_SIZE"),
            Cooja.getExternalToolsSetting("COMMAND_VAR_SEC_DATA"));
    SectionParser bssSecParser = new ContikiMoteType.CommandSectionParser(
            commandData,
            Cooja.getExternalToolsSetting("COMMAND_BSS_START"),
            Cooja.getExternalToolsSetting("COMMAND_BSS_SIZE"),
            Cooja.getExternalToolsSetting("COMMAND_VAR_SEC_BSS"));

    dataSecParser.parse(0);
    bssSecParser.parse(0);
    relDataSectionAddr = dataSecParser.getStartAddr();
    dataSectionSize = dataSecParser.getSize();
    relBssSectionAddr = bssSecParser.getStartAddr();
    bssSectionSize = bssSecParser.getSize();
    testOutput.addMessage("Data section address: 0x" + Integer.toHexString(relDataSectionAddr));
    testOutput.addMessage("Data section size: 0x" + Integer.toHexString(dataSectionSize));
    testOutput.addMessage("BSS section address: 0x" + Integer.toHexString(relBssSectionAddr));
    testOutput.addMessage("BSS section size: 0x" + Integer.toHexString(bssSectionSize));
    if (relDataSectionAddr < 0) {
      testOutput.addMessage("Data section address < 0: 0x" + Integer.toHexString(relDataSectionAddr), MessageList.ERROR);
      return false;
    }
    if (relBssSectionAddr < 0) {
      testOutput.addMessage("BSS section address < 0: 0x" + Integer.toHexString(relBssSectionAddr), MessageList.ERROR);
      return false;
    }
    if (dataSectionSize <= 0) {
      testOutput.addMessage("Data section size <= 0: 0x" + Integer.toHexString(dataSectionSize), MessageList.ERROR);
      return false;
    }
    if (bssSectionSize <= 0) {
      testOutput.addMessage("BSS section size <= 0: 0x" + Integer.toHexString(bssSectionSize), MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Looking for known Contiki variables");
    String varName;
    varName = "var1";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "var2";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "arr1";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "arr2";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "uvar1";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }
    varName = "uvar2";
    if (!addresses.containsKey(varName)) {
      testOutput.addMessage("Could not find address of: " + varName, MessageList.ERROR);
      return false;
    }

    return true;
  }


  public static boolean performMemoryReplacementTest(MessageListUI testOutput, PrintStream normalStream, PrintStream errorStream) {
    MessageListUI dummy = new MessageListUI();
    PrintStream dummyStream = dummy.getInputStream(MessageList.NORMAL);
    if (!performCompileCTest(dummy, dummyStream, errorStream)) {
      return false;
    }
    if (!performLoadTest(dummy, dummyStream, errorStream)) {
      return false;
    }
    boolean parseWithCommand = Boolean.parseBoolean(Cooja.getExternalToolsSetting("PARSE_WITH_COMMAND", "false"));
    if (parseWithCommand) {
      if (!performCommandAddressTest(dummy, dummyStream, errorStream)) {
        return false;
      }
    } else {
      if (!performMapAddressTest(dummy, dummyStream, errorStream)) {
        return false;
      }
    }
    dummyStream.close();

    testOutput.addMessage("### Testing Contiki library memory replacement");

    testOutput.addMessage("### Configuring Contiki using parsed reference address");
    if (!addresses.containsKey("referenceVar")) {
      testOutput.addMessage("Could not find address of referenceVar", MessageList.ERROR);
      return false;
    }
    int relRefAddress = (int) addresses.get("referenceVar").addr;
    javaLibrary.setReferenceAddress(relRefAddress);

    testOutput.addMessage("### Creating data and BSS memory sections");
    byte[] initialDataSection = new byte[dataSectionSize];
    byte[] initialBssSection = new byte[bssSectionSize];
    javaLibrary.getMemory(relDataSectionAddr, dataSectionSize, initialDataSection);
    javaLibrary.getMemory(relBssSectionAddr, bssSectionSize, initialBssSection);
    SectionMoteMemory memory = new SectionMoteMemory(addresses);
    VarMemory varMem = new VarMemory(memory);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);

    int contikiDataCounter, contikiBSSCounter;

    testOutput.addMessage("### Checking initial variable values: 1,0");
    contikiDataCounter = varMem.getIntValueOf("var1");
    contikiBSSCounter = varMem.getIntValueOf("uvar1");
    int javaDataCounter = 1;
    int javaBSSCounter = 0;
    if (contikiDataCounter != javaDataCounter) {
      testOutput.addMessage("### Data section mismatch (" + contikiDataCounter + " != " + javaDataCounter + ")", MessageList.ERROR);
      return false;
    }
    if (contikiBSSCounter != javaBSSCounter) {
      testOutput.addMessage("### BSS section mismatch (" + contikiBSSCounter + " != " + javaBSSCounter + ")", MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Increasing data and BSS counters 5 times: 6,5");
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;

    testOutput.addMessage("### Fetching memory, comparing counters");
    javaLibrary.getMemory(relDataSectionAddr, dataSectionSize, initialDataSection);
    javaLibrary.getMemory(relBssSectionAddr, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    contikiDataCounter = varMem.getIntValueOf("var1");
    contikiBSSCounter = varMem.getIntValueOf("uvar1");
    if (contikiDataCounter != javaDataCounter) {
      testOutput.addMessage("### Data section mismatch (" + contikiDataCounter + " != " + javaDataCounter + ")", MessageList.ERROR);
      return false;
    }
    if (contikiBSSCounter != javaBSSCounter) {
      testOutput.addMessage("### BSS section mismatch (" + contikiBSSCounter + " != " + javaBSSCounter + ")", MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Backup memory sections: 6,5");
    byte[] savedDataSection = new byte[dataSectionSize];
    byte[] savedBssSection = new byte[bssSectionSize];
    int backupDataCounter = contikiDataCounter;
    int backupBssCounter = contikiBSSCounter;
    javaLibrary.getMemory(relDataSectionAddr, dataSectionSize, savedDataSection);
    javaLibrary.getMemory(relBssSectionAddr, bssSectionSize, savedBssSection);

    testOutput.addMessage("### Increasing data and BSS counters 3 times: 9,8");
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;

    testOutput.addMessage("### Fetching memory, comparing counters");
    javaLibrary.getMemory(relDataSectionAddr, dataSectionSize, initialDataSection);
    javaLibrary.getMemory(relBssSectionAddr, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    contikiDataCounter = varMem.getIntValueOf("var1");
    contikiBSSCounter = varMem.getIntValueOf("uvar1");
    if (contikiDataCounter != javaDataCounter) {
      testOutput.addMessage("### Data section mismatch (" + contikiDataCounter + " != " + javaDataCounter + ")", MessageList.ERROR);
      return false;
    }
    if (contikiBSSCounter != javaBSSCounter) {
      testOutput.addMessage("### BSS section mismatch (" + contikiBSSCounter + " != " + javaBSSCounter + ")", MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Restoring backup data section: 6,8");
    javaLibrary.setMemory(relDataSectionAddr, dataSectionSize, savedDataSection);
    javaDataCounter = backupDataCounter;

    testOutput.addMessage("### Increasing data and BSS counters 3 times: 9,11");
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;
    javaLibrary.tick(); javaDataCounter++; javaBSSCounter++;

    testOutput.addMessage("### Fetching memory, comparing counters");
    javaLibrary.getMemory(relDataSectionAddr, dataSectionSize, initialDataSection);
    javaLibrary.getMemory(relBssSectionAddr, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    contikiDataCounter = varMem.getIntValueOf("var1");
    contikiBSSCounter = varMem.getIntValueOf("uvar1");
    if (contikiDataCounter != javaDataCounter) {
      testOutput.addMessage("### Data section mismatch (" + contikiDataCounter + " != " + javaDataCounter + ")", MessageList.ERROR);
      return false;
    }
    if (contikiBSSCounter != javaBSSCounter) {
      testOutput.addMessage("### BSS section mismatch (" + contikiBSSCounter + " != " + javaBSSCounter + ")", MessageList.ERROR);
      return false;
    }

    testOutput.addMessage("### Restoring backup BSS section: 9,5");
    javaLibrary.setMemory(relBssSectionAddr, bssSectionSize, savedBssSection);
    javaBSSCounter = backupBssCounter;

    testOutput.addMessage("### Fetching memory, comparing counters");
    javaLibrary.getMemory(relDataSectionAddr, dataSectionSize, initialDataSection);
    javaLibrary.getMemory(relBssSectionAddr, bssSectionSize, initialBssSection);
    memory.setMemorySegment(relDataSectionAddr, initialDataSection);
    memory.setMemorySegment(relBssSectionAddr, initialBssSection);
    contikiDataCounter = varMem.getIntValueOf("var1");
    contikiBSSCounter = varMem.getIntValueOf("uvar1");
    if (contikiDataCounter != javaDataCounter) {
      testOutput.addMessage("### Data section mismatch (" + contikiDataCounter + " != " + javaDataCounter + ")", MessageList.ERROR);
      return false;
    }
    if (contikiBSSCounter != javaBSSCounter) {
      testOutput.addMessage("### BSS section mismatch (" + contikiBSSCounter + " != " + javaBSSCounter + ")", MessageList.ERROR);
      return false;
    }

    return true;
  }

}
