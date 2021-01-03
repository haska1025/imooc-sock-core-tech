package com.haska.udpio;

import java.io.*;
import java.net.DatagramPacket;
        import java.net.DatagramSocket;
import java.util.Random;

public class UDPServerIO {
    private static final int BIND_PORT = 9002;
    private static final String BIND_HOST = "127.0.0.1";
    private static final int RECV_BUFF_LEN = 1500;
    private static byte[] inBuff = new byte[RECV_BUFF_LEN];
    private static final int SEND_BUFF_LEN = 512;
    private static ByteArrayOutputStream outArray = new ByteArrayOutputStream(SEND_BUFF_LEN);
    private static short sequence = 1000;

    public static void main(String[] args) {
        // 构造服务器 Socket，绑定到一个固定的端口，监听的 IP 是 0.0.0.0
        try (DatagramSocket udpServer = new DatagramSocket(BIND_PORT)) {
            // 构造接收消息的数据包，需要传入 byte 数组。
            // 我们将这条语句放在循环外，不需要每次消息收发都构造此结构
            DatagramPacket inMessage = new DatagramPacket(inBuff, inBuff.length);
            byte[] body = new byte[RECV_BUFF_LEN];

            while (true){
                // 接收客户端消息
                udpServer.receive(inMessage);
                Message rMsg = new Message();
                rMsg.deserialize(inMessage);
                System.out.println("Recv UDP " + rMsg
                        + " from Client:" + inMessage.getSocketAddress().toString());

                String rsp = "Hello Client!";
                Message sMsg = new Message();
                sMsg.setVersion((byte)1);
                sMsg.setFlag((byte)21);
                sMsg.setSequence(sequence++);
                sMsg.setTimestamp((int)System.currentTimeMillis()&0xFFFFFFFF);
                sMsg.setBody(rsp.getBytes());

                // 构造发送的消息结构
                // 注意！！！对于服务器来说，发送的目标地址一定是接收消息时的源地址，所以从 inMessage 结构获取
                DatagramPacket outMessage = sMsg.serialize();
                outMessage.setSocketAddress(inMessage.getSocketAddress());

                // 发送消息
                udpServer.send(outMessage);
                System.out.println("Send UDP message:"
                        + rsp + " to Client:" + outMessage.getSocketAddress().toString());
                // 重置接收数据包消息长度，准备接收下一个消息
                inMessage.setLength(inBuff.length);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
