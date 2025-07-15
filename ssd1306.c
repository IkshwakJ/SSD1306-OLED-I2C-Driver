/*
*   ssd1306.c
*   Created on 06/29/2025
*   Author: Ikshwak Jinesh 
*/
#include "ssd1306.h"


#define pi 3.14
// Flags to protect the display frames from different functions when modifying it.
volatile bool frame_is_free = true;

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
    if(vertex_count < 3 || point_count < 1){
        return false;
    }
    int16_t cnt, is_anticlockwise, vx_1, vx_2, vy_1, vy_2;
    uint8_t k;
    int16_t px, py;
    for(uint16_t i = 0; i < point_count; i++){
        px = x0[i];
        py = y0[i];
        cnt = 0;
        for(uint8_t j = 0; j < vertex_count; j++){
            k = (j + 1);
            if(k == vertex_count){
                k = 0;
            }
            vx_1 = x[j];
            vy_1 = y[j];
            vx_2 = x[k];
            vy_2 = y[k];
            if((vy_1 <= py && py < vy_2) || (vy_1 > py && py >= vy_2)){
                is_anticlockwise = IsAntiClockwise(px,py,vx_1,vy_1,vx_2,vy_2);
                if(is_anticlockwise > 0){
                    cnt ++;
                }
                else if(is_anticlockwise < 0){
                    cnt--;
                }
                else{
                    cnt = 1; // For all the cases where the point is on the edge, but the edge is not horizontal. 
                    break;
                }
            }
            else if(vy_1 == vy_2 && vy_1 == py){
                if((vx_1 <= px && px <= vx_2) || (vx_1 > px && px > vx_2)){
                    cnt = 1;
                    break;
                }
            }
        }
        if(cnt != 0){
            results[i] = true;
        }
        else{
            results[i] = false;
        }
    }
    return true;
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
    int64_t cross = (int64_t)(x1-x0)*(y-y0) - (int64_t)(y1-y0)*(x-x0);
    return (int8_t)((cross>0) - (cross < 0));
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