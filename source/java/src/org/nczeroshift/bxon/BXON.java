package org.nczeroshift.bxon;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.time.LocalDateTime;

import static org.nczeroshift.bxon.BXON.ObjectType.*;

public abstract class BXON {
    protected static final int MASK_TYPE = 0x0F;
    protected static final int MASK_LENGTH = 0x30;
    protected static final int MASK_FLAG = 0xC0;

    protected long length;
    protected byte type;

    public enum Type {
        TYPE_NIL(0, 0),
        TYPE_STRING(1, 0),
        TYPE_BOOLEAN(2, 1),
        TYPE_INT(3, 4),
        TYPE_LONG(4, 8),
        TYPE_FLOAT(5, 4),
        TYPE_DOUBLE(6, 8),
        TYPE_BYTE(7, 1),
        TYPE_DATE(8, 8);
        public int code;
        public int size;
        Type(int code, int size) {
            this.code = code;
            this.size = size;
        }

        public static Type typeForCode(byte code){
            if(code > 8)
                return null;
            return Type.values()[(int)code];
        }
    }

    public enum Length {
        LENGTH_8(0x00),
        LENGTH_16(0x10),
        LENGTH_32(0x20),
        LENGTH_64(0x30);
        public int code;

        Length(int code) {
            this.code = code;
        }
    }

    public enum ObjectType {
        FLAG_OBJECT(0x00),
        FLAG_ARRAY(0x40),
        FLAG_MAP(0x80);
        public int code;

        ObjectType(int code) {
            this.code = code;
        }
    }

    protected abstract void updateLengths();

    public Type getTypeFlag() {
        int code = type & MASK_TYPE;
        for (Type t : Type.values())
            if (t.code == code)
                return t;
        return null;
    }

    public Length getLengthFlag(){
        int code = type & MASK_LENGTH;
        for (Length t : Length.values())
            if (t.code == code)
                return t;
        return null;
    }

    public ObjectType getObjectFlag(){
        int code = type & MASK_FLAG;
        for (ObjectType t : ObjectType.values())
            if (t.code == code)
                return t;
        return null;
    }

    protected long GetLength(){
        return length;
    }

    protected static String toString(int depth, String output, Object value) {
        if (value == null) {
            output += "nil";
        } else if (value instanceof BXONMap) {
            output +=  ((BXONMap) value).toJSONString(depth + 1) ;
        } else if (value instanceof BXONArray) {
            output +=   ((BXONArray) value).toJSONString(depth + 1) ;
        } else if (value instanceof String) {
            output += "\"" + value + "\"";
        } else if (value instanceof Float) {
            output += value;
        } else if (value instanceof Double) {
            output += value;
        } else if (value instanceof Integer) {
            output += value;
        } else if (value instanceof Boolean) {
            output += Boolean.TRUE.equals(value) ? "true" : "false";
        } else if (value instanceof Byte) {
            output += String.format("0x%x", (int) ((Byte) value).intValue());
        } else if (value instanceof LocalDateTime) {
            LocalDateTime t = (LocalDateTime) value;
            output += String.format("\"%d/%02d/%02d %02d:%02d:%02d.%02d\"", t.getYear(), t.getMonthValue(), t.getDayOfMonth(), t.getHour(), t.getMinute(), t.getSecond(), t.getNano() / 1000);
        }
        return output;
    }

    private static int neededBytesForSize(long length){
        int lByteSize = 0;
        if (length <= 0xFF)
            lByteSize = 1;
        else if (length <= 0xFFFF)
            lByteSize = 2;
        else if (length <= 0xFFFFFFFF)
            lByteSize = 4;
        else
            lByteSize = 8;
        return lByteSize;
    }

    protected static long entrySize(Object obj) {
        if (obj != null) {
            if (obj instanceof BXON) {
                long length = ((BXON) obj).GetLength();

                return 1 + neededBytesForSize(length) + length;
            }
            else if (obj instanceof String) {
                try {
                    byte[] stringBytes = ((String) obj).getBytes("UTF8");
                    int length =  stringBytes.length;
                    return 1 + neededBytesForSize(length) + length;
                } catch (Exception e) {
                    return 1; // nil
                }
            } else if (obj instanceof Integer)
                return 1 + Type.TYPE_INT.size;
            else if (obj instanceof Double)
                return 1 + Type.TYPE_DOUBLE.size;
            else if (obj instanceof Float)
                return 1 + Type.TYPE_FLOAT.size;
            else if (obj instanceof Byte)
                return  1 + Type.TYPE_BYTE.size;
            else if (obj instanceof Long)
                return 1 + Type.TYPE_LONG.size;
            else if (obj instanceof Boolean)
                return 1 + Type.TYPE_BOOLEAN.size;
            else if (obj instanceof LocalDateTime)
                return 1 + Type.TYPE_DATE.size;
        }
        return 1; // nil
    }

    protected void write(OutputStream oS) throws IOException {
        DataOutputStream dos = new DataOutputStream(oS);

        byte lengthFlag = 0;
        byte lengthBytes[] = null;

        if (GetLength() <= 0xFF) {
            lengthFlag = (byte) Length.LENGTH_8.code;
            lengthBytes = ByteBuffer.allocate(1).put((byte) GetLength()).array();
        }
        else if (GetLength() <= 0xFFFF) {
            lengthFlag = (byte) Length.LENGTH_16.code;
            lengthBytes = ByteBuffer.allocate(2).putShort((short) GetLength()).array();
        }
        else if (GetLength() < 0xFFFFFFFF) {
            lengthFlag = (byte) Length.LENGTH_32.code;
            lengthBytes = ByteBuffer.allocate(4).putInt((int) GetLength()).array();
        }
        else {
            lengthFlag = (byte) Length.LENGTH_64.code;
            lengthBytes = ByteBuffer.allocate(8).putLong((long) GetLength()).array();
        }

        Byte typeFlag = 0;
        if(this instanceof BXONMap)
            typeFlag = (byte) FLAG_MAP.code;
        else
            typeFlag = (byte)ObjectType.FLAG_ARRAY.code;

        dos.writeByte(typeFlag | lengthFlag);
        dos.write(lengthBytes);
    }

    public static Object read(InputStream iS) throws IOException, BXONException {
        return readObject(iS);
    }

    protected static void writeObject(OutputStream outputStream, Object value) throws IOException {
        DataOutputStream dos = new DataOutputStream(outputStream);

        if (value == null) {
            dos.writeByte(Type.TYPE_NIL.code);
        } else if (value instanceof BXONMap) {
            ((BXONMap) value).write(outputStream);
        } else if (value instanceof BXONArray) {
            ((BXONArray) value).write(outputStream);
        } else if (value instanceof String) {
            byte[] stringBytes = ((String) value).getBytes("UTF8");
            int sizeFlag = 0;
            byte[] lengthBytes = null;

            if (stringBytes.length < 0xFF) {
                lengthBytes = ByteBuffer.allocate(1).put((byte) stringBytes.length).array();
                sizeFlag = Length.LENGTH_8.code;
            } else if (stringBytes.length < 0xFFFF) {
                lengthBytes = ByteBuffer.allocate(2).putShort((short) stringBytes.length).array();
                sizeFlag = Length.LENGTH_16.code;
            } else {
                lengthBytes = ByteBuffer.allocate(4).putInt(stringBytes.length).array();
                sizeFlag = Length.LENGTH_32.code;
            }

            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_STRING.code | sizeFlag);
            dos.write(lengthBytes);
            dos.write(stringBytes);
        } else if (value instanceof Float) {
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_FLOAT.code );
            dos.write(ByteBuffer.allocate(4).putFloat((Float)value).array());
        } else if (value instanceof Double) {
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_DOUBLE.code );
            dos.write(ByteBuffer.allocate(8).putDouble((Double)value).array());
        } else if (value instanceof Integer) {
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_INT.code );
            dos.write(ByteBuffer.allocate(4).putInt((Integer)value).array());
        } else if (value instanceof Long) {
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_LONG.code);
            dos.write(ByteBuffer.allocate(8).putLong((Long) value).array());
        } else if (value instanceof Boolean) {
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_BOOLEAN.code);
            dos.write(Boolean.TRUE.equals(value) ? 0xFF : 0);
        } else if (value instanceof Byte) {
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_BYTE.code);
            dos.write((byte)value);
        } else if (value instanceof LocalDateTime) {
            LocalDateTime date = (LocalDateTime)value;
//            year  28 bits
//            month 4 bits (16) 0-11

//            day   5 bits (32) 0-31
//            hour  5 bits (32) 0-24
//            min   6 bits (64) 0-59
//            secs  6 bits (64) 0-59
//            mil	10 bits (64) 1ms
            dos.writeByte(ObjectType.FLAG_OBJECT.code | Type.TYPE_DATE.code);

            int year = date.getYear();
            int month = date.getMonthValue();
            int p1 = (0xFFFFFFF0 & (year << 4)) | month & 0x0000000F;
            dos.writeInt(p1);

            int day = date.getDayOfMonth();
            int hour = date.getHour();
            int min = date.getMinute();
            int secs = date.getSecond();
            int mil = date.getNano() / 1000;

            int p2 = (0xF8000000 & (day << 27)) |
                    (0x07C00000 & (hour << 22)) |
                    (0x003F0000 & (min << 16)) |
                    (0x0000FC00 & (secs << 10)) |
                    (0x000003FF & mil);


            dos.writeInt(p2);
        }
    }

    protected static Object readObject(InputStream inputStream) throws IOException, BXONException {
        DataInputStream dos = new DataInputStream(inputStream);

        byte flags = dos.readByte();

        Long length = 0L;
        if ((flags & BXON.MASK_FLAG) != FLAG_OBJECT.code || (flags & BXON.MASK_TYPE) == Type.TYPE_STRING.code) {
            if ((flags & BXON.MASK_LENGTH) == Length.LENGTH_64.code)
                length = dos.readLong();
            else if ((flags & MASK_LENGTH) == Length.LENGTH_32.code)
                length = (long) dos.readInt();
            else if ((flags & MASK_LENGTH) == Length.LENGTH_16.code)
                length = (long) dos.readShort();
            else
                length = (long) dos.readByte();
        }

        if ((flags & BXON.MASK_FLAG) == FLAG_MAP.code) {
            return BXONMap.read(length, inputStream);
        } else if ((flags & BXON.MASK_FLAG) == FLAG_ARRAY.code) {
            return BXONArray.read(length, inputStream);
        }

        Type t = Type.typeForCode((byte)(flags & BXON.MASK_TYPE));

        if (t == null || t.equals(Type.TYPE_NIL))
            return null;

        if (t.size != 0)
            length = (long) t.size;

        byte [] dataBytes = new byte[(int)((long)length)];
        dos.read(dataBytes);

        if (t.equals(Type.TYPE_STRING)) {
            return new String(dataBytes, StandardCharsets.UTF_8);
        }else {

            ByteBuffer bb = ByteBuffer.wrap(dataBytes);
            if (t.equals(Type.TYPE_BOOLEAN)) {
                byte b = bb.get();
                return b == 0 ? Boolean.FALSE : Boolean.TRUE;
            } else if (t.equals(Type.TYPE_INT)) {
                return bb.getInt();
            } else if (t.equals(Type.TYPE_LONG)) {
                return bb.getLong();
            } else if (t.equals(Type.TYPE_FLOAT)) {
                return bb.getFloat();
            } else if (t.equals(Type.TYPE_DOUBLE)) {
                return bb.getDouble();
            } else if (t.equals(Type.TYPE_BYTE)) {
                return bb.get();
            } else if (t.equals(Type.TYPE_DATE)) {
                int p1 = bb.getInt();
                int p2 = bb.getInt();

                int year = (p1 & 0xFFFFFFF0) >>> 4;
                int month = (p1 & 0x0000000F);

                int day = (p2 & 0xF8000000 ) >>>27 ;
                int hour = (p2 & 0x07C00000) >>> 22;
                int min = (p2 & 0x003F0000) >>> 16;
                int secs = (p2 & 0x0000FC00) >>> 10;
                int milis = (p2 & 0x000003FF) * 1000;

                return LocalDateTime.of(year,month,day,hour,min,secs,milis);
            }
        }

        return null;
    }

    public void writeToStream(OutputStream oS) throws IOException {
        this.updateLengths();
        this.write(oS);
    }
}

