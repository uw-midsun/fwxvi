/************************************************************************************************
 * @file   afe_datagram.cc
 *
 * @brief  Source file defining the afe datagram class
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "adbms_afe_datagram.h"
#include "serialization.h"

namespace Datagram{
ADBMS_AFE::ADBMS_AFE(Payload &data){
    m_afeDatagram = data; 
}

std::string ADBMS_AFE::serialize(const CommandCode &commandCode) const{
    std::string serializedData; 

    serializeInteger<uint8_t>(serializedData, m_afeDatagram.index);
    serializeInteger<std::size_t>(serializedData, m_afeDatagram.dev_index); 
    serializeInteger<uint16_t>(serializedData, m_afeDatagram.voltage);
    
    return encodeCommand(commandCode, serializedData);
}

void ADBMS_AFE::deserialize(std::string &afeDatagramPayload){
    std::size_t offset = 0; 

    m_afeDatagram.index = deserializeInteger<uint8_t>(afeDatagramPayload, offset);
    m_afeDatagram.dev_index = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 
    m_afeDatagram.voltage = deserializeInteger<uint16_t>(afeDatagramPayload, offset);
}

/* SETTERS
--------------------------------------------*/
void ADBMS_AFE::setIndex(uint8_t new_index){
    m_afeDatagram.index = new_index; 
}

void ADBMS_AFE::setDeviceIndex(std::size_t new_index){
    m_afeDatagram.dev_index = new_index; 
} 

void ADBMS_AFE::setVoltage(uint16_t voltage){
    m_afeDatagram.voltage = voltage; 
}

/* GETTERS
--------------------------------------------*/
uint8_t ADBMS_AFE::getIndex() const{
    return m_afeDatagram.index;
}

std::size_t ADBMS_AFE::getDevIndex() const{
    return m_afeDatagram.dev_index;
}

uint16_t ADBMS_AFE::getVoltage() const{
    return m_afeDatagram.voltage; 
}

} // namespace Datagram