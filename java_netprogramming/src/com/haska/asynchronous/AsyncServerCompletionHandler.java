package com.haska.asynchronous;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousServerSocketChannel;
import java.nio.channels.AsynchronousSocketChannel;
import java.nio.channels.CompletionHandler;

public class AsyncServerCompletionHandler {
    private static final int PORT =56002;
    private AsynchronousServerSocketChannel server = null;
    private static final int ASYNC_READ = 1;
    private static final int ASYNC_WRITE = 2;
    private static final int ASYNC_ACCEPT = 3;
    private static final int ASYNC_CONNECT = 4;

    private static class AsyncIOOP {
        private int op_type;
        private ByteBuffer read_buffer;
        private AsynchronousSocketChannel client;

        public int getOp_type() {
            return op_type;
        }

        public void setOp_type(int op_type) {
            this.op_type = op_type;
        }

        public ByteBuffer getRead_buffer() {
            return read_buffer;
        }

        public void setRead_buffer(ByteBuffer read_buffer) {
            this.read_buffer = read_buffer;
        }

        public AsynchronousSocketChannel getClient() {
            return client;
        }

        public void setClient(AsynchronousSocketChannel client) {
            this.client = client;
        }

        public AsyncIOOP(int op) {
            this(op, null, null);
        }
        public AsyncIOOP(int op, ByteBuffer b) {
            this(op, b, null);
        }
        public AsyncIOOP(int op, ByteBuffer b, AsynchronousSocketChannel ch) {
            this.op_type = op;
            this.read_buffer = b;
            this.client = ch;
        }
    }
    private static class AsyncIOOPCompletionHandler implements CompletionHandler<Integer, AsyncIOOP>
    {
        private AsyncServerCompletionHandler server;

        public AsyncIOOPCompletionHandler(AsyncServerCompletionHandler server){
            this.server = server;
        }
        @Override
        public void completed(Integer result, AsyncIOOP attachment) {
            if (attachment.op_type == ASYNC_READ) {
                server.async_write(attachment.getClient(), "Hello Client!");

                ByteBuffer inBuffer = attachment.getRead_buffer();
                System.out.println("Recv message from client:" + new String(inBuffer.array()).trim());

                server.async_read(attachment.getClient());
            } else if (attachment.op_type == ASYNC_WRITE) {

            }
        }

        @Override
        public void failed(Throwable exc, AsyncIOOP attachment) {
            try {
                attachment.getClient().close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    private static class AsyncAcceptCompletionHandler implements CompletionHandler<AsynchronousSocketChannel, AsyncIOOP>
    {
        private AsyncServerCompletionHandler server;

        public AsyncAcceptCompletionHandler(AsyncServerCompletionHandler server) {
            this.server = server;
        }

        @Override
        public void completed(AsynchronousSocketChannel result, AsyncIOOP attachment) {
            if (attachment.op_type == ASYNC_ACCEPT) {
                server.accept_new_client();

                if (result != null && result.isOpen()) {
                    server.async_read(result);
                }
            }
        }

        @Override
        public void failed(Throwable exc, AsyncIOOP attachment) {

        }
    }

    public void start() {
        try {
            server = AsynchronousServerSocketChannel.open();
            server.bind(new InetSocketAddress("127.0.0.1", PORT));
            accept_new_client();
        } catch (Exception e) {
            e.printStackTrace();
            stop();
        }
    }

    public void accept_new_client() {
        server.accept(new AsyncIOOP(ASYNC_ACCEPT), new AsyncAcceptCompletionHandler(this));
    }

    public void async_read(AsynchronousSocketChannel client){
        ByteBuffer inBuffer = ByteBuffer.allocate(128);
        AsyncIOOP ioop = new AsyncIOOP(ASYNC_READ, inBuffer, client);
        client.read(inBuffer, ioop, new AsyncIOOPCompletionHandler(this));
    }
    public void async_write(AsynchronousSocketChannel client, String message){
        ByteBuffer outBuffer = ByteBuffer.wrap(message.getBytes());
        AsyncIOOP ioop = new AsyncIOOP(ASYNC_WRITE, outBuffer, client);
        client.write(outBuffer, ioop, new AsyncIOOPCompletionHandler(this));
    }
    public void stop(){
        if (server != null){
            try {
                server.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    public static void main(String[] args) {
        AsyncServerCompletionHandler server = new AsyncServerCompletionHandler();
        server.start();

        try {
            Thread.sleep(1000*1000);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
