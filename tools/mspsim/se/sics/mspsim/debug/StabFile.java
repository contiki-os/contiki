/**
 * Copyright (c) 2010, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of MSPSim.
 *
 * -----------------------------------------------------------------
 *
 * StabDebug
 *
 * Author  : Joakim Eriksson
 * Created : Sun Oct 21 22:00:00 2009
 * Updated : $Date: 2010-07-09 23:22:13 +0200 (Fri, 09 Jul 2010) $
 *           $Revision: 717 $
 */

package se.sics.mspsim.debug;

import java.util.ArrayList;
import java.util.HashMap;
import se.sics.mspsim.debug.StabDebug.Stab;

public class StabFile {

    public int startAddress;

    public String path;
    public String file;

    public int stabIndex;
    
    private ArrayList<StabFunction> functions = new ArrayList<StabFunction>();
    private HashMap<String, StabType> types = new HashMap<String,StabType>();
    private StabFunction lastFunction;
    
    public void handleStabs(Stab[] stabs) {
        int i = stabIndex;
        while(i < stabs.length) {
            StabDebug.Stab stab = stabs[i];
            System.out.println("Handling stab: " + stab);
            switch(stab.type) {
            case StabDebug.N_SO:
                if (stab.value != startAddress) {
                    return;
                }
                if (stab.data.length() > 0) {
                    if (path == null) {
                        path = stab.data;
                    } else if (file == null) {
                        file = stab.data;
                    }
                }
                i++;
                break;
            case StabDebug.N_FUN:
                i += addFunction(i, stabs);
                break;
            case StabDebug.N_LSYM:
                i += addType(i, stabs);
                break;
            default:
                i++;
            }
        }
    }
    
    private int addType(int i, Stab[] stabs) {
        Stab stab = stabs[i];
        StabType type = new StabType(stab, types);
        types.put(type.name, type);
        types.put(type.internalName, type);
        return 1;
    }

    private int addFunction(int i, Stab[] stabs) {
        int index = i;
        Stab stab = stabs[index];
        /* name:ReturnType */
        if (stab.data.length() == 0) {
           /* just ens last function */
           if (lastFunction != null) {
               lastFunction.endAddress = lastFunction.startAddress + stab.value;
           }
           return 1;
        }
        StabFunction fun = new StabFunction();
        functions.add(fun);
        lastFunction = fun;
        String[] fname = stab.data.split(":");
        fun.name = fname[0];
        fun.returnType = fname[1];
        fun.startAddress = stab.value;
        fun.startLine = stab.desc;
        index++;

        while (index < stabs.length && isParam(stabs[index])) {
            fun.addParameter(stabs[index]);
            index++;
        }

        return index - i;
    }

    private boolean isParam(Stab stab) {
        return (stab.type == StabDebug.N_REG_PARAM ||
            stab.type == StabDebug.N_VAR_PARAM);
    }

    public String toString() {
        StringBuffer sb = new StringBuffer();
        sb.append("File: " + path + file + " starts at: " + startAddress + "\n");
        for (int i = 0; i < functions.size(); i++) {
            sb.append("  ").append(functions.get(i)).append("\n");
        }
        return sb.toString();
    }
}
