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

static uint8_t ads122_reg_config[ADS122_NUM_REGS];

StatusCode init_acs122(){


    return STATUS_CODE_OK;

}

/* Combines the command and the register*/
static uint8_t acs122_create_command(ADS122C14ITER_Register reg, ADS122C14ITER_Command command){
    return command || reg;
}

/* Combines the command and the register, adds the to a data output array <- Should we add the data the reg wants as well?*/
static StatusCode acs122_create_command(ADS122C14ITER_Register reg, ADS122C14ITER_Command command, uint8_t *data[], uint16_t element){
    data[element] = acs122_create_command( reg, command);
    return STATUS_CODE_OK;
}

/* Combines the command and the register, adds to a specified spot in the data output (write specifically) then adds data to write*/
static StatusCode acs122_create_write_command(ADS122C14ITER_Register reg, ADS122C14ITER_Command command, uint8_t *tx_data[], uint16_t element, uint8_t *data[]){
    tx_data[element] = acs122_create_command( reg, command);
    memcpy(tx_data + (element + 1), data, sizeof(data));
    return STATUS_CODE_OK;
}

//how big should the read data be
//read protocol: S-target-WRITE --ACK-- RREG --ACK-- Sr-Target-READ --ACK--DATA-- (-ACK- --CRC--) -ACK-STOP-
/* Reads a specified register and gets the data */
static StatusCode acs122_read_register(ADS122Storage * storage, uint8_t *data, ADS122C14ITER_Register reg){
    if(&storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

   uint8_t reg_address = acs122_create_command(reg, ADS122_READ_COMMAND);

    uint8_t rx_data;

    StatusCode status_read = i2c_read_mem(storage->i2c_port, storage->i2c_address, reg_address, &rx_data, sizeof(rx_data));
    if(status_read != STATUS_CODE_OK){
        return status_read;
    }

    return STATUS_CODE_OK;
}

/* Writes to ONE register -> data needs to be in the right format already*/
static StatusCode acs122_write_register(ADS122Storage * storage, uint8_t* data[]){
    if(&storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    }

    StatusCode status = i2c_write(storage->i2c_port, storage->i2c_address, &data, 2U);
    if(status != STATUS_CODE_OK){
        return status;
    }

    return STATUS_CODE_OK;
}

/* Writes to ONE register, will combine data and register*/
static StatusCode acs122_write_register(ADS122Storage * storage, uint8_t *data, ADS122C14ITER_Register reg){
    uint8_t tx_data [] = {acs122_create_command(reg, ADS122_WRITE_COMMAND), data};
    
    return acs122_write_register(&storage, &tx_data);
}

/* Writes to a series of register - 8 bits for command - 8 bits for data*/
static StatusCode acs122_write_register(ADS122Storage * storage, uint8_t *data[], ADS122C14ITER_Register reg[], uint32_t data_length){
    if(&storage == NULL){
        return STATUS_CODE_INVALID_ARGS;
    } 

    // TODO: is there a max length that can be sent (or should be sent)
    data_length *= 2;

    //size of doesn't work on pointers
    uint8_t tx_data[sizeof(&data)];

    StatusCode status = i2c_write(storage->i2c_port, storage->i2c_address, &data, 1U);
    if(status != STATUS_CODE_OK){
        return status;
    }

    return STATUS_CODE_OK;
}



// Writing can write to multiple registers by putting another WREG frame in
//what is the maximum data that can be passed in?? -> should be checked


//write to regs
    //get data
    //write regs
    //read regs

// get data

//the crc thing
//is new data ready?


