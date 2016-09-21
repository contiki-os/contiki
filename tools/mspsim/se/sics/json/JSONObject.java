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
 * $Id: JSONObject.java 95 2011-04-19 13:29:41Z nfi $
 *
 * -----------------------------------------------------------------
 *
 * JSONObject
 *
 * Authors : Joakim Eriksson, Niclas Finne
 * Created : 18 apr 2011
 * Updated : $Date: 2011-04-19 15:29:41 +0200 (Tue, 19 Apr 2011) $
 *           $Revision: 95 $
 */

package se.sics.json;

import java.io.IOException;
import java.io.Reader;
import java.io.Writer;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import org.json.simple.parser.ContainerFactory;
import org.json.simple.parser.ContentHandler;
import org.json.simple.parser.JSONParser;

/**
 *
 */
public class JSONObject extends HashMap<String,Object> implements JSONAware, JSONStreamAware {

    private static final long serialVersionUID = -4565653621517131043L;

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

    @Override
    public Object put(String key, Object value) {
        checkForCycles(value);
        return super.put(key, value);
    }

    @Override
    public void putAll(Map<? extends String,? extends Object> map) {
        for (Object v : map.values()) {
            checkForCycles(v);
        }
        super.putAll(map);
    }

    @Override
    public Object remove(Object key) {
        return super.remove(key);
    }

    @Override
    public void clear() {
        super.clear();
    }

    public boolean has(String key) {
        return containsKey(key);
    }

    public JSONObject set(String key, Object value) {
        put(key, value);
        return this;
    }

    public String[] getKeys() {
        return keySet().toArray(new String[0]);
    }

    public String getAsString(String key) {
        return getAsString(key, null);
    }

    public String getAsString(String key, String defaultValue) {
        Object v = get(key);
        return v != null ? v.toString() : defaultValue;
    }

    public int getAsInt(String key, int defaultValue) {
        Object v = get(key);
        if (v instanceof Number) {
            return ((Number) v).intValue();
        }
        return defaultValue;
    }

    public long getAsLong(String key, long defaultValue) {
        Object v = get(key);
        if (v instanceof Number) {
            return ((Number) v).longValue();
        }
        return defaultValue;
    }

    public float getAsFloat(String key, float defaultValue) {
        Object v = get(key);
        if (v instanceof Number) {
            return ((Number) v).floatValue();
        }
        return defaultValue;
    }

    public double getAsDouble(String key, double defaultValue) {
        Object v = get(key);
        if (v instanceof Number) {
            return ((Number) v).doubleValue();
        }
        return defaultValue;
    }

    public boolean getAsBoolean(String key, boolean defaultValue) {
        Object v = get(key);
        if (v instanceof Boolean) {
            return ((Boolean) v).booleanValue();
        }
        return defaultValue;
    }

    public JSONObject getJSONObject(String key) {
        Object v = get(key);
        if (v instanceof JSONObject) {
            return (JSONObject) v;
        }
        return null;
    }

    public JSONArray getJSONArray(String key) {
        Object v = get(key);
        if (v instanceof JSONArray) {
            return (JSONArray) v;
        }
        return null;
    }

    public void update(JSONObject source) {
        for(Map.Entry<String,Object> entry : source.entrySet()) {
            if (containsKey(entry.getKey())) {
                Object target = get(entry.getKey());
                Object v = entry.getValue();
                if (v instanceof JSONObject) {
                    if (target instanceof JSONObject) {
                        ((JSONObject) target).update((JSONObject) v);
                    }
                } else if (v instanceof JSONArray) {
                    if (target instanceof JSONArray) {
                        ((JSONArray) target).update((JSONArray) v);
                    }
                } else if (target instanceof JSONObject || target instanceof JSONArray) {
                    // Compound values can not be replaced by primitive values
                } else {
                    put(entry.getKey(), entry.getValue());
                }
            }
        }
    }

    public void merge(JSONObject source) {
        for(Map.Entry<String,Object> entry : source.entrySet()) {
            Object target = get(entry.getKey());
            Object v = entry.getValue();
            if (target != null) {
                if (v instanceof JSONObject) {
                    if (target instanceof JSONObject) {
                        ((JSONObject) target).merge((JSONObject) v);
                    }
                } else if (v instanceof JSONArray) {
                    if (target instanceof JSONArray) {
                        ((JSONArray) target).merge((JSONArray) v);
                    }
                } else if (target instanceof JSONObject || target instanceof JSONArray) {
                    // Compound values can not be replaced by primitive values
                } else {
                    put(entry.getKey(), entry.getValue());
                }
            } else {
                /* New value */
                if (v instanceof JSONObject) {
                    v = ((JSONObject) v).clone();
                } else if (v instanceof JSONArray) {
                    v = ((JSONArray) v).clone();
                }
                put(entry.getKey(), v);
            }
        }
    }

    @Override
    public JSONObject clone() {
        JSONObject clone = (JSONObject) super.clone();
        // Create deep copy
        for (String key : clone.getKeys()) {
            Object value = clone.get(key);
            if (value instanceof JSONObject) {
                clone.put(key, ((JSONObject) value).clone());
            } else if (value instanceof JSONArray) {
                clone.put(key, ((JSONArray) value).clone());
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
        return org.json.simple.JSONObject.toJSONString(this);
    }

    @Override
    public void writeJSONString(Writer out) throws IOException {
        org.json.simple.JSONObject.writeJSONString(this, out);
    }

    public static Object parseJSON(String input) throws ParseException {
        try {
            JSONParser parser = new JSONParser();
            return parser.parse(input, jsonObjectFactory);
        } catch (org.json.simple.parser.ParseException e) {
            throw new ParseException(e.getMessage(), e);
        }
    }

    public static Object parseJSON(Reader input) throws IOException, ParseException {
        try {
            JSONParser parser = new JSONParser();
            JSONObjectHandler doh = new JSONObjectHandler();
            parser.parse(input, doh);
            return doh.getResult();
        } catch (org.json.simple.parser.ParseException e) {
            throw new ParseException(e.getMessage(), e);
        }
    }

    public static JSONObject parseJSONObject(String input) throws ParseException {
        Object value = parseJSON(input);
        if (value instanceof JSONObject) {
            return (JSONObject) value;
        }
        throw new ParseException("not a JSON object: " + input);
    }

    public static JSONObject parseJSONObject(Reader input) throws ParseException, IOException {
        Object value = parseJSON(input);
        if (value instanceof JSONObject) {
            return (JSONObject) value;
        }
        throw new ParseException("not a JSON object: " + input);
    }

    private static ContainerFactory jsonObjectFactory = new ContainerFactory() {

        @Override
        public List<Object> creatArrayContainer() {
            return new JSONArray();
        }

        @Override
        public Map<String,Object> createObjectContainer() {
            return new JSONObject();
        }

    };

    private static class JSONObjectHandler implements ContentHandler {

        private Stack<Object> valueStack = new Stack<Object>();

        public Object getResult() {
            if (this.valueStack.size() == 0) {
                return null;
            }
            return this.valueStack.peek();
        }

        @Override
        public boolean endArray() throws IOException {
            trackBack();
            return true;
        }

        @Override
        public void endJSON() throws IOException {
        }

        @Override
        public boolean endObject() throws IOException {
            trackBack();
            // System.out.println("End JSON Object: " + getResult() + " Stack: "
            // + valueStack.size());
            /* we are finished if value stack is 1 */
            return this.valueStack.size() > 1;
        }

        @Override
        public boolean endObjectEntry() throws IOException {
            Object value = this.valueStack.pop();
            Object key = this.valueStack.pop();
            JSONObject parent = (JSONObject) this.valueStack.peek();
            parent.put((String) key, value);
            return true;
        }

        private void trackBack() {
            if (this.valueStack.size() > 1) {
                Object value = this.valueStack.pop();
                Object prev = this.valueStack.peek();
                if (prev instanceof String) {
                    this.valueStack.push(value);
                }
            }
        }

        private void consumeValue(Object value) {
            if (this.valueStack.size() == 0) {
                this.valueStack.push(value);
            } else {
                Object prev = this.valueStack.peek();
                if (prev instanceof JSONArray) {
                    JSONArray array = (JSONArray) prev;
                    array.add(value);
                } else {
                    this.valueStack.push(value);
                }
            }
        }

        @Override
        public boolean primitive(Object value) throws IOException {
            consumeValue(value);
            return true;
        }

        @Override
        public boolean startArray() throws IOException {
            JSONArray array = new JSONArray();
            consumeValue(array);
            this.valueStack.push(array);
            return true;
        }

        @Override
        public void startJSON() throws IOException {
            this.valueStack.clear();
        }

        @Override
        public boolean startObject() throws IOException {
            JSONObject object = new JSONObject();
            consumeValue(object);
            this.valueStack.push(object);
            return true;
        }

        @Override
        public boolean startObjectEntry(String key) throws IOException {
            this.valueStack.push(key);
            return true;
        }
    }

}
