#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>
#include "udpClientConnection.h"  // Ensure this is included if needed
#include "threadpool.h"
Server::Server(int port) : m_running(true) {
    m_serverListening = false;
    m_listeningSocket = -1;
    m_epollFd = -1;
    pthread_mutex_init(&m_mutex, nullptr);
}

Server::~Server() {
    pthread_mutex_destroy(&m_mutex);
}

void Server::listenClients(int port, messageCallback cb) {
    if (m_serverListening) return;

    m_messageCallback = cb;
    m_serverListening = true;

    m_listeningSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_listeningSocket < 0) throw std::runtime_error("UDP socket creation failed");

    int opt = 1;
    setsockopt(m_listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&m_serverAddress, 0, sizeof(m_serverAddress));
    m_serverAddress.sin_family = AF_INET;
    m_serverAddress.sin_port = htons(port);
    m_serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_listeningSocket, (sockaddr*)&m_serverAddress, sizeof(m_serverAddress)) < 0) {
        throw std::runtime_error("bind failed");
    }

    m_epollFd = epoll_create1(0);
    if (m_epollFd < 0) throw std::runtime_error("epoll_create1 failed");

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = m_listeningSocket;

    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_listeningSocket, &ev) < 0) {
        throw std::runtime_error("epoll_ctl failed");
    }

    pthread_create(&m_epollThreadId, nullptr, epollClientsWrapper, this);
}

void* epollClientsWrapper(void* param) {
    Server* server = static_cast<Server*>(param);
    server->epollClientsProcedure();  // lowercase 'server', not Server
    return nullptr;
}

void Server::epollClientsProcedure() {
    char buffer[MAX_CLIENT_READ_SIZE + 1];

    while (m_serverListening) {
        int nfds = epoll_wait(m_epollFd, m_epollEvents, MAX_SERVER_EPOLL_EVENTS, -1);
        if (nfds < 0) {
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (m_epollEvents[i].data.fd == m_listeningSocket) {
                sockaddr_in clientAddr{};
                socklen_t addrLen = sizeof(clientAddr);

                ssize_t bytesReceived = recvfrom(
                    m_listeningSocket,
                    buffer,
                    MAX_CLIENT_READ_SIZE,
                    0,
                    (sockaddr*)&clientAddr,
                    &addrLen
                );

                if (bytesReceived <= 0) continue;

                buffer[bytesReceived] = '\0';
                std::string message(buffer, bytesReceived);

                auto client = new ClientConnection(this);
                client->setClientAddress(clientAddr);
                client->setName(client->getClientAddress());

                pthread_mutex_lock(&m_mutex);
                if (m_connections.find(client->getClientAddress()) == m_connections.end()) {
                    m_connections[client->getClientAddress()] = client;
                } else {
                    delete client;
                    client = m_connections[client->getClientAddress()];
                }
                pthread_mutex_unlock(&m_mutex);

                if (m_messageCallback) {
                    m_messageCallback(this, client, message);
                }
            }
        }
    }
}

void Server::sendMessage(ClientConnection* client, const std::string& message) {
    client->sendMessage(m_listeningSocket, message);
}

void Server::broadcastMessage(const std::string& message) {
    pthread_mutex_lock(&m_mutex);
    for (auto& [_, client] : m_connections) {
        client->sendMessage(m_listeningSocket, message);
    }
    pthread_mutex_unlock(&m_mutex);
}
