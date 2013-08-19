package com.thingsquare.cooja.mspsim;

import java.awt.Container;
import java.io.File;

import org.apache.log4j.Logger;

import se.sics.cooja.AbstractionLevelDescription;
import se.sics.cooja.ClassDescription;
import se.sics.cooja.MoteInterface;
import se.sics.cooja.MoteType;
import se.sics.cooja.Simulation;
import se.sics.cooja.dialogs.CompileContiki;
import se.sics.cooja.dialogs.MessageList;
import se.sics.cooja.dialogs.MessageList.MessageContainer;
import se.sics.cooja.interfaces.IPAddress;
import se.sics.cooja.interfaces.Mote2MoteRelations;
import se.sics.cooja.interfaces.MoteAttributes;
import se.sics.cooja.interfaces.Position;
import se.sics.cooja.interfaces.RimeAddress;
import se.sics.cooja.mspmote.Exp5438MoteType;
import se.sics.cooja.mspmote.MspCompileDialog;
import se.sics.cooja.mspmote.interfaces.Exp5438LED;
import se.sics.cooja.mspmote.interfaces.Msp802154Radio;
import se.sics.cooja.mspmote.interfaces.MspClock;
import se.sics.cooja.mspmote.interfaces.MspDebugOutput;
import se.sics.cooja.mspmote.interfaces.MspMoteID;
import se.sics.cooja.mspmote.interfaces.UsciA1Serial;

@ClassDescription("Exp2420 mote (MSP430F5438)")
@AbstractionLevelDescription("Emulated level")
public class Exp2420MoteType extends Exp5438MoteType {
  private static Logger logger = Logger.getLogger(Exp2420MoteType.class);

  public boolean configureAndInit(Container parentContainer, Simulation simulation, boolean visAvailable)
  throws MoteTypeCreationException {

    /* If visualized, show compile dialog and let user configure */
    if (visAvailable) {

      /* Create unique identifier */
      if (getIdentifier() == null) {
        int counter = 0;
        boolean identifierOK = false;
        while (!identifierOK) {
          identifierOK = true;

          counter++;
          setIdentifier("exp2420#" + counter);

          for (MoteType existingMoteType : simulation.getMoteTypes()) {
            if (existingMoteType == this) {
              continue;
            }
            if (existingMoteType.getIdentifier().equals(getIdentifier())) {
              identifierOK = false;
              break;
            }
          }
        }
      }

      /* Create initial description */
      if (getDescription() == null) {
        setDescription("Exp2420 Mote Type " + getIdentifier());
      }

      return MspCompileDialog.showDialog(parentContainer, simulation, this, "exp2420");
    }

    /* Not visualized: Compile Contiki immediately */
    if (getIdentifier() == null) {
      throw new MoteTypeCreationException("No identifier");
    }

    final MessageList compilationOutput = new MessageList();

    if (getCompileCommands() != null) {
      /* Handle multiple compilation commands one by one */
      String[] arr = getCompileCommands().split("\n");
      for (String cmd: arr) {
        if (cmd.trim().isEmpty()) {
          continue;
        }

        try {
          CompileContiki.compile(
              cmd,
              null,
              null /* Do not observe output firmware file */,
              getContikiSourceFile().getParentFile(),
              null,
              null,
              compilationOutput,
              true
          );
        } catch (Exception e) {
          MoteTypeCreationException newException =
            new MoteTypeCreationException("Mote type creation failed: " + e.getMessage());
          newException = (MoteTypeCreationException) newException.initCause(e);
          newException.setCompilationOutput(compilationOutput);

          /* Print last 10 compilation errors to console */
          MessageContainer[] messages = compilationOutput.getMessages();
          for (int i=messages.length-10; i < messages.length; i++) {
            if (i < 0) {
              continue;
            }
            logger.fatal(">> " + messages[i]);
          }

          logger.fatal("Compilation error: " + e.getMessage());
          throw newException;
        }
      }
    }

    if (getContikiFirmwareFile() == null ||
        !getContikiFirmwareFile().exists()) {
      throw new MoteTypeCreationException("Contiki firmware file does not exist: " + getContikiFirmwareFile());
    }
    return true;
  }

  public Class<? extends MoteInterface>[] getDefaultMoteInterfaceClasses() {
	    return new Class[] {
	            Position.class,
	            RimeAddress.class,
	            IPAddress.class,
	            Mote2MoteRelations.class,
	            MoteAttributes.class,
	            MspClock.class,
	            MspMoteID.class,
	            Msp802154Radio.class,
	            UsciA1Serial.class,
	            Exp5438LED.class,
	            /*Exp5438LCD.class,*/ /* TODO */
	            MspDebugOutput.class
	        };
  }
  public Class<? extends MoteInterface>[] getAllMoteInterfaceClasses() {
    return new Class[] {
        Position.class,
        RimeAddress.class,
        IPAddress.class,
        Mote2MoteRelations.class,
        MoteAttributes.class,
        MspClock.class,
        MspMoteID.class,
        Msp802154Radio.class,
        CC1101Radio.class,
        CC1120Radio.class,
        Msp802154Radio.class,
        UsciA1Serial.class,
        Exp5438LED.class,
        /*Exp5438LCD.class,*/ /* TODO */
        MspDebugOutput.class
    };
  }

  public File getExpectedFirmwareFile(File source) {
    File parentDir = source.getParentFile();
    String sourceNoExtension = source.getName().substring(0, source.getName().length()-2);

    return new File(parentDir, sourceNoExtension + ".exp2420");
  }

  protected String getTargetName() {
  	return "exp2420";
  }

}
