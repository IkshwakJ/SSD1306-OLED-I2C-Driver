#ifdef SSD1306_USE_ESP_ARDUINO

#include "ssd1306_platform.h"
#include <Wire.h>
#include <Arduino.h>

static ssd1306_platform_t ctx;

void ssd1306_platform_init(TwoWire *wire, uint8_t addr)
{
    ctx.wire = wire;
    ctx.i2c_addr = static_cast<uint8_t>(addr); // store 7-bit I2C address
}

bool ssd1306_platform_write_command(uint8_t cmd)
{
    ctx.wire->beginTransmission(ctx.i2c_addr);
    ctx.wire->write(0x00);  // Control byte for command
    ctx.wire->write(cmd);
    return ctx.wire->endTransmission() == 0;
}

bool ssd1306_platform_write_multi_command(const uint8_t *cmd, uint16_t size)
{
    if (size == 0){
        return true;
    }
    uint8_t control = 0x00;
    uint16_t sent = 0;

    // First byte with control byte 0x00.
    ctx.wire->beginTransmission(ctx.i2c_addr);
    ctx.wire->write(control);
    control = 0x80;
    ctx.wire->write(cmd[sent++]);
    ctx.wire->endTransmission((size > 1 ? false : true) != 0);
    while (sent < size) {
        ctx.wire->beginTransmission(ctx.i2c_addr);
        ctx.wire->write(control);

        // Send up to 16 bytes at a time for compatibility
        uint16_t chunk = (size - sent > 16) ? 16 : (size - sent);
        ctx.wire->write(cmd + sent, chunk);
        sent += chunk;

        if (ctx.wire->endTransmission(sent < size) != 0) {
            return false;
        }
    }

    // Final STOP
    return true;
}

bool ssd1306_platform_write_data(const uint8_t *data, size_t size)
{
    const uint8_t control = 0x40;
    uint16_t sent = 0;
    while (sent < size) {
        ctx.wire->beginTransmission(ctx.i2c_addr);
        ctx.wire->write(control);

        // Send up to 16 bytes at a time for compatibility
        uint16_t chunk = (size - sent > 16) ? 16 : (size - sent);
        ctx.wire->write(data + sent, chunk);
        sent += chunk;

        if (ctx.wire->endTransmission(sent < size) != 0) {
            return false;
        }
    }

    // Final STOP
    return true;
}

bool ssd1306_platform_start_data_dma(const uint8_t *data, size_t size)
{
    // Arduino Wire library doesn't support non-blocking DMA,
    // fallback to blocking write.
    return ssd1306_platform_write_data(data, size);
}

bool ssd1306_platform_is_dma_done()
{
    // Always done since we use blocking transfer
    return true;
}

bool ssd1306_platform_delay_us(uint32_t us)
{
    delayMicroseconds(us);
    return true;
}
#endif // SSD1306_USE_ESP_ARDUINO





// The following is for the ESP32 on the ESP-IDF.



#ifdef SSD1306_USE_ESP_IDF

#include "ssd1306_platform.h"
#include "driver/i2c.h"
#include "esp_rom/ets_sys.h"  // for esp_rom_delay_us
#include <string.h>           // for memcpy

static ssd1306_platform_t ctx;

void ssd1306_platform_init(i2c_port_t i2c_port, uint8_t addr)
{
    ctx.i2c_port = i2c_port;
    ctx.i2c_addr = addr;  // store 7-bit address
}

static bool i2c_write(uint8_t control_byte, const uint8_t *data, size_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    if (!cmd) return false;

    esp_err_t res = ESP_OK;
    res |= i2c_master_start(cmd);
    res |= i2c_master_write_byte(cmd, (ctx.i2c_addr << 1) | I2C_MASTER_WRITE, true);
    res |= i2c_master_write_byte(cmd, control_byte, true);
    if (size > 0) {
        res |= i2c_master_write(cmd, (uint8_t *)data, size, true);
    }
    res |= i2c_master_stop(cmd);
    res |= i2c_master_cmd_begin(ctx.i2c_port, cmd, pdMS_TO_TICKS(100));
    i2c_cmd_link_delete(cmd);

    return res == ESP_OK;
}

bool ssd1306_platform_write_command(uint8_t cmd)
{
    return i2c_write(0x00, &cmd, 1); // Co = 0, D/C# = 0
}

bool ssd1306_platform_write_multi_command(const uint8_t *cmd, uint16_t size)
{
    if (size == 0) return true;
    if (size == 1) return ssd1306_platform_write_command(cmd[0]);

    // Send first byte with control = 0x00
    if (!i2c_write(0x00, &cmd[0], 1)) return false;

    // Send remaining bytes with control = 0x80 (continuation)
    return i2c_write(0x80, &cmd[1], size - 1);
}

bool ssd1306_platform_write_data(const uint8_t *data, size_t size)
{
    const uint8_t control = 0x40; // Co = 0, D/C# = 1
    size_t sent = 0;

    while (sent < size) {
        size_t chunk = (size - sent > 16) ? 16 : (size - sent); // safe I2C size
        if (!i2c_write(control, data + sent, chunk)) return false;
        sent += chunk;
    }

    return true;
}

bool ssd1306_platform_start_data_dma(const uint8_t *data, size_t size)
{
    // ESP-IDF I2C driver doesn't support DMA for I2C master as user-controlled
    return ssd1306_platform_write_data(data, size);
}

bool ssd1306_platform_is_dma_done()
{
    return true; // Always complete in blocking mode
}

bool ssd1306_platform_delay_us(uint32_t us)
{
    esp_rom_delay_us(us);
    return true;
}

#endif // SSD1306_USE_ESP_IDF