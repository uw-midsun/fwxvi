#include "udpClientConnection.h"
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <arpa/inet.h>  


ClientConnection::ClientConnection(Server* server)
    : m_server(server) {
    std::memset(&m_clientAddress, 0, sizeof(m_clientAddress));
}

ClientConnection::~ClientConnection() {}

void ClientConnection::setClientAddress(sockaddr_in& addr) {
    m_clientAddress = addr;
}

const sockaddr_in& ClientConnection::getSockAddr() const {
    return m_clientAddress;
}

std::string ClientConnection::getClientAddress() const {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(m_clientAddress.sin_addr), ip, INET_ADDRSTRLEN);
    return std::string(ip) + ":" + std::to_string(ntohs(m_clientAddress.sin_port));
}

std::string ClientConnection::getClientName() const {
    return m_clientName;
}

void ClientConnection::setName(const std::string& name) {
    m_clientName = name;
}

void ClientConnection::sendMessage(int socketFd, const std::string& message) const {
    sendto(socketFd, message.c_str(), message.size(), 0, (const sockaddr*)&m_clientAddress, sizeof(m_clientAddress));
}
