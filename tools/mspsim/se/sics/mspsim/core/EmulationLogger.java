package se.sics.mspsim.core;

public interface EmulationLogger {

  /* warning mode for CPU errors such as unaligned word access */
  public enum WarningMode {SILENT, PRINT, EXCEPTION};

  /* warning types */
  public enum WarningType {
      EMULATION_ERROR, EXECUTION,
      MISALIGNED_READ, MISALIGNED_WRITE,
      ADDRESS_OUT_OF_BOUNDS_READ, ADDRESS_OUT_OF_BOUNDS_WRITE,
      ILLEGAL_IO_WRITE, VOID_IO_READ, VOID_IO_WRITE
  };
  
  public void log(Loggable source, String message);
  public void logw(Loggable source, WarningType type, String message) throws EmulationException;

  public WarningMode getDefaultWarningMode();
  public void setDefaultWarningMode(WarningMode mode);
  public WarningMode getWarningMode(WarningType type);
  public void setWarningMode(WarningType type, WarningMode mode);
  
  public void addLogListener(LogListener listener);
  public void removeLogListener(LogListener listener);
  
}
