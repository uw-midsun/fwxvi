// #include "gui_client_interface.h"
// #include <iostream>

// GuiClientInterface::GuiClientInterface(std::unique_ptr<GuiSocket> bus): 
//   m_bus{std::move(bus)}
// {
//   // Register topic handlers
//   m_bus->addHandler("server.clients.snapshot",
//     [this](const GuiSocket::Event &event){ handleClientsSnapshot(event); });

//   m_bus->addHandler("server.clients.added",
//     [this](const GuiSocket::Event &event){ handleClientAdded(event); });

//   m_bus->addHandler("server.clients.removed",
//     [this](const GuiSocket::Event &event){ handleClientRemoved(event); });

//   m_bus->addHandler("server.clients.switched",
//     [this](const GuiSocket::Event &event){ handleClientSwitched(event); });

//   m_bus->addHandler("payload.snapshot",
//     [this](const GuiSocket::Event &event){ handlePayloadSnapshot(event); });

//   m_bus->addDefaultHandler([](const GuiSocket::Event &event){
//     std::cerr << "[GUI] " << event.topic << " " << event.payload.dump() << "\n";
//   });
// }

// void GuiClientInterface::subscribeAll() {
//   m_bus->publish("sub", nlohmann::json{{"topics","*"}});
// }

// void GuiClientInterface::switchClient(const std::string &absPath) {
//   m_bus->publish("server.clients.switch", nlohmann::json{{"file", absPath}});
// }

// void GuiClientInterface::handleClientsSnapshot(const GuiSocket::Event &event) {
//   std::vector<std::string> files;
//   const auto &pay = event.payload;

//   // Support both: array of strings OR array of objects {path,name}
//   if (pay.contains("files") && pay["files"].is_array()) {
//     for (auto &item : pay["files"]) {
//       if (item.is_string()) {
//         files.push_back(item.get<std::string>());
//       } else if (item.is_object()) {
//         if (item.contains("path") && item["path"].is_string()) {
//           files.push_back(item["path"].get<std::string>());
//         }
//       }
//     }
//   }
//   const std::string current = pay.value("current", std::string{});
//   if (onClientsSnapshot) onClientsSnapshot(files, current);
// }

// void GuiClientInterface::handleClientAdded(const GuiSocket::Event &event) {
//   const auto file = event.payload.value("file", std::string{});
//   if (onClientAdded) onClientAdded(file);
// }

// void GuiClientInterface::handleClientRemoved(const GuiSocket::Event &event) {
//   const auto file = event.payload.value("file", std::string{});
//   if (onClientRemoved) onClientRemoved(file);
// }

// void GuiClientInterface::handleClientSwitched(const GuiSocket::Event &event) {
//   const auto cur = event.payload.value("file", std::string{});
//   if (onClientSwitched) onClientSwitched(cur);
// }

// void GuiClientInterface::handlePayloadSnapshot(const GuiSocket::Event &event) {
//   // Expect: { "client": "/abs/file.json", "data": { ... full state ... } }
//   const auto clientPath = event.payload.value("client", std::string{});
//   if (!event.payload.contains("data")) return;

//   const auto &data = event.payload["data"];
//   if (onPayloadSnapshot) onPayloadSnapshot(clientPath, data);
// }