package org.contikios.cooja.util;

import java.util.Observable;

public class ScnObservable extends Observable {
  public void setChangedAndNotify() {
    setChanged();
    notifyObservers();
  }
	
  public void setChangedAndNotify(Object obj) {
    setChanged();
    notifyObservers(obj);
  }
    
}
