package pers.hw.test;

import pers.hw.net.Buffer;
import pers.hw.net.TcpClient;
import pers.hw.net.TcpConnection;

import java.net.InetSocketAddress;
import java.net.SocketAddress;

public class TimeClient extends TcpClient {

    public TimeClient(SocketAddress serverAddr) {
        super(serverAddr);
        setCallbacks(new Callbacks() {
            @Override
            public void onConnection(TcpConnection conn) {
                System.out.println(conn.getLocalAddr().toString() + " -> " +
                        conn.getPeerAddr().toString() + " is " +
                        (conn.isConnected() ? "UP" : "DOWN"));
            }

            @Override
            public void onMessage(TcpConnection conn, Buffer buf) {
                System.out.println("recv" + buf.readableBytes());
                if (buf.readableBytes() >= 4) {
                    int val = buf.readInt32();
                    System.out.println("val: " + val);
                }

            }

            @Override
            public void onWriteComplete(TcpConnection conn) {

            }
        });

    }

    public static void main(String[] args) {
        TimeClient client = new TimeClient(new InetSocketAddress("127.0.0.1", 2037));
        client.start();
        client.waitForFinished();
    }
}
