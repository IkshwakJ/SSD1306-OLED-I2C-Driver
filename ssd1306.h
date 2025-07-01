/*
*   ssd1306.h
*   Created on 06/29/2025
*   Author Ikshwak Jinesh
*/
#ifndef SSD1306_H
#define SSD1306_H

#include "stdbool.h"
#include "stdint.h"
#include "ssd1306_fonts.h"
#include "math.h"

// Core functions.

// Initialization, and Power sequence.

/**
 * @brief Sends the commands to set up the display at default or zeroed values.
 * @retval true if the device has been set up successfully, false otherwise. 
 */
bool ssd1306_Init(void);

/**
 * @brief  Sets the display vertical offset from COM0.
 * @param  offset Number of rows (0–63) to shift the display vertically.
 * @retval true if the offset command was sent successfully, false otherwise.
 */
bool ssd1306_SetDisplayOffset(uint8_t offset);

/**
 * @brief  Sets the display start line (row in GDDRAM mapped to COM0).
 * @param  start_line The start line (0–63) for mapping GDDRAM to COM0.
 * @retval true if the start line command was sent successfully, false otherwise.
 */
bool ssd1306_SetStartLine(uint8_t start_line);

/**
 * @brief  Remaps column address 0 to either SEG0 or SEG127.
 * @param  remap true to map SEG0→column127 (horizontal flip), false for SEG0→column0.
 * @retval true if the segment-remap command was sent successfully, false otherwise.
 */
bool ssd1306_SetSegmentRemap(bool remap);

/**
 * @brief  Sets COM output scan direction (row order).
 * @param  remap true to scan from COM[N–1]→COM0 (vertical flip), false for COM0→COM[N–1].
 * @retval true if the COM scan-direction command was sent successfully, false otherwise.
 */
bool ssd1306_SetCOMOutputScanDirection(bool remap);

/**
 * @brief  Sets the multiplex ratio (number of displayed rows).
 * @param  ratio Multiplex ratio (1–64) determining how many COM lines are used.
 * @retval true if the multiplex-ratio command was sent successfully, false otherwise.
 */
bool ssd1306_SetMultiplexRatio(uint8_t ratio);

/**
 * @brief  Configures the display clock divide ratio and oscillator frequency.
 * @param  divide_ratio Clock divide ratio (0–15) for the display clock.
 * @param  osc_freq    Oscillator frequency (0–15) setting for the internal RC oscillator.
 * @retval true if the display-clock command sequence was sent successfully, false otherwise.
 */
bool ssd1306_SetDisplayClockDiv(uint8_t divide_ratio, uint8_t osc_freq);

/**
 * @brief  Sets the pre‑charge period for the display.
 * @param  phase1 Number of DCLKs for phase 1 (0–15).
 * @param  phase2 Number of DCLKs for phase 2 (0–15).
 * @retval true if the pre‑charge-period command sequence was sent successfully, false otherwise.
 */
bool ssd1306_SetPreChargePeriod(uint8_t phase1, uint8_t phase2);

/**
 * @brief  Sets the VCOMH deselect level.
 * @param  level VCOMH level (0–7) mapping to ~0.65×VCC–~0.85×VCC.
 * @retval true if the VCOMH-level command was sent successfully, false otherwise.
 */
bool ssd1306_SetVCOMHLevel(uint8_t level);

/**
 * @brief  Enables or disables the internal charge pump.
 * @param  enable true to enable charge‑pump (required for VCC ≤ 3.3 V), false to disable.
 * @retval true if the charge‑pump command was sent successfully, false otherwise.
 */
bool ssd1306_SetChargePump(bool enable);

/**
 * @brief  Set the device to sleep mode.
 * @retval true if the sleep command has been sent successfully, false otherwise.
 */
bool ssd1306_Sleep(void);

/**
 * @brief  Wake the device from a sleep mode. 
 * @retval true if the wake command has been sent successfully, false otherwise.
 */
bool ssd1306_Wake(void);

/**
 * @brief  Powers on the display safely as per the datasheet. 
 * @retval true if the process completes succefully, false otherwise.
 */
bool ssd1306_PowerOnSequence(void);

/**
 * @brief  Powers off the display safely as per the datasheet. 
 * @retval true if the process completes succefully, false otherwise.
 */
bool ssd1306_PowerOffSequence(void);

// Screen buffer management.

/**
 * @brief  Clears the GDDRAM and sets all values to zeros. 
 * @retval true if the GDDRAM has been cleared, false otherwise. 
 */
bool ssd1306_Clear(void);

/**
 * @brief  Refreshes the display with the last developed frame. 
 * @retval true if the display is updated, false otherwise. 
 */
bool ssd1306_UpdateScreen(void);

// Addressing and mapping

/**
 * @brief  Configures how the SSD1306 interprets the memory writes to GDDRAM. How the address pointer moves when pixel data is sent.
 * @param  mode The direction in which writes happen. Horizontal, left to right (0x00); vertical, top to bottom(0x01); legacy mode (0x02).
 * @retval true if the mode has been set, false otherwise. 
 */
bool ssd1306_SetMemoryAddressingMode(uint8_t mode);

/**
 * @brief  Configures which coloumns to write to, i.e. horizontal address window. 
 * @param  start The column where the writes are supposed to start at, including the passed column (0, 127).
 * @param  end The column where the writes are supposed to end at, including the passed column (0, 127). 
 * @retval true if the column addresses to write to have been set, false otherwise. 
 */
bool ssd1306_SetColumnAddress(uint8_t start, uint8_t end);

/**
 * @brief  Configures which pages to write to, i.e. vertical address window. 
 * @param  start The page where the writes are supposed to start at, including the passed row (0, 7).
 * @param  end The page where the writes are supposed to end at, including the passed column (0, 7). 
 * @retval true if the page addresses to write to have been set, false otherwise. 
 */
bool ssd1306_SetPageAddress(uint8_t start, uint8_t end);

// Display Control

/**
 * @brief  Turns on the display without changing anything else, including the GDDRAM.
 * @retval true if the display has been turned on, false otherwise. 
 */
bool ssd1306_DisplayOn(void);

/**
 * @brief  Turns off the display without changing anything else, including the GDDRAM.
 * @retval true if the display has been turned off, false otherwise. 
 */
bool ssd1306_DisplayOFF(void);

/**
 * @brief  Sets the display setting to invert the display colors or to make it default. 
 * @retval true if the settings has been changed successfully, false otherwise. 
 */
bool ssd1306_InvertDisplay(bool invert);

/**
 * @brief  Turns on all the pixels regardless of the inversion status. 
 * @retval true if the display has been successfully turned on, false otherwise. 
*/
bool ssd1306_EntireDisplayOn(bool on);

/**
 * @brief  Sets the contrast between 0 and 100 with 255 steps in between.
 * @retval true if the setting has been changed, false otherwise.  
 */
bool ssd1306_SetContrast(uint8_t contrast);

// Graphics Primitives

/**
 * @brief  Draws at a specific pixel, this requires access to the frame the display is showing currently. 
 * @param  x horizontal component of the position of the pixel.
 * @param  y vertical component of the position of the pixel.
 * @param  color on or off for the monochromatic oled. 
 * @retval true if the pixel has been drawn, false otherwise.
 */
bool ssd1306_DrawPixel(uint8_t x, uint8_t y, bool color);

/**
 * @brief  Draws a line on the display between the points passed.
 * @param  x0 Horizontal component of the first point of the line.
 * @param  y0 Vertical component of the first point of the line.
 * @param  x1 Horizontal component of the end point of the line.
 * @param  y1 Vertical component of the end point of the line.
 * @param  thickness The number of pixels thick that the line is.
 * @param  color Turn on or off for the monochromatic oled along the line.
 * @retval true if the line is drawn on the display, false otherwise. 
 */
bool ssd1306_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t thickness, bool color);

/**
 * @brief  Draws a circle on the display, for a given center and radius. 
 * @param  x0 Horizontal component of the origin of the circle.
 * @param  y0 Vertical component of the origin of the circle. 
 * @param  r Radius of the circle.
 * @param  thickness The number of pixels thick that the line is.
 * @param  color Turn on or off the monochromatic oled along the circle.
 * @retval true if the circle is drawn on the display, false otherwise. 
 */
bool ssd1306_DrawCircle(int16_t x0, int16_t y0, uint16_t r, uint8_t thickness, bool color);

/**
 * @brief Fills a circle of a given center and radius with a given color. 
 * @param  x0 Horizontal component of the origin of the circle.
 * @param  y0 Vertical component of the origin of the circle. 
 * @param  r Radius of the circle.
 * @param  color Turn on or off the monochromatic oled along the circle.
 * @retval true if the filled circle is drawn on the display, false otherwise. 
 */
bool ssd1306_FillCircle(int16_t x0, int16_t y0, uint16_t r, bool color);

/**
 * @brief  Draws an unfilled rectangle on the display.
 * @param  x Horizontal coordinate of the top‑left corner.
 * @param  y Vertical coordinate of the top‑left corner.
 * @param  w Width of the rectangle in pixels.
 * @param  h Height of the rectangle in pixels.
 * @param  thickness Line thickness in pixels.
 * @param  color Pixel on/off (true = on, false = off).
 * @retval true if the rectangle was drawn successfully, false otherwise.
 */
bool ssd1306_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t thickness, bool color);

/**
 * @brief  Draws a filled rectangle on the display.
 * @param  x Horizontal coordinate of the top‑left corner.
 * @param  y Vertical coordinate of the top‑left corner.
 * @param  w Width of the rectangle in pixels.
 * @param  h Height of the rectangle in pixels.
 * @param  color Pixel on/off (true = on, false = off).
 * @retval true if the filled rectangle was drawn successfully, false otherwise.
 */
bool ssd1306_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, bool color);

/**
 * @brief Draws a polygon between two points in the array selected in sequence, including both ends. 
 * @param  x Pointer to the array of x axis components of the vertices for the polygon.
 * @param  y Pointer to the array of y axis components of the vertices for the polygon.
 * @param  vertex_count The number or vertices the polygon has. 
 * @param  thickness The number of pixels thick that the line is.
 * @param  color Turn on or off the monochromatic oled along the polygon.
 * @retval true if the polygon is drawn on the display, false otherwise. 
 */
bool ssd1306_DrawPoly(int16_t* x, int16_t* y, uint8_t vertex_count, uint8_t thickness, bool color);

/**
 * @brief Fills a polygon of a given parameters with a given color. 
 * @param  x Pointer to the array of x axis components of the vertices for the polygon.
 * @param  y Pointer to the array of y axis components of the vertices for the polygon.
 * @param  vertex_count The number or vertices the polygon has. 
 * @param  color Turn on or off the monochromatic oled along the polygon.
 * @retval true if the filled polygon is drawn on the display, false otherwise. 
 */
bool ssd1306_FillPoly(int16_t* x, int16_t* y, uint8_t vertex_count, bool color);

/**
 * @brief  Draws a bitmap onto the display while maintaining anything else on the screen. 
 * @param  x The location of the horizontal component of the position of the top left bit.
 * @param  y The location of the vertical component of the position of the top left bit.
 * @param  bitmap Column major, page alligned, bitmap to display.
 * @param  w Total number of columns to display.
 * @param  h Total number of rows to display.  
 * @param  color Turn on or off the monochromatic oled along the bitmap.
 * @retval true if the bitmap is drawn onto the display, false otherwise. 
 * .
 */
bool ssd1306_DrawBitmap(int16_t x, int16_t y, const uint8_t* bitmap, int w, int h, bool color);

/**
 * @brief  Writes a single character of the passed font onto the display.
 * @param  x The horizontal component of the top left position of the character.
 * @param  y The vertical component of the top left position of the character.
 * @param  ch The character to be displayed. 
 * @param  font The font the character needs to be displayed in. 
 * @param  color Trun on or of the monochromatic oled along the character. 
 * @retval true if the character is displayed onto the image, false otherwise. 
 */
bool ssd1306_WriteChar(int16_t x, int16_t y, char ch, FontDef font, bool color);

/**
 * @brief  Writes a string onto the display. Additional or oversized characters beyond the display will be truncated.  
 * @param  x The horizontal component of the top left position of the first character in the string.
 * @param  y The vertical component of the top left position of the first character in the string.
 * @param  str Pointer to the character array containing the string. 
 * @param  len The length of the character array. 
 * @param  font The font the string needs to be displayed in. 
 * @param  color Trun on or of the monochromatic oled along the string. 
 * @retval true if the string is displayed onto the image, false otherwise. 
 */
bool ssd1306_WriteString(int16_t x, int16_t y, const char* str, uint8_t len, FontDef font, bool color);

// Scrolling effects. Hardware based

/**
 * @brief  Scrolls the display along the horizontal, vertical(move lines down) or diagonal direction. 
 * @param  right The direction to scroll the display along, right if true, left otherwise.
 * @param  startPage Starting page (8bits) where the scrolling needs to begin at. 
 * @param  endPage Ending page where the scrolling needs to stop at. 
 * @param  speed The speed at which the scroll needs to happen. 
 * @param  topFixedRows The number of rows that stays fixed when the remaining scrollRows are moved down by verticalOffset per frame.
 * @param  scrollRows The number of rows that are to be moved. The rows that remain stay fixed vertically.
 * @param  verticalOffset The number of rows to be moved by in the vertical direction.
 * @retval true if the scroll command has been sent successfully, false otherwise.  
 */
bool ssd1306_StartScroll(bool right, uint8_t startPage, uint8_t endPage, uint8_t speed, uint8_t topFixedRows, uint8_t scrollRows, uint8_t verticalOffset);

/**
 * @brief  Sends the request to stop scrolling.
 * @retval true if the command has been sent successfully, false otherwise.
 */
bool ssd1306_StopScroll(void);

#endif