package se.sics.mspsim.util;

import java.io.PrintStream;

import se.sics.mspsim.core.EmulationException;
import se.sics.mspsim.core.EmulationLogger;
import se.sics.mspsim.core.LogListener;
import se.sics.mspsim.core.Loggable;
import se.sics.mspsim.core.MSP430Core;

public class DefaultEmulationLogger implements EmulationLogger {

  private final MSP430Core cpu;
  private final WarningMode[] warningModes = new WarningMode[WarningType.values().length];
  private WarningMode defaultMode = WarningMode.PRINT;
  private PrintStream out;
  private LogListener[] logListeners;
  
  public DefaultEmulationLogger(MSP430Core cpu, PrintStream out) {
    this.cpu = cpu;
    this.out = out;
  }

  protected WarningMode getMode(WarningType type) {
      WarningMode mode = warningModes[type.ordinal()];
      if (mode == null) {
          mode = defaultMode;
      }
      return mode;
  }

  @Override
  public void log(Loggable source, String message) {
//      out.println(source.getID() + ": " + message);
      LogListener[] listeners = this.logListeners;
      if (listeners != null) {
          for (LogListener l : listeners) {
              l.log(source, message);
          }
      }
  }

  @Override
  public void logw(Loggable source, WarningType type, String message)
          throws EmulationException {
      switch (getMode(type)) {
      case SILENT:
          break;
      case PRINT:
          out.println(source.getID() + ": " + message);
          cpu.generateTrace(out);
          break;
      case EXCEPTION:
          out.println(source.getID() + ": " + message);
          cpu.generateTrace(out);
          throw new EmulationException(message);
      }

      LogListener[] listeners = this.logListeners;
      if (listeners != null) {
          for (LogListener l : listeners) {
              l.logw(source, type, message);
          }
      }
  }

  @Override
  public WarningMode getDefaultWarningMode() {
      return defaultMode;
  }

  @Override
  public void setDefaultWarningMode(WarningMode mode) {
      this.defaultMode = mode;
  }

  @Override
  public WarningMode getWarningMode(WarningType type) {
      return warningModes[type.ordinal()];
  }

  @Override
  public void setWarningMode(WarningType type, WarningMode mode) {
      warningModes[type.ordinal()] = mode;
  }

  @Override
  public synchronized void addLogListener(LogListener listener) {
      logListeners = ArrayUtils.add(LogListener.class, logListeners, listener);
  }

  @Override
  public synchronized void removeLogListener(LogListener listener) {
      logListeners = ArrayUtils.remove(logListeners, listener);
  }
}
