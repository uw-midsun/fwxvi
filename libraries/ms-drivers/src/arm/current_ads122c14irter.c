/************************************************************************************************
 * @file   current_ads122c14irter.c
 *
 * @brief  Source file to implement
 *
 * @date   2026-06-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include "string.h"

/* Inter-component Headers */
#include "delay.h"

/* Intra-component Headers */
#include "current_ads122c14irter.h"

/* Combines the command and the register*/
static uint8_t ads122_create_command(ADS122C14ITER_Register reg, ADS122C14ITER_Command command){
    return command | reg;
}

//how big should the read data be
//read protocol: S-target-WRITE --ACK-- RREG --ACK-- Sr-Target-READ --ACK--DATA-- (-ACK- --CRC--) -ACK-STOP-
/* Reads a specified register and gets the data */
static StatusCode ads122_read_register(ADS122Storage * storage, uint8_t *rx_data, ADS122C14ITER_Register reg){
    if(storage == NULL || rx_data == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

    uint8_t reg_address = ads122_create_command(reg, ADS122_READ_COMMAND);

    status_ok_or_return(i2c_read_mem(storage->i2c_port, storage->i2c_address, reg_address, rx_data, 1U));

    return STATUS_CODE_OK;
}

/* Writes to ONE register*/
static StatusCode ads122_write_register(ADS122Storage * storage, uint8_t data, ADS122C14ITER_Register reg){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    uint8_t tx_data[2];

    tx_data [0] = ads122_create_command(reg, ADS122_WRITE_COMMAND);
    tx_data [1] = data;

    StatusCode status = i2c_write(storage->i2c_port, storage->i2c_address, tx_data, 2U);
    if(status != STATUS_CODE_OK){
        return status;
    }

    return STATUS_CODE_OK;
}

/* Writes to a series of registers - 8 bits for command - 8 bits for data*/
static StatusCode ads122_write_multiple_registers(ADS122Storage * storage, uint8_t data[], ADS122C14ITER_Register reg[], uint32_t data_length){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

    uint8_t tx_data[data_length*2];

    for (uint32_t i = 0; i < data_length; i++) {
        tx_data[i * 2] = ads122_create_command(reg[i], ADS122_WRITE_COMMAND);
        tx_data[i * 2 + 1] = data[i];
    }

    return i2c_write(storage->i2c_port, storage->i2c_address, tx_data, data_length * 2U);
}

StatusCode ads122_start_conversion(ADS122Storage * storage){
    if(storage == NULL){

    }
    uint8_t conversion_ctrl = 0x00;
    status_ok_or_return(ads122_read_register(storage, &conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    conversion_ctrl |= (1 << 1);
    status_ok_or_return(ads122_write_register(storage, conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    return STATUS_CODE_OK;
}

StatusCode ads122_change_MUX(ADS122Storage * storage, uint8_t MUX_CFG){
    status_ok_or_return(ads122_write_register(storage, MUX_CFG, ADS122_REG_MUX_CFG));
    return STATUS_CODE_OK;
}

StatusCode ads122_init(ADS122Storage * storage, I2CPort i2c_port_storage, I2CAddress i2c_address_storage, uint8_t register_map[], I2CSettings * i2c_settings_storage){
    if(storage == NULL || register_map == NULL || i2c_settings_storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }
    
    storage->i2c_port = i2c_port_storage;
    storage->i2c_address = i2c_address_storage;
    storage->i2c_settings = *i2c_settings_storage;

    StatusCode status = i2c_init(i2c_port_storage, i2c_settings_storage);
    if(status != STATUS_CODE_OK){
        return status;
    }

    /* Ensure the device is powered on and communicating*/
    uint8_t device_id;
    status_ok_or_return(ads122_read_register(storage, &device_id, ADS122_REG_DEVICE_ID));
    if(device_id == 0){
        return STATUS_CODE_UNREACHABLE;
    }

    /* Reset RESETn and AVDD_UVn fault flags*/
    uint8_t reset_status_msb = 0xC0;
    status_ok_or_return(ads122_write_register(storage, reset_status_msb, ADS122_REG_STATUS_MSB));

    /* Set init configs -> put init values into a ADS122_CONFIG_REGISTERS*/
    status_ok_or_return(ads122_write_multiple_registers(storage, register_map, ADS122_CONFIG_REGISTERS, 11U));

    status_ok_or_return(ads122_start_conversion(storage));

    return STATUS_CODE_OK;
}

/* get data from board*/
static StatusCode ads122_read_conversion(ADS122Storage *storage, uint8_t data[]){
    if (storage == NULL || data == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }

 
    return i2c_read_mem(storage->i2c_port, storage->i2c_address, ADS122_READ_CONVERSION_COMMAND, data, 5U);

}


StatusCode ads122_get_conversion_data(ADS122Storage * storage, uint8_t rx_data[]){
    if(storage == NULL || rx_data == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    delay_ms(1);
    //TODO: determine if the delay is needed

    return ads122_read_conversion(storage, rx_data);
}
