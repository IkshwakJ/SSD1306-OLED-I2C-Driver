/*
*   ssd1306_platform.h
*   Created on 06/29/2025
*   Author Ikshwak Jinesh
*/
#ifndef SSD1306_PLATFORM_H
#define SSD1306_PLATFORM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ensure that only one of the following three defines are uncommented. 
 *        This is to select which platform to use the driver on.
 */
// #define SSD1306_USE_STM32
// #define SSD1306_USE_ESP_ARDUINO
// #define SSD1306_USE_ESP_IDF

/**
 * @brief Platform-specific context for SSD1306 driver
 *
 * Members are only valid under the matching macro:
 *  - SSD1306_USE_STM32: hi2c, hdma_tx, i2c_addr
 *  - SSD1306_USE_ESP_ARDUINO: wire, i2c_addr
 */
typedef struct {
#ifdef SSD1306_USE_STM32
    I2C_HandleTypeDef *hi2c;    /**< HAL I2C handle pointer */
    DMA_HandleTypeDef *hdma_tx; /**< HAL DMA handle pointer */
    uint8_t           i2c_addr;/**< 8‑bit address (7‑bit<<1) */
#endif

#ifdef SSD1306_USE_ESP_ARDUINO
    TwoWire          *wire;     /**< Arduino Wire instance */
    uint8_t          i2c_addr; /**< 7‑bit I2C address */
#endif

#ifdef SSD1306_USE_ESP_IDF
    i2c_port_t        i2c_port; /**< ESP-IDF I2C port number */
    uint8_t           i2c_addr; /**< 7‑bit I2C address */
#endif

} ssd1306_platform_t;

/**
 * @brief Initialize the platform context
 * On STM32: pass hi2c, hdma_tx, and (7‑bit) addr.
 * On ESP Arduino: pass wire and (7‑bit) addr.
 */
void ssd1306_platform_init(
                            #ifdef SSD1306_USE_STM32
                                I2C_HandleTypeDef *hi2c,
                                DMA_HandleTypeDef *hdma_tx,
                            #endif
                            #ifdef SSD1306_USE_ESP_ARDUINO
                                TwoWire *wire,
                            #endif
                            #ifdef SSD1306_USE_ESP_IDF
                                i2c_port_t i2c_port,
                            #endif
                            uint8_t addr
                            );

bool ssd1306_platform_write_command(uint8_t cmd);
bool ssd1306_platform_write_multi_command(const uint8_t *cmd, uint16_t size);
bool ssd1306_platform_write_data(const uint8_t *data, uint16_t size);
bool ssd1306_platform_start_data_dma(const uint8_t *data, uint16_t size);
bool ssd1306_platform_is_dma_done();
bool ssd1306_platform_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif // SSD1306_PLATFORM_H