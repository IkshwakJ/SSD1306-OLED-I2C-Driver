#ifdef SSD1306_USE_STM32

#include "ssd1306_platform.h"
#include "stm32f1xx_hal.h" // Or adjust to your MCU family

#ifndef SSD1306_DELAY_TIMER
    #define SSD1306_DELAY_TIMER TIM14 //  change this to your prefered clock.
#endif

#if SSD1306_DELAY_TIMER == TIM6
    #define SSD1306_ENABLE_TIMER_CLOCK() __HAL_RCC_TIM6_CLK_ENABLE()
#elif SSD1306_DELAY_TIMER == TIM7
    #define SSD1306_ENABLE_TIMER_CLOCK() __HAL_RCC_TIM7_CLK_ENABLE()
#elif SSD1306_DELAY_TIMER == TIM14
    #define SSD1306_ENABLE_TIMER_CLOCK() __HAL_RCC_TIM14_CLK_ENABLE()
#else
    #error "Define SSD1306_ENABLE_TIMER_CLOCK() for your selected SSD1306_DELAY_TIMER"
#endif

static ssd1306_platform_t ctx;
extern volatile bool ssd1306_dma_done; // Define and update this in your DMA complete callback
static TIM_HandleTypeDef htim_delay;
static bool delay_timer_initialized = false;


void ssd1306_platform_init(I2C_HandleTypeDef *hi2c, DMA_HandleTypeDef *hdma_tx, uint8_t addr){
    ctx.hi2c     = hi2c;
    ctx.hdma_tx  = hdma_tx;
    ctx.i2c_addr = (uint8_t)(addr << 1); // HAL expects 8-bit address (7-bit << 1)
}

bool ssd1306_platform_write_command(uint8_t cmd){
    uint8_t control = 0x00; // Co = 0, D/C# = 0
    return HAL_I2C_Mem_Write(ctx.hi2c, ctx.i2c_addr, control, I2C_MEMADD_SIZE_8BIT, &cmd, 1, HAL_MAX_DELAY) == HAL_OK;
}

bool ssd1306_platform_write_multi_command(const uint8_t *cmd, uint16_t size){
    if (size == 0) return true;

    // Send first command byte with 0x00
    if (HAL_I2C_Mem_Write(ctx.hi2c, ctx.i2c_addr, 0x00, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&cmd[0], 1, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    if (size == 1){ 
        return true;
    }
    
    // Send remaining command bytes with 0x80 (continuation)
    return HAL_I2C_Mem_Write(ctx.hi2c, ctx.i2c_addr, 0x80, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&cmd[1], size - 1, HAL_MAX_DELAY) == HAL_OK;
}

bool ssd1306_platform_write_data(const uint8_t *data, size_t size){
    uint8_t control = 0x40; // Co = 0, D/C# = 1
    return HAL_I2C_Mem_Write(ctx.hi2c, ctx.i2c_addr, control, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, size, HAL_MAX_DELAY) == HAL_OK;
}

bool ssd1306_platform_start_data_dma(const uint8_t *data, size_t size){
    ssd1306_dma_done = false;
    uint8_t control = 0x40;
    return HAL_I2C_Mem_Write_DMA(ctx.hi2c, ctx.i2c_addr, control, I2C_MEMADD_SIZE_8BIT, (uint8_t *)data, size) == HAL_OK;
}

/**
 * @brief  Starts a DMA transfer of data to the SSD1306.
 *
 * NOTE:
 * - Before calling this, make sure DMA is properly initialized and linked to I2C TX.
 * - You must define and manage the `ssd1306_dma_done` flag.
 *
 * Example for your I2C DMA complete callback:
 *
 * void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c) {
 *     if (hi2c == ssd1306_ctx.hi2c) {
 *         ssd1306_dma_done = true;
 *     }
 * }
 */
bool ssd1306_platform_is_dma_done(){
    return ssd1306_dma_done;
}

bool ssd1306_platform_delay_us(uint32_t us){
    if(!delay_timer_initialized){
        SSD1306_ENABLE_TIMER_CLOCK();
        htim_delay.Instance = SSD1306_DELAY_TIMER;
        htim_delay.Init.Prescaler = (SystemCoreClock + 500000) / 1000000 - 1;
        htim_delay.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim_delay.Init.Period = 0xFFFF;
        htim_delay.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        htim_delay.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

        HAL_TIM_Base_Init(&htim_delay);
        HAL_TIM_Base_Start(&htim_delay);

        delay_timer_initialized = true;
    }

    while (us > 60000){
        __HAL_TIM_SET_COUNTER(&htim_delay, 0);
        while (__HAL_TIM_GET_COUNTER(&htim_delay) < 60000);
        us -= 60000;
    }
    __HAL_TIM_SET_COUNTER(&htim_delay, 0);
    while(__HAL_TIM_GET_COUNTER(&htim_delay)<us);
    return true;
}

#endif // SSD1306_USE_STM32