/*
*   ssd1306.c
*   Created on 06/29/2025
*   Author: Ikshwak Jinesh 
*/
#include "ssd1306.h"


// Internal helper functions.

/**
 * @brief  Writes single byte commands and settings to the SSD1306 controller. 
 * @param  cmd Predefined controll commands as per the datasheet. 
 * @retval true if the command has been sent successfully, false otherwise. 
*/
static bool ssd1306_WriteCommand(uint8_t cmd);

/**
 * @brief  Writes data to the SSD1306 controller. Typically used to send the display bitmap. 
 * @param  data Pointer to the data array that needs to be written. 
 * @param  size The number of bytes in the passed array
 * @retval true if the data has been transfered successfully, false otherwise. 
 */
static bool ssd1306_WriteData(const uint8_t* data, uint16_t size);

/**
 * @brief  Writes a sequence of command bytes (a command and its arguments).
 * @param  cmds Pointer to the array of command bytes.
 * @param  size Number of bytes in the command array.
 * @retval true if the command sequence was sent successfully.
 */
static bool ssd1306_WriteMultiCommand(const uint8_t* cmds, uint16_t size);


/**
 * @brief  Hardware reset pulse.
 * @retval true if successfully reintialized the I2C bus, false otherwise. 
 */
static bool ssd1306_Reset(void);

/**
 * @brief  Provides a blocking delay for the required period.
 * @param  us Time to delay for in microseconds. 
 * @retval true if delay has been successfully completed, false otherwise. 
 */
static bool ssd1306_DelayUs(uint32_t us);

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
static bool InPoly(uint8_t* x0, uint8_t* y0, uint16_t point_count, int16_t* x, int16_t* y, uint8_t vertex_count, bool* results);
