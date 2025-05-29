#pragma once
#include <string>
#include <atomic>
#include <string>
#include <queue>
#include<functional>
#include <semaphore.h>
#include <pthread.h>
#include <atomic>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

class Client {
public:
    using messageCallback=std::function<void(Client *client, std::string &)>;
    static constexpr size_t MAX_BUFFER=1024;
    Client(const std::string& serverIp, uint16_t port);
    ~Client();
   void setMessageCallback(messageCallback cb);


    bool sendMessage(const std::string& message);
    bool receiveMessage(std::string& message);
private:
    int m_socket;
    struct sockaddr_in m_serverAddr;
    std::string m_host;
    std::atomic<bool> m_isConnected{false};
    std::queue<std::string>m_messageQueue;
    pthread_mutex_t m_mutex;
    sem_t m_messageSemaphore;
    pthread_t m_receiverThreadId;
    pthread_t m_processMessageThreadId;
    messageCallback m_callback;
    static void* receiverThreadWrapper(void* arg);
    static void* processMessageThreadWrapper(void* arg);
};
