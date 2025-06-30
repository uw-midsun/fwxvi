#include "i2c_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <json/json.h> // Assuming JSON library for configuration



void I2CManager::loadI2CConfig(std::string &projectName) {
    // Load I2C configuration from a JSON file
    std::string configPath = "configs/" + projectName + "_i2c.json";
    std::ifstream configFile(configPath);
    
    if (!configFile.is_open()) {
        std::cerr << "Failed to open I2C configuration file: " << configPath << std::endl;
        return;
    }
    
    Json::Value root;
    Json::Reader reader;
    
    if (!reader.parse(configFile, root)) {
        std::cerr << "Failed to parse I2C configuration JSON" << std::endl;
        return;
    }
    
    // Clear existing cache
    m_deviceCache.clear();
    m_i2cInfo.clear();
    
    // Process the configuration
    const Json::Value devices = root["devices"];
    for (const auto &device : devices) {
        uint8_t deviceAddr = device["address"].asUInt();
        
        // Add device to the cache
        RegisterMap registers;
        
        const Json::Value regs = device["registers"];
        for (const auto &reg : regs) {
            uint8_t regAddr = reg["address"].asUInt();
            uint8_t regValue = reg["value"].asUInt();
            registers[regAddr] = regValue;
        }
        
        m_deviceCache[deviceAddr] = registers;
        
        // Store I2C data buffers if they exist
        if (device.isMember("buffers")) {
            const Json::Value buffers = device["buffers"];
            std::string deviceKey = "device_" + std::to_string(deviceAddr);
            I2CData dataBuffers;
            
            for (const auto &buffer : buffers) {
                std::vector<uint8_t> dataBuffer;
                for (const auto &value : buffer) {
                    dataBuffer.push_back(value.asUInt());
                }
                dataBuffers.push_back(dataBuffer);
            }
            
            m_i2cInfo[deviceKey] = dataBuffers;
        }
    }
    
    configFile.close();
}

void I2CManager::saveI2CConfig(std::string &projectName) {
    std::string configPath = "configs/" + projectName + "_i2c.json";
    Json::Value root;
    Json::Value devices(Json::arrayValue);
    
    for (const auto &devicePair : m_deviceCache) {
        Json::Value device;
        device["address"] = devicePair.first;
        
        Json::Value registers(Json::arrayValue);
        for (const auto &regPair : devicePair.second) {
            Json::Value reg;
            reg["address"] = regPair.first;
            reg["value"] = regPair.second;
            registers.append(reg);
        }
        device["registers"] = registers;
        
        // Add buffers if they exist
        std::string deviceKey = "device_" + std::to_string(devicePair.first);
        if (m_i2cInfo.find(deviceKey) != m_i2cInfo.end()) {
            Json::Value buffers(Json::arrayValue);
            
            for (const auto &buffer : m_i2cInfo[deviceKey]) {
                Json::Value jsonBuffer(Json::arrayValue);
                for (const auto &value : buffer) {
                    jsonBuffer.append(value);
                }
                buffers.append(jsonBuffer);
            }
            device["buffers"] = buffers;
        }
        
        devices.append(device);
    }
    
    root["devices"] = devices;
    
    std::ofstream configFile(configPath);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open I2C configuration file for writing: " << configPath << std::endl;
        return;
    }
    
    Json::StyledWriter writer;
    configFile << writer.write(root);
    configFile.close();
}

std::string I2CManager::createWriteCommand(uint8_t deviceAddr, uint8_t regAddr, uint8_t value) {
    std::stringstream cmd;
    cmd << "WRITE:" << static_cast<int>(deviceAddr) << ":" << static_cast<int>(regAddr) << ":" << static_cast<int>(value);
    
    if (m_deviceCache.find(deviceAddr) == m_deviceCache.end()) {
        m_deviceCache[deviceAddr] = RegisterMap();
    }
    m_deviceCache[deviceAddr][regAddr] = value;
    
    return cmd.str();
}

std::string I2CManager::createReadCommand(uint8_t deviceAddr, uint8_t regAddr) {
    // Format: READ:deviceAddr:regAddr
    std::stringstream cmd;
    cmd << "READ:" << static_cast<int>(deviceAddr) << ":" << static_cast<int>(regAddr);
    return cmd.str();
}

void I2CManager::updateRegister(uint8_t deviceAddr, uint8_t regAddr, uint8_t value) {
    if (m_deviceCache.find(deviceAddr) == m_deviceCache.end()) {
        m_deviceCache[deviceAddr] = RegisterMap();
    }
    m_deviceCache[deviceAddr][regAddr] = value;
    
    std::string cmd = createWriteCommand(deviceAddr, regAddr, value);
    
    // Configure and send the I2C datagram
    m_i2cDatagram.setPort(Datagram::I2C::Port::I2C1); // Default port, could be parameterized
    m_i2cDatagram.setDeviceAddress(deviceAddr);
    m_i2cDatagram.setRegisterAddress(regAddr);
    m_i2cDatagram.setValue(value);
    m_i2cDatagram.setCommand(Datagram::I2C::Command::WRITE);
    

}

void I2CManager::updateAllRegisters(uint8_t deviceAddr, uint8_t startRegAddr, const std::string& payload) {
    std::vector<uint8_t> values;
    std::stringstream ss(va
      payload);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        try {
            uint8_t value = static_cast<uint8_t>(std::stoi(item, nullptr, 0));
            values.push_back(value);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing payload item: " << item << std::endl;
        }
    }
    
    for (size_t i = 0; i < values.size(); i++) {
        uint8_t regAddr = startRegAddr + static_cast<uint8_t>(i);
        updateRegister(deviceAddr, regAddr, values[i]);
    }
    
    std::string deviceKey = "device_" + std::to_string(deviceAddr);
    if (m_i2cInfo.find(deviceKey) == m_i2cInfo.end()) {
        m_i2cInfo[deviceKey] = I2CData();
    }
    m_i2cInfo[deviceKey].push_back(values);
}

uint8_t I2CManager::readRegister(uint8_t deviceAddr, uint8_t regAddr) {
    if (m_deviceCache.find(deviceAddr) != m_deviceCache.end() &&
        m_deviceCache[deviceAddr].find(regAddr) != m_deviceCache[deviceAddr].end()) {
        return m_deviceCache[deviceAddr][regAddr];
    }
    
    std::string cmd = createReadCommand(deviceAddr, regAddr);
    
    m_i2cDatagram.setPort(Datagram::I2C::Port::I2C1);
    m_i2cDatagram.setRegisterAddress(regAddr);
    m_i2cDatagram.setCommand(Datagram::I2C::Command::READ);
    

    uint8_t defaultValue = 0;
    return defaultValue;
}