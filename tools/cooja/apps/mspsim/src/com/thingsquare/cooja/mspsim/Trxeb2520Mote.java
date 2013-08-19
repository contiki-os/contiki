package com.thingsquare.cooja.mspsim;

import java.io.File;

import se.sics.cooja.Simulation;
import se.sics.cooja.mspmote.Exp5438Mote;
import se.sics.cooja.mspmote.MspMoteType;

/**
 * @author Fredrik Osterlind
 */
public class Trxeb2520Mote extends Exp5438Mote {
  private String desc = "";
  
  public Trxeb2520Mote(MspMoteType moteType, Simulation sim) {
    super(moteType, sim);
  }

  protected boolean initEmulator(File fileELF) {
    return super.initEmulator(fileELF);
  }

  public String toString() {
    return desc + " " + getID();
  }
}
