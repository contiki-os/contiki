package se.sics.mspsim.debug;

import java.util.HashMap;
import se.sics.mspsim.debug.StabDebug.Stab;

public class StabType {

    String name;
    String internalName;
    int size;
    boolean signed;
    StabType typeDef;

    private HashMap<String,StabType> types;

    /* typical stab that define a type */
    /* int:t(0,1)=r(0,1);-32768;32767; */
    public StabType(Stab stab, HashMap<String,StabType> types) {
        this.types = types;
        String typeDef = stab.data;
        String[] parts = typeDef.split(":");
        name = parts[0];
        parts = parts[1].split("=");
        internalName = parts[0];
        if (!handleKnown(name)) {
            for (int i = 0; i < parts.length; i++) {
                handleTypeDef(parts[i]);
            }
        }
    }

    public int getSize() {
        if (typeDef != null) return typeDef.getSize();
        return size;
    }

    private void signed(int size) {
        this.size = size;
        signed = true;
    }
    
    private void unsigned(int size) {
        this.size = size;
        signed = false;
    }
    
    /* handle the known types */
    private boolean handleKnown(String name) {
        if ("int".equals(name)) {
            signed(2);
        } else if ("unsigned int".equals(name)) {
            unsigned(2);
        } else if ("char".equals(name)) {
            unsigned(1);
        } else if ("unsigned char".equals(name)) {
            unsigned(1);
        } else if ("signed char".equals(name)) {
            signed(1);
        } else if ("long int".equals(name)) {
            signed(4);
        } else if ("long unsigned int".equals(name)) {
            unsigned(4);
        } else if ("long long int".equals(name)) {
            signed(8);
        } else {
            return false;
        }
        return true;
    }

    /* format:
     *  r(0,1);-321768;32767; (int)
     *  R3;8;0;
     *  r(0,1);4;0; (float)
     *  or (0,1) => same as type t(0,1)
     *   */
    private void handleTypeDef(String def) {
        if (def.startsWith("(")) {
            StabType tDef = types.get("t" + def);
            if (tDef != null) {
                typeDef = tDef;
            } else {
                System.out.println("ERROR: can not find: " + def);
            }
        }
    }
}