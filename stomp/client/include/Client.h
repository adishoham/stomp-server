#pragma once
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <event.h>
#include <ConnectionHandler.h>
#include <optional>
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <ctime>

using json = nlohmann::json;


class Client{
    private:
     //holds events by key:channel name, val: map of users and their events
    std::map<std::string, std::map<std::string, std::vector<Event>>> channels;
    //holds subscriptions by key:channel name, val: subscription number and vise versa
    std::map<std::string, std::string> subscriptions;  
    ConnectionHandler handler;
    bool active; 
    int counter;
    std::string user_name;


    public:
    Client(std::string host, int port, std::string username);
    void handleLogin(std::string& username, std::string& password, std::condition_variable& cv);
    void handleJoin (std::string& channel);
    void handleExit (std::string& channel);
    void handleLogout();
    void handleSummary(std::string& channel, std::string& username, std::string& filepath);
    void handleReport(std::string& filepath);
    std::string epochToDate(time_t epochTime);
//all of above functions are being called from the user (keyboard/configfile);


    void handleConnected();
    void handleError(std::string& message);
    void handleReciept(std::string id);
    void handleDisconnect();
    void handleMessage(std::string& message);
//these functions are invoked by a message from the server

//a function that recieves a string and splits it into lines.
std::vector<std::string> split2lines(std::string& str);
ConnectionHandler& getConnectionHandler();

std::string formatSendFrame(const std::string& channel_name, const std::string& user, const Event& event);
static time_t convertToEpoch(const std::string& datetime);
bool isactive();
};