package org.contikios.cooja.avrmote.interfaces;

import avrora.sim.mcu.AtmelMicrocontroller;
import avrora.sim.platform.MicaZ;
import org.contikios.cooja.Mote;
import org.contikios.cooja.avrmote.MicaZMote;
import org.contikios.cooja.dialogs.SerialUI;

public class MicaSerial extends SerialUI {

  MicaZMote mote;
  
  public MicaSerial(Mote micaZMote) {
    mote = (MicaZMote) micaZMote;
    MicaZ micaZ = mote.getMicaZ();
    /* this should go into some other piece of code for serial data */
    AtmelMicrocontroller mcu = (AtmelMicrocontroller) micaZ.getMicrocontroller();
    avrora.sim.mcu.USART usart = (avrora.sim.mcu.USART) mcu.getDevice("usart0");
    if (usart != null) {
      usart.connect(  new avrora.sim.mcu.USART.USARTDevice() {
        public avrora.sim.mcu.USART.Frame transmitFrame() {
          return null;
          // return new avrora.sim.mcu.USART.Frame((byte)'a', false, 8);
        }
        public void receiveFrame(avrora.sim.mcu.USART.Frame frame) {
          dataReceived(frame.value);
        }
      });
    } else {
     System.out.println("*** Warning MicaZ: could not find usart1 interface..."); 
    }
  }
  
  public Mote getMote() {
    return mote;
  }
  
  /* not yet implemented ...*/
  public void writeArray(byte[] s) {
  }

  public void writeByte(byte b) {
  }

  public void writeString(String s) {
  }
}
