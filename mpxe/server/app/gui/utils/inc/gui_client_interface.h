
// #include "gui_socket.h"
// #include <functional>
// #include <memory>
// #include <string>
// #include <vector>

// class GuiClientInterface {
//   private:
//     std::unique_ptr<GuiSocket> m_bus; 

//     void handleClientsSnapshot(const GuiSocket::Event &event);
//     void handleClientAdded(const GuiSocket::Event &event);
//     void handleClientRemoved(const GuiSocket::Event &event);
//     void handleClientSwitched(const GuiSocket::Event &event);
//     void handlePayloadSnapshot(const GuiSocket::Event &event);

//   public: 
//     explicit GuiClientInterface(std::unique_ptr<GuiSocket> bus); 
    
//     std::function<void(const std::vector<std::string> &files, const std::string &current)> onClientsSnapshot;
//     std::function<void(const std::string &file)> onClientAdded;
//     std::function<void(const std::string &file)> onClientRemoved;
//     std::function<void(const std::string &current)> onClientSwitched;

//     std::function<void(const std::string &clientPath, const nlohmann::json &data)> onPayloadSnapshot;

//     void subscribeAll();                      
//     void switchClient(const std::string &absPath); 

//     GuiSocket &bus() { return *m_bus; }
// };