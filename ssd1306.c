/*
*   ssd1306.c
*   Created on 06/29/2025
*   Author: Ikshwak Jinesh 
*/
#include "ssd1306.h"

#define SSD1306_WIDTH    128
#define SSD1306_HEIGHT   64
#define SSD1306_BUFFER_SIZE  (SSD1306_WIDTH * SSD1306_HEIGHT / 8)

#ifndef pi
#define pi 3.1415926f
#endif
// Flags to protect the display frames from different functions when modifying it.
volatile bool frame_is_free = true;


static uint8_t buffer[SSD1306_BUFFER_SIZE];


// Internal helper functions.

/**
 * @brief  Writes single byte commands and settings to the SSD1306 controller. 
 * @param  cmd Predefined controll commands as per the datasheet. 
 * @retval true if the command has been sent successfully, false otherwise. 
*/
static bool ssd1306_WriteCommand(uint8_t cmd){
    return ssd1306_platform_write_command(cmd);
}

/**
 * @brief  Writes data to the SSD1306 controller. Typically used to send the display bitmap. 
 * @param  data Pointer to the data array that needs to be written. 
 * @param  size The number of bytes in the passed array
 * @retval true if the data has been transfered successfully, false otherwise. 
 */
static bool ssd1306_WriteData(const uint8_t* data, uint16_t size){
    if(size == 0){
        return false;
    }
    return ssd1306_platform_write_data(data, size);
}

/**
 * @brief  Writes a sequence of command bytes (a command and its arguments).
 * @param  cmds Pointer to the array of command bytes.
 * @param  size Number of bytes in the command array.
 * @retval true if the command sequence was sent successfully.
 */
static bool ssd1306_WriteMultiCommand(const uint8_t* cmds, uint16_t size){
    if(size == 0){
        return false;
    }
    return ssd1306_platform_write_multi_command(cmds, size);
}

/**
 * @brief  Hardware reset pulse.
 * @retval true if successfully reintialized the I2C bus, false otherwise. 
 */
static bool ssd1306_Reset(void){
    // If your platform supports a reset pin, toggle it here.
    // Otherwise just delay to allow internal reset.
    return ssd1306_DelayUs(2000);
}

/**
 * @brief  Provides a blocking delay for the required period.
 * @param  us Time to delay for in microseconds. 
 * @retval true if delay has been successfully completed, false otherwise. 
 */
static bool ssd1306_DelayUs(uint32_t us){
    if(ssd1306_platform_delay_us(us)){
        return true;
    }
    return false;
}

/**
 * @brief  Provides a result checking if the point is on clockwise or anticlockwise to the edge.
 * @param  x The x component of the point under consideration.
 * @param  y The y component of the point under consideration. 
 * @param  x0 The x component of the first vertex under consideration.
 * @param  y0 The y component of the first vertex under consideration. 
 * @param  x1 The x component of the second vertex under consideration.
 * @param  y1 The y component of the second vertex under consideration. 
 * @retval 1 if the point is anticlockwise to the edge and the first point, -1 if to the right/clockwise and 0 if the points are colinear.
 */
static int8_t IsAntiClockwise(int16_t x, int16_t y, int16_t x0, int16_t y0, int16_t x1, int16_t y1){
    int32_t cross = (int32_t)(x1-x0)*(y-y0) - (int32_t)(y1-y0)*(x-x0);
    return (int8_t)((cross>0) - (cross < 0));
}

/**
 * @brief  Provides confirmation to if a point is inside a given polygon. 
 * @param  x0 Pointer to the array of x axis component of the points under consideration. 
 * @param  y0 Pointer to the array of y axis component of the points under consideration.
 * @param  point_count The number of points that needs to be checked. 
 * @param  x Pointer to the array of x axis components of the vertices for the polygon.
 * @param  y Pointer to the array of y axis components of the vertices for the polygon.
 * @param  vertex_count The number or vertices the polygon has. 
 * @param  results Pointer to an array of bool values of the size point_count. true if inside the polygon, false otherwise.
 * @retval true if the check has completed successfully, false otherwise. 
 */
static bool InPoly(uint8_t* x0, uint8_t* y0, uint16_t point_count, int16_t* x, int16_t* y, uint8_t vertex_count, bool* results){
    if (vertex_count < 3 || point_count < 1)
        return false;

    int16_t cnt;
    uint8_t k;
    int16_t px, py, vx1, vy1, vx2, vy2;

    for (uint16_t i = 0; i < point_count; i++) {
        px = x0[i];
        py = y0[i];
        cnt = 0;

        for (uint8_t j = 0; j < vertex_count; j++) {
            k = (j + 1);
            if(k == vertex_count){
                k = 0;
            }
            vx1 = x[j]; vy1 = y[j];
            vx2 = x[k]; vy2 = y[k];

            if ((vy1 <= py && py < vy2) || (vy2 <= py && py < vy1)) {
                int8_t orient = IsAntiClockwise(px, py, vx1, vy1, vx2, vy2);
                if (orient > 0) cnt++;
                else if (orient < 0) cnt--;
                else {
                    cnt = 1; // Point exactly on edge
                    break;
                }
            } else if (vy1 == vy2 && py == vy1) {
                if ((vx1 <= px && px <= vx2) || (vx2 <= px && px <= vx1)) {
                    cnt = 1; // Point on horizontal edge
                    break;
                }
            }
        }
        results[i] = (cnt != 0);
    }
    return true;
}

// The following function is the added angle method to calculate if the points are within a polygon. Use it only if absolutely needed. 
// It is too slow for realtime display.
/*
static bool InPoly(uint8_t* x0, uint8_t* y0, uint16_t point_count, int16_t* x, int16_t* y, uint8_t vertex_count, bool* results){
    if(vertex_count < 3 || point_count < 1){
        return false;
    }
    float v_x1, v_x2, v_y1, v_y2,p_x, p_y,dot, cross, len1, len2 ,angle, angle_sum;  
    float EPSILON = 0.05;
    for (uint16_t i = 0; i < point_count; i++) {
        p_x = x0[i];
        p_y = y0[i];
        angle_sum = 0.0;

        for (uint8_t j = 0; j < vertex_count; j++) {
            v_x1 = x[j] - p_x;
            v_y1 = y[j] - p_y;
            v_x2 = x[(j + 1) % vertex_count] - p_x;
            v_y2 = y[(j + 1) % vertex_count] - p_y;

            float dot = v_x1 * v_x2 + v_y1 * v_y2;
            float cross = v_x1 * v_y2 - v_y1 * v_x2;
            float len1 = sqrtf(v_x1 * v_x1 + v_y1 * v_y1);
            float len2 = sqrtf(v_x2 * v_x2 + v_y2 * v_y2);

            if (len1 == 0 || len2 == 0) {
                angle_sum = 0;
                break;  // Point coincides with a vertex; treat as on edge
            }

            float angle = atan2f(cross, dot);  // Signed angle
            angle_sum += angle;
            if(fabs(angle_sum - 2*pi) < EPSILON){ // pi is defined, and is 3.14.
                results[i] = true;
            }
            else{
                results[i] = false;
            }
        }

    }
    return true;
}
*/

bool ssd1306_Init(void){
    // reset
    if (!ssd1306_Reset()) return false;

    // Initialization sequence (from datasheet)
    const uint8_t init_seq[] = {
        0xAE,             // Display OFF
        0x20, 0x00,       // Memory Addressing Mode: Horizontal
        0xB0,             // Set Page Start Address for Page Addressing Mode
        0xC8,             // COM Output Scan Direction: remapped
        0x00,             // Low column address
        0x10,             // High column address
        0x40,             // Set start line at 0
        0x81, 0x7F,       // Set contrast to 0x7F
        0xA1,             // Segment re-map: column address 127 is mapped to SEG0
        0xA6,             // Normal display
        0xA8, 0x3F,       // Multiplex ratio = 64
        0xA4,             // Output follows RAM content
        0xD3, 0x00,       // Display offset = 0
        0xD5, 0x80,       // Display clock div ratio = 0x0, osc freq = 0x8
        0xD9, 0xF1,       // Pre-charge period
        0xDA, 0x12,       // COM pins hardware config
        0xDB, 0x40,       // VCOMH deselect level
        0x8D, 0x14,       // Charge pump settings: enable
        0xAF              // Display ON
    };
    return ssd1306_WriteData(init_seq, sizeof(init_seq));
}

bool ssd1306_SetDisplayOffset(uint8_t offset){
    const uint8_t cmd[] = { 0xD3, offset };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_SetStartLine(uint8_t start_line){
    return ssd1306_WriteCommand(0x40 | (start_line & 0x3F));
}

bool ssd1306_SetSegmentRemap(bool remap){
    return ssd1306_WriteCommand(remap ? 0xA1 : 0xA0);
}

bool ssd1306_SetCOMOutputScanDirection(bool remap){
    return ssd1306_WriteCommand(remap ? 0xC0 : 0xC8);
}

bool ssd1306_SetMultiplexRatio(uint8_t ratio){
    if (ratio == 0 || ratio > 64) return false;
    const uint8_t cmd[] = { 0xA8, (uint8_t)(ratio - 1) };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_SetDisplayClockDiv(uint8_t divide_ratio, uint8_t osc_freq){
    const uint8_t cmd[] = { 0xD5, (uint8_t)((divide_ratio << 4) | (osc_freq & 0x0F)) };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_SetPreChargePeriod(uint8_t phase1, uint8_t phase2){
    const uint8_t cmd[] = { 0xD9, (uint8_t)((phase1 << 4) | (phase2 & 0x0F)) };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_SetVCOMHLevel(uint8_t level){
    if (level > 7) return false;
    const uint8_t cmd[] = { 0xDB, level << 4 };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_SetChargePump(bool enable){
    const uint8_t cmd[] = { 0x8D, enable ? 0x14 : 0x10 };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_Sleep(void){
    return ssd1306_WriteCommand(0xAE);
}

bool ssd1306_Wake(void){
    return ssd1306_WriteCommand(0xAF);
}

bool ssd1306_PowerOnSequence(void){
    // Often just Init covers this; but to mirror datasheet:
    if (!ssd1306_WriteCommand(0xAF)) return false; // Display ON
    return ssd1306_DelayUs(100000);                 // tAF = 100 ms
}

bool ssd1306_PowerOffSequence(void){
    if (!ssd1306_WriteCommand(0xAE)) return false; // Display OFF
    return ssd1306_DelayUs(100000);                // tOFF = 100 ms
}

bool ssd1306_Clear(void){
    memset(buffer, 0, SSD1306_BUFFER_SIZE);
    return true;
}

bool ssd1306_UpdateScreen(void){

    if(!frame_is_free){
        return false;
    }
    frame_is_free = false;
    // set page and column addresses to full screen
    if (!ssd1306_SetMemoryAddressingMode(0x00)) return false;
    if (!ssd1306_SetColumnAddress(0, SSD1306_WIDTH - 1)) return false;
    if (!ssd1306_SetPageAddress(0, (SSD1306_HEIGHT/8) - 1)) return false;

    // send all buffer via DMA or blocking
    if(ssd1306_platform_start_data_dma(buffer, SSD1306_BUFFER_SIZE)){
        frame_is_free = true;
        return true;
    }
    return false;
}

bool ssd1306_SetMemoryAddressingMode(uint8_t mode){
    if (mode > 0x02) return false;
    const uint8_t cmd[] = { 0x20, mode };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

bool ssd1306_SetColumnAddress(uint8_t start, uint8_t end){
    const uint8_t cmd[] = { 0x21, start, end };
    return ssd1306_WriteMultiCommand(cmd, 3);
}

bool ssd1306_SetPageAddress(uint8_t start, uint8_t end){
    const uint8_t cmd[] = { 0x22, start, end };
    return ssd1306_WriteMultiCommand(cmd, 3);
}

bool ssd1306_DisplayOn(void){
    return ssd1306_WriteCommand(0xAF);
}

bool ssd1306_DisplayOff(void){
    return ssd1306_WriteCommand(0xAE);
}

bool ssd1306_InvertDisplay(bool inv) {
    return ssd1306_WriteCommand(inv ? 0xA7 : 0xA6);
}

bool ssd1306_EntireDisplayOn(bool on) {
    return ssd1306_WriteCommand(on ? 0xA5 : 0xA4);
}

bool ssd1306_SetContrast(uint8_t contrast) {
    const uint8_t cmd[] = { 0x81, contrast };
    return ssd1306_WriteMultiCommand(cmd, 2);
}

// Internal Helper
static inline void ssd1306_SetPixel(int16_t x, int16_t y, bool color) {
    if (x < 0 || x >= SSD1306_WIDTH || y < 0 || y >= SSD1306_HEIGHT)
        return;
    uint16_t byteIndex = x + (y / 8) * SSD1306_WIDTH;
    uint8_t bitMask = 1 << (y % 8);
    if (color)
        buffer[byteIndex] |= bitMask;
    else
        buffer[byteIndex] &= ~bitMask;
}

bool ssd1306_DrawPixel(uint8_t x, uint8_t y, bool color) {
    ssd1306_SetPixel(x, y, color);
    return true;
}

bool ssd1306_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t thickness, bool color) {
    int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, e2;

    while (1) {
        for (uint8_t t = 0; t < thickness; ++t)
            ssd1306_SetPixel(x0, y0 + t, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return true;
}

bool ssd1306_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t thickness, bool color) {
    ssd1306_DrawLine(x, y, x + w - 1, y, thickness, color);           // Top
    ssd1306_DrawLine(x, y + h - 1, x + w - 1, y + h - 1, thickness, color); // Bottom
    ssd1306_DrawLine(x, y, x, y + h - 1, thickness, color);           // Left
    ssd1306_DrawLine(x + w - 1, y, x + w - 1, y + h - 1, thickness, color); // Right
    return true;
}

bool ssd1306_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color) {
    for (int16_t i = 0; i < h; i++) {
        ssd1306_DrawLine(x, y + i, x + w - 1, y + i, 1, color);
    }
    return true;
}

bool ssd1306_DrawCircle(int16_t x0, int16_t y0, uint16_t r, uint8_t thickness, bool color) {
    int16_t f = 1 - r;
    int16_t dx = 1, dy = -2 * r;
    int16_t x = 0, y = r;

    while (x <= y) {
        for (uint8_t t = 0; t < thickness; ++t) {
            ssd1306_SetPixel(x0 + x, y0 + y - t, color);
            ssd1306_SetPixel(x0 - x, y0 + y - t, color);
            ssd1306_SetPixel(x0 + x, y0 - y + t, color);
            ssd1306_SetPixel(x0 - x, y0 - y + t, color);
            ssd1306_SetPixel(x0 + y, y0 + x - t, color);
            ssd1306_SetPixel(x0 - y, y0 + x - t, color);
            ssd1306_SetPixel(x0 + y, y0 - x + t, color);
            ssd1306_SetPixel(x0 - y, y0 - x + t, color);
        }
        if (f >= 0) { y--; dy += 2; f += dy; }
        x++; dx += 2; f += dx;
    }
    return true;
}

bool ssd1306_FillCircle(int16_t x0, int16_t y0, uint16_t r, bool color) {
    for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r)
                ssd1306_SetPixel(x0 + x, y0 + y, color);
        }
    }
    return true;
}

bool ssd1306_DrawPoly(int16_t* x, int16_t* y, uint8_t vertex_count, uint8_t thickness, bool color) {
    for (uint8_t i = 0; i < vertex_count; i++) {
        uint8_t next = (i + 1) % vertex_count;
        ssd1306_DrawLine(x[i], y[i], x[next], y[next], thickness, color);
    }
    return true;
}

bool ssd1306_FillPoly(int16_t* x, int16_t* y, uint8_t vertex_count, bool color) {
    uint8_t px[SSD1306_WIDTH * SSD1306_HEIGHT];
    uint8_t py[SSD1306_WIDTH * SSD1306_HEIGHT];
    bool results[SSD1306_WIDTH * SSD1306_HEIGHT];
    uint16_t count = 0;
    for (uint8_t i = 0; i < SSD1306_WIDTH; i++) {
        for (uint8_t j = 0; j < SSD1306_HEIGHT; j++) {
            px[count] = i;
            py[count] = j;
            count++;
        }
    }
    InPoly(px, py, count, x, y, vertex_count, results);
    for (uint16_t i = 0; i < count; i++) {
        if (results[i])
            ssd1306_SetPixel(px[i], py[i], color);
    }
    return true;
}

bool ssd1306_DrawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int w, int h, bool color) {
    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            if (bitmap[i + (j / 8) * w] & (1 << (j % 8))) {
                ssd1306_SetPixel(x + i, y + j, color);
            }
        }
    }
    return true;
}

bool ssd1306_WriteChar(int16_t x, int16_t y, char ch, FontDef font, bool color) {
    uint16_t idx = (ch - 32) * font.height;
    for (uint8_t row = 0; row < font.height; row++) {
        uint8_t line = font.data[idx + row];
        for (uint8_t col = 0; col < font.width; col++) {
            if (line & (1 << col)) {
                ssd1306_SetPixel(x + col, y + row, color);
            }
        }
    }
    return true;
}

bool ssd1306_WriteString(int16_t x, int16_t y, const char* str, uint8_t len, FontDef font, bool color) {
    const int16_t max_x       = SSD1306_WIDTH;
    const int16_t max_y       = SSD1306_HEIGHT;
    const int16_t line_height = font.height + 1;  // 1px spacing between lines

    for (uint8_t i = 0; i < len; i++) {
        // Wrap to next line if this glyph would exceed right edge
        if (x + font.width > max_x) {
            x = 0;                 // back to left margin
            y += line_height;      // down by one line

            // If we’ve exceeded the bottom edge, stop drawing
            if (y + font.height > max_y) {
                return false;
            }
        }

        // Draw the character; bail out if WriteChar fails
        if (!ssd1306_WriteChar(x, y, str[i], font, color)) {
            return false;
        }

        // Advance cursor
        x += font.width;
    }

    return true;
}

bool ssd1306_StartScroll(bool right, uint8_t startPage, uint8_t endPage, uint8_t speed, uint8_t topFixedRows, uint8_t scrollRows, uint8_t verticalOffset)
{
    // Set vertical scroll area
    uint8_t a3[] = { 0xA3, topFixedRows, scrollRows };
    if (!ssd1306_WriteMultiCommand(a3, 3)) return false;

    // Choose diagonal or horizontal
    if (verticalOffset == 0) {
        uint8_t cmd[] = {
            right ? 0x26 : 0x27, 0x00, startPage, endPage, speed
        };
        if (!ssd1306_WriteMultiCommand(cmd, 5)) return false;
    } else {
        uint8_t cmd[] = {
            right ? 0x29 : 0x2A, 0x00, startPage, endPage,
            verticalOffset, speed
        };
        if (!ssd1306_WriteMultiCommand(cmd, 6)) return false;
    }
    return ssd1306_WriteCommand(0x2F);
}

bool ssd1306_StopScroll(void) {
    return ssd1306_WriteCommand(0x2E);
}