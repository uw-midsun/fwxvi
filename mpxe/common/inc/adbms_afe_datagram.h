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
  static constexpr uint8_t AFE_MAX_DEVICES = 3;                /**< Max number of AFE devices supported */
  static constexpr uint8_t AFE_MAX_CELLS_PER_DEVICE = 18;      /**< Max cell channels per AFE device */
  static constexpr uint8_t AFE_MAX_THERMISTORS_PER_DEVICE = 9; /**< Max thermistors per AFE device */

  static constexpr uint8_t AFE_MAX_CELLS = AFE_MAX_DEVICES * AFE_MAX_CELLS_PER_DEVICE;             /**< Total cell channels supported */
  static constexpr uint8_t AFE_MAX_THERMISTORS = AFE_MAX_DEVICES * AFE_MAX_THERMISTORS_PER_DEVICE; /**< Total thermistor channels supported */

  /**
   * @brief Index defintions for cache array
   */
  enum class CacheIndex {
    CELL_DEV_0 = 0, /**< Store cell voltage value for device 0 */
    CELL_DEV_1,     /**< Store cell voltage value for device 1 */
    CELL_DEV_2,     /**< Store cell voltage value for device 2 */
    CELL_PACK,      /**< Store cell voltage value for pack */
    AUX_DEV_0,      /**< Store aux voltage value for device 0 */
    AUX_DEV_1,      /**< Store aux voltage value for device 1 */
    AUX_DEV_2,      /**< Store aux voltage value for device 2 */
    AUX_PACK,       /**< Store aux voltage value for pack */
    DISCHARGE_PACK, /**< Store cell discharge value for pack */
    MAX_CACHE,      /**< Max cache index */
  };

  static constexpr size_t CACHE_SIZE = static_cast<size_t>(CacheIndex::MAX_CACHE); /**< Max cache index */

  /**
   * @brief  Cell Discharge State
   */
  enum class DischargeState {
    DISCHARGE_OFF = 0, /**< Cell discharge is off */
    DISCHARGE_ON,      /**< Cell discharge is on */
  };

  /**
   * @brief Ltc Afe Datagram payload storage
   */
  struct Payload {
    uint8_t index;         /**< Cell/Aux index */
    std::size_t dev_index; /**< device Index */

    uint16_t cell_voltages[AFE_MAX_CELLS];      /**< Data storage for cell voltages */
    uint16_t aux_voltages[AFE_MAX_THERMISTORS]; /**< Data for aux voltages */
    bool cell_discharges[AFE_MAX_CELLS];        /**< Cell discharges enabled/disabled */

    uint16_t cache[CACHE_SIZE]; /**< Stores temporary dev/pack values */
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
   * @brief Set a single cell voltage reading
   * @param index Global cell index (0, ..., AFE_MAX_CELLS-1)
   * @param voltage Voltage in mV
   */
  void setCellVoltage(uint8_t index, uint16_t voltage);

  /**
   * @brief Set a single aux voltage reading
   * @param index Global aux index (0, ..., AFE_MAX_THERMISTORS-1)
   * @param voltage Voltage in mV
   */
  void setAuxVoltage(uint8_t index, uint16_t voltage);

  /**
   * @brief Set cell voltages for a device
   * @param dev_index Device index
   * @param voltage Voltage in mV
   */
  void setDeviceCellVoltage(std::size_t dev_index, uint16_t voltage);

  /**
   * @brief Set aux voltages for a device
   * @param dev_index device index
   * @param voltage Voltage in mV
   */
  void setDeviceAuxVoltage(std::size_t dev_index, uint16_t voltage);

  /**
   * @brief Set a pack cell voltage reading
   * @param voltage Voltage in mV
   */
  void setPackCellVoltage(uint16_t voltage);

  /**
   * @brief Set a pack aux voltage reading
   * @param voltage Voltage in mV
   */
  void setPackAuxVoltage(uint16_t voltage);

  /**
   * @brief Set discharges of cell
   * @param is_discharge Is discharge enabled for cell
   */
  void setCellDischarge(bool is_discharge, uint8_t cell_index);
  
  /**
   * @brief Set the Cell Pack Discharge 
   * @param is_discharge Is discharge enabled for cell
   */
  void setCellPackDischarge(bool is_discharge); 
  
  /**
   * @brief Set the Cache value for index
   * @param cache_index the specific value to change
   * @param value the new value to add
   */
  void setCache(CacheIndex cache_index, uint16_t value);

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
   * @brief Get the cell discharge status of a cell
   * @param cell_index Cell index
   */
  bool getCellDischarge(uint8_t cell_index) const;

  /**
   * @brief Get the Cache value
   * @param cache_index the index of the cache value you want to get
   */
  uint16_t getCache(CacheIndex cache_index) const;

 private:
  Payload m_afeDatagram;
};

}  // namespace Datagram

/** @} */
