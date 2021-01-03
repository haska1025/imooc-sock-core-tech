package com.haska.nonblock;

import java.io.*;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.*;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Iterator;
import java.util.Set;

public class NonblockTCPServer {
    // 服务器监听的端口
    private final static int PORT = 9082;
    private Selector selector = null;
    private ServerSocketChannel serverChannel = null;

    private static class Client{
        // 接收 buffer 长度
        private final static int RECV_BUF_LEN = 1024;
        // 接收buffer 声明
        private ByteBuffer recvBuff = null;
        // 发送 buffer 长度
        private static final int SEND_BUFF_LEN = 1024;
        // 发送 buffer 声明
        private ByteBuffer sendBuff = null;
        // the Selector
        private Selector selector = null;
        // SocketChannel 引用声明，表示一个连接
        private SocketChannel socketChannel = null;
        private SelectionKey sk_ = null;
        private boolean canSend = true;

        public Client(Selector selector, SocketChannel newSock){
            this.selector = selector;
            this.socketChannel = newSock;
            this.recvBuff = ByteBuffer.allocate(RECV_BUF_LEN);
            this.sendBuff = ByteBuffer.allocate(SEND_BUFF_LEN);
            this.register(SelectionKey.OP_READ);
        }

        private void register(int op){
            try {
                if (sk_ == null){
                    sk_ = this.socketChannel.register(selector, op, this);
                } else {
                    sk_.interestOps(op | sk_.interestOps());
                }
            } catch (ClosedChannelException e) {
                e.printStackTrace();
            }
        }

        public void cancelEvent(int ops){
            if (sk_ == null)
                return;

            sk_.interestOps(sk_.interestOps() & (~ops));
        }

        public void sendData() {
            try {
                int totalSendBytes = 0;
                String resp = null;
                if (canSend){
                    //设置日期格式
                    SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                    resp = "The server time : " + df.format(new Date());
                    sendBuff.putInt(resp.length());
                    sendBuff.put(resp.getBytes());
                    totalSendBytes = resp.length() + 4;

                    sendBuff.flip();
                }else {
                    totalSendBytes = sendBuff.remaining();
                }

                int sbytes = this.socketChannel.write(sendBuff);
                System.out.println("Send to client about message :" + resp);
                if (sbytes < totalSendBytes) {
                    this.register(SelectionKey.OP_WRITE);
                    canSend = false;
                } else {
                    if (!canSend){
                        canSend = true;
                    }
                    sendBuff.rewind();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public int recvData(){
            try {
                int recvBytes = this.socketChannel.read(this.recvBuff);
                if (recvBytes < 0){
                    System.out.println("Meet error or the end of stream");
                    close();
                    return -1;
                }else if (recvBytes == 0){
                    return 0;// eagain
                }

                this.recvBuff.flip();
                while (this.recvBuff.remaining() > 0) {
                    // Incomplete message header
                    if (this.recvBuff.remaining() < 4) {
                        break;
                    }

                    int bodyLen = this.recvBuff.getInt();
                    if (bodyLen > this.recvBuff.remaining()) {
                        // Incomplete message body
                        break;
                    }

                    byte[] body = new byte[bodyLen];
                    this.recvBuff.get(body, 0, bodyLen);
                    System.out.println("Recv message from client: " +
                            new String(body, 0, bodyLen));
                }
                // flip recv buffer
                this.recvBuff.compact();
                return 0;
            } catch (IOException e) {
                e.printStackTrace();
                close();
            }
            return -1;
        }

        public void close(){
            try {
                cancelEvent(SelectionKey.OP_WRITE | SelectionKey.OP_READ);
                if (this.socketChannel != null){
                    this.socketChannel.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void start(){
        try {
            selector = Selector.open();

            serverChannel = ServerSocketChannel.open();
            // 绑定监听的 socket 地址,监听 any_addr
            serverChannel.socket().bind(new InetSocketAddress(PORT));
            // 设置 SO_REUSEADDR 选项，作为服务器，这是基本的要求
            serverChannel.socket().setReuseAddress(true);
            // 设置非阻塞模式，作为服务器，也是基本要求
            serverChannel.configureBlocking(false);
            // 注册 accept 事件
            serverChannel.register(selector, SelectionKey.OP_ACCEPT, serverChannel);
        } catch (IOException e) {
            e.printStackTrace();
            stop();
        }
    }

    public void process() {
        try {
            while (true) {
                int readyChannels = selector.select();
                if (readyChannels == 0) {
                    System.out.println("No socket has i/o events");
                    continue;
                }

                Set<SelectionKey> selectedKeys = selector.selectedKeys();
                Iterator<SelectionKey> keyIterator = selectedKeys.iterator();

                while (keyIterator.hasNext()) {
                    SelectionKey key = keyIterator.next();
                    if (key != null) {
                        if (key.isAcceptable()) {
                            // a connection was accepted by a ServerSocketChannel.
                            ServerSocketChannel ssc = (ServerSocketChannel) key.attachment();
                            SocketChannel newSock = ssc.accept();
                            newSock.configureBlocking(false);
                            Client client = new Client(selector, newSock);
                        } else if (key.isConnectable()) {
                            // a connection was established with a remote server.
                        } else if (key.isReadable()) {
                            // a channel is ready for reading
                            Client client = (Client) key.attachment();
                            int rc = client.recvData();
                            if (rc == 0) {
                                client.sendData();
                            }
                        } else if (key.isWritable()) {
                            // a channel is ready for writing
                            Client client = (Client) key.attachment();
                            client.cancelEvent(SelectionKey.OP_WRITE);
                            client.sendData();
                        }
                    }
                    keyIterator.remove();
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void stop(){
        try {
            if (serverChannel != null){
                serverChannel.close();
                serverChannel = null;
            }
            if (selector != null) {
                selector.close();
                selector = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public static void main(String[] args) {
        NonblockTCPServer tcp = new NonblockTCPServer();
        tcp.start();
        tcp.process();
    }
}
