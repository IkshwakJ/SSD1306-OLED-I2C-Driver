/*
*   ssd1306_fonts.h
*   Created on 06/29/2025
*   Author Ikshwak Jinesh
*/

#ifndef SSD1306_FONTS_H
#define SSD1306_FONTS_H

#include <stdint.h>

typedef struct {
    const uint8_t *data;
    uint8_t        width;
    uint8_t        height;
} FontDef;

#endif // SSD1306_FONTS_H