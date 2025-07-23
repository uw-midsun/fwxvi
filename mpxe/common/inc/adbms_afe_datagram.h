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
     */
    struct Payload{
        uint8_t index;          /**< Cell/Aux index */
        std::size_t dev_index;  /**< device Index */
        uint16_t voltage;       /**< Voltage of whatever we are setting */
    };

    /**
     * @brief Constructs a ADBMS_AFE object with provided payload data
     */
    explicit ADBMS_AFE(Payload &data);

    /**
     * @brief Default constructor for ADBMS_AFE object
     */
    ADBMS_AFE() = default; 

    /**
     * @brief   Serializes afe data with command code for transmission
     * @param   commandCode Command code to include in serialized data
     * @return  Serialized string containing afe data
     */
    std::string serialize(const CommandCode &commandCode) const; 

    /**
     * @brief Deserializes ltc afe data from payload string
     * @param afeDatagramPayload String containing serialized Ltc Afe data 
     */
    void deserialize(std::string &afeDatagramPayload);   

    /**
     * @brief Sets index of cell to set
     * @param new_index The new index to set
     */
    void setIndex(uint8_t new_index); 

    /**
     * @brief Sets device index to set
     * @param new_index The new index to set
     */
    void setDeviceIndex(std::size_t new_index); 

    /**
     * @brief Voltage to be set
     * @param voltage the new voltage to be set
     */
    void setVoltage(uint16_t voltage); 

    /**
     * @brief   Get the Index 
     * @return  Index 
     */
    uint8_t getIndex() const; 

    /**
     * @brief   Get the Device Index 
     * @return  Device index
     */
    std::size_t getDevIndex() const;

    /**
     * @brief   Get the Voltage in millivolts
     * @return  Voltage in millivolts
     */
    uint16_t getVoltage() const;
      
    private:
      Payload m_afeDatagram; 
};

} // namespace Datagram

/** @} */
