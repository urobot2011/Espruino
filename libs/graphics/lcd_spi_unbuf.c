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
  "class" : "lcd_spi_unbuf"
}*/

#include "lcd_spi_unbuf.h"
#include "jsutils.h"
#include "jsinteractive.h"
#include "jswrapper.h"
#include "jswrap_graphics.h"
#include "jshardware.h"

static int _pin_mosi;
static int _pin_clk;
static int _pin_cs;
static int _pin_dc;
static int _colstart;
static int _rowstart;
static int _lastx=-1;
static int _lasty=-1;
static uint16_t _chunk_buffers[2][LCD_SPI_UNBUF_LEN];
volatile uint16_t *_chunk_buffer = &_chunk_buffers[0][0];
volatile int _current_buf = 0;
static int _chunk_index = 0;
IOEventFlags _device;

volatile int _cs_count=0; // to manage selected pin

static void set_cs(){
  if (!_cs_count) {
#ifdef SPIFLASH_SHARED_SPI
    jshSPIEnable(_device,true);
#endif
    jshPinSetValue(_pin_cs, 0);
  }
  ++_cs_count;
}

static void rel_cs(){
  --_cs_count;
  if (!_cs_count) {
    jshPinSetValue(_pin_cs, 1);
#ifdef SPIFLASH_SHARED_SPI
    jshSPIEnable(_device,false);
#endif
  }
}

static void spi_cmd(const uint8_t cmd, uint8_t *data, int dsize){
  jshPinSetValue(_pin_dc, 0);
  jshSPISendMany(_device, &cmd, NULL, 1, NULL);
  jshPinSetValue(_pin_dc, 1);
  if (data) jshSPISendMany(_device, data, NULL, dsize, NULL);
}

static void endxfer(){
  rel_cs();
}

static void flush_chunk_buffer(){
  if(_chunk_index == 0) return;
  set_cs();
  jshSPISendMany(_device,(uint8_t *)_chunk_buffer,NULL, _chunk_index*2,&endxfer);
  _chunk_index = 0;
  _current_buf = _current_buf?0:1;
  _chunk_buffer = &_chunk_buffers[_current_buf][0];
}

 /// flush chunk buffer to screen
void lcd_flip(JsVar *parent) {
  if(_chunk_index == 0) return;
  set_cs();
  flush_chunk_buffer();
  rel_cs();
}

void jshLCD_SPI_UNBUFInitInfo(JshLCD_SPI_UNBUFInfo *inf) {
  inf->pinCS         = PIN_UNDEFINED;
  inf->pinDC         = PIN_UNDEFINED;
  inf->width         = 240;
  inf->height        = 320;
  inf->colstart        = 0;
  inf->rowstart        = 0;
}

bool jsspiPopulateOptionsInfo( JshLCD_SPI_UNBUFInfo *inf, JsVar *options){
  jshLCD_SPI_UNBUFInitInfo(inf);
  jsvConfigObject configs[] = {
    {"cs", JSV_PIN, &inf->pinCS},
    {"dc", JSV_PIN, &inf->pinDC},
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
  "generate" : "jswrap_lcd_spi_unbuf_idle"
}*/
bool jswrap_lcd_spi_unbuf_idle() {
    lcd_flip(NULL);
    return false;
}

/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_spi_unbuf",
  "name" : "connect",
  "generate" : "jswrap_lcd_spi_unbuf_connect",
  "params" : [
    ["device","JsVar","The used SPI device"],
    ["options","JsVar","An Object containing extra information"]
  ],
  "return" : ["JsVar","The new Graphics object"],
  "return_object" : "Graphics"
}*/
JsVar *jswrap_lcd_spi_unbuf_connect(JsVar *device, JsVar *options) { 
  JsVar *parent = jspNewObject(0, "Graphics");
  if (!parent) {
    jsExceptionHere(JSET_ERROR,"creating new object Graphics");
    return NULL;  
  }

  JshLCD_SPI_UNBUFInfo inf;

  if (!jsspiPopulateOptionsInfo(&inf, options)) {
    jsExceptionHere(JSET_ERROR,"pins not supplied correctly");
    jsvUnLock(parent);
    return NULL;
  }
  _pin_cs = inf.pinCS;
  _pin_dc = inf.pinDC;
  _colstart = inf.colstart;
  _rowstart = inf.rowstart;
  _device = jsiGetDeviceFromClass(device);

  if (!DEVICE_IS_SPI(_device)) { 
    jsExceptionHere(JSET_ERROR,"Software SPI is not supported for now");
    jsvUnLock(parent);
    return NULL;
  }

  JsGraphics gfx;
  graphicsStructInit(&gfx,inf.width,inf.height,16);
  gfx.data.type = JSGRAPHICSTYPE_LCD_SPI_UNBUF;
  gfx.graphicsVar = parent;
  
  jshPinOutput(_pin_dc, 1);
  jshPinSetValue(_pin_dc, 1);

  jshPinOutput(_pin_cs, 1);
  jshPinSetValue(_pin_cs, 1);
  
  lcd_spi_unbuf_setCallbacks(&gfx);
  graphicsSetVar(&gfx); 

  // Create 'flip' fn
  JsVar *fn;
  fn = jsvNewNativeFunction((void (*)(void))lcd_flip, JSWAT_VOID|JSWAT_THIS_ARG);
  jsvObjectSetChildAndUnLock(parent,"flip",fn);

  return parent;
}

void disp_spi_transfer_addrwin(int x1, int y1, int x2, int y2) {
  unsigned char wd[4];
  flush_chunk_buffer();
  jshSPIWait(_device); //wait for any async transfer to finish
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

void lcd_spi_unbuf_setPixel(JsGraphics *gfx, int x, int y, unsigned int col) {
  uint16_t color =   (col>>8) | (col<<8); 
  if (x!=_lastx+1 || y!=_lasty) {
    set_cs();
    disp_spi_transfer_addrwin(x, y, gfx->data.width, y+1);  
    rel_cs(); //will never flush after 
    _chunk_buffer[_chunk_index++] = color;
    _lastx = x;
    _lasty = y;
  } else {
    _lastx++; 
    if ( _chunk_index == LCD_SPI_UNBUF_LEN - 1){
      _chunk_buffer[_chunk_index++] = color;
      flush_chunk_buffer();
    } else {
        _chunk_buffer[_chunk_index++] = color;
    } 
  }
}

/*
* Optimised so that we only fill buffer with pixel color once 
* Leaves _chunk_index at 0 and all pixels transferred
* does nout use double buffering as not helpful here.
*/
void lcd_spi_unbuf_fillRect(JsGraphics *gfx, int x1, int y1, int x2, int y2, unsigned int col) {
  int pixels = (1+x2-x1)*(1+y2-y1); 
  uint16_t color =   (col>>8) | (col<<8); 
  set_cs();
  disp_spi_transfer_addrwin(x1, y1, x2, y2); //always flushes buffer 
  int fill = pixels>LCD_SPI_UNBUF_LEN ? LCD_SPI_UNBUF_LEN : pixels;
  for (int i=0; i<fill; i++) _chunk_buffer[i] = color; //fill buffer with color for reuse
  while (pixels>=fill) {
       jshSPISendMany(_device,(uint8_t *)_chunk_buffer,NULL, fill*2,NULL);
       pixels-=fill;
  }
  if (pixels>0) jshSPISendMany(_device,(uint8_t *)_chunk_buffer,NULL, pixels*2,NULL);
  rel_cs();
  _lastx=-1;
  _lasty=-1;
}

void lcd_spi_unbuf_setCallbacks(JsGraphics *gfx) {
  gfx->setPixel = lcd_spi_unbuf_setPixel;
  gfx->fillRect = lcd_spi_unbuf_fillRect;
}


/*JSON{
  "type" : "staticmethod",
  "class" : "lcd_spi_unbuf",
  "name" : "command",
  "generate" : "jswrap_lcd_spi_unbuf_command",
  "params" : [
     ["cmd","int32","The 8 bit command"],
    ["data","JsVar","The data to send with the command either an integer, array, or string"]
  ]
}
Used to send initialisation commands to LCD driver - has to be in native C to allow sharing SPI pins with SPI flash
Must not be called before connect sets up device.
 */
void jswrap_lcd_spi_unbuf_command(int cmd, JsVar *data) {
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

