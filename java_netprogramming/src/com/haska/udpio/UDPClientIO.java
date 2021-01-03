package com.haska.udpio;

import java.io.*;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetSocketAddress;
import java.net.SocketAddress;

public class UDPClientIO {
    private static final int PORT = 9002;
    private static final String DST_HOST = "127.0.0.1";
    private static final int RECV_BUFF_LEN = 1500;
    private static byte[] inBuff = new byte[RECV_BUFF_LEN];
    private static short sequence = 1;

    public static void main(String[] args) {
        // 创建 UDP 客户端 Socket，选择无参构造方法，由系统分配本地端口号和网络接口
        try (DatagramSocket udpClient = new DatagramSocket()){
            // 构造发送的目标地址，指定目标 IP 和目标端口号
            SocketAddress to = new InetSocketAddress(DST_HOST, PORT);
            byte[] body = new byte[RECV_BUFF_LEN];

            while (true){
                String req = "Hello Server!";
                Message sMsg = new Message();
                sMsg.setVersion((byte)1);
                sMsg.setFlag((byte)21);
                sMsg.setSequence(sequence++);
                sMsg.setTimestamp((int)System.currentTimeMillis()&0xFFFFFFFF);
                sMsg.setBody(req.getBytes());
                DatagramPacket outMessage = sMsg.serialize();
                outMessage.setSocketAddress(to);
                // 发送消息
                udpClient.send(outMessage);
                System.out.println("Send UDP message:"
                        + req + " to server:" + outMessage.getSocketAddress().toString());

                // 构造接收消息的数据包，需要传入 byte 数组
                DatagramPacket inMessage = new DatagramPacket(inBuff, inBuff.length);
                // 接收消息
                udpClient.receive(inMessage);

                Message rMsg = new Message();
                rMsg.deserialize(inMessage);
                System.out.println("Recv UDP " + rMsg
                        + " from Client:" + inMessage.getSocketAddress().toString());

                // 每隔 2 秒发送一次消息
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
