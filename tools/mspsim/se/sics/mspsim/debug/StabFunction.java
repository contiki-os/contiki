
package se.sics.mspsim.debug;

import java.util.ArrayList;
import se.sics.mspsim.util.Utils;
import se.sics.mspsim.debug.StabDebug.Stab;

public class StabFunction {

    public String name;
    public String returnType; /* string representation of return type */
    public ArrayList<Param> params;
    public int endAddress;
    public int startAddress;
    public int startLine;

    public void addParameter(Stab stab) {
        if (params == null) params = new ArrayList<Param>();
        String[] name = stab.data.split(":");
        params.add(new Param(name[0]));
    }
    
    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("Function: " + name);
        if (params != null) {
            sb.append("(");
            for (int i = 0; i < params.size(); i++) {
                if (i > 0) sb.append(", ");
                sb.append(params.get(i).name);
            }
            sb.append(")");
            sb.append(" at ").append(Utils.hex16(startAddress));
            sb.append(" - ").append(Utils.hex16(endAddress));
        }
        return sb.toString();
    }
    
    class Param {
        String name;
        Param(String name) {
            this.name = name;
        }
    }
}
