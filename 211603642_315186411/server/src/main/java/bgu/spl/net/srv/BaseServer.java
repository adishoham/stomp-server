package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import bgu.spl.net.api.StompMessagingProtocolImpl;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Supplier;
import bgu.spl.net.api.MessageEncoderDecoderImpl;

public abstract class BaseServer<T> implements Server<T> {

    private final int port;
    private final Supplier<StompMessagingProtocolImpl<T>> protocolFactory;
    private final Supplier <MessageEncoderDecoderImpl<T>> encdecFactory;
    private ServerSocket sock;
    private connectionImpl connections;
    private AtomicInteger counter = new AtomicInteger(0);

    public BaseServer(
            int port,
            Supplier<StompMessagingProtocolImpl<T>> protocolFactory,
            Supplier<MessageEncoderDecoderImpl<T>> encdecFactory) {

        this.port = port;
        this.protocolFactory = protocolFactory;
        this.encdecFactory = encdecFactory;
		this.sock = null;
        this.connections = new connectionImpl();
    }

    @Override
    public void serve() {

        try (ServerSocket serverSock = new ServerSocket(port)) {
			System.out.println("Server started");

            this.sock = serverSock; //just to be able to close

            while (!Thread.currentThread().isInterrupted()) {

                Socket clientSock = serverSock.accept();
                int connectionId = counter.getAndIncrement();
                

                BlockingConnectionHandler<T> handler = new BlockingConnectionHandler<>(        //initiating the handler
                        clientSock,
                        encdecFactory.get(),
                        protocolFactory.get(),
                        connectionId);

                connections.connect(connectionId, handler);           //connecting to the connections map
                handler.start(connections);                           //initiating the protocol
                execute(handler);
            }
        } catch (IOException ex) {
        }

        System.out.println("server closed!!!");
    }

    @Override
    public void close() throws IOException {
		if (sock != null)
			sock.close();
    }

    protected abstract void execute(BlockingConnectionHandler<T>  handler);

}
