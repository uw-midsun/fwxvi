
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
AfeManager::AfeManager(){
  if (adbms_afe_init(&s_afe, &s_settings) != STATUS_CODE_OK){
    throw std::runtime_error("AFE not initialized");
  };
}

/* SET 
-------------------------------------------------*/
void AfeManager::setAfeCell(std::string &payload){
  std::cout << "SETTING AFE CELL" << std::endl;
  m_afeDatagram.deserialize(payload); 

  uint16_t cell_index = m_afeDatagram.getIndex();
  uint16_t device_index = cell_index / ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  uint8_t voltage = m_afeDatagram.getVoltage();

  s_afe.settings->num_cells = m_afeDatagram.getNumCells();
  s_afe.settings->cell_bitset[device_index] = m_afeDatagram.getCellBitsetForDevice(device_index); 

  adbms_afe_set_cell_voltage(&s_afe, cell_index, voltage); 
}

void AfeManager::setAfeAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint16_t aux_index = m_afeDatagram.getIndex();
  std::size_t device_index =  aux_index / ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE; 
  uint8_t voltage = m_afeDatagram.getVoltage();

  s_afe.settings->num_thermistors = m_afeDatagram.getNumThermistors();
  s_afe.settings->aux_bitset[device_index] = m_afeDatagram.getAuxBitsetForDevice(device_index); 

  adbms_afe_set_aux_voltage(&s_afe, aux_index, voltage);
}

void AfeManager::setAfeDevCell(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  std::size_t device_index =  m_afeDatagram.getDevIndex(); 
  uint8_t voltage = m_afeDatagram.getVoltage();

  s_afe.settings->num_cells = m_afeDatagram.getNumCells();
  s_afe.settings->cell_bitset[device_index] = m_afeDatagram.getCellBitsetForDevice(device_index); 

  adbms_afe_set_afe_dev_cell_voltages(&s_afe, device_index, voltage);
}

void AfeManager::setAfeDevAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  std::size_t device_index = m_afeDatagram.getDevIndex(); 
  uint8_t voltage = m_afeDatagram.getVoltage(); 

  s_afe.settings->num_thermistors = m_afeDatagram.getNumThermistors();
  s_afe.settings->aux_bitset[device_index] = m_afeDatagram.getAuxBitsetForDevice(device_index); 

  adbms_afe_set_afe_dev_aux_voltages(&s_afe, device_index, voltage);
}

void AfeManager::setAfePackCell(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint16_t cell_index = m_afeDatagram.getIndex(); 
  uint8_t voltage = m_afeDatagram.getVoltage(); 

  s_afe.settings->num_cells = m_afeDatagram.getNumCells();
  std::size_t num_devices = m_afeDatagram.getNumDevices(); 

  for (std::size_t device_index = 0; device_index < num_devices; ++device_index){
    s_afe.settings->cell_bitset[device_index] = m_afeDatagram.getCellBitsetForDevice(device_index); 
  }

  adbms_afe_set_pack_cell_voltages(&s_afe, voltage);
}

void AfeManager::setAfePackAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 
  
  uint8_t voltage = m_afeDatagram.getVoltage(); 

  s_afe.settings->num_thermistors = m_afeDatagram.getNumThermistors();
  std::size_t num_devices = m_afeDatagram.getNumDevices(); 

  for (std::size_t device_index = 0; device_index < num_devices; ++device_index){
    s_afe.settings->aux_bitset[device_index] = m_afeDatagram.getAuxBitsetForDevice(device_index); 
  }

  adbms_afe_set_pack_aux_voltages(&s_afe, voltage);
}

/* PROCESS
-------------------------------------------------------- */
std::string AfeManager::processAfeCell(std::string &payload){
  m_afeDatagram.deserialize(payload);

  uint16_t cell_index = m_afeDatagram.getIndex(); 
  
  uint16_t voltage = adbms_afe_get_cell_voltage(cell_index);
  m_afeDatagram.setCellVoltage(cell_index, voltage); 
  
  return m_afeDatagram.serialize(CommandCode::AFE_GET_CELL);

} 
std::string AfeManager::processAfeAux(std::string &payload){
  m_afeDatagram.deserialize(payload); 

  uint16_t aux_index = m_afeDatagram.getIndex(); 
  
  uint16_t voltage = adbms_afe_get_aux_voltage(aux_index);
  m_afeDatagram.setAuxVoltage(aux_index, voltage); 

  return m_afeDatagram.serialize(CommandCode::AFE_GET_AUX); 
}

std::string AfeManager::processAfeDevCell(std::string &payload){
  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 
  const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
  const uint16_t end   = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

  for (uint16_t cell = start; cell < end; ++cell){
    uint16_t voltage = adbms_afe_get_cell_voltage(cell);
    m_afeDatagram.setCellVoltage(cell, voltage); 
  }
  
  return m_afeDatagram.serialize(CommandCode::AFE_GET_DEV_CELL);
}
std::string AfeManager::processAfeDevAux(std::string &payload){
  m_afeDatagram.deserialize(payload);

  std::size_t dev_index = m_afeDatagram.getDevIndex(); 
  const uint16_t start = dev_index * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;
  const uint16_t end   = start + ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;

  for (uint16_t aux = start; aux < end; ++aux){
    uint16_t voltage = adbms_afe_get_aux_voltage(aux);
    m_afeDatagram.setAuxVoltage(aux, voltage); 
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_DEV_AUX); 
}

std::string AfeManager::processAfePackCell(){
  // Could use a helper function (but too much work) 

  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index){
    const uint16_t start = dev_index * ADBMS_AFE_MAX_CELLS_PER_DEVICE;
    const uint16_t end   = start + ADBMS_AFE_MAX_CELLS_PER_DEVICE;

    for (uint16_t cell = start; cell < end; ++cell){
      uint16_t voltage = adbms_afe_get_cell_voltage(cell);
      m_afeDatagram.setCellVoltage(cell, voltage); 
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_CELL);
}
std::string AfeManager::processAfePackAux(){
  for (std::size_t dev_index = 0; dev_index < ADBMS_AFE_MAX_DEVICES; ++dev_index){
    const uint16_t start = dev_index * ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;
    const uint16_t end   = start + ADBMS_AFE_MAX_THERMISTORS_PER_DEVICE;

    for (uint16_t aux = start; aux < end; ++aux){
      uint16_t voltage = adbms_afe_get_aux_voltage(aux);
      m_afeDatagram.setAuxVoltage(aux, voltage); 
    }
  }

  return m_afeDatagram.serialize(CommandCode::AFE_GET_PACK_AUX); 
} 
