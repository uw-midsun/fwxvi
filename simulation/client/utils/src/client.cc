#include "client.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>


Client::Client(const std::string& serverIP, uint16_t port){
    m_socket=socket(AF_INET, SOCK_DGRAM, 0);
    if(m_socket<0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    m_serverAddr.sin_family=AF_INET;
    inet_pton(AF_INET, serverIP.c_str(), &m_serverAddr.sin_addr);
    pthread_mutex_init(&m_mutex, nullptr);
    sem_init(&m_messageSemaphore, 0, 0);
    m_isConnected=true;
    pthread_create(&m_receiverThreadId, nullptr, receiverThreadWrapper, this);
    pthread_create(&m_processMessageThreadId, nullptr, processMessageThreadWrapper, this);
}


Client::~Client(){
    m_isConnected=false;
    close(m_socket);
    pthread_cancel(m_receiverThreadId);
    pthread_cancel(m_processMessageThreadId);
    pthread_join(m_receiverThreadId, nullptr);
    pthread_join(m_processMessageThreadId, nullptr);
    pthread_mutex_destroy(&m_mutex);
    sem_destroy(&m_messageSemaphore);
}

void Client::setMessageCallback(messageCallback cb){
    m_callback=cb;
}



bool Client::sendMessage(const std::string& message){
    ssize_t sent=sendto(m_socket, message.c_str(), message.length(), 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    return sent>=0;
}


void* Client::receiverThreadWrapper(void* arg){
    Client* client=static_cast<Client*>(arg);
    char buffer[MAX_BUFFER];
    while(client->m_isConnected){
        sockaddr_in fromAddr{};
        socklen_t addrLen=sizeof(fromAddr);
        ssize_t len=recvfrom(client->m_socket, buffer, MAX_BUFFER, 0, (sockaddr*)&fromAddr, &addrLen);
        if (len>0){
            std::string msg(buffer, len);
            pthread_mutex_lock(&client->m_mutex);
            client->m_messageQueue.push(msg);
            pthread_mutex_unlock(&client->m_mutex);
            sem_post(&client->m_messageSemaphore);
        }
    }
    return nullptr;
}

void* Client::processMessageThreadWrapper(void* arg){
    Client* client=static_cast<Client*>(arg);
    while (client->m_isConnected){
        sem_wait(&client->m_messageSemaphore);
        std::string msg;
        pthread_mutex_lock(&client->m_mutex);
        //check if message queue empty
        if(!client->m_messageQueue.empty()){
            msg=client->m_messageQueue.front();
            client->m_messageQueue.pop();
        }
        pthread_mutex_unlock(&client->m_mutex);
        if (!msg.empty() && client->m_callback){
            client->m_callback(client, msg);
        }
    }
    return nullptr;
}