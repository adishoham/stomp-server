#include <iostream>
#include <thread>
#include <atomic>
#include "Client.h" 
#include <mutex>
#include <condition_variable>



std::mutex mtx;
std::condition_variable cv;

bool running(false);

std::string username, password, channel, filepath, message;
#include <iostream>
#include <sstream>
#include <string>


void handleUserCommands(Client& client) {
    std::string input, command;

    while (running) {
        std::getline(std::cin, input);
        std::istringstream iss(input);
        iss >> command; // Extract only the first word as the command
        if (command == "logout") {
            client.handleLogout();
        } 
        else if (command == "login") {
            std::string hostport, username, password;
            iss >> hostport >> username >> password;
            client.handleLogin(username, password, cv);
        } 
        else if (command == "join") {
            std::string channel;
            iss >> channel;
            client.handleJoin(channel);
        } 
        else if (command == "summary") {
            std::string channel, username, filepath;
            iss >> channel >> username >> filepath;
            client.handleSummary(channel, username, filepath);
        } 
        else if (command == "report") {
            std::string filepath;
            iss >> filepath;
            client.handleReport(filepath);
        } 
        else if (command == "exit") {
            std::string channel;
            iss >> channel;
            client.handleExit(channel);
        } 
        else {
            std::cout << "invalid command" << std::endl;
        }
    }
}


void handleServerCommands(Client& client) {
    while (running) {
            std::unique_lock<std::mutex> lock(mtx);  
            cv.wait(lock, [&client] { return client.isactive(); }); 
		if (client.getConnectionHandler().getLine(message)) {
            if (message.find("CONNECTED") != std::string::npos)
			{
                client.handleConnected();
            }
            else if (message.find("ERROR") != std::string::npos) 
			{
                client.handleError(message);
            }
            else if (message.find("RECEIPT") != std::string::npos) 
{
            size_t firstNewline = message.find("\n"); // Find end of first line
            if (firstNewline != std::string::npos) {
               size_t secondLineStart = firstNewline + 1; // Start of second line
               std::string secondLine = message.substr(secondLineStart); // Extract second line

        // Find position after "reciept- id:"
             size_t colonPos = secondLine.find(":");
             if (colonPos != std::string::npos) {
                std::string receiptId = secondLine.substr(colonPos + 1); // Extract after ':'
                receiptId.erase(0, receiptId.find_first_not_of(" \t")); // Trim leading spaces

                client.handleReciept(receiptId); // Pass only the number
        }
    }
}

            else if (message.find("MESSAGE") != std::string::npos) 
			{
                client.handleMessage(message);
            }
            else if (message.find("DISCONNECT") != std::string::npos) 
			{
                client.handleDisconnect();
            }
        }
    }
}

int main(int argc, char *argv[]) {
	std::string command, hostPort, username, password;
    std::string host;
    int port;
    while (true){
    std::getline(std::cin, command);
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    if (cmd == "login") {
        iss >> hostPort >> username >> password;
        size_t colonPos = hostPort.find(':');
        if (colonPos != std::string::npos) {
            host = hostPort.substr(0, colonPos);
            port = std::stoi(hostPort.substr(colonPos + 1));
        }
        break;
    }
    else {
        std::cout << "you need to log in before doing anything else" << std::endl;
    }
    }
    Client client("stomp.cs.bgu.ac.il", port, username);
    running = true;
    std::thread serverThread([&client] { handleServerCommands(client); });
    client.handleLogin(username, password, cv);
    std::thread userThread([&client] { handleUserCommands(client); });


    userThread.join();
    serverThread.join();
	return 0;
}
