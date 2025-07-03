

/* Standard library Headers */
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

/* Inter-component Headers */

/* Intra-component Headers */
#include "ltc_afe_datagram.h"
#include "serialization.h"

namespace Datagram{
LTC_AFE::LTC_AFE(Payload &data){
    m_afeDatagram = data; 
}

std::string LTC_AFE::serialize(const CommandCode &commandCode) const{
    std::string serializedData; 

    serializeInteger<std::size_t>(serializedData, m_afeDatagram.num_cells);
    serializeInteger<std::size_t>(serializedData, m_afeDatagram.num_thermistors);
    serializeInteger<std::size_t>(serializedData, m_afeDatagram.num_devices); 

    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_bitset), LTC_AFE_MAX_DEVICES * sizeof(uint16_t));
    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.aux_bitset), LTC_AFE_MAX_DEVICES * sizeof(uint16_t));

    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.cell_voltages), LTC_AFE_MAX_CELLS * sizeof(uint16_t));
    serializedData.append(reinterpret_cast<const char *>(m_afeDatagram.aux_voltages), LTC_AFE_MAX_THERMISTORS * sizeof(uint16_t));

    return encodeCommand(commandCode, serializedData);
}

void LTC_AFE::deserialize(std::string &afeDatagramPayload){
    std::size_t expected_size = 3 * sizeof(std::size_t) +                     /* num_cells, num_thermistors, num_devices  */
                                LTC_AFE_MAX_DEVICES * sizeof(uint16_t) +      /* cell_bitset */
                                LTC_AFE_MAX_DEVICES * sizeof(uint16_t) +      /* aux_bitset */
                                LTC_AFE_MAX_CELLS * sizeof(uint16_t) +        /* cell_voltages */
                                LTC_AFE_MAX_THERMISTORS * sizeof(uint16_t);   /* aux_voltages */

    if (afeDatagramPayload.size() < expected_size) {
        throw std::runtime_error("AFE datagram payload size is smaller than expected");
    }

    std::size_t offset = 0; 

    m_afeDatagram.num_cells = deserializeInteger<std::size_t>(afeDatagramPayload, offset);
    m_afeDatagram.num_thermistors = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 
    m_afeDatagram.num_devices = deserializeInteger<std::size_t>(afeDatagramPayload, offset); 
    
    std::memcpy(m_afeDatagram.cell_bitset, afeDatagramPayload.data() + offset, LTC_AFE_MAX_DEVICES * sizeof(uint16_t)); 
    offset += LTC_AFE_MAX_DEVICES * sizeof(uint16_t);
    std::memcpy(m_afeDatagram.aux_bitset, afeDatagramPayload.data() + offset, LTC_AFE_MAX_DEVICES * sizeof(uint16_t));
    offset += LTC_AFE_MAX_DEVICES * sizeof(uint16_t);
    
    std::memcpy(m_afeDatagram.cell_voltages, afeDatagramPayload.data() + offset, LTC_AFE_MAX_CELLS * sizeof(uint16_t));
    offset += LTC_AFE_MAX_CELLS * sizeof(uint16_t); 

    std::memcpy(m_afeDatagram.aux_voltages, afeDatagramPayload.data() + offset, LTC_AFE_MAX_THERMISTORS * sizeof(uint16_t));
}

void LTC_AFE::setNumCells(std::size_t cells){
    m_afeDatagram.num_cells = cells; 
}

void LTC_AFE::setNumThermistors(std::size_t thermistors){
    m_afeDatagram.num_thermistors = thermistors; 
}

void LTC_AFE::setNumDevices(std::size_t devices){
    m_afeDatagram.num_devices = devices; 
}

void LTC_AFE::setCellBitsetForDevice(std::size_t deviceIndex, uint16_t bitmask){
    m_afeDatagram.cell_bitset[deviceIndex] = bitmask; 
}

void LTC_AFE::setAuxBitsetForDevice(std::size_t deviceIndex, uint16_t bitmask){
    m_afeDatagram.aux_bitset[deviceIndex] = bitmask; 
}

void LTC_AFE::setCellVoltage(uint16_t index, uint16_t voltage){
    if (index >= m_afeDatagram.num_cells){throw std::out_of_range("invalid cell index");}

    std::size_t deviceIndex = index / LTC_AFE_MAX_CELLS_PER_DEVICE;
    std::size_t deviceCell  = index % LTC_AFE_MAX_CELLS_PER_DEVICE;

    if ((m_afeDatagram.cell_bitset[deviceIndex] >> deviceCell) & 0x1) {
        m_afeDatagram.cell_voltages[index] = voltage;
    }
}

void LTC_AFE::setAuxVoltage(uint16_t index, uint16_t voltage){
    if (index > m_afeDatagram.num_thermistors){throw std::out_of_range("invalid aux index");}

    std::size_t deviceIndex = index / LTC_AFE_MAX_THERMISTORS_PER_DEVICE;
    std::size_t deviceAux  = index % LTC_AFE_MAX_THERMISTORS_PER_DEVICE;

    if ((m_afeDatagram.aux_bitset[deviceIndex] >> deviceAux) & 0x1) {
        m_afeDatagram.aux_voltages[index] = voltage;
    }
}

void LTC_AFE::setDeviceCellsVoltage(uint16_t device_index, uint16_t voltage){
    if (device_index >= m_afeDatagram.num_devices) {
        throw std::out_of_range("Invalid AFE device index");
    }

    const std::size_t start = device_index * LTC_AFE_MAX_CELLS_PER_DEVICE;
    const std::size_t end   = start + LTC_AFE_MAX_CELLS_PER_DEVICE;

    for (std::size_t idx = start; idx < end; ++idx) {
        setCellVoltage(idx, voltage);
    }
}

void LTC_AFE::setDeviceAuxsVoltage(uint16_t device_index, uint16_t voltage){
    if (device_index >= m_afeDatagram.num_devices) {
        throw std::out_of_range("Invalid AFE device index");
    }

    const std::size_t start = device_index * LTC_AFE_MAX_THERMISTORS_PER_DEVICE;
    const std::size_t end   = start + LTC_AFE_MAX_THERMISTORS_PER_DEVICE;

    for (std::size_t idx = start; idx < end; ++idx) {
        setAuxVoltage(idx, voltage);
    }
}

void LTC_AFE::setCellPackVoltage(uint16_t voltage){
    for (std::size_t dev = 0; dev < m_afeDatagram.num_devices; ++dev){
        setDeviceCellsVoltage(dev, voltage);
    }
}

void LTC_AFE::setAuxPackVoltage(uint16_t voltage){
    for (std::size_t dev = 0; dev < m_afeDatagram.num_devices; ++dev){
        setDeviceAuxsVoltage(dev, voltage);
    }
}

std::size_t LTC_AFE::getNumCells() const{
    return m_afeDatagram.num_cells;      
}

std::size_t LTC_AFE::getNumThermistors() const{
    return m_afeDatagram.num_thermistors;
}

std::size_t LTC_AFE::getNumDevices() const{
    return m_afeDatagram.num_devices; 
}

uint16_t LTC_AFE::getCellBitsetForDevice(std::size_t deviceIndex) const{
    return m_afeDatagram.cell_bitset[deviceIndex];    
}

uint16_t LTC_AFE::getAuxBitsetForDevice(std::size_t deviceIndex) const{
    return m_afeDatagram.aux_bitset[deviceIndex];    
}

uint16_t LTC_AFE::getCellVoltage(std::size_t index) const{
    return m_afeDatagram.cell_voltages[index]; 
}

uint16_t LTC_AFE::getAuxVoltage(std::size_t index) const{
    return m_afeDatagram.aux_voltages[index];
}

}