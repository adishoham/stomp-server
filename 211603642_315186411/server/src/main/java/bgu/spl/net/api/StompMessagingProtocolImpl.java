package bgu.spl.net.api;

import java.nio.Buffer;
import java.util.HashMap;

import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.connectionImpl;

public class StompMessagingProtocolImpl<T> implements StompMessagingProtocol {
     private boolean shouldTerminate = false;
     private int id = 0;
     private String currentMessage = "";
     private connectionImpl<T> connections = null;
     private HashMap <Integer, String> subs = new HashMap<>();




     @Override
     public void start(int connectionId, Connections connections) {
       this.id = connectionId;
       this.connections = (connectionImpl) connections;

     }

   

    @Override
    public void process(Object message) {
      if (message != null){
       this.currentMessage += (String)message;
       String[] lines = currentMessage.split("\n");//splitting to lines
       String command = lines[0]; //parsing the command
       if (command.equals("CONNECT")){
        handleConnect(lines);
        currentMessage = "";
         return;
       }
       if(command.equals("SUBSCRIBE")){
         handleSubscribe(lines);
         currentMessage = "";
         return;
       }
       if(command.equals("SEND")){
         handleSend(lines, (String)message);
         currentMessage = "";
         return;
       }
       if(command.equals("UNSUBSCRIBE")){
         handleUnsubscribe(lines);
         currentMessage = "";
         return;
       }
       else handleDisconnect(lines);
    currentMessage = "";
  }}

    @Override
    public boolean shouldTerminate() {
       return shouldTerminate;
    }


    public void handleSubscribe(String[] lines){
        HashMap<String, String> map = new HashMap<String, String>();
        for(int i = 1; i < lines.length; i++){
          int index = lines[i].indexOf(":");
          map.put(lines[i].substring(0,index), lines[i].substring(index + 1));
        }
        connections.subscribe(id,Integer.parseInt(map.get("id")), map.get("destination")); //adding the clients to the subscribors list of the channel in connections
        subs.put(Integer.parseInt(map.get("id")), map.get("destination")); //adding the channel to this subscriptions list
        if (map.containsKey("receipt")) {
        connections.send(id, "RECEIPT\nreciept-id:" + map.get("receipt") + "\0");} //sending the client the reciept
    }



    public void handleSend(String[] lines, String message){
        String topic = "";
        int destinationbeginning = message.indexOf("destination: ") + 13;
        int destinationending = destinationbeginning + 1 + message.substring(destinationbeginning+1).indexOf("\n");
        topic = message.substring(destinationbeginning, destinationending);
        int index = message.indexOf("receipt:");
        String receipt = "";
        if(index != -1){
          int endindex = index + 1 + message.substring(index+1).indexOf("\n");
          receipt = message.substring(index, endindex) + "\n";
        }
        if(!connections.isSubscribed(id, topic)){ //if the client is not subscribed to the channel
          String errormsg = ("ERROR\n" + receipt 
          + "message: the client is not subscripted\nthe message:\n"); //conducting an error message
          errormsg += message;
          errormsg += "\0";
          connections.send(id, errormsg); //sending the error message 
          shouldTerminate();
          connections.disconnect(id);
          return;
        }
        message = message.substring(5);
        message += "\0";
        connections.send(topic, message); //sending the message to the topic
        if(index != -1) {
          String str = "RECEIPT\n" + "receipt-id:" + receipt; 
          connections.send(id, str);
        }
    }


    public void handleUnsubscribe(String[] lines){
      HashMap<String, String> map = new HashMap<String, String>();
      for(int i = 1; i < lines.length; i++){
        int index = lines[i].indexOf(":");
        map.put(lines[i].substring(0,index), lines[i].substring(index + 1));
      }
        connections.unSubscribe(id, subs.get(Integer.parseInt(map.get("id"))));//removing client from the subcribors list of the channel in connections
        if(map.containsKey("receipt")){
        connections.send(id, "RECEIPT\nreciept-id:" + map.get("receipt") + "\0");} //sending unsubscribe reciept
        subs.remove(Integer.parseInt(map.get("id"))); //removing the topic from this subscribors list
    }


    public void handleDisconnect(String[] lines){
      HashMap<String, String> map = new HashMap<String, String>();
      for(int i = 1; i < lines.length; i++){
        int index = lines[i].indexOf(":");
        map.put(lines[i].substring(0,index), lines[i].substring(index + 1));
      }
      if(map.containsKey("receipt")){
        String msg = "RECEIPT\n";
        connections.send(id, msg + "receipt-id:" + map.get("receipt")  + "\0");
      }
        connections.disconnect(id);
        shouldTerminate = true;
      
    }


    public void handleConnect(String[] lines){
      HashMap<String, String> map = new HashMap<String, String>();
      for(int i = 1; i < lines.length; i++){
        int index = lines[i].indexOf(":");
        map.put(lines[i].substring(0,index), lines[i].substring(index + 1));
      }

      String msg = connections.authenticateLogin(map.get("login"), map.get("passcode"), id);
      if (msg.equals("connected")){ //if the authentication is succesfull
        connections.send(id, "CONNECTED\nversion:1.2\0");
      }
      else if(msg.equals("The client is already logged in, log out before trying again")){
        System.out.println(msg);
      }
      else {
      connections.send(id, "ERROR\n" + msg + "\0"); //if the authentication failed we forward the failed message from connections
      shouldTerminate = true;
    }

  }


}

