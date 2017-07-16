package org.nczeroshift;

import org.nczeroshift.bxon.BXON;
import org.nczeroshift.bxon.BXONArray;
import org.nczeroshift.bxon.BXONException;
import org.nczeroshift.bxon.BXONMap;

import java.io.*;
import java.time.LocalDateTime;

public class Main {

    public static void main(String[] args) throws BXONException, IOException {

        BXONMap map = new BXONMap();

        map.setInt("int_1", 1);
        map.setLong("long_2", 100000000L);
        map.setString("string_3", "bxon test string");
        map.setFloat("float_4", 10.000001f);
        map.setDouble("double_5", 12.000001f);
        map.setBoolean("boolean_6", false);
        map.setBoolean("boolean_7", true);
        map.setByte("byte_8", (byte)0xBA);
        map.setNil("nil_9");
        map.setDate("date_10", LocalDateTime.of(2017,6,23,12,25,46,80));

        BXONMap map2 = new BXONMap();
        for(int i = 0;i<20;i++){
            map2.setInt("v_"+i+"_test_value",i);
        }

        map.setMap("map_10",map2);

        BXONArray array1 = new BXONArray();
        for (int i = 0; i < 20; i++) {
            array1.add((float) (i / 20.0));
        }

        map.setArray("array_11",array1);

        File testFile = new File("out.bxon");

        BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(testFile));

        map.writeToStream(bos);

        bos.close();

        BufferedInputStream bis = new BufferedInputStream(new FileInputStream(testFile));
        BXON obj = (BXON) BXON.read(bis);

        System.out.println(map.toJSONString());

        System.out.println(((BXONMap)obj).toJSONString());
    }
}
