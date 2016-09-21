package se.sics.mspsim.cli;
import java.io.PrintStream;

import se.sics.mspsim.core.MSP430Constants;
import se.sics.mspsim.util.MapTable;
import se.sics.mspsim.util.Utils;

public class CommandContext {

  private String[] args;
  private String commandLine;
  private MapTable mapTable;
  private int pid = -1;
  private boolean exited = false;
  private Command command;
  
  public PrintStream out;
  public PrintStream err;
  private CommandHandler commandHandler;
  
  public CommandContext(CommandHandler ch, MapTable table, String commandLine, String[] args,
			int pid, Command command, PrintStream out, PrintStream err) {
    this(ch, table, commandLine, args, pid, command);
    setOutput(out, err);
  }
  
  public CommandContext(CommandHandler ch,MapTable table, String commandLine, String[] args,
			int pid, Command command) {
    this.commandLine = commandLine;
    this.args = args;
    this.pid = pid;
    this.mapTable = table;
    this.command = command;
    this.commandHandler = ch;
  }
  
  void setOutput(PrintStream out, PrintStream err) {
    this.out = out;
    this.err = err;
  }
  
  Command getCommand() {
    return command;
  }

  // Called by CommandHandler to stop this command.
  void stopCommand() {
    if (!exited) {
      exited = true;

      if (command instanceof AsyncCommand) {
        AsyncCommand ac = (AsyncCommand) command;
        ac.stopCommand(this);
      }
    }
  }

  String getCommandLine() {
    return commandLine;
  }

  public int getPID() {
    return pid;
  }

  public boolean hasExited() {
    return exited;
  }
  
  /**
   * exit needs to be called as soon as the command is completed (or stopped).
   * @param exitCode - the exit code of the command
   */
  public void exit(int exitCode) {
    // TODO: Clean up can be done now!
    exited = true;
    commandHandler.exit(this, exitCode, pid);
  }

  // Requests that this command chain should be killed. Used by for example
  // FileTarget to close all connected commands when the file is closed.
  void kill() {
    if (!exited) {
      commandHandler.exit(this, -9, pid);
    }
  }

  public MapTable getMapTable() {
    return mapTable;
  }

  public String getCommandName() {
    return args[0];
  }

  public int getArgumentCount() {
    return args.length - 1;
  }
  
  public String getArgument(int index) {
    return args[index + 1];
  }
  
  public int getArgumentAsAddress(int index) {
    String adr = getArgument(index);
    if (adr == null || adr.length() == 0) return 0;
    char c = adr.charAt(0);
    if (!Character.isLetter(c) && c != '_' && c != '.') {
      try {
        return Utils.decodeInt(adr);
      } catch (Exception e) {
        err.println("Illegal address format: " + adr);
      }
    } else {
      // Assume that it is a symbol
      if (mapTable != null) {
        return mapTable.getFunctionAddress(adr);
      }
    }
    return -1;
  }

  public int getArgumentAsRegister(int index) {
    String symbol = getArgument(index);
    for (int i = 0, n = MSP430Constants.REGISTER_NAMES.length; i < n; i++) {
      if (MSP430Constants.REGISTER_NAMES[i].equalsIgnoreCase(symbol)) {
        return i;
      }
    }
    String reg = (symbol.startsWith("R") || symbol.startsWith("r")) ? symbol.substring(1) : symbol;
    try {
      int register = Integer.parseInt(reg);
      if (register >= 0 && register <= 15) {
        return register;
      }
    } catch (Exception e) {
      // Ignore
    }
    err.println("illegal register: " + symbol);
    return -1;
  }

  public int getArgumentAsInt(int index) {
    return getArgumentAsInt(index, 0);
  }

  public int getArgumentAsInt(int index, int defaultValue) {
    try {
      return Utils.decodeInt(getArgument(index));
    } catch (Exception e) {
      err.println("Illegal number format: " + getArgument(index));
      return defaultValue;
    }
  }

  public long getArgumentAsLong(int index) {
    return getArgumentAsLong(index, 0L);
  }

  public long getArgumentAsLong(int index, long defaultValue) {
    try {
      return Utils.decodeLong(getArgument(index));
    } catch (Exception e) {
      err.println("Illegal number format: " + getArgument(index));
      return defaultValue;
    }
  }

  public float getArgumentAsFloat(int index) {
    return getArgumentAsFloat(index, 0f);
  }

  public float getArgumentAsFloat(int index, float defaultValue) {
    try {
      return Float.parseFloat(getArgument(index));
    } catch (Exception e) {
      err.println("Illegal number format: " + getArgument(index));
      return defaultValue;
    }
  }

  public double getArgumentAsDouble(int index) {
    return getArgumentAsDouble(index, 0.0);
  }

  public double getArgumentAsDouble(int index, double defaultValue) {
    String arg = getArgument(index);
    try {
      return Double.parseDouble(arg);
    } catch (Exception e) {
      err.println("Illegal number format: " + getArgument(index));
      return defaultValue;
    }
  }

  public boolean getOption(String optionName) {
      for (int i = 0; i < args.length; i++) {
          if (args[i].equals("-" + optionName)) {
              return true;
          }
      }
      return false;
  }
  
  public boolean getArgumentAsBoolean(int index) {
    String v = getArgument(index);
    return "true".equalsIgnoreCase(v) || "1".equals(v);
  }

  public int executeCommand(String command) {
    return commandHandler.executeCommand(command, this);
  }

  public String toString() {
    return (pid >= 0 ? ("" + pid) : "?") + '\t' + (commandLine == null ? getCommandName() : commandLine);
  }

}
