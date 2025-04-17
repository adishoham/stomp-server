package bgu.spl.net.impl.stomp;

import java.util.function.Supplier;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessageEncoderDecoderImpl;
import bgu.spl.net.api.StompMessagingProtocolImpl;
import bgu.spl.net.srv.Server;

public class StompServer {

    public static  void main(String[] args) {
        int port = Integer.parseInt(args[0]);
        Supplier<StompMessagingProtocolImpl<String>> protocolFactory = () -> new StompMessagingProtocolImpl<>();
        Supplier<MessageEncoderDecoderImpl<String>> encoderDecoderFactory = MessageEncoderDecoderImpl::new;
        if(args[1].equals("Reactor")){
           Server.reactor(
               Runtime.getRuntime().availableProcessors(), 
               port, 
               protocolFactory,
               encoderDecoderFactory).serve();
        }
        else{
            Server.threadPerClient(
            port,
            protocolFactory,
            encoderDecoderFactory).serve();
        } 
    }
}
