package bgu.spl.net.api;

import java.util.HashMap;
import java.util.Map;

public class ClientData {

    private String userName;
    private String password;
    private boolean connected = true;
    private HashMap <Integer, String> subscriptions = new HashMap();
    private int id;



    public ClientData(String userName, String password, int id){
        this.userName = userName;
        this.password = password;
        this.id = id;
    }


    public Map getSubs(){
        return subscriptions;   
    }


    public void disconnect(){
        connected = false;
    }


    public String authenticate(String passcode){
        if (passcode.equals(password)){
            if(!connected){
                connected = true;
                return "connected";
            }
            return  "The client is already logged in, log out before trying again";
        }
        if(!connected){
        return "invalid passcode";
        }
        return "User already logged in";

    }


    public void setId(int id){
        this.id = id;
    }

    public int getId(){
        return this.id;
    }


    public String getUserName(){
        return this.userName;
    }

}
