package com.haska.udp;

import java.io.IOException;
        import java.net.DatagramPacket;
        import java.net.DatagramSocket;
        import java.net.InetSocketAddress;
        import java.net.SocketAddress;

public class UDPClient {
    private static final int PORT = 9002;
    private static final String DST_HOST = "127.0.0.1";
    private static final int RECV_BUFF_LEN = 1500;
    private static byte[] inBuff = new byte[RECV_BUFF_LEN];

    public static void main(String[] args) {
        // 创建 UDP 客户端 Socket，选择无参构造方法，由系统分配本地端口号和网络接口
        try (DatagramSocket udpClient = new DatagramSocket()){
            // 构造发送的目标地址，指定目标 IP 和目标端口号
            SocketAddress to = new InetSocketAddress(DST_HOST, PORT);
            while (true){
                String req = "Hello Server!";
                // 构造发送数据包，需要传入消息内容和目标地址结构 SocketAddress
                DatagramPacket message = new DatagramPacket(req.getBytes(), req.length(), to);
                // 发送消息
                udpClient.send(message);
                System.out.println("Send UDP message:"
                        + req + " to server:" + message.getSocketAddress().toString());

                // 构造接收消息的数据包，需要传入 byte 数组
                DatagramPacket inMessage = new DatagramPacket(inBuff, inBuff.length);
                // 接收消息
                udpClient.receive(inMessage);

                System.out.println("Recv UDP message:"
                        + new String(inMessage.getData(), 0, inMessage.getLength())
                        + " from server:" + inMessage.getSocketAddress().toString());

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
