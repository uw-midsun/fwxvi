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

    serializeInteger<uint8_t>(serializedData, AFE_MAX_CELLS * sizeof(uint16_t)); 
    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_voltages), AFE_MAX_CELLS * sizeof(uint16_t));

    serializeInteger<uint8_t>(serializedData, AFE_MAX_THERMISTORS * sizeof(uint16_t)); 
    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.aux_voltages), AFE_MAX_THERMISTORS * sizeof(uint16_t));

    serializeInteger<uint8_t>(serializedData, AFE_MAX_CELLS * sizeof(uint16_t)); 
    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_discharges), AFE_MAX_CELLS * sizeof(uint16_t));

    return encodeCommand(commandCode, serializedData);
}

void ADBMS_AFE::deserialize(std::string &afeDatagramPayload){
    std::size_t offset = 0; 

    m_afeDatagram.index = deserializeInteger<uint8_t>(afeDatagramPayload, offset);
    m_afeDatagram.dev_index = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 

    deserializeInteger<uint8_t>(afeDatagramPayload, offset); 
    std::memcpy(m_afeDatagram.cell_voltages, afeDatagramPayload.data() + offset, AFE_MAX_CELLS * sizeof(uint16_t));
    offset += AFE_MAX_CELLS * sizeof(uint16_t);

    deserializeInteger<uint8_t>(afeDatagramPayload, offset); 
    std::memcpy(m_afeDatagram.aux_voltages, afeDatagramPayload.data() + offset, AFE_MAX_THERMISTORS * sizeof(uint16_t));
    offset += AFE_MAX_CELLS * sizeof(uint16_t); 

    deserializeInteger<uint8_t>(afeDatagramPayload, offset); 
    std::memcpy(m_afeDatagram.cell_discharges, afeDatagramPayload.data() + offset, AFE_MAX_CELLS * sizeof(uint16_t));
}

/* SETTERS
--------------------------------------------*/
void ADBMS_AFE::setIndex(uint8_t new_index){
    m_afeDatagram.index = new_index; 
}

void ADBMS_AFE::setDeviceIndex(std::size_t new_index){
    m_afeDatagram.dev_index = new_index; 
} 

void ADBMS_AFE::setCellVoltage(uint8_t index, uint16_t voltage) {
    if (index >= AFE_MAX_CELLS){
        std::cout << "Invalid Index" << std::endl;
        return;
    }

    m_afeDatagram.cell_voltages[index] = voltage;
}

void ADBMS_AFE::setAuxVoltage(uint8_t index, uint16_t voltage) {
    if (index >= AFE_MAX_THERMISTORS){
        std::cout << "Invalid Index" << std::endl;
        return;
    }

    m_afeDatagram.aux_voltages[index] = voltage;
}

void ADBMS_AFE::setDeviceCellVoltage(std::size_t dev_index, uint16_t voltage) {
    uint8_t start = dev_index * AFE_MAX_CELLS_PER_DEVICE;
    for (uint8_t i = 0; i < AFE_MAX_CELLS_PER_DEVICE; ++i) {
        setCellVoltage((start + i), voltage);
    }
}

void ADBMS_AFE::setDeviceAuxVoltage(std::size_t dev_index, uint16_t voltage) {
    uint8_t start = dev_index * AFE_MAX_THERMISTORS_PER_DEVICE;
    for (uint8_t i = 0; i < AFE_MAX_THERMISTORS_PER_DEVICE; ++i) {
        setAuxVoltage((start + i), voltage);
    }
}

void ADBMS_AFE::setPackCellVoltage(uint16_t voltage) {
    for (uint8_t i = 0; i < AFE_MAX_CELLS; ++i) {
        setCellVoltage((i), voltage);
    }
}

void ADBMS_AFE::setPackAuxVoltage(uint16_t voltage) {
    for (uint8_t i = 0; i < AFE_MAX_THERMISTORS; ++i) {
        setAuxVoltage((i), voltage);
    }
}

void ADBMS_AFE::setCellDischarge(bool is_discharge, uint8_t cell_index){
    if (cell_index < AFE_MAX_THERMISTORS){
        std::cout << "Invalid Index" << std::endl; 
        return;
    }
    m_afeDatagram.cell_discharges[cell_index] = is_discharge; 
}

/* GETTERS
--------------------------------------------*/
uint8_t ADBMS_AFE::getIndex() const{
    return m_afeDatagram.index;
}

std::size_t ADBMS_AFE::getDevIndex() const{
    return m_afeDatagram.dev_index;
}

uint16_t ADBMS_AFE::getCellVoltage(std::size_t index) const {
    if (index < AFE_MAX_CELLS)
        return m_afeDatagram.cell_voltages[index];

    std::cout << "Invalid Index" << std::endl; 
    return -1;
}

uint16_t ADBMS_AFE::getAuxVoltage(std::size_t index) const {
    if (index < AFE_MAX_THERMISTORS)
        return m_afeDatagram.aux_voltages[index];
    std::cout << "Invalid Index" << std::endl; 
    return -1;
}

bool ADBMS_AFE::getCellDischarge(uint8_t cell_index) const {
    if (cell_index < AFE_MAX_CELLS)
        return m_afeDatagram.aux_voltages[cell_index];
    std::cout << "Invalid Index" << std::endl; 
    return -1; 
}

} // namespace Datagram