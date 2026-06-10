/************************************************************************************************
 * @file   current_ads122c14irter.c
 *
 * @brief  Source file to implement
 *
 * @date   2026-06-04
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include "string.h";

/* Inter-component Headers */

/* Intra-component Headers */
#include "current_ads122c14irter.h";


//static uint8_t ads122_reg_config[ADS122_NUM_REGS];

#define ADS122_READ_CONVERSION_LENGTH 2U;

//static ADS122Storage storage;

//let the user take the 

StatusCode ads122_init(ADS122Storage * storage, I2CPort i2c_port_storage, I2CAddress i2c_address_storage, uint8_t ads122_reg_config[]){
    storage->i2c_port = i2c_port_storage;
    storage->i2c_address = i2c_address_storage;

    /* Ensure the device is powered on and communicating*/
    uint8_t device_id;
    status_ok_or_return(ads122_read_register(&storage, device_id, ADS122_REG_DEVICE_ID));
    if(device_id == 0){
        return STATUS_CODE_UNREACHABLE;
    }

    /* Reset RESETn and AVDD_UVn fault flags*/
    uint8_t reset_status_msb = 0xC0;
    status_ok_or_return(ads122_write_register(&storage, reset_status_msb, ADS122_REG_STATUS_MSB));

    /* Set init configs */

    // Set default configs
    for(int i = ADS122_REG_DEVICE_CFG; i < ADS122_NUM_REGS; i++){
        status_ok_or_return(ads122_write_register(&storage, ads122_reg_config[i-ADS122_REG_DEVICE_CFG], i));
    }

    ads122_start_conversion(storage);

    return STATUS_CODE_OK;
}

void ads122_change_reg_config();

/* Combines the command and the register*/
static uint8_t ads122_create_command(ADS122C14ITER_Register reg, ADS122C14ITER_Command command){
    return command || reg;
}

//how big should the read data be
//read protocol: S-target-WRITE --ACK-- RREG --ACK-- Sr-Target-READ --ACK--DATA-- (-ACK- --CRC--) -ACK-STOP-
/* Reads a specified register and gets the data */
static StatusCode ads122_read_register(ADS122Storage * storage, uint8_t *data, ADS122C14ITER_Register reg){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

   uint8_t reg_address = ads122_create_command(reg, ADS122_READ_COMMAND);

    StatusCode status_read = i2c_read_mem(storage->i2c_port, storage->i2c_address, reg_address, &data, 1U);
    if(status_read != STATUS_CODE_OK){
        return status_read;
    }

    return STATUS_CODE_OK;
}

/* Writes to ONE register, will combine data and register*/

//ADS122_REG_WRITE_FAULT <- use to check for faults
static StatusCode ads122_write_register(ADS122Storage * storage, uint8_t *data, ADS122C14ITER_Register reg){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    uint8_t tx_data [] = {ads122_create_command(reg, ADS122_WRITE_COMMAND), data};

    StatusCode status = i2c_write(storage->i2c_port, storage->i2c_address, &data, 2U);
    if(status != STATUS_CODE_OK){
        return status;
    }

    return STATUS_CODE_OK;
}

/* Writes to a series of register - 8 bits for command - 8 bits for data*/
static StatusCode ads122_write_multiple_registers(ADS122Storage * storage, uint8_t *data[], ADS122C14ITER_Register reg[], uint32_t data_length, uint8_t *tx_data){
    if(storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

    for (uint32_t i = 0; i < data_length; i++) {
        tx_data[i * 2] = ads122_create_command(reg[i], ADS122_WRITE_COMMAND);
        tx_data[i * 2 + 1] = data[i];
    }

    return i2c_write(storage->i2c_port, storage->i2c_address, tx_data, data_length * 2U);
}

static StatusCode ads122_read_conversion(ADS122Storage *storage, uint8_t *data[])
{
    if (storage == NULL || data == NULL) {
        return STATUS_CODE_INVALID_ARGS;
    }

    uint8_t command = ADS122_READ_CONVERSION_COMMAND;
    StatusCode status = i2c_read_mem(storage->i2c_port, storage->i2c_address, command,  data, ADS122_READ_CONVERSION_LENGTH);
    if(status != STATUS_CODE_OK){
        return status;
    }
    return STATUS_CODE_OK;
}

static StatusCode ads122_start_conversion(ADS122Storage * storage){
    uint8_t conversion_ctrl;
    status_ok_or_return(ads122_read_register(storage, conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    conversion_ctrl |= (1 << 1);
    status_ok_or_return(ads122_write_register(storage, &conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    return STATUS_CODE_OK;
}

static StatusCode ads122_stop_conversion(ADS122Storage * storage){
    uint8_t conversion_ctrl;
    status_ok_or_return(ads122_read_register(storage, conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    conversion_ctrl |= (1 << 0);
    status_ok_or_return(ads122_write_register(storage, &conversion_ctrl, ADS122_REG_CONVERSION_CTRL));
    return STATUS_CODE_OK;
}



/* things for init*/

// Set default for the array


// bool true/ false (on / off) ->
// get the bit
// get the register

//what is the maximum data that can be passed in?? -> should be checked

//need to configure GPIO, what conversion mode

// single poll mode (not continuous)

// how to make sure you're getting new stuff
// - poll the DRDY bit -> 1b means the conversion data is new
// do we want continuous data even if it is old???

//write to regs
    //get data
    //write regs
    //read regs

// get data

// no crc, no status -> bare bones
//read is wrong, ne

//is new data ready?


