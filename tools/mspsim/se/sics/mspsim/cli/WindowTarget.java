package se.sics.mspsim.cli;

import java.awt.Font;
import java.util.Hashtable;

import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import se.sics.mspsim.extutil.jfreechart.LineChart;
import se.sics.mspsim.extutil.jfreechart.LineSampleChart;
import se.sics.mspsim.ui.ManagedWindow;
import se.sics.mspsim.ui.WindowManager;

public class WindowTarget extends Target {

  private ManagedWindow window;
  // Default in the current version - TODO: replace with better
  private JTextArea jta = new JTextArea(40,80);
  private WindowDataHandler dataHandler = null;

  public WindowTarget(Hashtable<String,Target> targets, String name) {
    super(targets, name, false);
  }

  final void init(WindowManager windowManager) {
    jta.setFont(Font.decode("Courier"));
    jta.setEditable(false);

    window = windowManager.createWindow(getName());
    window.add(new JScrollPane(jta, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER));
    window.setVisible(true);
  }

  protected void handleLine(final CommandContext context, final String line) {
    if (line != null && window != null) {
      java.awt.EventQueue.invokeLater(new Runnable() {
        public void run() {
          processLine(context, line);
        }
      });
    }
  }

  private void processLine(CommandContext context, String line) {
    if (line.startsWith("#!")) {
      line = line.substring(2);
      String[] parts = CommandParser.parseLine(line);
      String cmd = parts[0];
      if ("bounds".equals(cmd)) {
        try {
          window.setBounds(Integer.parseInt(parts[1]), Integer.parseInt(parts[2]),
              Integer.parseInt(parts[3]), Integer.parseInt(parts[4]));
        } catch (Exception e) {
          context.err.println("Could not set bounds: " + line);
        }
      } else if ("title".equals(cmd)) {
        String args = CommandParser.toString(parts, 1, parts.length);
        window.setTitle(args);
        if (dataHandler != null) {
          dataHandler.setProperty("title", new String[] {args});
        }
      } else if ("type".equals(cmd)) {
        if ("line-sample".equals(parts[1])) {
          dataHandler = new LineSampleChart();
        } else if ("line".equals(parts[1])) {
          dataHandler = new LineChart();
        } else {
          context.err.println("Unknown window data handler type: " + parts[1]);
        }
        if (dataHandler != null) {
          System.out.println("Replacing window data handler! " + parts[1] + " " + dataHandler);
          JComponent dataComponent = dataHandler.getComponent();
          window.removeAll();
          window.add(dataComponent);
          String title = window.getTitle();
          if (title != null) {
            // Set title for the new data handler
            dataHandler.setProperty("title", new String[] { title });
          }
          dataComponent.repaint();
        }
      } else if (dataHandler != null) {
        dataHandler.handleCommand(parts);
      } else if ("clear".equals(cmd)) {
        jta.setText("");
      } else if ("tabsize".equals(cmd)) {
        try {
          jta.setTabSize(Integer.parseInt(parts[1]));
        } catch (Exception e) {
          context.err.println("Could not set tab size: " + line);
        }
      } else if ("font".equals(cmd)) {
        jta.setFont(Font.decode(parts[1]));
      }
    } else if (!line.startsWith("#")){
      if (dataHandler != null) {
        dataHandler.lineRead(line);
      } else {
        jta.append(line + '\n');
      }
    }
  }

  protected void closeTarget() {
      if (window != null) {
          window.setVisible(false);
          window = null;
      }
  }

}
