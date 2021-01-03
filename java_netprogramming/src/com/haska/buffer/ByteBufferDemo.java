package com.haska.buffer;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;

public class ByteBufferDemo {
    public static void test_bytebuffer(){
        ByteBuffer newBuffer = ByteBuffer.allocate(1024);
        System.out.println("new ByteBuffer capacity=" + newBuffer.capacity()
        + " position=" + newBuffer.position() + " limit=" + newBuffer.limit());

        byte[] tmpByteArray = new byte[512];
        ByteBuffer wrapBuffer = ByteBuffer.wrap(tmpByteArray);
        System.out.println("new ByteBuffer capacity=" + wrapBuffer.capacity()
                + " position=" + wrapBuffer.position() + " limit=" + wrapBuffer.limit());

        tmpByteArray[0] = (byte)0x11;
        tmpByteArray[1] = (byte)0x22;
        newBuffer.put((byte)0xAA);
        newBuffer.put((byte)0xBB);
        newBuffer.put(tmpByteArray, 0, 2);
        System.out.println("new ByteBuffer capacity=" + newBuffer.capacity()
                + " position=" + newBuffer.position() + " limit=" + newBuffer.limit());

        newBuffer.flip();
        System.out.println("new ByteBuffer capacity=" + newBuffer.capacity()
                + " position=" + newBuffer.position() + " limit=" + newBuffer.limit());

        newBuffer.get();
        newBuffer.get(tmpByteArray, 0, 2);
        System.out.println("new ByteBuffer capacity=" + newBuffer.capacity()
                + " position=" + newBuffer.position() + " limit=" + newBuffer.limit());

        newBuffer.rewind();
        newBuffer.compact();
    }
    public static void main(String[] args) {
test_bytebuffer();
    }
}
