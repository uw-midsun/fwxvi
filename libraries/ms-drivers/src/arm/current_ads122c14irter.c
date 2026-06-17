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
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

    uint8_t reg_address = ads122_create_command(reg, ADS122_READ_COMMAND);

    status_ok_or_return(i2c_read_mem(storage->i2c_port, storage->i2c_address, reg_address, rx_data, 1U));

    return STATUS_CODE_OK;
}

/* Writes to ONE register, will combine data and register*/
static StatusCode ads122_write_register(ADS122Storage * storage, uint8_t data, ADS122C14ITER_Register reg){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    uint8_t tx_data [] = {ads122_create_command(reg, ADS122_WRITE_COMMAND), data};

    status_ok_or_return(i2c_write(storage->i2c_port, storage->i2c_address, tx_data, 2U));
    
    /*Checks the write flag*/
    uint8_t reg_address = ads122_create_command(ADS122_REG_STATUS_MSB, ADS122_READ_COMMAND);
    uint8_t status_msb_data = 0x00;
    status_ok_or_return(i2c_read_mem(storage->i2c_port, storage->i2c_address, reg_address, &status_msb_data, 1U));
    if(((status_msb_data >> ADS122_REG_WRITE_FAULT_BITOFFSET) & 0x01) == 1 ){
        return STATUS_CODE_INVALID_ARGS;
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
    uint8_t conversion_ctrl = 0x00;
    status_ok_or_return(ads122_read_register(storage, &conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    conversion_ctrl |= (1 << 1);
    status_ok_or_return(ads122_write_register(storage, conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    return STATUS_CODE_OK;
}

StatusCode ads122_stop_conversion(ADS122Storage * storage){
    uint8_t conversion_ctrl;
    status_ok_or_return(ads122_read_register(storage, &conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    conversion_ctrl |= (1 << 0);
    status_ok_or_return(ads122_write_register(storage, conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    return STATUS_CODE_OK;
}

StatusCode ads122_init(ADS122Storage * storage, I2CPort i2c_port_storage, I2CAddress i2c_address_storage, uint8_t register_map[]){
    storage->i2c_port = i2c_port_storage;
    storage->i2c_address = i2c_address_storage;

    /* Ensure the device is powered on and communicating*/
    uint8_t device_id;
    status_ok_or_return(ads122_read_register(storage, &device_id, ADS122_REG_DEVICE_ID));
    if(device_id == 0){
        return STATUS_CODE_UNREACHABLE;
    }

    /* Reset RESETn and AVDD_UVn fault flags*/
    uint8_t reset_status_msb = 0xC0;
    status_ok_or_return(ads122_write_register(storage, reset_status_msb, ADS122_REG_STATUS_MSB));

    //TODO: add a read?

    /* Set init configs -> put init values into a ADS122_CONFIG_REGISTERS*/
    status_ok_or_return(ads122_write_multiple_registers(storage, register_map, ADS122_CONFIG_REGISTERS, 11U));

    /*Start conversion*/
    status_ok_or_return(ads122_start_conversion(storage));

    return STATUS_CODE_OK;
}

/* get data from board*/
static StatusCode ads122_read_conversion(ADS122Storage *storage, uint8_t data[]){
    if (storage == NULL || data == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }

    status_ok_or_return(i2c_read_mem(storage->i2c_port, storage->i2c_address, ADS122_READ_CONVERSION_COMMAND, data, 3U));
 
    return STATUS_CODE_OK;
}


StatusCode ads122_get_conversion_data(ADS122Storage * storage, uint8_t rx_data[]){
    uint8_t status_msb_data;
    status_ok_or_return(ads122_read_register(storage, &status_msb_data, ADS122_REG_STATUS_MSB));

    /*Checks DRDY pin to ensure new data is availible*/
    uint8_t DRDY_pin = ((status_msb_data >> ADS122_DRDY_BITOFFSET) & 0x01);
    //Does this work? Does the bit update properly if STATUS header is not enabled?
    if(status_msb_data != 1){
        return STATUS_CODE_OK;
    }

    /* reset DRDY pin*/
    ads122_write_register(storage, status_msb_data & (~DRDY_pin), ADS122_REG_STATUS_MSB);
    //Do I need to reset the DRDY pin?

    return ads122_read_conversion(storage, rx_data);
}