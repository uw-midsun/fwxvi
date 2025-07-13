/************************************************************************************************
 * @file   afe_datagram.cc
 *
 * @brief  Source file defining the afe datagram class
 *
 * @date   2025-07-03
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

    serializeInteger<std::size_t>(serializedData, m_afeDatagram.num_cells);
    serializeInteger<std::size_t>(serializedData, m_afeDatagram.num_thermistors);
    serializeInteger<std::size_t>(serializedData, m_afeDatagram.num_devices); 

    serializeInteger<uint16_t>(serializedData, m_afeDatagram.index);
    serializeInteger<std::size_t>(serializedData, m_afeDatagram.dev_index); 
    serializeInteger<uint8_t>(serializedData, m_afeDatagram.voltage);

    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_bitset), AFE_MAX_DEVICES * sizeof(uint16_t));
    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.aux_bitset), AFE_MAX_DEVICES * sizeof(uint16_t));

    // serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_voltages), AFE_MAX_CELLS * sizeof(uint16_t));
    // serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.aux_voltages), AFE_MAX_THERMISTORS * sizeof(uint16_t));
    
    return encodeCommand(commandCode, serializedData);
}

void ADBMS_AFE::deserialize(std::string &afeDatagramPayload){
    std::size_t expected_size = 4 * sizeof(std::size_t) +                       /* `num_cells`, `num_thermistors`, `num_devices`, `dev_index`  */
                                sizeof(uint16_t) + sizeof(uint8_t) +            /*  `index`, `voltage` */
                                AFE_MAX_DEVICES * sizeof(uint16_t) +      /* `cell_bitset` */
                                AFE_MAX_DEVICES * sizeof(uint16_t) +      /* `aux_bitset` */
                                AFE_MAX_CELLS * sizeof(uint16_t) +        /* `cell_voltages` */
                                AFE_MAX_THERMISTORS * sizeof(uint16_t);   /* `aux_voltages` */

    if (afeDatagramPayload.size() < expected_size) {
        throw std::runtime_error("AFE datagram payload size is smaller than expected");
    }

    std::size_t offset = 0; 

    m_afeDatagram.num_cells = deserializeInteger<std::size_t>(afeDatagramPayload, offset);
    m_afeDatagram.num_thermistors = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 
    m_afeDatagram.num_devices = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 

    m_afeDatagram.index = deserializeInteger<uint16_t>(afeDatagramPayload, offset);
    m_afeDatagram.dev_index = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 
    m_afeDatagram.voltage = deserializeInteger<uint8_t>(afeDatagramPayload, offset);
    
    std::memcpy(m_afeDatagram.cell_bitset, afeDatagramPayload.data() + offset, AFE_MAX_DEVICES * sizeof(uint16_t)); 
    offset += AFE_MAX_DEVICES * sizeof(uint16_t);
    std::memcpy(m_afeDatagram.aux_bitset, afeDatagramPayload.data() + offset, AFE_MAX_DEVICES * sizeof(uint16_t));
    offset += AFE_MAX_DEVICES * sizeof(uint16_t);
    
    // std::memcpy(m_afeDatagram.cell_voltages, afeDatagramPayload.data() + offset, AFE_MAX_CELLS * sizeof(uint16_t));
    // offset += AFE_MAX_CELLS * sizeof(uint16_t); 

    // std::memcpy(m_afeDatagram.aux_voltages, afeDatagramPayload.data() + offset, AFE_MAX_THERMISTORS * sizeof(uint16_t));

}

void ADBMS_AFE::setNumCells(std::size_t cells){
    m_afeDatagram.num_cells = cells; 
}

void ADBMS_AFE::setNumThermistors(std::size_t thermistors){
    m_afeDatagram.num_thermistors = thermistors; 
}

void ADBMS_AFE::setNumDevices(std::size_t devices){
    m_afeDatagram.num_devices = devices; 
}

void ADBMS_AFE::setCellBitsetForDevice(std::size_t deviceIndex, uint16_t bitmask){
    m_afeDatagram.cell_bitset[deviceIndex] = bitmask; 
}

void ADBMS_AFE::setAuxBitsetForDevice(std::size_t deviceIndex, uint16_t bitmask){
    m_afeDatagram.aux_bitset[deviceIndex] = bitmask; 
}

void ADBMS_AFE::setCellVoltage(uint16_t index, uint16_t voltage){
    if (index >= m_afeDatagram.num_cells){throw std::out_of_range("invalid cell index");}

    m_afeDatagram.cell_voltages[index] = voltage;
}

void ADBMS_AFE::setAuxVoltage(uint16_t index, uint16_t voltage){
    if (index > m_afeDatagram.num_thermistors){throw std::out_of_range("invalid aux index");}

    m_afeDatagram.aux_voltages[index] = voltage;
}

void ADBMS_AFE::setIndex(uint16_t new_index){
    m_afeDatagram.index = new_index; 
}
void ADBMS_AFE::setDeviceIndex(std::size_t new_index){
    m_afeDatagram.dev_index = new_index; 
} 
void ADBMS_AFE::setVoltage(uint8_t voltage){
    m_afeDatagram.voltage = voltage; 
}

/*GETTERS
--------------------------------------------*/
std::size_t ADBMS_AFE::getNumCells() const{
    return m_afeDatagram.num_cells;      
}

std::size_t ADBMS_AFE::getNumThermistors() const{
    return m_afeDatagram.num_thermistors;
}

std::size_t ADBMS_AFE::getNumDevices() const{
    return m_afeDatagram.num_devices; 
}

uint16_t ADBMS_AFE::getCellBitsetForDevice(std::size_t deviceIndex) const{
    return m_afeDatagram.cell_bitset[deviceIndex];    
}

uint16_t ADBMS_AFE::getAuxBitsetForDevice(std::size_t deviceIndex) const{
    return m_afeDatagram.aux_bitset[deviceIndex];    
}

uint16_t ADBMS_AFE::getCellVoltage(std::size_t index) const{
    return m_afeDatagram.cell_voltages[index]; 
}

uint16_t ADBMS_AFE::getAuxVoltage(std::size_t index) const{
    return m_afeDatagram.aux_voltages[index];
}

uint16_t ADBMS_AFE::getIndex() const{
    return m_afeDatagram.index;
}

std::size_t ADBMS_AFE::getDevIndex() const{
    return m_afeDatagram.dev_index;
}

uint8_t ADBMS_AFE::getVoltage() const{
    return m_afeDatagram.voltage; 
}

}