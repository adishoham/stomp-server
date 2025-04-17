package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import bgu.spl.net.api.MessageEncoderDecoderImpl;
import bgu.spl.net.api.StompMessagingProtocolImpl;


public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final StompMessagingProtocolImpl<T> protocol;
    private final MessageEncoderDecoderImpl<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private int id;
    

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoderImpl<T> reader, StompMessagingProtocolImpl<T> protocol, int id) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        this.id = id;
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { //just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());
            out = new BufferedOutputStream(sock.getOutputStream());

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                protocol.process(encdec.decodeNextByte((byte) read));
            }
            

        } catch (IOException e) {
                    e.printStackTrace();
        } 
    }
    

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    @Override
    public void send(T msg) {
    try {
        byte[]newmsg = encdec.encode((String)msg);
        out.write(newmsg); //sending
        out.flush(); //insuring that the message is deliverd right away
    } catch (IOException e) {
        e.printStackTrace();
    }
}

    public int getId(){
        return this.id;
    }


    public void start(connectionImpl<T> connections){
        protocol.start(id, connections);
    }
    
}