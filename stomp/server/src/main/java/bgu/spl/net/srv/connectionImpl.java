package bgu.spl.net.srv;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.api.ClientData;
import bgu.spl.net.api.MessageEncoderDecoder;


public class connectionImpl<T> implements Connections{
    private ConcurrentHashMap<Integer, ConnectionHandler<T>> handlers  = new ConcurrentHashMap();
    private ConcurrentHashMap<String, ConcurrentHashMap<Integer, ConnectionHandler<T>>> channels = new ConcurrentHashMap();
    private ConcurrentHashMap <String, ClientData> users = new ConcurrentHashMap<>();
    private int counter = 0;
    private ConcurrentHashMap <String, ConcurrentHashMap<Integer, Integer>> subscribors = new ConcurrentHashMap();



    

    

    @Override
    public void send(String channel, Object msg) {
        ConcurrentHashMap<Integer, ConnectionHandler<T>> subscription  = channels.get(channel);
        if (subscription != null){
            synchronized (subscription){
            for(Integer key: subscription.keySet()){ //sending the message to all subscribers to the channel
            String newmessage = "MESSAGE\n" + "subscription:" + Integer.toString((subscribors.get(channel)).get(key))
            + "\n" + "message-id:" + counter + "\n";
            newmessage += (String)msg;
                subscription.get(key).send((T) newmessage);
            }
        }
            counter++;
        } 
       
    }

    @Override
    public void disconnect(int connectionId) { 
       handlers.remove(connectionId);         //removes the client's connection handler from the map.
       for(String key : channels.keySet()){ //cancell all client's subscriptions
        unSubscribe(connectionId, key);
    }
        users.get(Integer.toString(connectionId)).disconnect();
       }
    

    @Override
    public boolean send(int connectionId, Object msg) {
        handlers.get(connectionId).send((T) msg);
        return true;
    }

    public void connect(int connectionid, ConnectionHandler handler){
        handlers.put(connectionid, handler);
    }


    public void subscribe(int id, int subscriptionid, String channel){
        channels.computeIfAbsent(channel, k -> new ConcurrentHashMap<>()) // If the channel doesn't exist, create a new map
            .put(id, handlers.get(id)); // Add the connectionId to the channel     
        subscribors.computeIfAbsent(channel, k -> new ConcurrentHashMap<>()).put(id, subscriptionid);
    }
    


    public void unSubscribe(int connectionId, String channel){
        synchronized(channels.get(channel)){
        channels.get(channel).remove(connectionId);
        }
        subscribors.get(channel).remove(connectionId);
    }


    public boolean isSubscribed(int id, String channel){
        return channels.containsKey(channel) //checks if the channel exist
         && channels.get(channel).containsKey(id); //if so, checks if theclient is subscribed to it
    }




    public String authenticateLogin(String username, String passcode, int id){
        if(!users.containsKey(username)){ //if its a new client
            ClientData user = new ClientData(username, passcode, id);
            users.put(username, user); //so we could access it by username
            users.put(Integer.toString(id), user); //so we could access it by id
            return "connected";
        }
        ClientData user = users.get(username);
        String str = user.authenticate(passcode);
        if (str.equals("connected")){ //if it is a loging in user
            users.remove(Integer.toString(user.getId())); 
            users.get(username).setId(id); //changing users id after reconnecting
            users.put(Integer.toString(id), user); //so we have acssess to it by its new id
        }
        return str;

    }


    public String getUsername(String id){
        return users.get(id).getUserName();
    }
   
}