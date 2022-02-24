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

/*JSON{
  "type" : "class",
  "class" : "lcd_spi_buf"
}*/
#include "platform_config.h"
#include "lcd_spi_buf.h"
#include "jsutils.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#include "jswrap_graphics.h"
#include "jshardware.h"

static int _pin_mosi;
static int _pin_clk;
static int _pin_cs;
static int _pin_dc;
static int _pin_flash_cs;
static int _colstart;
static int _rowstart;
static int _lastx=-1;
static int _lasty=-1;
static IOEventFlags _device;


#define LCD_STRIDE ((LCD_WIDTH*LCD_BPP+7)>>3)
unsigned char lcdBuffer[LCD_STRIDE*LCD_HEIGHT];

static void set_cs(){
#ifdef ESPR_USE_SPI3
  // anomaly 195 workaround - enable SPI before use
  *(volatile uint32_t *)0x4002F500 = 7;
#endif
  jshPinSetValue(_pin_cs, 0);
}

static void rel_cs(){
  jshPinSetValue(_pin_cs, 1);
#ifdef ESPR_USE_SPI3
  // anomaly 195 workaround - disable SPI when done
  *(volatile uint32_t *)0x4002F500 = 0;
  *(volatile uint32_t *)0x4002F004 = 1;
#endif
}

static void spi_cmd(const uint8_t cmd, uint8_t *data, int dsize){
  jshPinSetValue(_pin_dc, 0);
  jshSPISendMany(_device, &cmd, NULL, 1, NULL);
  jshPinSetValue(_pin_dc, 1);
  if (data) jshSPISendMany(_device, data, NULL, dsize, NULL);
}

static void disp_spi_transfer_addrwin(int x1, int y1, int x2, int y2) {
  unsigned char wd[4];
  x1 += _colstart;
  y1 += _rowstart;
  x2 += _colstart;
  y2 += _rowstart;
  wd[0] = x1>>8;
  wd[1] = x1 & 0xFF;
  wd[2] = x2>>8;
  wd[3] = x2 & 0xFF;
  spi_cmd(0x2A, &wd, 4);
  wd[0] = y1>>8;
  wd[1] = y1 & 0xFF;
  wd[2] = y2>>8;
  wd[3] = y2 & 0xFF;
  spi_cmd(0x2B, &wd, 4);
  spi_cmd(0x2C, NULL, NULL);
}

static void endxfer(){  
  // do nothing
}

void lcd_spi_buf_flip(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!
  // Just send full rows as this allows us to issue a single SPI
  int x1 = 0;
  int y1 = gfx->data.modMinY; 
  int x2 = LCD_WIDTH-1; 
  int y2 = gfx->data.modMaxY;  
  set_cs();
  disp_spi_transfer_addrwin(x1, y1, x2, y2);
  // FIXME: hack because SPI send on NRF52 fails for >65k transfers
  // we should fix this in jshardware.c
  unsigned char *p = &lcdBuffer[LCD_STRIDE*y1];
  int c = (y2+1-y1)*LCD_STRIDE;
  while (c) {
    int n = c;
    if (n>65535) n=65535;
    jshSPISendMany(
        _device,
        p,
        0,
        n,
        NULL);
    p+=n;
    c-=n;
  }
  rel_cs();
    // Reset modified-ness
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}

void graphicsInternalFlip() {
  lcd_spi_buf_flip(&graphicsInternal);
}

/// Flip buffer contents with the screen.
void lcd_flip(JsVar *parent, bool all) {
  if (all) {
    graphicsInternal.data.modMinX = 0;
    graphicsInternal.data.modMinY = 0;
    graphicsInternal.data.modMaxX = LCD_WIDTH-1;
    graphicsInternal.data.modMaxY = LCD_HEIGHT-1;
  }
  graphicsInternalFlip();
}

void jshLCD_SPI_InitInfo(JshLCD_SPI_Info *inf) {
  inf->pinCS         = PIN_UNDEFINED;
  inf->pinDC         = PIN_UNDEFINED;
  inf->pinflashCS    = 5;
  inf->width         = 454;
  inf->height        = 454;
  inf->colstart        = 0;
  inf->rowstart        = 0;
}

bool jsspiPopulateOptionsInfo( JshLCD_SPI_Info *inf, JsVar *options){
  jshLCD_SPI_InitInfo(inf);
  jsvConfigObject configs[] = {
    {"cs", JSV_PIN, &inf->pinCS},
    {"dc", JSV_PIN, &inf->pinDC},
    {"flashcs", JSV_PIN, &inf->pinflashCS},
    {"width", JSV_INTEGER , &inf->width},
    {"height", JSV_INTEGER , &inf->height},
    {"colstart", JSV_INTEGER , &inf->colstart},
    {"rowstart", JSV_INTEGER , &inf->rowstart},
  };  
  
  return jsvReadConfigObject(options, configs, sizeof(configs) / sizeof(jsvConfigObject))
          && inf->pinDC != PIN_UNDEFINED;
}

/*JSON{
  "type" : "idle",
  "generate" : "jswrap_lcd_spi_buf_idle"
}*/
bool jswrap_lcd_spi_buf_idle() {
    graphicsInternalFlip();
    return false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_spi_buf",
  "name" : "connect",
  "generate" : "jswrap_lcd_spi_buf_connect",
  "params" : [
    ["device","JsVar","The used SPI device"],
    ["options","JsVar","An Object containing extra information"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}*/
JsVar *jswrap_lcd_spi_buf_connect(JsVar *device, JsVar *options) { 
  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) {
    jsExceptionHere(JSET_ERROR,"creating new object Graphics");
    return NULL;  
  }

  JshLCD_SPI_Info inf;

  if (!jsspiPopulateOptionsInfo(&inf, options)) {
    jsExceptionHere(JSET_ERROR,"pins not supplied correctly");
    jsvUnLock(parent);
    return NULL;
  }
  _pin_cs = inf.pinCS;
  _pin_dc = inf.pinDC;
  _pin_flash_cs = inf.pinflashCS;
  _colstart = inf.colstart;
  _rowstart = inf.rowstart;
  _device = jsiGetDeviceFromClass(device);

  if (!DEVICE_IS_SPI(_device)) { 
    jsExceptionHere(JSET_ERROR,"Software SPI is not supported for now");
    jsvUnLock(parent);
    return NULL;
  }
  graphicsStructInit(&graphicsInternal,inf.width,inf.height,16);
  graphicsInternal.data.type = JSGRAPHICSTYPE_LCD_SPI_BUF;
  graphicsInternal.graphicsVar = parent;
  
  jshPinOutput(_pin_dc, 1);
  jshPinSetValue(_pin_dc, 1);

  jshPinOutput(_pin_cs, 1);
  jshPinSetValue(_pin_cs, 1);
  
  lcd_spi_buf_setCallbacks(&graphicsInternal);

// Create 'flip' fn
  JsVar *fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(parent,"flip",fn);

  return parent;
}

unsigned int lcd_spi_buf_getPixel(JsGraphics *gfx, int x, int y) {
  int addr = (x<<1) + (y*LCD_STRIDE);
  uint16_t *p = (uint16_t*)(&lcdBuffer[addr]);
  return __builtin_bswap16(*p);
}

void lcd_spi_buf_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  int addr = (x<<1) + (y*LCD_STRIDE);
  uint16_t *p = (uint16_t*)(&lcdBuffer[addr]);
  *p = __builtin_bswap16(col);
}

void lcd_spi_buf_fillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  // or update just part of it.
  uint16_t c = __builtin_bswap16(col);
  uint16_t *ptr = (uint16_t*)(lcdBuffer) + x1 + (y1*LCD_WIDTH);
  if (y1==y2) {
    // if doing one line, avoid stride calculations.
    for (int x=x1;x<=x2;x++)
      *(ptr++) = c;
  } else {
    // handle cases where we can just memset
    if (x1==0 && x2==LCD_WIDTH-1 && (col&255)==((col>>8)&255)) {
      memset(&lcdBuffer[y1*LCD_STRIDE], col&255, LCD_STRIDE*(y2+1-y1));
    } else {
      // otherwise update a rect
      int stride = LCD_WIDTH - (x2+1-x1);
      for (int y=y1;y<=y2;y++) {
        for (int x=x1;x<=x2;x++)
          *(ptr++) = c;
        ptr += stride;
      }
    }
  }
}

void lcd_spi_buf_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcd_spi_buf_setPixel;
  gfx->fillRect = lcd_spi_buf_fillRect;
  gfx->getPixel = lcd_spi_buf_getPixel;
}



/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_spi_buf",
  "name" : "command",
  "generate" : "jswrap_lcd_spi_buf_command",
  "params" : [
     ["cmd","int32","The 8 bit command"],
    ["data","JsVar","The data to send with the command either an integer, array, or string"]
  ]
}
Used to send initialisation commands to LCD driver - has to be in native C to allow sharing SPI pins with SPI flash
Must not be called before connect sets up device.
 */
void jswrap_lcd_spi_buf_command(int cmd, JsVar *data) {
   unsigned char cc = (unsigned char)cmd;
   unsigned char bb[64]; // allow for up to 64 data bytes;
   int len = 0;
   set_cs(); 
  if (!data) {
    spi_cmd(cc,NULL,NULL);
  } else if (jsvIsNumeric(data)) {
    bb[0] = (unsigned char)jsvGetInteger(data);len =1;
    spi_cmd(cc,&bb,1);
  } else if (jsvIsIterable(data)) {
    JsvIterator it;
    jsvIteratorNew(&it, data, JSIF_EVERY_ARRAY_ELEMENT);
    while (jsvIteratorHasElement(&it) && len<64) {
      bb[len] = (unsigned char)jsvIteratorGetIntegerValue(&it); ++len;
      jsvIteratorNext(&it);
    }
    jsvIteratorFree(&it);
    spi_cmd(cc,&bb,len);
  } else {
    jsExceptionHere(JSET_ERROR, "data Variable type %t not suited to command operation", data);
  }
  rel_cs();
}
