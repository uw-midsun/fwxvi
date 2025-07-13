#pragma once

/************************************************************************************************
 * @file   command_code.h
 *
 * @brief  Header file defining the afe_datagram
 *
 * @date   2025-01-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <string>

/* Intra-component Headers */
#include "command_code.h"

namespace Datagram {

class ADBMS_AFE {
  public: 
    static constexpr uint8_t AFE_MAX_DEVICES                 = 3;   /**< Max number of AFE devices supported */
    static constexpr uint8_t AFE_MAX_CELLS_PER_DEVICE        = 12;  /**< Max cell channels per AFE device */
    static constexpr uint8_t AFE_MAX_THERMISTORS_PER_DEVICE  = 8;   /**< Max thermistors per AFE device */

    static constexpr uint8_t AFE_MAX_CELLS       = AFE_MAX_DEVICES * AFE_MAX_CELLS_PER_DEVICE;        /**< Total cell channels supported */
    static constexpr uint8_t AFE_MAX_THERMISTORS = AFE_MAX_DEVICES * AFE_MAX_THERMISTORS_PER_DEVICE;  /**< Total thermistor channels supported */
    
    /**
     * @brief Ltc Afe Datagram payload storage
     * 
     */
    struct Payload{
        std::size_t num_cells;                            /**< Total number of cells on all devices */
        std::size_t num_thermistors;                      /**< Total number of thermistors on all devices */
        std::size_t num_devices;                          /**< Total number of devices */

        uint16_t index;                                   /**< Cell/Aux index */
        std::size_t dev_index;                            /**< device Index */
        uint8_t voltage;                                  /**< Voltage of whatever we are setting */
        
        /* Could use vector for below to be more space efficient  */
        uint16_t cell_bitset[AFE_MAX_DEVICES];            /**< Bitset showing cells are enabled for each device */
        uint16_t aux_bitset[AFE_MAX_DEVICES];             /**< Bitset showing aux inputs enabled for each device */

        uint16_t cell_voltages[AFE_MAX_CELLS];            /**< Cell voltages for all devices */
        uint16_t aux_voltages[AFE_MAX_THERMISTORS];       /**< Aux voltages for all devices */

    };

    /**
     * @brief Constructs a ADBMS_AFE object with provided payload data
     * 
     */
    explicit ADBMS_AFE(Payload &data);

    /**
     * @brief Default constructor for ADBMS_AFE object
     */
    ADBMS_AFE() = default; 

    /**
     * @brief             Serializes afe data with command code for transmission
     * @param commandCode Command code to include in serialized data
     * @return            Serialized string containing afe data
     */
    std::string serialize(const CommandCode &commandCode) const; 

    /**
     * @brief Deserializes ltc afe data from payload string
     * @param afeDatagramPayload String containing serialized Ltc Afe data 
     */
    void deserialize(std::string &afeDatagramPayload);   

    /**
     * @brief Set the number of cells
     * @param cells Number of cells
     */
    void setNumCells(std::size_t cells); 

    /**
     * @brief Set the number of thermistors
     * @param thermistors Number of thermistors
     */
    void setNumThermistors(std::size_t thermistors); 

    /**
     * @brief Set the number of afe devices
     * @param devices Number of devices
     */
    void setNumDevices(std::size_t devices);

    void setIndex(uint16_t new_index); 
    void setDeviceIndex(std::size_t new_index); 
    void setVoltage(uint8_t voltage); 

    /**
     * @brief Set the cell bitset (Shows enabled cells for each device)
     * @param deviceIndex The device's bitmask you want to replace
     * @param bitmask Bitmask outlining which cells are on
     */
    void setCellBitsetForDevice(std::size_t deviceIndex, uint16_t bitmask);

    /**
     * @brief Set the aux bitset (Shows enabled thermistors for each device)
     * @param deviceIndex The device's bitmask you want to replace
     * @param bitmask Bitmask outlining which cells are on
     */
    void setAuxBitsetForDevice (std::size_t deviceIndex, uint16_t bitmask);
    
    /**
     * @brief Set a single cell voltage reading
     * @param index Global cell index (0, ..., AFE_MAX_CELLS-1)
     * @param voltage Voltage in mV
     */
    void setCellVoltage(uint16_t index, uint16_t voltage);

    /**
     * @brief Set a single aux voltage reading
     * @param index Global aux index (0, ..., AFE_MAX_THERMISTORS-1)
     * @param voltage Voltage in mV
     */
    void setAuxVoltage(uint16_t index, uint16_t voltage);

    /**
     * @brief Get the total number of cells on all devices
     * @return Total number of cells
     */
    std::size_t getNumCells() const;

    /**
     * @brief Get the total number of thermistors on all devices
     * @return Total number of thermistors
     */
    std::size_t getNumThermistors() const;

    /**
     * @brief Get the total number of AFE devices
     * @return Total number of devices
     */
    std::size_t getNumDevices() const;

    /**
     * @brief Get the enabled‐cells bitset for a specific device
     * @param deviceIndex Index of the device (0 .. AFE_MAX_DEVICES-1)
     * @return Bitmask of enabled cells for that device
     */
    uint16_t getCellBitsetForDevice(std::size_t deviceIndex) const;

    /**
     * @brief Get the enabled‐aux (thermistor) bitset for a specific device
     * @param deviceIndex Index of the device (0 .. AFE_MAX_DEVICES-1)
     * @return Bitmask of enabled aux inputs for that device
     */
    uint16_t getAuxBitsetForDevice(std::size_t deviceIndex) const;

    /**
     * @brief Get a single cell voltage reading
     * @param index Global cell index (0 .. AFE_MAX_CELLS-1)
     * @return Voltage in millivolts
     */
    uint16_t getCellVoltage(std::size_t index) const;

    /**
     * @brief Get a single aux (thermistor) voltage reading
     * @param index Global aux index (0 .. AFE_MAX_THERMISTORS-1)
     * @return Voltage in millivolts
     */
    uint16_t getAuxVoltage(std::size_t index) const;

    /**
     * @brief Get the Index 
     * @return Index 
     */
    uint16_t getIndex() const; 

    /**
     * @brief Get the Device Index 
     * @return Device index
     */
    std::size_t getDevIndex() const;

    /**
     * @brief Get the Voltage in millivolts
     * @return Voltage in millivolts
     */
    uint8_t getVoltage() const;
      
    private:
      Payload m_afeDatagram; 
};

}