package com.haska.tcp;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;

public class TCPClient {
    // 服务器监听的端口号
    private static final int PORT = 56002;
    private static final int TIMEOUT = 15000;

    public static void main(String[] args) {
        Socket client = null;
        try {
            // 在构造方法中传入 host 和 port
            // client = new Socket("192.168.43.49", PORT);

            // 调用无参构造方法
            client = new Socket();
            // 构造服务器地址结构
            SocketAddress serverAddr = new InetSocketAddress("192.168.0.101", PORT);
            // 连接服务器，超时时间是 15 毫秒
            client.connect(serverAddr, TIMEOUT);

            System.out.println("Client start:" + client.getLocalSocketAddress().toString());

            // 向服务器发送数据
            OutputStream out = new BufferedOutputStream(client.getOutputStream());
            String req = "Hello Server!\n";
            out.write(req.getBytes());
            // 不能忘记 flush 方法的调用
            out.flush();
            System.out.println("Send to server:" + req);

            // 接收服务器的数据
            BufferedInputStream in = new BufferedInputStream(client.getInputStream());
            StringBuilder inMessage = new StringBuilder();
            while(true){
                int c = in.read();
                if (c == -1 || c == '\n')
                    break;
                inMessage.append((char)c);
            }
            System.out.println("Recv from server:" + inMessage.toString());
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
