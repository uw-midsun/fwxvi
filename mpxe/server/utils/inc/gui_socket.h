#pragma once


#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <netinet/in.h>

#include <nlohmann/json.hpp>

/**
 * @class Write later
 * 
 */
class GuiSocket {
  public:
    /** @brief The event message to be delivered */
    struct Event {
      std::string topic;        /**< The topic of the event, ex: client added, client removed */ 
      nlohmann::json payload;   /**< The data being transmitted */
    };

  /** @brief Callback to handle incoming events */
  using Handler = std::function<void(const Event&)>;

  static const constexpr unsigned int RX_BUFFER_SIZE = 5120; /**< The maximum number of bytes held by the rx buffer */
  
  private:
    int m_socket;                             /**< Socket's file descriptor */
    std::string m_host;                       /**< The server host's address (ie: 127.0.0.1) */
    int m_port;                               /**< The port the socket is connected on */
    struct sockaddr_in m_serverAddress;       /**< The servers address */
    std::atomic<bool> m_isConnected{false};  /**< Flag to indicate connection status */

    std::thread m_rxThread;    /**< Receiving thread */
    std::string m_rxBuffer;    /**< A rx buffer to receive incoming messages (in the case of partial messages)*/

    std::mutex m_handler_mx;                                /**< Mutex for when dealing with handlers variables (below) */
    std::unordered_map<std::string, Handler> m_handlers;    /**< Stores callback functions for specific topics/tasks */
    std::vector<Handler> m_defaultHandlers;                /**< Stores default handlers for cases where topics have not been described */
    
    /**
     * @brief   
     * 
     */
    void rxLoop();

    /**
     * @brief   
     * 
     * @param   data 
     * @param   len 
     */
    void onRawBytes(const char *data, std::size_t len);
    
    /**
     * @brief   
     * 
     */
    void connectOrThrow(); 

    /**
     * @brief   
     * 
     */
    void closeSocket(); 

    /**
     * @brief   Assigns event to approrpriate handler function
     * @param   event The event to be handled
     */
    void dispatch(const Event &event);

    /**
     * @brief   Convert json to a string and send to the server
     * @param   json The json to be sent
     */
    void sendJson(nlohmann::json json);

  public:
    /**
     * @brief   Construct a new Gui Socket object
     * 
     * @param   host 
     * @param   port 
     */
    GuiSocket(const std::string &host, int port);

    /**
     * @brief   Destroy the Gui Socket object
     * 
     */
    ~GuiSocket();

    /**
     * @brief   
     * 
     * @param   topic 
     * @param   payload 
     */
    void publish(const std::string &topic, const nlohmann::json &payload);

    /**
     * @brief   
     * 
     * @param   topic 
     * @param   h 
     */
    void addHandler(const std::string &topic, Handler handler);

    /**
     * @brief   
     * 
     * @param   h 
     */
    void addDefaultHandler(Handler defaultHandler);

    /**
     * @brief   Disconnect from the server
     * @details ...
     */
    void stop();

};
