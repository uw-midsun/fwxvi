#pragma once
#include <string>

class Client {
public:
    Client(const std::string& serverIP, uint16_t port);
    ~Client();

    bool sendMessage(const std::string& message);
    bool receiveMessage(std::string& message);
private:
    int m_socket;
    struct sockaddr_in m_serverAddr;
};
