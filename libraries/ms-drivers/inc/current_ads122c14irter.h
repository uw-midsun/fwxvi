#pragma once

/************************************************************************************************
 * @file   current_ads122c14iter.h
 *
 * @brief  Header file to implement the current sensing from the ADS122C14ITER ADC
 *
 * @date   2026-06-16
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stdint.h>

/* Inter-component Headers */
#include "gpio.h"
#include "i2c.h"
#include "status.h"

/* Intra-component Headers */

/**
 * @defgroup ADS122C14ITER Current Sensing
 * @brief    ADS122C14ITER Current Sensing library
 * @{
 */

 // this is a 24 bit guy

//# define I2CAddress ADC_ADDRESS = 0x40; /* A0 and A1 are both wired to GND*/

#define ADS122_NUM_REGS 16U

#define _PACKED __attribute__((packed))   

#define REG_MAP_CRC_VALUES 0

typedef struct {
  I2CPort i2c_port;
  I2CAddress i2c_address;
} ADS122Storage;

/*All registers of ADS122*/
typedef enum : uint8_t {
  ADS122_REG_DEVICE_ID = 0b00000000,
  ADS122_REG_REVISION_ID = 0b00000001,
  ADS122_REG_STATUS_MSB = 0b00000010,
  ADS122_REG_STATUS_LSB = 0b00000011,
  ADS122_REG_CONVERSION_CTRL = 0b00000100,
  ADS122_REG_DEVICE_CFG = 0b00000101, /* Start init configs here*/
  ADS122_REG_DATA_RATE_CFG = 0b00000110,
  ADS122_REG_MUX_CFG = 0b00000111,
  ADS122_REG_GAIN_CFG = 0b00001000,
  ADS122_REG_REFERENCE_CFG = 0b00001001,
  ADS122_REG_DIGITAL_CFG = 0b00001010,
  ADS122_REG_GPIO_CFG = 0b00001011,
  ADS122_REG_GPIO_DATA_OUTPUT = 0b00001100,
  ADS122_REG_IDAC_MAG_CFG = 0b00001101,
  ADS122_REG_IDAC_MUX_CFG = 0b00001110,
  ADS122_REG_REG_MAP_CRC = 0b00001111,
} ADS122C14ITER_Register;

/*All config registers*/
static uint8_t ADS122_CONFIG_REGISTERS[] = {
  ADS122_REG_DEVICE_CFG,
  ADS122_REG_DATA_RATE_CFG,
  ADS122_REG_MUX_CFG,
  ADS122_REG_GAIN_CFG,
  ADS122_REG_REFERENCE_CFG,
  ADS122_REG_DIGITAL_CFG,
  ADS122_REG_GPIO_CFG,
  ADS122_REG_GPIO_DATA_OUTPUT,
  ADS122_REG_IDAC_MAG_CFG,
  ADS122_REG_IDAC_MUX_CFG,
  ADS122_REG_REG_MAP_CRC};

/*Commands*/  
typedef enum : uint8_t{
  ADS122_WRITE_COMMAND = 0b10000000,
  ADS122_READ_COMMAND = 0b01000000,
  ADS122_READ_CONVERSION_COMMAND = 0b00000000,
} ADS122C14ITER_Command;

/* Make sure things are the right size*/
_Static_assert(sizeof(ADS122C14ITER_Register) == 1U );
_Static_assert(sizeof(ADS122C14ITER_Command) == 1U );

/**
 * @brief Get the conversion data
 * @param storage - pointer to the initilized ADS122Storage struct
 * @param rx_Data - data collection array
 * @return STATUS_CODE_OK on success
 */
StatusCode ads122_get_conversion_data(ADS122Storage * storage, uint8_t rx_data[]);

/**
 * @brief Initialize the ADS122 driver
 * @param storage - pointer to an uninitalized ADS122Storage struct
 * @param I2CPort - I2C port peripheral
 * @param I2CAddress - I2C address peripheral
 * @param register_map - array of register inits
 * @return STATUS_CODE_OK on success
 */
StatusCode ads122_init(ADS122Storage * storage, I2CPort i2c_port_storage, I2CAddress i2c_address_storage, uint8_t register_map[]);

/**
 * @brief Start the conversion of the ADS122 driver
 * @param storage - pointer to an initalized ADS122Storage struct
 * @return STATUS_CODE_OK on success
 */
StatusCode ads122_start_conversion(ADS122Storage * storage);

/**
 * @brief Stop the conversion of the ADS122 driver
 * @param storage - pointer to an initalized ADS122Storage struct
 * @return STATUS_CODE_OK on success
 */
StatusCode ads122_stop_conversion(ADS122Storage * storage);




/*ADS122_REG_STATUS_MSB*/
#define ADS122_RESETn_BITOFFSET 7
#define ADS122_RESETn_MASK (1 << 7)

#define ADS122_AVDD_UV_BITOFFSET 6
#define ADS122_AVDD_UV_MASK (1 << 6)

#define ADS122_REF_UV_BITOFFSET 5
#define ADS122_REF_UV_MASK (1 << 5)

#define ADS122_REG_MAP_CRC_FAULT_BITOFFSET 3
#define ADS122_REG_MAP_CRC_FAULT_MASK (1 << 3)

#define ADS122_MEM_FAULT_BITOFFSET 2
#define ADS122_MEM_FAULT_MASK (1 << 2)

#define ADS122_REG_WRITE_FAULT_BITOFFSET 1
#define ADS122_REG_WRITE_FAULT_MASK (1 << 1)

#define ADS122_DRDY_BITOFFSET 0
#define ADS122_DRDY_MASK (1 << 0)

/* ADS122_REG_STATUS_LSB*/
#define ADS122_CONV_COUNT_BITOFFSET 4 //should this be 4 or 7
#define ADS122_CONV_COUNT_MASK (1 << 7) |  (1 << 6) | (1 << 5) | (1 << 4)

#define ADS122_GPIO3_DAT_IN_BITOFFSET 3
#define ADS122_GPIO3_DAT_IN_MASK (1 << 3)

#define ADS122_GPIO2_DAT_IN_BITOFFSET 2
#define ADS122_GPIO2_DAT_IN_MASK (1 << 2)

#define ADS122_GPIO1_DAT_IN_BITOFFSET 1
#define ADS122_GPIO1_DAT_IN_MASK (1 << 1)

#define ADS122_GPIO0_DAT_IN_BITOFFSET 0
#define ADS122_GPIO0_DAT_IN_MASK (1 << 0)

/*ADS122_REG_CONVERSION_CTRL*/
#define ADS122_RESET_BITOFFSET 2
#define ADS122_RESET_MASK (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2)

#define ADS122_START_BITOFFSET 1
#define ADS122_START_MASK (1 << 1)

#define ADS122_STOP_BITOFFSET 0
#define ADS122_STOP_MASK (1 << 0)

/*ADS122_REG_DEVICE_CFG*/
//which speed??
#define ADS122_REG_DEVICE_CFG_DEFAULT ((uint8_t) 0b00000100)

#define ADS122_PWDN_BITOFFSET 7
#define ADS122_PWDN_MASK (1 << 7)

#define ADS122_STBY_MODE_BITOFFSET 6
#define ADS122_STBY_MODE_MASK (1 << 6)

#define ADS122_BOCS_BITOFFSET 4
#define ADS122_BOCS_MASK (1 << 5) | (1 << 4)

#define ADS122_CLK_SEL_BITOFFSET 3
#define ADS122_CLK_SEL_MASK (1 << 3)

#define ADS122_CONV_MODE_BITOFFSET 2
#define ADS122_CONV_MODE_MASK (1 << 2)

#define ADS122_SPEED_MODE_BITOFFSET 0
#define ADS122_SPEED_MODE_MASK (1 << 1) | (1 << 0)

/* ADS122_REG_DATA_RATE_CFG*/
#define ADS122_REG_DATA_RATE_CFG_DEFAULT ((uint8_t) 0x00)

#define ADS122_DELAY_BITOFFSET 4
#define ADS122_DELAY_MASK (1 << 7) | (1 << 6) || (1 << 5) || (1 << 4)

#define ADS122_GC_EN_BITOFFSET 3
#define ADS122_GC_EN_MASK (1 << 3)

#define ADS122_FLTR_OSR_BITOFFSET 0
#define ADS122_FLTR_OSR_MASK (1 << 2) | (1 << 1) | (1 << 0) 

/* ADS122_REG_MUX_CFG*/
#define ADS122_REG_MUX_CFG_DEFAULT ((uint8_t) 0x01)

#define ADS122_AINP_BITOFFSET 4
#define ADS122_AINP_MASK (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4)

#define ADS122_AINN_BITOFFSET 0
#define ADS122_AINN_MASK (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0)

/*ADS122_REG_GAIN_CFG*/
#define ADS122_REG_GAIN_CFG_DEFAULT ((uint8_t) 0x01)

#define ADS122_SYS_MON_BITOFFSET 4
#define ADS122_SYS_MON_MASK (1 << 6) | (1 << 5) | (1 << 4)

#define ADS122_GAIN_BITOFFSET 0
#define ADS122_GAIN_MASK (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0)

/*ADS122_REG_REFERENCE_CFG */
#define ADS122_REG_REFERENCE_CFG_DEFAULT ((uint8_t) 0x00)

#define ADS122_REF_UV_EN_BITOFFSET 7
#define ADS122_REF_UV_EN_MASK (1 << 7)

#define ADS122_REFP_BUF_EN_BITOFFSET 5
#define ADS122_REFP_BUF_EN_MASK (1 << 5)

#define ADS122_REFN_BUF_EN_BITOFFSET 4
#define ADS122_REFN_BUF_EN_MASK (1 << 4)

#define ADS122_REF_VAL_BITOFFSET 2
#define ADS122_REF_VAL_MASK (1 << 2)

#define ADS122_REF_SEL_BITOFFSET 0
#define ADS122_REF_SEL_MASK (1 << 1) | (1 << 0)

/*ADS122_REG_DIGITAL_CFG*/
#define ADS122_REG_DIGITAL_CFG_DEFAULT ((uint8_t) 0x00)

#define ADS122_REG_MAP_CRC_EN_BITOFFSET 6
#define ADS122_REG_MAP_CRC_EN_MASK (1 << 6)

#define ADS122_I2C_CRC_EN_BITOFFSET 5
#define ADS122_I2C_CRC_EN_MASK (1 << 5)

#define ADS122_STATUS_EN_BITOFFSET 4
#define ADS122_STATUS_EN_MASK (1 << 4)

#define ADS122_FAULT_PIN_BEHAVIOR_BITOFFSET 3
#define ADS122_FAULT_PIN_BEHAVIOR_MASK (1 << 3)

#define ADS122_CODING_BITOFFSET 1
#define ADS122_CODING_MASK (1 << 1)

/*ADS122_REG_GPIO_CFG*/
#define ADS122_REG_GPIO_CFG_DEFAULT ((uint8_t) 0x00)

#define ADS122_GPIO3_CFG_BITOFFSET 6
#define ADS122_GPIO3_CFG_MASK  (1 << 7) |(1 << 6)

#define ADS122_GPIO2_CFG_BITOFFSET 4
#define ADS122_GPIO2_CFG_MASK (1 << 5) | (1 << 4)

#define ADS122_GPIO1_CFG_BITOFFSET 2
#define ADS122_GPIO1_CFGR_MASK (1 << 3) | (1 << 2)

#define ADS122_GPIO0_CFG_BITOFFSET 0
#define ADS122_GPIO0_CFG_MASK (1 << 1) | (1 << 0)

/*ADS122_REG_GPIO_DATA_OUTPUT*/
#define ADS122_REG_GPIO_DATA_OUTPUT_DEFAULT ((uint8_t) 0x00)

#define ADS122_GPIO3_SRC_BITOFFSET 7
#define ADS122_GPIO3_SRC_MASK (1 << 7)

#define ADS122_GPIO2_SRC_BITOFFSET 6
#define ADS122_GPIO2_SRC_MASK (1 << 6)

#define ADS122_GPIO3_DAT_OUT_BITOFFSET 3
#define ADS122_GPIO3_DAT_OUT_MASK (1 << 3)

#define ADS122_GPIO2_DAT_OUT_BITOFFSET 2
#define ADS122_GPIO2_DAT_OUT_MASK (1 << 2)

#define ADS122_GPIO1_DAT_OUT_BITOFFSET 1
#define ADS122_GPIO1_DAT_OUT_MASK (1 << 1)

#define ADS122_GPIO0_DAT_OUT_BITOFFSET 0
#define ADS122_GPIO0_DAT_OUT_MASK (1 << 0)

/*ADS122_REG_IDAC_MAG_CFG*/
#define ADS122_REG_IDAC_MAG_CFG_DEFAULT ((uint8_t) 0x00)

#define ADS122_I2MAG_BITOFFSET 4
#define ADS122_I2MAG_MASK (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4)

#define ADS122_I1MAG_BITOFFSET 0
#define ADS122_I1MAG_MASK (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0)

//shift left by 4 for I2MUX
typedef enum : uint8_t{
  Disabled = 0,
  Iuint_1x,
  Iuint_10x,
  Iuint_20x,
  Iuint_30x,
  Iuint_40x,
  Iuint_50x,
  Iuint_60x,
  Iuint_70x,
  Iuint_80x,
  Iuint_90x,
  Iuint_100x,
} ADS122_IUINT;

#define ADS122_IUNIT_Length 4

/*ADS122_REG_IDAC_MUX_CFG*/
#define ADS122_REG_IDAC_MUX_CFG_DEFAULT ((uint8_t) 0x10)
//check the default IDAC2 output pin selection

#define ADS122_IUNIT_BITOFFSET 7
#define ADS122_IUNIT_MASK (1 << 7)

#define ADS122_I2MUX_BITOFFSET 4
#define ADS122_I2MUX_MASK (1 << 6) | (1 << 5) | (1 << 4)

#define ADS122_I1MUX_BITOFFSET 0
#define ADS122_I1MUX_MASK (1 << 2) | (1 << 1) | (1 << 0)

//shift left by 3 for I2MUX
typedef enum : uint8_t{
  AIN0 = 0,
  AIN1,
  AIN2,
  AIN3,
  AIN4,
  AIN5,
  AIN6,
  AIN7,
} ADS122_AIN_pins;

#define ADS122_Ain_pins_length 3

/*ADS122_REG_REG_MAP_CRC*/
#define ADS122_REG_REG_MAP_CRC_DEFAULT ((uint8_t) 0x00)


// /* What timing mode? <- timing requirements <-- how do?*/