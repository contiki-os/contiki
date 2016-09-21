package se.sics.mspsim.cli;

public abstract class Command {

  /**
   * Returns a text describing this command. First line is a short description that may be followed by
   * more lines with detailed descriptions.
   *
   * @param commandName the name of the command
   * @return a text describing the command
   */
  public abstract String getCommandHelp(String commandName);

  /**
   * Returns a text describing the arguments for this command. Required arguments should be surrounded by
   * '<' and '>', optional arguments should be surrounded by '[' and ']'.
   *
   * @param commandName the name of the command
   * @return a text describing the arguments for this command
   */
  public abstract String getArgumentHelp(String commandName);

  public abstract int executeCommand(CommandContext context);

  /* default behavior is that it returns *this* instance (which might be bad in some cases) */
  public Object getInstance() {
      return this;
  }
}
