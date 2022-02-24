/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>, atc1441, MaBecker, Jeffmer
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Graphics Backend for drawing to SPI displays in unbuffered mode 
 * ----------------------------------------------------------------------------
 */

#include "graphics.h"
#include "jsvariterator.h"
#include "jshardware.h"

typedef struct {
  Pin pinCS;                //!< Pin to use for cs.
  Pin pinDC;                //!< Pin to use for Data Command.
  Pin pinflashCS;            //!< Pin to restart SPI flash read
  int width;                //!< Display pixel size X
  int height;               //!< Display pixel size Y
  int colstart;             //!< Aditional starting address some pixels dont begin at 0
  int rowstart;             //!< Aditional starting address some pixels dont begin at 0
} JshLCD_SPI_Info;

bool jswrap_lcd_spi_buf_idle();
JsVar *jswrap_lcd_spi_buf_connect(JsVar *device, JsVar *options);
void lcd_spi_buf_setCallbacks(JsGraphics *gfx);
void jswrap_lcd_spi_buf_command(int cmd, JsVar *data);
