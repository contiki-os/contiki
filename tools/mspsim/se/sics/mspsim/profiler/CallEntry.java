package se.sics.mspsim.profiler;
import java.util.HashMap;
import se.sics.mspsim.util.MapEntry;

public class CallEntry {
    
    static class CallCounter implements Comparable<CallCounter> {
        public int count = 0;

        public int compareTo(CallCounter o) {
            return (count < o.count ? -1 : (count == o.count ? 0 : 1));
        }
    }

    
    int fromPC;
    MapEntry function;
    long cycles;
    long exclusiveCycles;
    int calls;
    int hide;
    int stackStart;
    int currentStackMax;
    
    HashMap<MapEntry,CallCounter> callers;
    
    public CallEntry() {
      callers = new HashMap<MapEntry,CallCounter>();
    }
    
    public MapEntry getFunction() {
        return function;
    }
  }
