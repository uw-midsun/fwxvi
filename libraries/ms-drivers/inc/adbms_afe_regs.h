#pragma once

/************************************************************************************************
 * @file   adbms_afe_regs.h
 *
 * @brief  Header file to define the ADBMS1818 AFE's register layout, configuration, and command code structures
 *
 * @date   2025-04-13
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <assert.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup ADBMS1818
 * @brief    ADBMS1818 library
 * @{
 */

/** @brief Stores all member variables in a struct without padding */
#if defined(__GNUC__)
#define _PACKED __attribute__((packed))
#else
#define _PACKED
#endif

/** @brief  Used internally by the ADBMS AFE driver (number of regs per reg group) */
#define ADBMS1818_CELLS_IN_REG 3
#define ADBMS1818_GPIOS_IN_REG 3

/** @brief  Used for the external mux (ADG731) connected to the AFE */
#define AUX_ADG731_NUM_PINS 32

/** @brief  Size of command code (2 bytes) + PEC (2 bytes) */
#define ADBMS1818_CMD_SIZE 4

/** @brief  3 bytes are required to send 24 clock cycles with our SPI driver for the STCOMM command */
#define ADBMS1818_NUM_COMM_REG_BYTES 3

/** @brief  Number of registers in PWM Register Group. See Table 70 on page 68. */
#define ADBMS1818_NUM_PWMR_REGS 6

/** @brief  Macros for PWM/S control register group B */
#define ADBMS1818_NUM_PSR_REGS 6         /**< Number of registers in PWM/S Control register group B */
#define ADBMS1818_NUM_PWM_REGS_IN_PSR 3  /**< Number of PWM registers in the PWM/S control register group B */
#define ADBMS1818_NUM_SCTL_REGS_IN_PSR 3 /**< Number of SCTL registers in the PWM/S control register group B */

/** @brief  Number of registers in PWM/S Register Group. See Table 71 on page 68. */
#define ADBMS1818_NUM_PWMS_REGS 6

/** @brief Number of voltage cells connected to each device */
#define ADBMS_AFE_MAX_CELLS_PER_DEVICE 18
/**
 * @brief List of ADBMS AFE registers
 * @details These registers are mapped to the `s_read_reg[]` array in `adbms_afe.c`
 */
typedef enum {
  ADBMS_AFE_REGISTER_CONFIG_A = 0,   /**< Configuration register A */
  ADBMS_AFE_REGISTER_CONFIG_B,       /**< Configuration register B */
  ADBMS_AFE_REGISTER_CELL_VOLTAGE_A, /**< Cell voltage register A */
  ADBMS_AFE_REGISTER_CELL_VOLTAGE_B, /**< Cell voltage register B */
  ADBMS_AFE_REGISTER_CELL_VOLTAGE_C, /**< Cell voltage register C */
  ADBMS_AFE_REGISTER_CELL_VOLTAGE_D, /**< Cell voltage register D */
  ADBMS_AFE_REGISTER_CELL_VOLTAGE_E, /**< Cell voltage register E */
  ADBMS_AFE_REGISTER_CELL_VOLTAGE_F, /**< Cell voltage register F */
  ADBMS_AFE_REGISTER_AUX_A,          /**< Auxiliary register A */
  ADBMS_AFE_REGISTER_AUX_B,          /**< Auxiliary register B */
  ADBMS_AFE_REGISTER_AUX_C,          /**< Auxiliary register C */
  ADBMS_AFE_REGISTER_AUX_D,          /**< Auxiliary register D */
  ADBMS_AFE_REGISTER_STATUS_A,       /**< Status register A */
  ADBMS_AFE_REGISTER_STATUS_B,       /**< Status register B */
  ADBMS_AFE_REGISTER_READ_COMM,      /**< Read communication register */
  ADBMS_AFE_REGISTER_START_COMM,     /**< Start communication register */
  NUM_ADBMS_AFE_REGISTERS            /**< Total number of ADBMS AFE registers */
} AdbmsAfeRegister;

/** @brief Voltage registers for easy mapping to `AdbmsAfeRegister` */
typedef enum {
  ADBMS_AFE_VOLTAGE_REGISTER_A = ADBMS_AFE_REGISTER_CELL_VOLTAGE_A,
  ADBMS_AFE_VOLTAGE_REGISTER_B = ADBMS_AFE_REGISTER_CELL_VOLTAGE_B,
  ADBMS_AFE_VOLTAGE_REGISTER_C = ADBMS_AFE_REGISTER_CELL_VOLTAGE_C,
  ADBMS_AFE_VOLTAGE_REGISTER_D = ADBMS_AFE_REGISTER_CELL_VOLTAGE_D,
  ADBMS_AFE_VOLTAGE_REGISTER_E = ADBMS_AFE_REGISTER_CELL_VOLTAGE_E,
  ADBMS_AFE_VOLTAGE_REGISTER_F = ADBMS_AFE_REGISTER_CELL_VOLTAGE_F,
  NUM_ADBMS_AFE_VOLTAGE_REGISTERS
} AdbmsAfeVoltageRegister;

/**
 * @brief Discharge timeout options for the AFE
 * @details See Table 72 on Page 68 for more details
 */
typedef enum {
  ADBMS_AFE_DISCHARGE_TIMEOUT_DISABLED = 0, /**< Discharge timeout disabled */
  ADBMS_AFE_DISCHARGE_TIMEOUT_30_S,         /**< 30-second timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_1_MIN,        /**< 1-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_2_MIN,        /**< 2-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_3_MIN,        /**< 3-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_4_MIN,        /**< 4-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_5_MIN,        /**< 5-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_10_MIN,       /**< 10-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_15_MIN,       /**< 15-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_20_MIN,       /**< 20-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_30_MIN,       /**< 30-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_40_MIN,       /**< 40-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_60_MIN,       /**< 60-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_75_MIN,       /**< 75-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_90_MIN,       /**< 90-minute timeout */
  ADBMS_AFE_DISCHARGE_TIMEOUT_120_MIN       /**< 120-minute timeout */
} AdbmsAfeDischargeTimeout;

/**
 * @brief   Configuration for the AFE configuration register group A (CFGRA).
 * @details The configuration includes:
 *          - ADC Mode Option Bit (`adcopt`):             Determines ADC conversion modes
 *          - Discharge Timer Enable (`dten`):            Enable/disable discharge timer
 *          - References Powered Up (`refon`):            Keeps references powered up until watchdog timeout
 *          - GPIO Pin Pull-Down Config (`gpio`):         Configures GPIO pull-downs
 *          - Undervoltage Threshold (`undervoltage`):    Sets undervoltage threshold
 *          - Overvoltage Threshold (`overvoltage`):      Sets overvoltage threshold
 *          - Discharge Cell Bitset (`discharge_bitset`): Enables/disables discharge for specific cells
 *          - Discharge Timeout (`discharge_timeout`):    Sets discharge timeout duration
 * @note    See Table 54 on Page 65 for the exact layout.
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
} _PACKED AdbmsAfeConfigRegisterAData;
static_assert(sizeof(AdbmsAfeConfigRegisterAData) == 6, "AdbmsAfeConfigRegisterAData must be 6 bytes");

/**
 * @brief   Configuration for the AFE configuration register group B (CFGRB).
 * @details The configuration includes:
 *          - GPIO Pin Pull-Down Config (`gpio`):                         Configures GPIO pull-downs
 *          - Discharge Cell Bitset (`discharge_bitset`):                 Enables/disables discharge for specific cells
 *          - Discharge Timeout (`discharge_timeout`):                    Sets discharge timeout duration
 *          - Digital redundancy path selection (`path_select`):          Selects which ADC has redundancy applied
 *          - Enable discharge timer monitor (`discharge_timer_monitor`): Enable/disable discharge timer monitor function
 *          - Force Digital redundancy failure (`force_fail`):            Force digital redundancy comparison for ADC conv to fail (if 1) else normal
 *          - Mute status (`mute`):                                       1: Mute is activated discharging is disabled, 0: mute is deactivated
 *          - Reserved bits (`reservedx`):                                Placeholder (doesn't really do anything)
 * @note    See Table 55 on Page 65 for the exact layout.
 */
typedef struct {
  uint8_t gpio : 4; /**< GPIO PD Config */

  uint8_t discharge_bitset : 7;        /**< Discharge Cell Bitset */
  uint8_t discharge_timer_monitor : 1; /**< Discharge cell timer monitor duration */

  uint8_t path_select : 2; /**< Digital redundancy path selection */
  uint8_t force_fail : 1;  /**< Force fail digital redundancy comparison */

  uint8_t mute : 1; /**< Mute status */

  uint8_t reserved1;
  uint8_t reserved2;
  uint8_t reserved3;
  uint8_t reserved4;
} _PACKED AdbmsAfeConfigRegisterBData;
static_assert(sizeof(AdbmsAfeConfigRegisterBData) == 6, "AdbmsAfeConfigRegisterBData must be 6 bytes");

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
} _PACKED AdbmsAfeCommRegisterData;
static_assert(sizeof(AdbmsAfeCommRegisterData) == 6, "AdbmsAfeCommRegisterData must be 6 bytes");

/** @brief WRCOMM + mux pin */
typedef struct {
  uint8_t wrcomm[ADBMS1818_CMD_SIZE];
  AdbmsAfeCommRegisterData reg;
  uint8_t pec;
} _PACKED AdbmsAfeWriteCommRegPacket;

/** @brief STMCOMM + clock cycles */
typedef struct {
  uint8_t stcomm[ADBMS1818_CMD_SIZE];
  uint8_t clk[ADBMS1818_NUM_COMM_REG_BYTES];
} _PACKED AdbmsAfeSendCommRegPacket;

/** @brief Configuration Register Group (CFGAR) packet for EACH device */
typedef struct {
  AdbmsAfeConfigRegisterAData reg;
  uint16_t pec;
} _PACKED AdbmsAfeWriteDeviceConfigAPacket;

/** @brief Configuration Register Group (CFGBxR) packet for EACH device */
typedef struct {
  AdbmsAfeConfigRegisterAData cfgA;
  uint16_t pecA;

  AdbmsAfeConfigRegisterBData cfgB;
  uint16_t pecB;
} _PACKED AdbmsAfeWriteDeviceConfigPacket;

/**
 * @brief WRCFG + all slave registers
 * @note  Devices are ordered with the last slave first */
typedef struct {
  uint8_t wrcfg[ADBMS1818_CMD_SIZE]; /**< Command for writing onto config register */

  AdbmsAfeWriteDeviceConfigPacket devices[ADBMS_AFE_MAX_CELLS_PER_DEVICE]; /**< Config for EACH device */
} _PACKED AdbmsAfeWriteConfigPacket;
#define SIZEOF_ADBMS_AFE_WRITE_CONFIG_PACKET(devices) (ADBMS1818_CMD_SIZE + (devices) * sizeof(AdbmsAfeWriteDeviceConfigPacket))

typedef union {
  uint16_t voltages[3]; /**< 3 voltage readings stored as 16-bit values */
  uint8_t values[6];    /**< Byte-wise view for SPI transmission */
} AdbmsAfeRegisterData;
static_assert(sizeof(AdbmsAfeRegisterData) == 6, "AdbmsAfeRegisterData must be 6 bytes");

typedef struct {
  AdbmsAfeRegisterData reg;
  uint16_t pec;
} _PACKED AdbmsAfeVoltageData;
static_assert(sizeof(AdbmsAfeVoltageData) == 8, "AdbmsAfeVoltageData must be 8 bytes");

typedef struct {
  AdbmsAfeRegisterData reg;
  uint16_t pec;
} _PACKED AdbmsAfeAuxData;
static_assert(sizeof(AdbmsAfeAuxData) == 8, "AdbmsAfeAuxData must be 8 bytes");

/**
 * @brief Command codes for ADBMS1818 AFE
 * @details  See Table 52 (p.61)
 */
#define ADBMS1818_WRCFGA_RESERVED (1 << 0)                       /**< Write Config Reg Group A */
#define ADBMS1818_WRCFGB_RESERVED (1 << 5) | (1 << 2)            /**< Write Config Reg Group B */
#define ADBMS1818_RDCFGA_RESERVED (1 << 1)                       /**< Read Config Reg Group A */
#define ADBMS1818_RDCFGB_RESERVED (1 << 5) | (1 << 2) | (1 << 1) /**< Read Config Reg Group B */

#define ADBMS1818_RDCVA_RESERVED (1 << 2)                       /**< Read Cell Voltage Reg Group A */
#define ADBMS1818_RDCVB_RESERVED (1 << 2) | (1 << 1)            /**< Read Cell Voltage Reg Group B */
#define ADBMS1818_RDCVC_RESERVED (1 << 3)                       /**< Read Cell Voltage Reg Group C */
#define ADBMS1818_RDCVD_RESERVED (1 << 3) | (1 << 1)            /**< Read Cell Voltage Reg Group D */
#define ADBMS1818_RDCVE_RESERVED (1 << 3) | (1 << 0)            /**< Read Cell Voltage Reg Group E */
#define ADBMS1818_RDCVF_RESERVED (1 << 3) | (1 << 1) | (1 << 0) /**< Read Cell Voltage Reg Group F */

#define ADBMS1818_RDAUXA_RESERVED ((1 << 3) | (1 << 2))                       /**< Read Auxillary Reg Group A */
#define ADBMS1818_RDAUXB_RESERVED ((1 << 3) | (1 << 2)) | (1 << 1)            /**< Read Auxillary Reg Group B */
#define ADBMS1818_RDAUXC_RESERVED ((1 << 3) | (1 << 2)) | (1 << 0)            /**< Read Auxillary Reg Group C */
#define ADBMS1818_RDAUXD_RESERVED ((1 << 3) | (1 << 2)) | (1 << 1) | (1 << 0) /**< Read Auxillary Reg Group D */

#define ADBMS1818_RDSTATA_RESERVED (1 << 4)            /**< Read Status Register Group A */
#define ADBMS1818_RDSTATB_RESERVED (1 << 4) | (1 << 1) /**< Read Status Register Group B */

#define ADBMS1818_WRSCTRL_RESERVED (1 << 4) | (1 << 2)                     /**< Write S Control Register Group */
#define ADBMS1818_WRPWM_RESERVED (1 << 5)                                  /**< Write PWM Register Group */
#define ADBMS1818_WRPSB_RESERVED (1 << 4) | (1 << 3) | (1 << 2)            /**< Write PWM/S Control Register Group B */
#define ADBMS1818_RDSCTRL_RESERVED (1 << 4) | (1 << 2) | (1 << 1)          /**< Read S Control Register Group */
#define ADBMS1818_RDPWM_RESERVED (1 << 5) | (1 << 2)                       /**< Read PWM Register Group */
#define ADBMS1818_RDPSB_RESERVED (1 << 4) | (1 << 3) | (1 << 2) | (1 << 1) /**< Read PWM Register Group */
#define ADBMS1818_STSCTRL_RESERVED (1 << 4) | (1 << 3) | (1 << 0)          /**< Start S Control Pulsing and Poll Status */
#define ADBMS1818_CLRSCTRL_RESERVED (1 << 4) | (1 << 3)                    /**< Clear S COntrol Register Group */

#define ADBMS1818_ADCV_RESERVED ((1 << 9) | (1 << 6) | (1 << 5))                                    /**< Start Cell Voltage ADC Conversion and Poll Status */
#define ADBMS1818_ADOW_RESERVED ((1 << 3) | (1 << 5) | (1 << 9))                                    /**< Start Open Wire ADC Conversion and Poll Status */
#define ADBMS1818_CVST_RESERVED ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 9))                         /**< Start Self Test Cell Voltage Conversion and Poll Status */
#define ADBMS1818_ADAX_RESERVED (1 << 10) | (1 << 6) | (1 << 5)                                     /**< Start GPIOs ADC Conversion and Poll Status */
#define ADBMS1818_CLRCELL_RESERVED (1 << 0) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)            /**< Clear Cell Voltage Register Groups */
#define ADBMS1818_CLRAUX_RESERVED (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)             /**< Clear Auxillary Register Groups */
#define ADBMS1818_CLRSTAT_RESERVED (1 << 0) | (1 << 1) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10) /**< Clear Status Register Groups */
#define ADBMS1818_PLADC_RESERVED (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)              /**< Poll ADC Conversion Status */
#define ADBMS1818_DIAGN_RESERVED (1 << 0) | (1 << 2) | (1 << 4) | (1 << 8) | (1 << 9) | (1 << 10)   /**< Diagnose MUX and Poll Status */
#define ADBMS1818_WRCOMM_RESERVED (1 << 0) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)             /**< Write COMM Register Group */
#define ADBMS1818_RDCOMM_RESERVED (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)             /**< Read COMM Register Group */
#define ADBMS1818_STCOMM_RESERVED (1 << 0) | (1 << 1) | (1 << 5) | (1 << 8) | (1 << 9) | (1 << 10)

/**
 * @brief GPIO pull-down configuration for the Config Register Group.
 * @note  See Table 72 (p.68) and `AdbmsAfeConfigReg  isterXData` struct for details.
 */
#define ADBMS1818_GPIO1_PD_ON (0 << 3)
#define ADBMS1818_GPIO1_PD_OFF (1 << 3)
#define ADBMS1818_GPIO2_PD_ON (0 << 4)
#define ADBMS1818_GPIO2_PD_OFF (1 << 4)
#define ADBMS1818_GPIO3_PD_ON (0 << 5)
#define ADBMS1818_GPIO3_PD_OFF (1 << 5)
#define ADBMS1818_GPIO4_PD_ON (0 << 6)
#define ADBMS1818_GPIO4_PD_OFF (1 << 6)
#define ADBMS1818_GPIO5_PD_ON (0 << 7)
#define ADBMS1818_GPIO5_PD_OFF (1 << 7)
#define ADBMS1818_GPIO6_PD_ON (0 << 0)
#define ADBMS1818_GPIO6_PD_OFF (1 << 0)
#define ADBMS1818_GPIO7_PD_ON (0 << 1)
#define ADBMS1818_GPIO7_PD_OFF (1 << 1)
#define ADBMS1818_GPIO8_PD_ON (0 << 2)
#define ADBMS1818_GPIO8_PD_OFF (1 << 2)
#define ADBMS1818_GPIO9_PD_ON (0 << 3)
#define ADBMS1818_GPIO9_PD_OFF (1 << 3)

/**
 * @brief   ADCV command macros for cell conversion.
 * @details Determines which cells to convert during ADCV commands.
 */
#define ADBMS1818_CNVT_CELL_ALL 0x00     /**< Convert all cells. */
#define ADBMS1818_CNVT_CELL_1_7_13 0x01  /**< Convert cells 1, 7, and 13.  */
#define ADBMS1818_CNVT_CELL_2_8_14 0x02  /**< Convert cells 2, 8, and 14.  */
#define ADBMS1818_CNVT_CELL_3_9_15 0x03  /**< Convert cells 3, 9, and 15.  */
#define ADBMS1818_CNVT_CELL_4_10_16 0x04 /**< Convert cells 4, 10, and 16. */
#define ADBMS1818_CNVT_CELL_5_11_17 0x05 /**< Convert cells 5, 11, and 17. */
#define ADBMS1818_CNVT_CELL_6_12_18 0x06 /**< Convert cells 6, 12, and 18. */

/**
 * @brief   ADCV discharge permission macros.
 * @details Determines if cell discharge is permitted during ADCV commands.
 */
#define ADBMS1818_ADCV_DISCHARGE_NOT_PERMITTED (0 << 4)
#define ADBMS1818_ADCV_DISCHARGE_PERMITTED (1 << 4)

/** @brief ADAX command macros */
#define ADBMS1818_ADAX_GPIO_ALL 0x00                 /**< Convert GPIO1–GPIO5, 2nd reference, and GPIO6–GPIO9. */
#define ADBMS1818_ADAX_GPIO1_6 0x01                  /**< Convert GPIO1 and GPIO6. */
#define ADBMS1818_ADAX_GPIO2_7 0x02                  /**< Convert GPIO2 and GPIO7. */
#define ADBMS1818_ADAX_GPIO3_8 0x03                  /**< Convert GPIO3 and GPIO8. */
#define ADBMS1818_ADAX_GPIO4_9 0x04                  /**< Convert GPIO4 and GPIO9. */
#define ADBMS1818_ADAX_GPIO5 0x05                    /**< Convert GPIO5 only. */
#define ADBMS1818_ADAX_REF2 0x06                     /**< Convert 2nd reference only (VREF2). */
#define ADBMS1818_ADAX_MODE_FAST (0 << 8) | (1 << 7) /**< Select fast ADC mode */

/** @brief Write Codes for ICOMn (n = 1, 2, 3) */
#define ADBMS1818_ICOM_CSBM_LOW (1 << 3)                                     /**< CSBM low signal */
#define ADBMS1818_ICOM_CSBM_FALL_EDGE (1 << 3) | (1 << 0)                    /**< CSBM high then low signal*/
#define ADBMS1818_ICOM_CSBM_HIGH (1 << 3) | (1 << 1)                         /**< CSBM high signal */
#define ADBMS1818_ICOM_NO_TRANSMIT (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0) /**< No Data is tranmitted*/

/** @brief Write Codes for FCOMn (n = 1, 2, 3) */
#define ADBMS1818_FCOM_CSBM_LOW (0 << 0)             /**< Holds CSBM low at end of transmission */
#define ADBMS1818_FCOM_CSBM_HIGH (1 << 3) | (1 << 0) /**< Transitions CSBM high at end of transmission */

/**
 * @brief PWM duty cycle configuration (100%).
 * @note See Table 17 (p.38) for details.
 */
#define ADBMS1818_PWMC_DC_100 (0xF)

/** @} */
