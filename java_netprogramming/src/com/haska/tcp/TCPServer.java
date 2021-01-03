package com.haska.tcp;

import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;

public class TCPServer {
    private static final int PORT =56002;

    public static void main(String[] args) {
        ServerSocket ss = null;
        try {
            // 创建一个服务器 Socket
            ss = new ServerSocket(PORT);
            // 监听新的连接请求
            Socket conn = ss.accept();
            System.out.println("Accept a new connection:" + conn.getRemoteSocketAddress().toString());

            // 读取客户端数据
            BufferedInputStream in = new BufferedInputStream(conn.getInputStream());
            StringBuilder inMessage = new StringBuilder();
            while(true){
                int c = in.read();
                if (c == -1 || c == '\n')
                    break;
                inMessage.append((char)c);
            }
            System.out.println("Recv from client:" + inMessage.toString());

            // 向客户端发送数据
            String rsp = "Hello Client!\n";
            BufferedOutputStream out = new BufferedOutputStream(conn.getOutputStream());
            out.write(rsp.getBytes());
            out.flush();
            System.out.println("Send to client:" + rsp);
            conn.close();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (ss != null){
                try {
                    ss.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }

        System.out.println("Server exit!");
    }
}
