/************************************************************************************************
 * @file   main.c
 *
 * @brief  Smoke test for adbms1818_spi
 *
 * @date   2025-01-27
 * @author Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <string.h>

/* Inter-component Headers */
#include "delay.h"
#include "gpio.h"
#include "log.h"
#include "mcu.h"
#include "spi.h"
#include "status.h"
#include "tasks.h"

#include "adbms_afe_regs.h"
#include "adbms_afe_crc15.h"

/* Intra-component Headers */

SpiSettings spi_test_settings = {
  .baudrate = SPI_BAUDRATE_625KHZ,
  .mode = SPI_MODE_3,
  .sdo = { .port = GPIO_PORT_B, .pin = 15 },
  .sdi = { .port = GPIO_PORT_B, .pin = 14 },
  .sclk = { .port = GPIO_PORT_B, .pin = 13 },
  .cs = { .port = GPIO_PORT_B, .pin = 12 },
};

void wakeup() {
  gpio_set_state(&spi_test_settings.cs, GPIO_STATE_LOW);
  delay_ms(1);
  gpio_set_state(&spi_test_settings.cs, GPIO_STATE_HIGH);
  delay_ms(1);
}

void build_cmd(uint16_t cmd_val, uint8_t *buf) {
  buf[0] = (uint8_t)(cmd_val >> 8);
  buf[1] = (uint8_t)(cmd_val & 0xFF);
  uint16_t crc = crc15_calculate(buf, 2);
  buf[2] = (uint8_t)(crc >> 8);
  buf[3] = (uint8_t)(crc & 0xFF);
}

void write_config_register(uint16_t cmd, const uint8_t *data) {
  uint8_t tx_packet[12]; // CMD(4) + DATA(6) + PEC(2)
  
  // Build command with PEC
  build_cmd(cmd, tx_packet);
  
  // Copy data
  memcpy(&tx_packet[4], data, 6);
  
  // Calculate and append data PEC
  uint16_t data_pec = crc15_calculate(data, 6);
  tx_packet[10] = (uint8_t)(data_pec >> 8);
  tx_packet[11] = (uint8_t)(data_pec & 0xFF);

  // Send complete packet
  spi_exchange(SPI_PORT_2, tx_packet, 12, NULL, 0);
}

TASK(adbms1818_spi, TASK_STACK_1024) {
    crc15_init_table();
    spi_init(SPI_PORT_2, &spi_test_settings);

    LOG_DEBUG("ADBMS1818 Config + ADAX + Full AUX + CFG Readback Test\r\n");

    uint8_t rx_aux[24]   = {0};  
    uint8_t rx_cfgA[8]   = {0};  
    uint8_t rx_cfgB[8]   = {0};
    uint8_t cmd[4] = {0};

    // /* REFON = 1, DTEN = 1, ADCOPT = 0, GPIO = 0x1F (all pull-downs off) */
    // uint8_t cfgA_data[6] = {0xFE, 0x00, 0x00, 0x00, 0x01, 0x10};
    
    // /* GPIO = 0x0F, all other fields zero */
    // uint8_t cfgB_data[6] = {0x1F, 0x00, 0x00, 0x00, 0x00, 0x00};

    // Start with just DTEN=1, ADCOPT=0, GPIO pull-downs on (0s)
    uint8_t cfgA_data[6] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
    // CFGAR0: GPIO[5:1]=00000, REFON=0, DTEN=1, ADCOPT=0

    uint8_t cfgB_data[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    while (true) {

        /* --- Write CFG A with proper PEC --- */
        wakeup();
        write_config_register(ADBMS1818_WRCFGA_RESERVED, cfgA_data);
        delay_ms(100);
        LOG_DEBUG("Writing CFG A: %02X %02X %02X %02X %02X %02X\r\n",
                  cfgA_data[0], cfgA_data[1], cfgA_data[2], 
                  cfgA_data[3], cfgA_data[4], cfgA_data[5]);

        /* --- Write CFG B with proper PEC --- */
        wakeup();
        write_config_register(ADBMS1818_WRCFGB_RESERVED, cfgB_data);
        delay_ms(100);
        LOG_DEBUG("Writing CFG B: %02X %02X %02X %02X %02X %02X\r\n",
                  cfgB_data[0], cfgB_data[1], cfgB_data[2],
                  cfgB_data[3], cfgB_data[4], cfgB_data[5]);

        /* --- Read back CFG A --- */
        wakeup();
        build_cmd(ADBMS1818_RDCFGA_RESERVED, cmd);
        spi_exchange(SPI_PORT_2, cmd, 4, rx_cfgA, 8);
        delay_ms(10);

        /* --- Read back CFG B --- */
        wakeup();
        build_cmd(ADBMS1818_RDCFGB_RESERVED, cmd);
        spi_exchange(SPI_PORT_2, cmd, 4, rx_cfgB, 8);
        delay_ms(10);

        /* --- Verify PEC on readback --- */
        uint16_t cfgA_pec_calc = crc15_calculate(rx_cfgA, 6);
        uint16_t cfgA_pec_recv = (rx_cfgA[6] << 8) | rx_cfgA[7];
        bool cfgA_pec_ok = (cfgA_pec_calc == cfgA_pec_recv);
        
        uint16_t cfgB_pec_calc = crc15_calculate(rx_cfgB, 6);
        uint16_t cfgB_pec_recv = (rx_cfgB[6] << 8) | rx_cfgB[7];
        bool cfgB_pec_ok = (cfgB_pec_calc == cfgB_pec_recv);

        /* --- Display Results --- */
        LOG_DEBUG("CFG A Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X] %s\r\n",
                  rx_cfgA[0], rx_cfgA[1], rx_cfgA[2], rx_cfgA[3], rx_cfgA[4], rx_cfgA[5], 
                  rx_cfgA[6], rx_cfgA[7], cfgA_pec_ok ? "OK" : "FAIL");
        delay_ms(10);

        LOG_DEBUG("CFG B Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X] %s\r\n",
                  rx_cfgB[0], rx_cfgB[1], rx_cfgB[2], rx_cfgB[3], rx_cfgB[4], rx_cfgB[5],
                  rx_cfgB[6], rx_cfgB[7], cfgB_pec_ok ? "OK" : "FAIL");
        delay_ms(10);

        /* --- Compare Write vs Read --- */
        bool cfgA_match = (memcmp(cfgA_data, rx_cfgA, 6) == 0);
        bool cfgB_match = (memcmp(cfgB_data, rx_cfgB, 6) == 0);
        
        LOG_DEBUG("CFG A: %s\r\n", cfgA_match ? "WRITE == READ" : "MISMATCH!");
        delay_ms(10);
        LOG_DEBUG("CFG B: %s\r\n", cfgB_match ? "WRITE == READ" : "MISMATCH!");
        delay_ms(10);

        /* --- Trigger ADAX --- */
        wakeup();
        build_cmd(ADBMS1818_ADAX_RESERVED | ADBMS1818_ADAX_GPIO_ALL | (0b10 << 7), cmd);
        spi_exchange(SPI_PORT_2, cmd, 4, NULL, 0);
        delay_ms(10);

        /* --- Read AUX A/B/C --- */
        memset(rx_aux, 0xAA, 24);
        
        wakeup();
        build_cmd(ADBMS1818_RDAUXA_RESERVED, cmd);
        spi_exchange(SPI_PORT_2, cmd, 4, &rx_aux[0], 8);
        delay_ms(10);

        wakeup();
        build_cmd(ADBMS1818_RDAUXB_RESERVED, cmd);
        spi_exchange(SPI_PORT_2, cmd, 4, &rx_aux[8], 8);
        delay_ms(10);

        wakeup();
        build_cmd(ADBMS1818_RDAUXC_RESERVED, cmd);
        spi_exchange(SPI_PORT_2, cmd, 4, &rx_aux[16], 8);
        delay_ms(10);

        LOG_DEBUG("AUX A Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X]\r\n",
                  rx_aux[0], rx_aux[1], rx_aux[2], rx_aux[3], rx_aux[4], rx_aux[5], 
                  rx_aux[6], rx_aux[7]);
        delay_ms(10);

        LOG_DEBUG("AUX B Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X]\r\n",
                  rx_aux[8], rx_aux[9], rx_aux[10], rx_aux[11], rx_aux[12], rx_aux[13],
                  rx_aux[14], rx_aux[15]);
        delay_ms(10);

        LOG_DEBUG("AUX C Read:  %02X %02X %02X %02X %02X %02X [PEC: %02X %02X]\r\n",
                  rx_aux[16], rx_aux[17], rx_aux[18], rx_aux[19], rx_aux[20], rx_aux[21],
                  rx_aux[22], rx_aux[23]);
        delay_ms(10);

        delay_ms(2000);
    }
}

#ifdef MS_PLATFORM_X86
#include "mpxe.h"
int main(int argc, char *argv[]) {
  mpxe_init(argc, argv);
#else
int main() {
#endif
  mcu_init();
  tasks_init();
  log_init();

  tasks_init_task(adbms1818_spi, TASK_PRIORITY(3), NULL);

  tasks_start();

  LOG_DEBUG("exiting main?");
  return 0;
}
