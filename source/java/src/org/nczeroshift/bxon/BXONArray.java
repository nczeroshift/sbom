package org.nczeroshift.bxon;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;

public class BXONArray extends BXON {
    private List<Object> objects = new ArrayList<>();

    private Class nativeType = null;

    private void check(int pos, Class c) throws BXONException {
        if (objects.size() >= pos)
            throw new BXONException("Index out of range");
        if (c != null && objects.get(pos) != null && !objects.get(pos).getClass().equals(c))
            throw new BXONException("Incorrect type of object");
    }

    public BXONArray() {
//        type = (byte) ObjectType.FLAG_ARRAY.code;
    }

    public BXONArray(int capacity) {
        objects = new ArrayList<Object>(capacity);
    }

    public int getSize() {
        return objects.size();
    }

    public BXONArray set(int i, Object o) throws BXONException {
        if (o instanceof String ||
                o instanceof Integer ||
                o instanceof Double ||
                o instanceof Float ||
                o instanceof Long ||
                o instanceof Boolean ||
                o instanceof LocalDateTime ||
                o instanceof LocalDate) {
            objects.set(i, o);
            return this;
        }
        throw new BXONException("Unsupported type");
    }

    public BXONArray add(Object obj) throws BXONException {
        objects.add(obj);
//        length += entrySize(obj);
        return this;
    }

    public String getString(int i) throws BXONException {
        check(i, String.class);
        return (String) objects.get(i);
    }

    public BXONMap getMap(int i) throws BXONException {
        check(i, BXONMap.class);
        return (BXONMap) objects.get(i);
    }

    public BXONArray getArray(int i) throws BXONException {
        check(i, BXONArray.class);
        return (BXONArray) objects.get(i);
    }

    public Integer getInteger(int i) throws BXONException {
        check(i, Integer.class);
        return (Integer) objects.get(i);
    }

    public Float getFloat(int i) throws BXONException {
        check(i, Float.class);
        return (Float) objects.get(i);
    }

    public Double getDouble(int i) throws BXONException {
        check(i, Double.class);
        return (Double) objects.get(i);
    }

    public Boolean getBoolean(int i) throws BXONException {
        check(i, Boolean.class);
        return (Boolean) objects.get(i);
    }

    public Long getLong(int i) throws BXONException {
        check(i, Long.class);
        return (Long) objects.get(i);
    }

    public Byte getByte(int i) throws BXONException {
        check(i, Byte.class);
        return (Byte) objects.get(i);
    }

    public LocalDateTime getDate(int i) throws BXONException {
        check(i, LocalDateTime.class);
        return (LocalDateTime) objects.get(i);
    }

    public Boolean isNil(int i) throws BXONException {
        check(i, null);
        return objects.get(i) == null;
    }

    public String toJSONString() {
        return toJSONString(0);
    }

    public String toJSONString(int depth) {
        String output = new String();

        output += "[\n";
        for (int i = 0; i < objects.size(); i++) {
            Object obj = objects.get(i);
            for (int j = 0; j < depth * 3; j++)
                output += " ";

            if (obj == null) {
                output += "nil";
            } else if (obj instanceof BXONMap) {
                output += "\"" + ((BXONMap) obj).toJSONString(depth + 1) + "\"";
            } else if (obj instanceof BXONArray) {
                output += "\"" + ((BXONArray) obj).toJSONString(depth + 1) + "\"";
            } else if (obj instanceof String) {
                output += "\"" + obj + "\"";
            } else if (obj instanceof Float) {
                output += obj;
            } else if (obj instanceof Double) {
                output += obj;
            } else if (obj instanceof Integer) {
                output += obj;
            } else if (obj instanceof Boolean) {
                output += Boolean.TRUE.equals(obj) ? "true" : "false";
            } else if (obj instanceof Byte) {
                output += String.format("0x%x", (Integer) obj);
            } else if (obj instanceof LocalDateTime) {
                LocalDateTime t = (LocalDateTime) obj;
                output += String.format("\"%d/%d/%d %d:%d:%d.%d\"", t.getYear(), t.getMonthValue(), t.getDayOfMonth(), t.getHour(), t.getMinute(), t.getSecond(), t.getNano() / 1000);
            }
            if (i < objects.size() - 1) {
                output += ",";
            }

            output += "\n";
        }
        for (int i = 0; i < (depth-1) * 3; i++)
            output += " ";
        output += "]";

        return output;
    }

    protected static BXONArray read(Long size, InputStream iS) throws IOException, BXONException {
        BXONArray array = new BXONArray();

        while (array.length < size) {
            Object value = BXON.readObject(iS);
            array.add(value);
            array.length += entrySize(value);
        }

        return array;
    }

    @Override
    protected void write(OutputStream outputStream) throws IOException {
        super.write(outputStream);
        for (Object val : objects) {
            writeObject(outputStream, val);
        }
    }

    protected void updateLengths() {
        long currentSize = 0;
        for (Object val : objects) {
            if(val != null && val instanceof BXON)
                ((BXON)val).updateLengths();
            currentSize += entrySize(val);
        }
        length = currentSize;
    }
}
