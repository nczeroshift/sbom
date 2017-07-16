package org.nczeroshift.bxon;


import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.LinkedHashMap;

public class BXONMap extends BXON {
    private LinkedHashMap<Object, Object> entries = new LinkedHashMap<>();
    private Class keyType = null;

    public BXONMap() {

    }

    private void checkKey(Object key) throws BXONException {
        if (keyType == null) {
            keyType = key.getClass();
            if (key.equals(String.class))
                type |= BXON.MASK_TYPE & Type.TYPE_STRING.code;
            else if (key.equals(Integer.class))
                type |= BXON.MASK_TYPE & Type.TYPE_INT.code;
            else if (key.equals(Double.class))
                type |= BXON.MASK_TYPE & Type.TYPE_DOUBLE.code;
            else if (key.equals(Float.class))
                type |= BXON.MASK_TYPE & Type.TYPE_FLOAT.code;
            else if (key.equals(Byte.class))
                type |= BXON.MASK_TYPE & Type.TYPE_BYTE.code;
            else if (key.equals(Long.class))
                type |= BXON.MASK_TYPE & Type.TYPE_LONG.code;
            else if (key.equals(LocalDateTime.class) || key.equals(LocalDate.class))
                type |= BXON.MASK_TYPE & Type.TYPE_DATE.code;
        } else if (!keyType.equals(key.getClass()))
            throw new BXONException("Map key type don't match new key");
    }

//    private void updateRemove(Object key){
//        if (entries.containsKey(key)) {
//            length -= entrySize(entries.get(key));
//            length -= entrySize(key);
//        }
//    }
//
//    private void updateAdd(Object key, Object value){
//        length += entrySize(key);
//        length += entrySize(value);
//    }

    public BXONArray getArray(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return null;
        if (obj instanceof BXONMap)
            return (BXONArray) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setArray(Object key, BXONArray array) throws BXONException {
//        updateRemove(key);
        entries.put(key, array);
//        updateAdd(key, array);
        return this;
    }

    public BXONMap getMap(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return null;
        if (obj instanceof BXONMap)
            return (BXONMap) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setMap(Object key, BXONMap map) throws BXONException {
//        updateRemove(key);
        entries.put(key, map);
//        updateAdd(key,map);
        return this;
    }

    public String optString(Object key, String opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof String)
            return (String) obj;
        throw new BXONException("Object for key not found");
    }

    public String getString(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof String)
            return (String) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setString(Object key, String value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, value);
//        updateAdd(key, value);
        return this;
    }

    public double optDouble(Object key, double opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof Double)
            return (Double) obj;
        throw new BXONException("Object for key not found");
    }

    public double getDouble(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof Double)
            return (Double) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setDouble(Object key, double value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, (Double) value);
//        updateAdd(key, value);
        return this;
    }

    public Float optFloat(Object key, float opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof Double)
            return (Float) obj;
        throw new BXONException("Object for key not found");
    }

    public Float getFloat(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return null;
        if (obj instanceof Float)
            return (Float) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setFloat(Object key, float value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, (Float) value);
//        updateAdd(key, value);
        return this;
    }


    public int optInt(Object key, int opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof Integer)
            return (Integer) obj;
        throw new BXONException("Object for key not found");
    }

    public int getInt(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof Integer)
            return (Integer) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setInt(Object key, int value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, (Integer) value);
//        updateAdd(key, value);
        return this;
    }

    public long optLong(Object key, long opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof Long)
            return (Integer) obj;
        throw new BXONException("Object for key not found");
    }

    public long getLong(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof Long)
            return (Integer) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setLong(Object key, long value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, (Long) value);
//        updateAdd(key, value);
        return this;
    }

    public byte optByte(Object key, byte opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof Byte)
            return (Byte) obj;
        throw new BXONException("Object for key not found");
    }

    public byte getByte(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof Byte)
            return (Byte) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setByte(Object key, byte value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, (Byte) value);
//        updateAdd(key, value);
        return this;
    }

    public boolean optBoolean(Object key, boolean opt) throws BXONException {
        Object obj = entries.get(key);
        if (obj == null)
            return opt;
        if (obj instanceof Boolean)
            return (Boolean) obj;
        throw new BXONException("Object for key not found");
    }

    public boolean getBoolean(Object key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof Boolean)
            return (Boolean) obj;
        throw new BXONException("Object for key not found");
    }

    public BXONMap setBoolean(Object key, boolean value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, (Boolean) value);
//        updateAdd(key, value);
        return this;
    }

    public boolean isNil(Object key) throws BXONException {
        if (!entries.containsKey(key))
            throw new BXONException("Object for key not found");
        if (entries.get(key) == null)
            return true;
        return false;
    }

    public void setNil(Object key) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, null);
//        updateAdd(key, null);
    }


    public BXONMap setDate(String key, LocalDateTime value) throws BXONException {
        checkKey(key);
//        updateRemove(key);
        entries.put(key, value);
//        updateAdd(key, value);
        return this;
    }

    public BXONMap setDate(String key, LocalDate value) throws BXONException {
        LocalDateTime v = value.atStartOfDay();
        checkKey(key);
//        updateRemove(key);
        entries.put(key, v);
//        updateAdd(key, value);
        return this;
    }

    public LocalDateTime getDate(String key) throws BXONException {
        Object obj = entries.get(key);
        if (obj instanceof LocalDateTime)
            return (LocalDateTime) obj;
        throw new BXONException("Object for key not found");
    }

    public String toJSONString(){
        return toJSONString(0);
    }

    public String toJSONString(int depth) {
        String output = new String();

        output += "{\n";

        ArrayList<Object> keysArray = new ArrayList<Object>(entries.keySet());

        depth++;

        for (int i = 0; i < keysArray.size(); i++) {
            Object key = keysArray.get(i);

            Object value = entries.get(key);

            for (int j = 0; j < depth * 3; j++)
                output += " ";

            output += "\"" + key.toString() + "\":";

            output = toString(depth, output, value);

            if (i < keysArray.size() - 1) {
                output += ",";
            }

            output += "\n";
        }
        for (int i = 0; i < (depth-1) * 3; i++)
            output += " ";
        output += "}";

        return output;
    }

    protected void write(OutputStream outputStream) throws IOException {
        super.write(outputStream);
        for (Object obj : entries.keySet()) {
            Object value = entries.get(obj);
            writeObject(outputStream, obj);
            writeObject(outputStream, value);
        }
    }


    protected static BXONMap read(Long size, InputStream iS) throws IOException, BXONException {
        BXONMap map = new BXONMap();

        while (map.length < size) {
            Object key = BXON.readObject(iS);
            Object value = BXON.readObject(iS);
            map.checkKey(key);
            map.entries.put(key, value);
            map.length += entrySize(key) + entrySize(value);
        }

        return map;
    }

    protected void updateLengths(){
        long currentSize = 0;
        for (Object key : entries.keySet()) {
            currentSize += entrySize(key);
            Object value = entries.get(key);
            if (value != null && value instanceof BXON)
                ((BXON)value).updateLengths();
            currentSize += entrySize(value);
        }
        length = currentSize;
    }
}
