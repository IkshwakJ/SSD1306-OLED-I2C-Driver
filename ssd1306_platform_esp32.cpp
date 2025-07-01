#ifdef SSD1306_USE_ESP_ARDUINO

#include "ssd1306_platform.h"
#include <Wire.h>

static ssd1306_platform_t ctx;

void ssd1306_platform_init(TwoWire *wire, uint16_t addr)
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

#endif // SSD1306_USE_ESP_ARDUINO