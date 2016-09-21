package se.sics.mspsim.core;

import se.sics.mspsim.core.EmulationLogger.WarningType;

public interface LogListener {

    public void log(Loggable source, String message);
    public void logw(Loggable source, WarningType type, String message) throws EmulationException;

}
