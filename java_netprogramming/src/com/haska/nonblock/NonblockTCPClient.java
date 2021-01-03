package com.haska.nonblock;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;
import java.text.SimpleDateFormat;
import java.util.Date;

public class NonblockTCPClient {
    // 服务器监听的端口
    private final static int PORT = 9082;

    public static void main(String[] args) {
        SocketChannel sock = null;
        try {
            // 创建服务器地址结构
            SocketAddress serverAddr = new InetSocketAddress("127.0.0.1", PORT);
            sock = SocketChannel.open(serverAddr);

            ByteBuffer recvBuff = ByteBuffer.allocate(1024);
            ByteBuffer sendBuff = ByteBuffer.allocate(1024);

            int rquest_times = 10;

            while (true){
                SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                String request = "Request time"+ df.format(new Date());
                sendBuff.putInt(request.length());
                sendBuff.put(request.getBytes());

                sendBuff.flip();
                sock.write(sendBuff);

                System.out.println("Send request to server");
                int bodyLen = -1;
                boolean isFlip = true;
                recvBuff.rewind();

                while (true){
                    int rbytes = sock.read(recvBuff);
                    if (rbytes == -1){
                        sock.close();
                        return;
                    }

                    if (bodyLen == -1){
                        if (rbytes < 4){
                            continue;
                        }
                        recvBuff.flip();

                        bodyLen = recvBuff.getInt();
                        isFlip =false;
                    }

                    if (isFlip ){
                        recvBuff.flip();
                    }
                    if (recvBuff.remaining() < bodyLen){
                        recvBuff.compact();
                        continue;
                    }

                    byte[] body = new byte[bodyLen];
                    recvBuff.get(body);

                    System.out.println("Recv server :" + new String(body, 0, bodyLen));
                    break;
                }

                if (rquest_times-- == 0) {
                    break;
                }

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }

                sendBuff.rewind();
            }
        } catch (IOException e) {
            e.printStackTrace();
            try {
                if (sock != null){
                    sock.close();
                }
            } catch (IOException e1) {
                e1.printStackTrace();
            }
        }
    }
}
