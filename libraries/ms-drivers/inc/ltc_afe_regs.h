#pragma once

/************************************************************************************************
 * @file   ltc_afe_regs.h
 *
 * @brief  Header file to define the LTC8611 AFE's register layout, configuration, and command code structures
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <assert.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup LTC6811
 * @brief    LTC6811 library
 * @{
 */

/** @brief Stores all member variables in a struct without padding */
#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

/** @brief  Used internally by the LTC AFE driver (number of regs per reg group) */
#define LTC6811_CELLS_IN_REG 3
#define LTC6811_GPIOS_IN_REG 3

/** @brief  Used for the external mux (ADG731) connected to the AFE */
#define AUX_ADG731_NUM_PINS 32

/** @brief  Size of command code (2 bytes) + PEC (2 bytes) */
#define LTC6811_CMD_SIZE 4

/** @brief  3 bytes are required to send 24 clock cycles with our SPI driver for the STCOMM command */
#define LTC6811_NUM_COMM_REG_BYTES 3

/** @brief  Number of registers in PWM Register Group. See Table 51 on page 64. */
#define LTC8611_NUM_PWM_REGS 6

/** @brief Number of voltage cells connected to each device */
#define LTC_AFE_MAX_CELLS_PER_DEVICE 12
/**
 * @brief List of LTC AFE registers
 * @details These registers are mapped to the `s_read_reg[]` array in `ltc_afe.c`
 */
typedef enum {
  LTC_AFE_REGISTER_CONFIG = 0,     /**< Configuration register */
  LTC_AFE_REGISTER_CELL_VOLTAGE_A, /**< Cell voltage register A */
  LTC_AFE_REGISTER_CELL_VOLTAGE_B, /**< Cell voltage register B */
  LTC_AFE_REGISTER_CELL_VOLTAGE_C, /**< Cell voltage register C */
  LTC_AFE_REGISTER_CELL_VOLTAGE_D, /**< Cell voltage register D */
  LTC_AFE_REGISTER_AUX_A,          /**< Auxiliary register A */
  LTC_AFE_REGISTER_AUX_B,          /**< Auxiliary register B */
  LTC_AFE_REGISTER_STATUS_A,       /**< Status register A */
  LTC_AFE_REGISTER_STATUS_B,       /**< Status register B */
  LTC_AFE_REGISTER_READ_COMM,      /**< Read communication register */
  LTC_AFE_REGISTER_START_COMM,     /**< Start communication register */
  NUM_LTC_AFE_REGISTERS            /**< Total number of LTC AFE registers */
} LtcAfeRegister;

/** @brief Voltage registers for easy mapping to `LtcAfeRegister` */
typedef enum {
  LTC_AFE_VOLTAGE_REGISTER_A = LTC_AFE_REGISTER_CELL_VOLTAGE_A,
  LTC_AFE_VOLTAGE_REGISTER_B = LTC_AFE_REGISTER_CELL_VOLTAGE_B,
  LTC_AFE_VOLTAGE_REGISTER_C = LTC_AFE_REGISTER_CELL_VOLTAGE_C,
  LTC_AFE_VOLTAGE_REGISTER_D = LTC_AFE_REGISTER_CELL_VOLTAGE_D,
  NUM_LTC_AFE_VOLTAGE_REGISTERS
} LtcAfeVoltageRegister;

/**
 * @brief Discharge timeout options for the AFE
 * @details See Table 52 on Page 65 for more details
 */
typedef enum {
  LTC_AFE_DISCHARGE_TIMEOUT_DISABLED = 0, /**< Discharge timeout disabled */
  LTC_AFE_DISCHARGE_TIMEOUT_30_S,         /**< 30-second timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_1_MIN,        /**< 1-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_2_MIN,        /**< 2-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_3_MIN,        /**< 3-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_4_MIN,        /**< 4-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_5_MIN,        /**< 5-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_10_MIN,       /**< 10-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_15_MIN,       /**< 15-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_20_MIN,       /**< 20-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_30_MIN,       /**< 30-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_40_MIN,       /**< 40-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_60_MIN,       /**< 60-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_75_MIN,       /**< 75-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_90_MIN,       /**< 90-minute timeout */
  LTC_AFE_DISCHARGE_TIMEOUT_120_MIN       /**< 120-minute timeout */
} LtcAfeDischargeTimeout;

/**
 * @brief   Configuration for the AFE configuration register group (CFGR).
 * @details The configuration includes:
 *          - ADC Mode Option Bit (`adcopt`):             Determines ADC conversion modes
 *          - Discharge Timer Enable (`dten`):            Enable/disable discharge timer
 *          - References Powered Up (`refon`):            Keeps references powered up until watchdog timeout
 *          - GPIO Pin Pull-Down Config (`gpio`):         Configures GPIO pull-downs
 *          - Undervoltage Threshold (`undervoltage`):    Sets undervoltage threshold
 *          - Overvoltage Threshold (`overvoltage`):      Sets overvoltage threshold
 *          - Discharge Cell Bitset (`discharge_bitset`): Enables/disables discharge for specific cells
 *          - Discharge Timeout (`discharge_timeout`):    Sets discharge timeout duration
 * @note    See Table 40 on Page 62 for the exact layout.
 */
typedef struct {
  uint8_t adcopt : 1; /**< ADC Mode Option Bit */
  uint8_t dten : 1;   /**< Dicharge Timer Enable */
  uint8_t refon : 1;  /**< References Powered Up */

  uint8_t gpio : 5; /**< GPIO PD Config */

  uint32_t undervoltage : 12; /**< Undervoltage Threshold */
  uint32_t overvoltage : 12;  /**< Overvoltage Threshold */

  uint16_t discharge_bitset : 12; /**< Discharge Cell Bitset */
  uint8_t discharge_timeout : 4;  /**< Discharge Timeout */
} _PACKED LtcAfeConfigRegisterData;
static_assert(sizeof(LtcAfeConfigRegisterData) == 6, "LtcAfeConfigRegisterData must be 6 bytes");

/**
 * @brief   COMM Register Config, refer to pg 64 table 49
 * @details The following outline the config settings variables:
 *          - Initial communication control bits (`ICOMx`): For SPI. Determine CSB signal behaviour (Write code). Value is 0111 (Read code)
 *          - Communication Data byte (`Dx`):               Data transmitted transmitted or received from slave
 *          - Final Communication Control Bits (`FCOMx`):   Specific signals for master/slave acknowledgments and stop conditions.
 * @note    For detailed notes check out Table 20 (p40), Table 21 (p40), Table 52 (p66)
 */
typedef struct {
  uint8_t icom0 : 4;
  uint8_t d0 : 8;
  uint8_t fcom0 : 4;

  uint8_t icom1 : 4;
  uint8_t d1 : 8;
  uint8_t fcom1 : 4;

  uint8_t icom2 : 4;
  uint8_t d2 : 8;
  uint8_t fcom2 : 4;
} _PACKED LtcAfeCommRegisterData;
static_assert(sizeof(LtcAfeCommRegisterData) == 6, "LtcAfeCommRegisterData must be 6 bytes");

/** @brief WRCOMM + mux pin */
typedef struct {
  uint8_t wrcomm[LTC6811_CMD_SIZE];
  LtcAfeCommRegisterData reg;
  uint8_t pec;
} _PACKED LtcAfeWriteCommRegPacket;

/** @brief STMCOMM + clock cycles */
typedef struct {
  uint8_t stcomm[LTC6811_CMD_SIZE];
  uint8_t clk[LTC6811_NUM_COMM_REG_BYTES];
} _PACKED LtcAfeSendCommRegPacket;

/** @brief Configuration Register Group (CFGR) packet for EACH device */
typedef struct {
  LtcAfeConfigRegisterData reg;
  uint16_t pec;
} _PACKED LtcAfeWriteDeviceConfigPacket;

/**
 * @brief WRCFG + all slave registers
 * @note  Devices are ordered with the last slave first */
typedef struct {
  uint8_t wrcfg[LTC6811_CMD_SIZE]; /**< Command for writing onto config register */

  LtcAfeWriteDeviceConfigPacket devices[LTC_AFE_MAX_CELLS_PER_DEVICE]; /**< Config for EACH device */
} _PACKED LtcAfeWriteConfigPacket;
#define SIZEOF_LTC_AFE_WRITE_CONFIG_PACKET(devices) (LTC6811_CMD_SIZE + (devices) * sizeof(LtcAfeWriteDeviceConfigPacket))

typedef union {
  uint16_t voltages[3]; /**< 3 voltage readings stored as 16-bit values */
  uint8_t values[6];    /**< Byte-wise view for SPI transmission */
} LtcAfeRegisterData;
static_assert(sizeof(LtcAfeRegisterData) == 6, "LtcAfeRegisterData must be 6 bytes");

typedef struct {
  LtcAfeRegisterData reg;
  uint16_t pec;
} _PACKED LtcAfeVoltageData;
static_assert(sizeof(LtcAfeVoltageData) == 8, "LtcAfeVoltageData must be 8 bytes");

typedef struct {
  LtcAfeRegisterData reg;
  uint16_t pec;
} _PACKED LtcAfeAuxData;
static_assert(sizeof(LtcAfeAuxData) == 8, "LtcAfeAuxData must be 8 bytes");

/**
 * @brief Command codes for LTC6811 AFE
 * @details  See Table 38 (p.59)
 */
#define LTC6811_WRCFG_RESERVED (1 << 0)                                                           /**< Write Config Reg Group A */
#define LTC6811_RDCFG_RESERVED (1 << 1)                                                           /**< Read Config Reg Group A */
#define LTC6811_RDCVA_RESERVED (1 << 2)                                                           /**< Read Cell Voltage Reg Group A */
#define LTC6811_RDCVB_RESERVED (1 << 2) | (1 << 1)                                                /**< Read Cell Voltage Reg Group B */
#define LTC6811_RDCVC_RESERVED (1 << 3)                                                           /**< Read Cell Voltage Reg Group C */
#define LTC6811_RDCVD_RESERVED (1 << 3) | (1 << 1)                                                /**< Read Cell Voltage Reg Group D */
#define LTC6811_RDAUXA_RESERVED ((1 << 3) | (1 << 2))                                             /**< Read Auxillary Reg Group A */
#define LTC6811_RDAUXB_RESERVED ((1 << 3) | (1 << 2)) | (1 << 1)                                  /**< Read Auxillary Reg Group B */
#define LTC6811_RDSTATA_RESERVED (1 << 4)                                                         /**< Read Status Register Group A */
#define LTC6811_RDSTATB_RESERVED (1 << 4) | (1 << 1)                                              /**< Read Status Register Group B */
#define LTC6811_ADCV_RESERVED ((1 << 9) | (1 << 6) | (1 << 5))                                    /**< Start Cell Voltage ADC Conversion and Poll Status */
#define LTC6811_ADOW_RESERVED ((1 << 3) | (1 << 5) | (1 << 9))                                    /**< Start Open Wire ADC Conversion and Poll Status */
#define LTC6811_CVST_RESERVED ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 9))                         /**< Start Self Test Cell Voltage Conversion and Poll Status */
#define LTC6811_ADAX_RESERVED (1 << 10) | (1 << 6) | (1 << 5)                                     /**< Start GPIOs ADC Conversion and Poll Status */
#define LTC6811_CLRCELL_RESERVED (1 << 0) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)            /**< Clear Cell Voltage Register Groups */
#define LTC6811_CLRAUX_RESERVED (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)             /**< Clear Auxillary Register Groups */
#define LTC6811_CLRSTAT_RESERVED (1 << 0) | (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10) /**< Clear Status Register Groups */
#define LTC6811_PLADC_RESERVED (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)              /**< Poll ADC Conversion Status */
#define LTC6811_DIAGN_RESERVED (1 << 0) | (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)   /**< Diagnose MUX and Poll Status */
#define LTC6811_WRCOMM_RESERVED (1 << 0) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)             /**< Write COMM Register Group */
#define LTC6811_RDCOMM_RESERVED (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)             /**< Read COMM Register Group */
#define LTC6811_STCOMM_RESERVED (1 << 0) | (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)
#define LTC6811_WRPWM_RESERVED (1 << 5)            /**< Write PWM Register Group */
#define LTC6811_RDPWM_RESERVED (1 << 5) | (1 << 2) /**< Read PWM Register Group */

/**
 * @brief GPIO pull-down configuration for the Config Register Group.
 * @note  See Table 40 (p.62) and `LtcAfeConfigRegisterData` struct for details.
 */
#define LTC6811_GPIO1_PD_ON (0 << 3)
#define LTC6811_GPIO1_PD_OFF (1 << 3)
#define LTC6811_GPIO2_PD_ON (0 << 4)
#define LTC6811_GPIO2_PD_OFF (1 << 4)
#define LTC6811_GPIO3_PD_ON (0 << 5)
#define LTC6811_GPIO3_PD_OFF (1 << 5)
#define LTC6811_GPIO4_PD_ON (0 << 6)
#define LTC6811_GPIO4_PD_OFF (1 << 6)
#define LTC6811_GPIO5_PD_ON (0 << 7)
#define LTC6811_GPIO5_PD_OFF (1 << 7)

/**
 * @brief   ADCV command macros for cell conversion.
 * @details Determines which cells to convert during ADCV commands.
 */
#define LTC6811_CNVT_CELL_ALL 0x00  /**< Convert all cells. */
#define LTC6811_CNVT_CELL_1_7 0x01  /**< Convert cells 1 and 7. */
#define LTC6811_CNVT_CELL_2_8 0x02  /**< Convert cells 2 and 8. */
#define LTC6811_CNVT_CELL_3_9 0x03  /**< Convert cells 3 and 9. */
#define LTC6811_CNVT_CELL_4_10 0x04 /**< Convert cells 4 and 10. */
#define LTC6811_CNVT_CELL_5_11 0x05 /**< Convert cells 5 and 11. */
#define LTC6811_CNVT_CELL_6_12 0x06 /**< Convert cells 6 and 12. */

/**
 * @brief   ADCV discharge permission macros.
 * @details Determines if cell discharge is permitted during ADCV commands.
 */
#define LTC6811_ADCV_DISCHARGE_NOT_PERMITTED (0 << 4)
#define LTC6811_ADCV_DISCHARGE_PERMITTED (1 << 4)

/** @brief ADAX command macros */
#define LTC6811_ADAX_GPIO1 0x01                    /**< Select GPIO 1 for ADC Conversion */
#define LTC6811_ADAX_GPIO4 0x04                    /**< Select GPIO 4 for ADC Conversion */
#define LTC6811_ADAX_MODE_FAST (0 << 8) | (1 << 7) /**< Select fast ADC mode */

/** @brief Write Codes for ICOMn (n = 1, 2, 3) */
#define LTC6811_ICOM_CSBM_LOW (1 << 3)                                     /**< CSBM low signal */
#define LTC6811_ICOM_CSBM_FALL_EDGE (1 << 3) | (1 << 0)                    /**< CSBM high then low signal*/
#define LTC6811_ICOM_CSBM_HIGH (1 << 3) | (1 << 1)                         /**< CSBM high signal */
#define LTC6811_ICOM_NO_TRANSMIT (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0) /**< No Data is tranmitted*/

/** @brief Write Codes for FCOMn (n = 1, 2, 3) */
#define LTC6811_FCOM_CSBM_LOW (0 << 0)             /**< Holds CSBM low at end of transmission */
#define LTC6811_FCOM_CSBM_HIGH (1 << 3) | (1 << 0) /**< Transitions CSBM high at end of transmission */

/**
 * @brief PWM duty cycle configuration (100%).
 * @note See Table 17 (p.38) for details.
 */
#define LTC6811_PWMC_DC_100 (0xF)

/** @} */
