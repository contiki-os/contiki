package org.contikios.cooja.mspmote;
import org.contikios.cooja.AbstractionLevelDescription;
import org.contikios.cooja.ClassDescription;
import org.contikios.cooja.MoteInterface;
import org.contikios.cooja.Simulation;
import org.contikios.cooja.interfaces.IPAddress;
import org.contikios.cooja.interfaces.Mote2MoteRelations;
import org.contikios.cooja.interfaces.MoteAttributes;
import org.contikios.cooja.interfaces.Position;
import org.contikios.cooja.interfaces.RimeAddress;
import org.contikios.cooja.mspmote.interfaces.Msp802154Radio;
import org.contikios.cooja.mspmote.interfaces.MspButton;
import org.contikios.cooja.mspmote.interfaces.MspClock;
import org.contikios.cooja.mspmote.interfaces.MspDebugOutput;
import org.contikios.cooja.mspmote.interfaces.MspDefaultSerial;
import org.contikios.cooja.mspmote.interfaces.MspLED;
import org.contikios.cooja.mspmote.interfaces.MspMoteID;

@ClassDescription("Z1X mote")
@AbstractionLevelDescription("Emulated level")
public class Z1XMoteType extends AbstractMspMoteType {

    @Override
    public String getMoteType() {
        return "z1x";
    }

    @Override
    public String getMoteName() {
        return "Z1X";
    }

    @Override
    protected String getMoteImage() {
        return "images/z1.jpg";
    }

    @Override
    protected MspMote createMote(Simulation simulation) {
        return new Z1XMote(this, simulation);
    }

    public Class<? extends MoteInterface>[] getDefaultMoteInterfaceClasses() {
  	  return getAllMoteInterfaceClasses();
    }
    public Class<? extends MoteInterface>[] getAllMoteInterfaceClasses() {
        @SuppressWarnings("unchecked")
        Class<? extends MoteInterface>[] list = createMoteInterfaceList(
                Position.class,
                RimeAddress.class,
                IPAddress.class,
                Mote2MoteRelations.class,
                MoteAttributes.class,
                MspClock.class,
                MspMoteID.class,
                MspButton.class,
                Msp802154Radio.class,
                MspDefaultSerial.class,
                MspLED.class,
                MspDebugOutput.class /* EXPERIMENTAL: Enable me for COOJA_DEBUG(..) */
        );
        return list;
    }
}
