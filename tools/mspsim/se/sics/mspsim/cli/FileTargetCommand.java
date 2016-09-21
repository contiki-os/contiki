package se.sics.mspsim.cli;

import java.io.FileWriter;
import java.io.IOException;
import java.util.Hashtable;

public class FileTargetCommand extends BasicLineCommand {
  private final Hashtable<String,Target> fileTargets;
  private final boolean print;
  private final boolean append;

  private Target ft;
  private CommandContext context;
  
  public FileTargetCommand(Hashtable<String,Target> fileTargets,
        String name, String desc, boolean print, boolean append) {
    super(name, desc);
    this.fileTargets = fileTargets;
    this.print = print;
    this.append = append;
  }

  public int executeCommand(CommandContext context) {
    this.context = context;
    String fileName = context.getArgument(0);
    IOException error = null;
    boolean alreadyOpened = false;

    synchronized (fileTargets) {
      ft = fileTargets.get(fileName);
      if (ft == null) {
        try {
          FileWriter writer = new FileWriter(fileName, append);
          ft = new FileTarget(fileTargets, fileName, writer);
        } catch (IOException e) {
          error = e;
        }
      } else if (!append) {
        alreadyOpened = true;
      }
    }

    if (error != null) {
      error.printStackTrace(context.err);
      return -1;
    }
    if (alreadyOpened) {
      context.err.println("File already opened: can not overwrite");
      return -1;
    }
    if (context.getPID() >= 0) {
      ft.addContext(context);
    }
    return 0;
  }

  public void lineRead(String line) {
    if (print) context.out.println(line);
    ft.lineRead(context, line);
  }

  public void stopCommand(CommandContext context) {
    if (ft != null) {
      ft.removeContext(context);
    }
  }
}
