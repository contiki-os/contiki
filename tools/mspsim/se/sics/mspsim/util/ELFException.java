package se.sics.mspsim.util;

import java.io.IOException;

public class ELFException extends IOException {
  private static final long serialVersionUID = 6288198720830818497L;

  ELFException(String message) {
    super(message);
  }
}
