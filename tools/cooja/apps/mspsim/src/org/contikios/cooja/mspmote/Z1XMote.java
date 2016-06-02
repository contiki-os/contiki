package org.contikios.cooja.mspmote;
import java.io.File;

import org.apache.log4j.Logger;

import org.contikios.cooja.Simulation;
import se.sics.mspsim.platform.z1x.Z1XNode;

public class Z1XMote extends MspMote {

    private static Logger logger = Logger.getLogger(Z1XMote.class);

    public Z1XMote(MspMoteType moteType, Simulation sim) {
        super(moteType, sim);
    }

    @Override
    protected boolean initEmulator(File fileELF) {
        try {
            Z1XNode z1xNode = new Z1XNode();
            // TODO Should not MspMote handle the registry?
            registry = z1xNode.getRegistry();
            prepareMote(fileELF, z1xNode);
        } catch (Exception e) {
            logger.fatal("Error when creating Z1XMote mote: ", e);
            return false;
        }
        return true;
    }

    @Override
    public String toString() {
        return "Z1X " + getID();
    }
}
