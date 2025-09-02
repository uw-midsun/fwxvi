#include "gui_socket.h"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

/* Helper function to remove whitespaces */
static inline std::string trimCopy(const std::string &str) {
  std::size_t a = str.find_first_not_of(" \t\r\n");
  std::size_t b = str.find_last_not_of(" \t\r\n");
  if (a == std::string::npos) { return {}; }

  return str.substr(a, b - a + 1);
}

void GuiSocket::connectOrThrow() {
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket < 0) {
    throw std::runtime_error("Error: socket() failed"); 
  }

  m_serverAddress = {}; 
  m_serverAddress.sin_family = AF_INET; 
  m_serverAddress.sin_port = htons(static_cast<uint16_t>(m_port)); 

  if (inet_pton(AF_INET, m_host.c_str(), &m_serverAddress.sin_addr) <= 0) {
    close(m_socket);
    m_socket = -1; 
    throw std::runtime_error("Error converting IPv4 host address to binary form");
  }

  if (connect(m_socket, reinterpret_cast<sockaddr*>(&m_serverAddress), sizeof(m_serverAddress)) < 0) {
    close(m_socket);
    m_socket = -1;
    throw std::runtime_error("Error connecting socket");
  }
}

void GuiSocket::closeSocket() {
  if (m_socket >= 0) {
    shutdown(m_socket, SHUT_RDWR); 
    close(m_socket); 
    m_socket = -1; 
  }
}

GuiSocket::GuiSocket(const std::string &host, int port) {
  connectOrThrow(); 
  m_isConnected = true; 
  m_rxThread = std::thread([this] {
    rxLoop(); 
  });
}

GuiSocket::~GuiSocket() { 
  stop(); 
}

void GuiSocket::stop() {
  m_isConnected = false;
  closeSocket(); 
}

void GuiSocket::publish(const std::string &topic, const nlohmann::json &payload) {
  nlohmann::json json; 
  json["topic"] = topic; 
  json["payload"] = payload; 
  sendJson(json);  
} 


void GuiSocket::dispatch(const Event &event) {
  Handler handler; 
  {
    std::lock_guard<std::mutex> lk(m_handler_mx); 
    std::unordered_map<std::string, Handler>::const_iterator it = m_handlers.find(event.topic);
    if (it != m_handlers.end()) {
      handler = it->second; 
    }
  }
  if (handler) {
    handler(event); 
    return;
  }

  /* If handler does not exist for the specified topic, let one of the default handlers "catch" it */
  std::vector<Handler> defaultHandlers;
  {
    std::lock_guard<std::mutex> lk(m_handler_mx); 
    defaultHandlers = m_defaultHandlers; 
  }
  for (Handler defaultHandler : defaultHandlers) {
    defaultHandler(event); 
  }
}

void GuiSocket::sendJson(nlohmann::json json) {
  const std::string line = json.dump() + "\n"; 
  const char *buf = line.data(); 
  std::size_t total = 0;
  
  while (total < line.size()) {
    ssize_t n = send(m_socket, buf + total, line.size() - total, 0); 
    if (n < 0) {
      if (errno == EINTR) { 
        continue; 
      }
      std::cerr << "Send error (GuiSocket)" << std::strerror(errno) << std::endl; 
      break;  
    }
    total += static_cast<std::size_t>(n); 
  }
}

void GuiSocket::rxLoop() { 
  constexpr std::size_t kBufSize = RX_BUFFER_SIZE; 
  char buf[kBufSize]; 

  while (m_isConnected) {
    fd_set rset; 
    FD_ZERO(&rset); 
    timeval tv{0, 200000};

    int rc = select(m_socket + 1, &rset, nullptr, nullptr, &tv); 
    if (rc < 0) { 
      if (errno == EINTR) { 
        continue; 
      }
      std::cerr << "Send error (GuiSocket)" << std::strerror(errno) << std::endl; 
      break;  
    }
    if (rc == 0) {
      continue;
    }

    if (FD_ISSET(m_socket, &rset)) {
      ssize_t n = recv(m_socket, buf, kBufSize, 0);
      if (n == 0) {
        std::cerr << "Server closed connection (GuiSocket)" << std::endl; 
        break;
      }
      if (n < 0) { 
        if (errno == EINTR) {
          continue;
        }
        std::cerr << "recv error (GuiSocket)" << std::strerror(errno) << std::endl;
        break;
      }
      onRawBytes(buf, static_cast<std::size_t>(n)); 
    }
  }

  closeSocket(); 
}

void GuiSocket::onRawBytes(const char *data, std::size_t len) { 
  m_rxBuffer.append(data, len); 

  std::size_t pos = 0; 
  while ((pos = m_rxBuffer.find("\n")) != std::string::npos) { 
    std::string line = m_rxBuffer.substr(0, pos); 
    m_rxBuffer.erase(0, pos+1);

    line = trimCopy(line); 
    if (line.empty()) {
      continue; 
    }

    nlohmann::json json; 
    try {
      json = nlohmann::json::parse(line); 
    }
    catch (...) {
      std::cerr << "JSON Parse error (GuiSocket) : " << line << std::endl; 
      continue;
    }

    Event event; 
    event.topic = json.value("topic", "");
    event.payload = json.contains("payload") ? json["payload"] : nlohmann::json::object(); 
    if (event.topic.empty()) {
      continue; 
    }
    dispatch(event); 
  }
}

void GuiSocket::addHandler(const std::string &topic, Handler handler) {
  std::lock_guard<std::mutex> lk(m_handler_mx); 
  m_handlers.insert({topic, std::move(handler)}); 
}

void GuiSocket::addDefaultHandler(Handler default_handler) {
  std::lock_guard<std::mutex> lk(m_handler_mx);
  m_defaultHandlers.push_back(std::move(default_handler));
}
