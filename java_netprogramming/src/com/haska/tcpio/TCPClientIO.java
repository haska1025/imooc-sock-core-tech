package com.haska.tcpio;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

public class TCPClientIO {
    // 服务器监听的端口号
    private static final int PORT = 56002;
    private static final int TIMEOUT = 15000;

    public static void main(String[] args) {
        Socket client = null;
        try {
            // 调用无参构造方法
            client = new Socket();
            // 构造服务器地址结构
            SocketAddress serverAddr = new InetSocketAddress("127.0.0.1", PORT);
            // 连接服务器，超时时间是 15 毫秒
            client.connect(serverAddr, TIMEOUT);

            System.out.println("Client start:" + client.getLocalSocketAddress().toString());

            // 向服务器发送数据
            DataOutputStream out = new DataOutputStream(
                    new BufferedOutputStream(client.getOutputStream()));
            String req = "Hello Server!\n";
            out.writeInt(req.getBytes().length);
            out.write(req.getBytes());
            // 不能忘记 flush 方法的调用
            out.flush();
            System.out.println("Send to server:" + req + " length:" +req.getBytes().length);

            // 接收服务器的数据
            DataInputStream in = new DataInputStream(
                    new BufferedInputStream(client.getInputStream()));

            int msgLen = in.readInt();
            byte[] inMessage = new byte[msgLen];
            in.read(inMessage);
            System.out.println("Recv from server:" + new String(inMessage) + " length:" + msgLen);
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (client != null){
                try {
                    client.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
