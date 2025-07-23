/************************************************************************************************
 * @file   adbms_afe_manager.cc
 *
 * @brief  Source file defining the AfeManager class for the client
 *
 * @date   2025-07-22
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>
#include <iostream>
#include <vector>

/* Inter-component Headers */
extern "C" { 
#include "adbms_afe.h"
}

#include "command_code.h"

/* Instra-componenet Headers */
#include "app.h"
#include "adbms_afe_manager.h"

/* Constructor */


/* SET 
-------------------------------------------------*/
void AfeManager::setAfeCell(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint8_t cell_index = m_afeDatagram.getIndex();  
  uint16_t voltage = m_afeDatagram.getCellVoltage(cell_index);   

  adbms_afe_set_cell_voltage(&s_afe, cell_index, voltage); 
}

void AfeManager::setAfeAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint8_t aux_index = m_afeDatagram.getIndex();
  uint16_t voltage = m_afeDatagram.getAuxVoltage(aux_index); 

  adbms_afe_set_aux_voltage(&s_afe, aux_index, voltage);
}

void AfeManager::setAfeDevCell(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  std::size_t device_index =  m_afeDatagram.getDevIndex(); 
  uint16_t voltage = m_afeDatagram.getCellVoltage(static_cast<uint8_t>(device_index));

  adbms_afe_set_afe_dev_cell_voltages(&s_afe, device_index, voltage);
}

void AfeManager::setAfeDevAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  std::size_t device_index = m_afeDatagram.getDevIndex(); 
  uint16_t voltage = m_afeDatagram.getAuxVoltage(static_cast<uint8_t>(device_index));

  adbms_afe_set_afe_dev_aux_voltages(&s_afe, device_index, voltage);
}

void AfeManager::setAfePackCell(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint16_t voltage = m_afeDatagram.getCellVoltage(0); 

  adbms_afe_set_pack_cell_voltages(&s_afe, voltage);
}

void AfeManager::setAfePackAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 
  
  uint16_t voltage = m_afeDatagram.getAuxVoltage(0); 

  adbms_afe_set_pack_aux_voltages(&s_afe, voltage);
}

/* PROCESS
-------------------------------------------------------- */
std::string AfeManager::processAfeCell(std::string &payload){
  m_afeDatagram.deserialize(payload);

  uint16_t cell_index = m_afeDatagram.getIndex(); 
  
  uint16_t voltage = adbms_afe_get_cell_voltage(&s_afe, cell_index);

  std::cout << "The cell Voltage has been processed to: " << voltage << std::endl;

  return m_afeDatagram.serialize(CommandCode::AFE_GET_CELL);

} 
std::string AfeManager::processAfeAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint16_t aux_index = m_afeDatagram.getIndex(); 
  
  uint16_t voltage = adbms_afe_get_aux_voltage(&s_afe, aux_index);

  return m_afeDatagram.serialize(CommandCode::AFE_GET_AUX); 
}

std::string AfeManager::processAfeDevCell(std::string &payload){
  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 
  const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  const uint16_t end   = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  for (uint16_t cell = start; cell < end; ++cell){
    uint16_t voltage = adbms_afe_get_cell_voltage(&s_afe, cell);
  }
  
  return m_afeDatagram.serialize(CommandCode::AFE_GET_DEV_CELL);
}
std::string AfeManager::processAfeDevAux(std::string &payload){
  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 
  const uint16_t start = dev_index * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;
  const uint16_t end   = start + ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;

  for (uint16_t aux = start; aux < end; ++aux){
    uint16_t voltage = adbms_afe_get_aux_voltage(&s_afe, aux);
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_DEV_AUX); 
}

std::string AfeManager::processAfePackCell(){
  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index){
    const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
    const uint16_t end   = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

    for (uint16_t cell = start; cell < end; ++cell){
      uint16_t voltage = adbms_afe_get_cell_voltage(&s_afe, cell);
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_CELL);
}
std::string AfeManager::processAfePackAux(){
  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index){
    const uint16_t start = dev_index * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;
    const uint16_t end   = start + ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;

    for (uint16_t aux = start; aux < end; ++aux){
      uint16_t voltage = adbms_afe_get_aux_voltage(&s_afe, aux);
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_AUX); 
} 
