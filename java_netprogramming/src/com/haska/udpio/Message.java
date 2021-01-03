package com.haska.udpio;

import java.io.*;
import java.net.DatagramPacket;

public class Message implements Serializable {
    /*
+-----------------+-----------------+-----------------|-----------------+
| version(8 bits) | flag(8 bits)    |          Sequence(16 bits)        |
+-----------------|-----------------+-----------------------------------+
|                  Timestamp(32 bits)                                   |
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
|                                                                       |
|                       Message Body                                    |
|                                                                       |
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     */
    private static final int SEND_BUFF_LEN = 512;
    private static ByteArrayOutputStream outArray = new ByteArrayOutputStream(SEND_BUFF_LEN);

    private byte version =1;
    private byte flag;
    private short sequence;
    private int timestamp;
    private byte[] body = null;
    private int bodyLength = 0;

    public byte getVersion() {
        return version;
    }

    public void setVersion(byte version) {
        this.version = version;
    }

    public byte getFlag() {
        return flag;
    }

    public void setFlag(byte flag) {
        this.flag = flag;
    }

    public short getSequence() {
        return sequence;
    }

    public void setSequence(short sequence) {
        this.sequence = sequence;
    }

    public int getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(int timestamp) {
        this.timestamp = timestamp;
    }

    public byte[] getBody() {
        return body;
    }

    public void setBody(byte[] body) {
        this.body = body;
    }

    public DatagramPacket serialize()
    {
        try {
            outArray.reset();
            DataOutputStream out = new DataOutputStream(outArray);

            out.writeByte(this.getVersion());
            out.writeByte(this.getFlag());
            out.writeShort(this.getSequence());
            out.writeInt(this.getTimestamp());
            out.write(this.body);
            // 构造发送数据包，需要传入消息内容和目标地址结构 SocketAddress
            byte[] outBytes = outArray.toByteArray();
            DatagramPacket message = new DatagramPacket(outBytes,  outBytes.length);

            return message;
        } catch (IOException e) {
            e.printStackTrace();
        }

        return null;
    }
    public void deserialize(DatagramPacket inMessage)
    {
        try {
            DataInputStream in = new DataInputStream(
                    new ByteArrayInputStream(inMessage.getData(), 0, inMessage.getLength()));
            this.version = in.readByte();
            this.flag = in.readByte();
            this.sequence = in.readShort();
            this.timestamp = in.readInt();
            this.body = new byte[512];
            this.bodyLength = in.read(this.body);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public String toString() {
        return " version: " + this.getVersion()
                + " flag: " + this.getFlag()
                + " sequence: " + this.getSequence()
                + " timestamp: " + this.getSequence()
                + " message body: " +new String(body, 0, this.bodyLength);
    }
}
