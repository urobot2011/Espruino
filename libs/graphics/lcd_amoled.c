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
  "class" : "lcd_amoled"
}*/

#include "lcd_amoled.h"
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

#define LCD_WIDTH 454
#define LCD_HEIGHT 454
#define LCD_BPP 4
#define LCD_STRIDE 227
unsigned char lcdBuffer[LCD_STRIDE*LCD_HEIGHT];
unsigned short lcdPalette[16];
unsigned short _chunk_buffer[256];


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

void lcd_amoled_flip(JsGraphics *gfx) {
  if (gfx->data.modMinX > gfx->data.modMaxX) return; // nothing to do!
  //start on even row and col addresses
  int x1 = (gfx->data.modMinX)&~1; 
  int y1 = (gfx->data.modMinY)&~1; 
  int x2 = (x1 == (gfx->data.modMaxX) ? x1+1 : (gfx->data.modMaxX)); 
  int y2 = (y1 == (gfx->data.modMaxY) ? y1+1 : (gfx->data.modMaxY)); 
  set_cs();
  disp_spi_transfer_addrwin(x1, y1, x2, y2);
  int chunk_index=0;
  for (int y=y1; y<=y2; y++) {
    for (int x=x1; x<x2; x+=2) {
      unsigned char c = lcdBuffer[y*LCD_STRIDE + (x>>1)];
      _chunk_buffer[chunk_index++] = lcdPalette[c >> 4];
      _chunk_buffer[chunk_index++] = lcdPalette[c & 15];
      if (chunk_index>=256) {
         jshSPISendMany(_device,(uint8_t *)_chunk_buffer,NULL,512,NULL);
         chunk_index=0;
      }
    }
  }
  if (chunk_index>0) {
         jshSPISendMany(_device,(uint8_t *)_chunk_buffer,NULL,chunk_index*2,NULL);
         chunk_index=0;
  }
  rel_cs();
    // Reset modified-ness
  gfx->data.modMaxX = -32768;
  gfx->data.modMaxY = -32768;
  gfx->data.modMinX = 32767;
  gfx->data.modMinY = 32767;
}

void graphicsInternalFlip() {
  lcd_amoled_flip(&graphicsInternal);
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
  "generate" : "jswrap_lcd_amoled_idle"
}*/
bool jswrap_lcd_amoled_idle() {
    graphicsInternalFlip();
    return false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_amoled",
  "name" : "connect",
  "generate" : "jswrap_lcd_amoled_connect",
  "params" : [
    ["device","JsVar","The used SPI device"],
    ["options","JsVar","An Object containing extra information"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}*/
JsVar *jswrap_lcd_amoled_connect(JsVar *device, JsVar *options) { 
  for(int i=0;i<16;i++) lcdPalette[i] = __builtin_bswap16(PALETTE_4BIT[i]);
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
  graphicsStructInit(&graphicsInternal,inf.width,inf.height,4);
  graphicsInternal.data.type = JSGRAPHICSTYPE_LCD_AMOLED;
  graphicsInternal.graphicsVar = parent;
  
  jshPinOutput(_pin_dc, 1);
  jshPinSetValue(_pin_dc, 1);

  jshPinOutput(_pin_cs, 1);
  jshPinSetValue(_pin_cs, 1);
  
  lcd_amoled_setCallbacks(&graphicsInternal);

// Create 'flip' fn
  JsVar *fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG|(JSWAT_BOOL << (JSWAT_BITS*1)));
  jsvObjectSetChildAndUnLock(parent,"flip",fn);

  return parent;
}

unsigned int lcd_amoled_getPixel(JsGraphics *gfx, int x, int y) {
  int addr = (x + (y*LCD_WIDTH)) >> 1;
  unsigned char b = lcdBuffer[addr];
  return (x&1) ? (b&15) : (b>>4);
}

void lcd_amoled_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  int addr = (x + (y*LCD_WIDTH)) >> 1;
  if (x&1) lcdBuffer[addr] = (lcdBuffer[addr] & 0xF0) | (col&0x0F);
  else lcdBuffer[addr] = (lcdBuffer[addr] & 0x0F) | (col << 4);
}

void lcd_amoled_fillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  unsigned char cc = col<<4 | col;
  for (int y=y1; y<=y2; y++) {
    int startx = x1; 
    int endx = x2;
    if (x1 & 1) {lcd_amoled_setPixel(gfx,x1,y,col); ++startx;}
    if (x2 & 1) {lcd_amoled_setPixel(gfx,x2,y,col); --endx;}
    for (int x=startx; x<=endx; x+=2) 
        lcdBuffer[y*LCD_STRIDE + (x>>1)]=cc;
  }
}

void lcd_amoled_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcd_amoled_setPixel;
  gfx->fillRect = lcd_amoled_fillRect;
  gfx->getPixel = lcd_amoled_getPixel;
}



/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_amoled",
  "name" : "command",
  "generate" : "jswrap_lcd_amoled_command",
  "params" : [
     ["cmd","int32","The 8 bit command"],
    ["data","JsVar","The data to send with the command either an integer, array, or string"]
  ]
}
Used to send initialisation commands to LCD driver - has to be in native C to allow sharing SPI pins with SPI flash
Must not be called before connect sets up device.
 */
void jswrap_lcd_amoled_command(int cmd, JsVar *data) {
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

