package com.haska.multithreadserver;

import java.io.*;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

public class TCPClientMultiThread {
    // 服务器监听的端口号
    private static final int PORT = 56002;
    // 连接超时时间
    private static final int TIMEOUT = 15000;
    // 客户端执行次数
    private static final int TEST_TIMES = 10;

    public static void main(String[] args) {
        Socket client = null;
        try {
            // 测试次数
            int testCount = 0;
            // 调用无参构造方法
            client = new Socket();
            // 构造服务器地址结构
            SocketAddress serverAddr = new InetSocketAddress("192.168.0.101", PORT);
            // 连接服务器，超时时间是 15 毫秒
            client.connect(serverAddr, TIMEOUT);

            System.out.println("Client start:" + client.getLocalSocketAddress().toString());
            while (true) {
                // 向服务器发送数据
                DataOutputStream out = new DataOutputStream(
                        new BufferedOutputStream(client.getOutputStream()));
                String req = "Hello Server!";
                out.writeInt(req.getBytes().length);
                out.write(req.getBytes());
                // 不能忘记 flush 方法的调用
                out.flush();
                System.out.println("Send to server:" + req);

                // 接收服务器的数据
                DataInputStream in = new DataInputStream(
                        new BufferedInputStream(client.getInputStream()));

                int msgLen = in.readInt();
                byte[] inMessage = new byte[msgLen];
                in.read(inMessage);
                System.out.println("Recv from server:" + new String(inMessage));

                // 如果执行次数已经达到上限，结束测试。
                if (++testCount >= TEST_TIMES) {
                    break;
                }

                // 等待 1 秒然后再执行
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
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
