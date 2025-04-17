 #include <map>
#include <string>
#include <vector>
#include "Client.h"
#include "ConnectionHandler.h"  
#include "event.h" 
#include <algorithm>
#include <iostream>
   

Client::Client(std::string host, int port, std::string username) 
        : channels(),         //Initializes the map to an empty map
          subscriptions(),
          handler(host, port),  //initializing the handler with default values        
          active(false),
          counter(1),
          user_name(username){}


 void Client::handleLogin(std::string& username, std::string& password, std::condition_variable& cv){
    if(active){ //if the client is allready loged in
        std::cout << "The client is already logged in, log out before trying again" << std::endl;
        return;
    }
    if (handler.connect() == true){ //if the connection is succesfull
        std::this_thread::sleep_for(std::chrono::seconds(3));  // Sleep for 3 seconds so the server could create a new connection
        active = true;
        cv.notify_all();
        std::string msg("CONNECT\naccept version:1.2\nhost:stomp.cs.bgu.ac.il\nlogin:" + username + "\npasscode:" + password);
        handler.sendLine(msg);
        return;
    }
    std::cout << "could not connect to server" << std::endl; //in case the user was not active and could not connect to the server
 }


void Client::handleJoin(std::string& channel){
    if(!active){
        std::cout << "The client is not logged in, can't join a channel" << std::endl;
        return;
    }
    if(subscriptions.count(channel)){
        std::cout << "the client is already subscribed to the channel" << std::endl;
        return;
    }
    std::string msg("SUBSCRIBE\ndestination:/" + channel + "\nid:" + std::to_string(counter) + "\nreceipt:" + std::to_string(counter) + "\n"); //the subscribe message
    subscriptions[std::to_string(counter)] = "subscribe" + channel; //saving the channel with id key and a "subscribe" line so we know its pending
    subscriptions[channel] = std::to_string(counter);//saving the id with channel key
    handler.sendLine(msg); //sending the message
    counter++; 
}

void Client::handleExit(std::string& channel){
    if(!active){
        std::cout << "The client is not logged in, can't exit a channel" << std::endl;
        return;
    }
    if(!subscriptions.count(channel)){
        std::cout << "The client is  already not subscribed to the channel" << std::endl;
        return;
    }
    std::string msg("UNSUBSCRIBE\nid:" + subscriptions[channel]
    + "\nreceipt:" + subscriptions[channel] + "\n"); //the unsubscribe message
    handler.sendLine(msg); //sendindg the message
}





void Client::handleLogout(){
    if (!active){
        std::cout << "The client is not logged in, can't log out" << std::endl;
        return;
    }
    std::string msg("DISCONNECT\nreceipt:0"); //a default number for disconected reciepts
    handler.sendLine(msg);
}

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip>
#include <sstream>

void Client::handleSummary(std::string& channel, std::string& username, std::string& filepath) {
    // Check if channel exists
    auto channelIt = channels.find(channel);
    if (channelIt == channels.end() || channelIt->second.find(username) == channelIt->second.end()) {
        std::cerr << "Error: No events found for user '" << username << "' in channel '" << channel << "'\n";
        return;
    }

    auto& events = channelIt->second.at(username);
    if (events.empty()) {
        std::cerr << "Error: No events available for user '" << username << "' in channel '" << channel << "'\n";
        return;
    }

    // Open file for writing (overwrite mode)
    std::ofstream file(filepath, std::ios::out);
    if (!file) {
        std::cerr << "Unable to open file: " << filepath << std::endl;
        return;
    }

    // Compute statistics
    int totalReports = events.size();
    int activeCount = 0;
    int forcesArrivalCount = 0;

    for (const auto& event : events) {
        auto generalInfo = event.get_general_information();
        if (generalInfo.find("active") != generalInfo.end() && generalInfo["active"] == "true") {
            activeCount++;
        }
        if (generalInfo.find("forces_arrival_at_scene") != generalInfo.end() && generalInfo["forces_arrival_at_scene"] == "true") {
            forcesArrivalCount++;
        }
    }

    // Write header
    file << "Channel <" << channel << ">\n";
    file << "Stats:\n";
    file << "Total: " << totalReports << "\n";
    file << "active: " << activeCount << "\n";
    file << "forces arrival at scene: " << forcesArrivalCount << "\n";
    file << "Event Reports:\n";

    // Sort a copy of events
    std::vector<Event> sortedEvents = events;
    std::sort(sortedEvents.begin(), sortedEvents.end(), [](const Event& a, const Event& b) {
        return (a.get_date_time() != b.get_date_time()) ? (a.get_date_time() < b.get_date_time()) : (a.get_name() < b.get_name());
    });

    // Write event reports
    for (size_t i = 0; i < sortedEvents.size(); ++i) {
        const auto& event = sortedEvents[i];

        // Format summary
        std::string summary = event.get_description();
        summary.erase(std::remove(summary.begin(), summary.end(), '\n'), summary.end()); // Remove line breaks
        summary = summary.substr(0, 27);
        if (event.get_description().size() > 27) summary += "...";

        file << "Report_" << (i + 1) << ":\n";
        file << "city: " << event.get_city() << "\n";
        file << "date time: " << epochToDate(event.get_date_time()) << "\n";
        file << "event name: " << event.get_name() << "\n";
        file << "summary: " << summary << "\n";
    }

    file.close();
    std::cout << "Summary written to '" << filepath << "' successfully.\n";
}

void Client::handleReport(std::string& filepath) {
    // Parse the events file
    names_and_events parsedData = parseEventsFile(filepath);
    std::string channel_name = parsedData.channel_name;
    std::vector<Event> events = parsedData.events;

    if (events.empty()) {
        std::cerr << "Error: No events found in file: " << filepath << std::endl;
        return;
    }

    // Ensure the client is logged in before sending reports
    if (!active) {
        std::cerr << "Error: No user logged in.\n";
        return;
    }

    for (auto& event : events) {
        event.setEventOwnerUser(user_name);
    }

    // Sort events by `date time`
    std::sort(events.begin(), events.end(), [](const Event& a, const Event& b) {
        return a.get_date_time() < b.get_date_time();
    });

    // Send each event immediately
    for (auto& event : events) {
        std::string sendFrame = formatSendFrame(channel_name, user_name, event);
        handler.sendLine(sendFrame); // Send event frame via ConnectionHandler
    }

    std::cout << "Successfully reported " << events.size() << " event(s) to channel '" << channel_name << "'.\n";
}

// Function to format the SEND frame for an event
std::string Client::formatSendFrame(const std::string& channel_name, const std::string& user, const Event& event) {
    std::ostringstream frame;

    frame << "SEND\n";
    frame << "destination: /" << channel_name << "\n";
    frame << "user: " << user << "\n";
    frame << "city: " << event.get_city() << "\n";
    frame << "event name: " << event.get_name() << "\n";
    frame << "date time: " << event.get_date_time() << "\n";
    frame << "general information:\n";

    // Add general information
    for (const auto& info : event.get_general_information()) {
        frame << info.first << ": " << info.second << "\n";
    }

    // Add description
    frame << "description:\n" << event.get_description() << "\n";

    return frame.str();
}

// Function to convert "DD/MM/YYYY_HH:MM" to epoch time
time_t Client::convertToEpoch(const std::string& datetime) {
    std::tm tm = {};
    std::istringstream ss(datetime);
    ss >> std::get_time(&tm, "%d/%m/%Y_%H:%M");
    return mktime(&tm);
}


 void Client::handleConnected() {
     active = true;
     std::cout << "Login successful" << std::endl;
 }


    void Client::handleError(std::string& message){
        handler.close();
        active = false;
        std::vector<std::string> lines = split2lines(message); //making a vector of lines from the message
        std::map<std::string, std::string> map;
        for(size_t i = 1; i < lines.size(); i++){
            std::string key = lines[i].substr(0, lines[i].find(':'));
            std::string value = lines[i].substr(lines[i].find(':') + 1);
            map[key] = value;
        }
        if (map.count("invalid passcode")) {
           std::cout << "Wrong password" << std::endl;
           active = false;
           handler.close();
           return; 
        }
        if(map.count("User already logged in")){
            std::cout << "user already logged in" << std::endl;
            return;
        }
        std::cout << "the user is not a subscriber to the channel, thus cant send it messages" << std::endl; //could be other errors
    }





    void Client::handleReciept(std::string id){
        if (id == "0"){ // the id for disconnecting
            handleDisconnect();
            return;

        }
        if (subscriptions.count(id) && subscriptions[id].length() > 8 && subscriptions[id].substr(0, 9) == "subscribe"){ //means that the pending subscribe request from the server has succeded
            subscriptions[id] = subscriptions[id].substr(9); //update the status of the key to non-pending
            std::cout << "Joined channel " + subscriptions[id] << std::endl;
            return;
        }
        std::cout << "Exited channel " << subscriptions[id] << std::endl; //if we made it till here so the reciept is from an unsubscribe request
        channels.erase(subscriptions[id]); // removing the channel from channels
        subscriptions.erase(subscriptions[id]); //removing the channel from the subscription
        subscriptions.erase(id);
    }


    void Client::handleDisconnect(){
        active = false; // set active to false
        handler.close(); //closing socket
    }



    void Client::handleMessage(std::string& message) {
        Event event(message); // Creating an event from the message
        channels[event.get_channel_name()][event.getEventOwnerUser()].push_back(event); // Saving the event by channel and user
    }

    ConnectionHandler& Client::getConnectionHandler() {
        return handler;
    }
    
    std::string Client::epochToDate(time_t epochTime) {
        std::tm* tm = std::localtime(&epochTime);
        std::ostringstream oss;
        oss << std::put_time(tm, "%d/%m/%Y %H:%M");
        return oss.str();
    }

    std::vector<std::string> Client::split2lines(std::string& str) {
        std::vector<std::string> lines;
        std::stringstream ss(str);
        std::string line;
        while (std::getline(ss, line, '\n')) {
            lines.push_back(line);
        }
        return lines;
    }

    bool Client::isactive() {
        return active;
    }

    
    


