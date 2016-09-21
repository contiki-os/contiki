package se.sics.mspsim.profiler;
import se.sics.mspsim.core.Profiler;

public interface CallListener {

  public void functionCall(Profiler source,  CallEntry entry);

  public void functionReturn(Profiler source, CallEntry entry);

}