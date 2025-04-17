#include "../include/ConnectionHandler.h"

using boost::asio::ip::tcp;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

ConnectionHandler::ConnectionHandler(string host, int port) : host_(host), port_(port), io_service_(),
                                                               socket_(io_service_) {}

ConnectionHandler::~ConnectionHandler() {
    close();
}

bool ConnectionHandler::connect() {
    try {
        boost::asio::io_context io;
        boost::asio::ip::tcp::resolver resolver(io);
        
        auto results = resolver.resolve("127.0.0.1", std::to_string(port_)); // Force IPv4

        if (results.begin() == results.end()) {
            return false;
        }

        boost::asio::ip::tcp::endpoint endpoint = *results.begin();

        boost::system::error_code error;
        socket_.connect(endpoint, error);

        if (error) {
            std::cerr << "Connection failed (Error: " << error.message() << ")" << std::endl;
            return false;
        }


    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::getBytes(char bytes[], unsigned int bytesToRead) {
    size_t tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToRead > tmp) {
            tmp += socket_.read_some(boost::asio::buffer(bytes + tmp, bytesToRead - tmp), error);
        }
        if (error)
            throw boost::system::system_error(error);
    } catch (std::exception &e) {
        std::cerr << "recv failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}

bool ConnectionHandler::getLine(std::string &line) {
    return getFrameAscii(line, '\0');  
}

bool ConnectionHandler::sendBytes(const char bytes[], int bytesToWrite) {
    for (int i = 0; i < bytesToWrite; i++) {
    }

    int tmp = 0;
    boost::system::error_code error;
    try {
        while (!error && bytesToWrite > tmp) {
            tmp += socket_.write_some(boost::asio::buffer(bytes + tmp, bytesToWrite - tmp), error);
        }
        if (error)
            throw boost::system::system_error(error);
    } catch (std::exception &e) {
        std::cerr << "send failed (Error: " << e.what() << ')' << std::endl;
        return false;
    }
    return true;
}


bool ConnectionHandler::sendLine(std::string &line) {
    return sendFrameAscii(line, '\0');  // Send message terminated by null character
}

bool ConnectionHandler::sendFrameAscii(const std::string &frame, char delimiter) {
    std::string fullMessage = frame + '\0';  
    
    
    return sendBytes(fullMessage.c_str(), fullMessage.length());
}


bool ConnectionHandler::getFrameAscii(std::string &frame, char delimiter) {
    char ch;
    frame.clear();
    
    while (true) {
        if (!getBytes(&ch, 1)) return false;
        if (ch == '\0') break;  
        frame.append(1, ch);
    }

    return true;
}

// Close down the connection properly.
void ConnectionHandler::close() {
    try {
        socket_.close();
    } catch (...) {
        std::cout << "closing failed: connection already closed" << std::endl;
    }
}
