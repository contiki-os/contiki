/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
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
 * $Id: JSONArray.java 95 2011-04-19 13:29:41Z nfi $
 *
 * -----------------------------------------------------------------
 *
 * JSONArray
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 18 apr 2011
 * Updated : $Date: 2011-04-19 15:29:41 +0200 (Tue, 19 Apr 2011) $
 *           $Revision: 95 $
 */

package se.sics.json;

import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;

/**
 *
 */
public class JSONArray extends ArrayList<Object> implements JSONAware, JSONStreamAware {

    private static final long serialVersionUID = 396701954050866614L;

    private void checkForCycles(Object value) {
        if (this == value) {
            throw new IllegalArgumentException("cycle detected");
        }
        if (value instanceof JSONObject) {
            JSONObject object = (JSONObject) value;
            for (Object v : object.values()) {
                checkForCycles(v);
            }
        } else if (value instanceof JSONArray) {
            JSONArray list = (JSONArray) value;
            for (Object v : list) {
                checkForCycles(v);
            }
        }
    }

    private void checkAllForCycles(Collection<? extends Object> c) {
        for(Object v : c) {
            checkForCycles(v);
        }
    }

    @Override
    public boolean add(Object e) {
        checkForCycles(e);
        return super.add(e);
    }

    @Override
    public boolean remove(Object o) {
        return super.remove(o);
    }

    @Override
    public boolean addAll(Collection<? extends Object> c) {
        checkAllForCycles(c);
        return super.addAll(c);
    }

    @Override
    public boolean addAll(int index, Collection<? extends Object> c) {
        checkAllForCycles(c);
        return super.addAll(index, c);
    }

    @Override
    public boolean removeAll(Collection<?> c) {
        return super.removeAll(c);
    }

    @Override
    public boolean retainAll(Collection<?> c) {
        return super.retainAll(c);
    }

    @Override
    public void clear() {
        super.clear();
    }

    @Override
    public Object set(int index, Object element) {
        checkForCycles(element);
        return super.set(index, element);
    }

    @Override
    public void add(int index, Object element) {
        checkForCycles(element);
        super.add(index, element);
    }

    @Override
    public Object remove(int index) {
        return super.remove(index);
    }

    public String getAsString(int index) {
        Object v = get(index);
        return v != null ? v.toString() : null;
    }

    public int getAsInt(int index, int defaultValue) {
        Object v = get(index);
        if (v instanceof Number) {
            return ((Number)v).intValue();
        }
        return defaultValue;
    }

    public long getAsLong(int index, long defaultValue) {
        Object v = get(index);
        if (v instanceof Number) {
            return ((Number)v).longValue();
        }
        return defaultValue;
    }

    public float getAsFloat(int index, float defaultValue) {
        Object v = get(index);
        if (v instanceof Number) {
            return ((Number)v).floatValue();
        }
        return defaultValue;
    }

    public double getAsDouble(int index, double defaultValue) {
        Object v = get(index);
        if (v instanceof Number) {
            return ((Number)v).doubleValue();
        }
        return defaultValue;
    }

    public boolean getAsBoolean(int index, boolean defaultValue) {
        Object v = get(index);
        if (v instanceof Boolean) {
            return ((Boolean)v).booleanValue();
        }
        return defaultValue;
    }

    public JSONObject getJSONObject(int index) {
        Object v = get(index);
        if (v instanceof JSONObject) {
            return (JSONObject)v;
        }
        return null;
    }

    public JSONArray getJSONArray(int index) {
        Object v = get(index);
        if (v instanceof JSONArray) {
            return (JSONArray)v;
        }
        return null;
    }

    public void update(JSONArray v) {
        int count = v.size();
        if (size() < count) {
            count = size();
        }
        for(int i = 0; i < count; i++) {
            Object target = get(i);
            Object source = v.get(i);
            if (source instanceof JSONObject) {
                if (target instanceof JSONObject) {
                    ((JSONObject) target).update((JSONObject) source);
                }
            } else if (source instanceof JSONArray) {
                if (target instanceof JSONArray) {
                    ((JSONArray) target).update((JSONArray) source);
                }
            } else if (target instanceof JSONObject || target instanceof JSONArray) {
                // Compound values can not be replaced by primitive values
            } else {
                set(i, source);
            }
        }
    }

    public void merge(JSONArray v) {
        int count = v.size();
        if (size() < count) {
            count = size();
        }
        for(int i = 0; i < count; i++) {
            Object target = get(i);
            Object source = v.get(i);
            if (source instanceof JSONObject) {
                if (target instanceof JSONObject) {
                    ((JSONObject) target).merge((JSONObject) source);
                }
            } else if (source instanceof JSONArray) {
                if (target instanceof JSONArray) {
                    ((JSONArray) target).merge((JSONArray) source);
                }
            } else if (target instanceof JSONObject || target instanceof JSONArray) {
                // Compound values can not be replaced by primitive values
            } else {
                set(i, source);
            }
        }
        if (v.size() > size()) {
            for(int i = size(), n = v.size(); i < n; i++) {
                Object source = v.get(i);
                if (source instanceof JSONObject) {
                    add(((JSONObject) source).clone());
                } else if (source instanceof JSONArray) {
                    add(((JSONArray) source).clone());
                } else {
                    add(source);
                }
            }
        }
    }

    @Override
    public JSONArray clone() {
        JSONArray clone = (JSONArray) super.clone();
        // Create deep copy
        for(int i = 0, n = clone.size(); i < n; i++) {
            Object value = clone.get(i);
            if (value instanceof JSONObject) {
                clone.set(i, ((JSONObject)value).clone());
            } else if (value instanceof JSONArray) {
                clone.set(i, ((JSONArray)value).clone());
            }
        }
        return clone;
    }

    @Override
    public String toString() {
        return toJSONString();
    }

    @Override
    public String toJSONString() {
        return org.json.simple.JSONArray.toJSONString(this);
    }

    @Override
    public void writeJSONString(Writer out) throws IOException {
        org.json.simple.JSONArray.writeJSONString(this, out);
    }

}
