/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2017 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Super small LCD driver for Pixl.js
 * ----------------------------------------------------------------------------
 */

#ifdef LCD_CONTROLLER_ST7567 // Pixl
#define LCD
#define LCD_DATA_WIDTH 128
#define LCD_DATA_HEIGHT 64
#endif
#ifdef LCD_CONTROLLER_ST7789V // iD205
#define LCD
#define LCD_DATA_WIDTH 120 // pixel doubled
#define LCD_DATA_HEIGHT 120 // pixel doubled
#define LCD_STORE_MODIFIED
#define LCD_BL_ON 1
#define LCD_START_X 12
#define LCD_START_Y 24
#endif
#ifdef LCD_CONTROLLER_ST7735 // F5
#define LCD
#define LCD_DATA_WIDTH 128
#define LCD_DATA_HEIGHT 96
#define LCD_STORE_MODIFIED
#define LCD_BL_ON 0
#endif
#ifdef LCD_CONTROLLER_ST7789_8BIT // Bangle.js
#define LCD
#define LCD_DATA_WIDTH 120 // pixel doubled
#define LCD_DATA_HEIGHT 140 // pixel doubled
//#define LCD_STORE_MODIFIED // removed just to try and scrape a few extra bytes!
#define I2C_SDA 15
#define I2C_SCL 14
#define LCD_START_Y 16
#endif
#ifdef LCD_CONTROLLER_LPM013M126
#define LCD
#define LCD_START_X 2
#define LCD_START_Y 2
#define LCD_DATA_WIDTH 88 // pixel doubled
#define LCD_DATA_HEIGHT 88 // pixel doubled
#define LCD_STORE_MODIFIED
#define LCD_BL_ON 1
#endif
#ifdef LCD_CONTROLLER_GC9A01
#define LCD
#define LCD_DATA_WIDTH 240
#define LCD_DATA_HEIGHT 240
#define LCD_STORE_MODIFIED
#define LCD_START_X 60
#define LCD_START_Y 60
#define LCD_BL_ON 1
#endif

#ifndef LCD_START_X
#define LCD_START_X 0
#endif
#ifndef LCD_START_Y
#define LCD_START_Y 0
#endif

void lcd_init();
void lcd_kill();
void lcd_clear();
void lcd_print(char *ch);
void lcd_print_hex(unsigned int v); // just for debugging - print a number
void lcd_println(char *ch);


