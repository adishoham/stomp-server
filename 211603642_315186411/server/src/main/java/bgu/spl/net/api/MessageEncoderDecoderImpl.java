package bgu.spl.net.api;

import java.nio.charset.StandardCharsets;
import java.util.Arrays;

public class MessageEncoderDecoderImpl<T> implements MessageEncoderDecoder<String> {

    private byte[] bytes = new byte[1 << 10]; // Start with 1KB buffer
    private int len = 0;

    @Override
    public String decodeNextByte(byte nextByte) {
        if (nextByte == '\0') {  
            return popString();
        }
    
        if (nextByte != 0) {  
            pushByte(nextByte);
        }
    
        return null;
    }

    @Override
        public byte[] encode(String message) {
            byte[] bytes = message.getBytes(StandardCharsets.UTF_8);
            return bytes;
        }

    private void pushByte(byte nextByte) {
        if (len >= bytes.length) {
            bytes = Arrays.copyOf(bytes, bytes.length + 512); 
        }
        bytes[len++] = nextByte;
    }

    private String popString() {
        String result = new String(bytes, 0, len, StandardCharsets.UTF_8);
        len = 0;
        result = result.replaceAll("\u0000", "");  
        return result;
    }
    
}
