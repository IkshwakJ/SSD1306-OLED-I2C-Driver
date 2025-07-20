# SSD1306 OLED I²C Driver

A lightweight, fully‑featured C driver for the SSD1306 128×64 monochrome OLED display, with platform abstraction layers for STM32 (HAL), ESP32 (ESP‑IDF), and more.

![SSD1306 Demo](docs/ssd1306-demo.png)

## Features

- **I²C interface** (no extra pins beyond SDA/SCL + power)
- **Graphics primitives**  
  - Draw pixels, lines, rectangles (filled/unfilled), circles (filled/unfilled), polygons  
  - Render bitmaps and icons
- **Text support**  
  - Built‑in 5×8 ASCII font (32–127)  
  - Easy to extend with additional font files
- **Platform abstraction**  
  - STM32 (HAL) implementation (`ssd1306_platform_stm32.c`)  
  - ESP32 (ESP‑IDF) implementation (`ssd1306_platform_esp32.cpp`)  
  - Add your own by implementing the `ssd1306_platform_*` function set
- **Double‑buffered frame buffer**  
  - 128×64 px local RAM mirror  
  - Single bulk update to SSD1306 GDDRAM

## Repository Structure
