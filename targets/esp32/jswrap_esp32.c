/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * ESP32 specific exposed components.
 * ----------------------------------------------------------------------------
 */
#include <stdio.h>

#include "jswrap_esp32.h"
#include "jshardwareAnalog.h"
#include "jsutils.h"
#include "jsinteractive.h"
#include "jsparse.h"
#include "jsflash.h"

#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_heap_caps.h"
#include "esp_wifi.h"
#include "driver/adc.h"
#include "soc/rtc.h"

#ifdef BLUETOOTH
#include "BLE/esp32_bluetooth_utils.h"
#endif
#include "jshardwareESP32.h"

#include "jsutils.h"
#include "jsinteractive.h"
#include "jsparse.h"


/*JSON{
  "type": "class",
  "class" : "ESP32",
  "ifdef" : "ESP32"
}
Class containing utility functions for the [ESP32](http://www.espruino.com/ESP32)
*/

/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
 "ifdef" : "ESP32",
 "name"     : "setAtten",
 "generate" : "jswrap_ESP32_setAtten",
 "params"   : [
   ["pin", "pin", "Pin for Analog read"],
   ["atten", "int", "Attenuate factor"]
 ]
}*/
void jswrap_ESP32_setAtten(Pin pin,int atten){
  printf("Atten:%d\n",atten);
  rangeADC(pin, atten);
}

/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "reboot",
  "generate" : "jswrap_ESP32_reboot"
}
Perform a hardware reset/reboot of the ESP32.
*/
void jswrap_ESP32_reboot() {
  jshReboot();
} // End of jswrap_ESP32_reboot


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "deepSleep",
  "generate" : "jswrap_ESP32_deepSleep",
  "params"   : [["us", "int", "Sleeptime in us"],
                ["wakepin", "JsVar", "optional - Pin to wake from sleep"],
                ["mode", "JsVar", "optional - 0 wake when pin low, 1 wake when pin high"]
               ]
}
Put device in deepsleep state for "us" microseconds.
If "us" is 0 deepsleep state is indefinite, specify wakepin to awaken device from this state.
If "us" is <0 do lightsleep, experimental
*/

void jswrap_ESP32_deepSleep(int us, JsVar *wakepin, JsVar *mode) {
    int md = 0;
    if (jsvIsNumeric(mode)) {
      md = jsvGetInteger(mode);
      md = md > 0?1:0;
    }
    Pin wp = jshGetPinFromVar(wakepin);
    if (jshIsPinValid(wp)) esp_sleep_enable_ext0_wakeup((gpio_num_t)wp, md);
    if (us > 0) esp_sleep_enable_timer_wakeup((uint64_t)(us));
    else if (us==0)
       esp_deep_sleep_start(); // This function does not return.
    esp_light_sleep_start();
} // End of jswrap_ESP32_deepSleep


/*JSON{
  "type"     : "staticmethod",
  "class"    : "ESP32",
  "ifdef" : "ESP32",
  "name"     : "getState",
  "generate" : "jswrap_ESP32_getState",
  "return"   : ["JsVar", "The state of the ESP32"]
}
Returns an object that contains details about the state of the ESP32 with the following fields:

* `sdkVersion`   - Version of the SDK.
* `freeHeap`     - Amount of free heap in bytes.
* `BLE`			 - Status of BLE, enabled if true.
* `Wifi`		 - Status of Wifi, enabled if true.
* `minHeap`      - Minimum heap, calculated by heap_caps_get_minimum_free_size

*/
JsVar *jswrap_ESP32_getState() {
  // Create a new variable and populate it with the properties of the ESP32 that we
  // wish to return.
  JsVar *esp32State = jsvNewObject();
  jsvObjectSetChildAndUnLock(esp32State, "sdkVersion",   jsvNewFromString(esp_get_idf_version()));
  jsvObjectSetChildAndUnLock(esp32State, "freeHeap",     jsvNewFromInteger(esp_get_free_heap_size()));
  jsvObjectSetChildAndUnLock(esp32State, "BLE",          jsvNewFromBool(ESP32_Get_NVS_Status(ESP_NETWORK_BLE)));
  jsvObjectSetChildAndUnLock(esp32State, "Wifi",         jsvNewFromBool(ESP32_Get_NVS_Status(ESP_NETWORK_WIFI)));  
  jsvObjectSetChildAndUnLock(esp32State, "minHeap",      jsvNewFromInteger(heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT)));
  jsvObjectSetChildAndUnLock(esp32State, "cpuFreq",      jsvNewFromInteger(rtc_clk_cpu_freq_value(rtc_clk_cpu_freq_get())));
  jsvObjectSetChildAndUnLock(esp32State, "xtalFreq",      jsvNewFromInteger(rtc_clk_xtal_freq_get()));
  jsvObjectSetChildAndUnLock(esp32State, "apbFreq",      jsvNewFromInteger(rtc_clk_apb_freq_get()));
  return esp32State;
} // End of jswrap_ESP32_getState

#ifdef BLUETOOTH
/*JSON{
 "type"     : "staticmethod",
 "class"    : "ESP32",
 "ifdef" : "ESP32",
 "name"     : "setBLE_Debug",
 "generate" : "jswrap_ESP32_setBLE_Debug",
 "params"   : [
   ["level", "int", "which events should be shown (GATTS, GATTC, GAP)"]
 ],
 "ifdef"	: "BLUETOOTH"
}
*/
void jswrap_ESP32_setBLE_Debug(int level){
	ESP32_setBLE_Debug(level);
}

/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "enableBLE",
 "generate"	: "jswrap_ESP32_enableBLE",
 "params"	: [
   ["enable", "bool", "switches Bluetooth on or off" ]
 ],
 "ifdef"	: "BLUETOOTH" 
}
Switches Bluetooth off/on, removes saved code from Flash, resets the board, 
and on restart creates jsVars depending on available heap (actual additional 1800)
*/
void jswrap_ESP32_enableBLE(bool enable){ //may be later, we will support BLEenable(ALL/SERVER/CLIENT)
  ESP32_Set_NVS_Status(ESP_NETWORK_BLE,enable);
  jsfRemoveCodeFromFlash();
  esp_restart();
}
#endif
/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "enableWifi",
 "generate"	: "jswrap_ESP32_enableWifi",
 "params"	: [
   ["enable", "bool", "switches Wifi on or off" ]
 ] 
}
Switches Wifi off/on, removes saved code from Flash, resets the board, 
and on restart creates jsVars depending on available heap (actual additional 3900)
*/
void jswrap_ESP32_enableWifi(bool enable){ //may be later, we will support BLEenable(ALL/SERVER/CLIENT)
  ESP32_Set_NVS_Status(ESP_NETWORK_WIFI,enable);
  jsfRemoveCodeFromFlash();
  esp_restart();
}

/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "wifiStart",
 "generate"	: "jswrap_ESP32_wifiStart",
 "params"	: [
   ["start", "bool", "true = start, false = stop" ]
 ],
 "return" : ["int","The esp32 return code"] 
}
wifi control false = stopwifi, true = startwifi
*/
int jswrap_ESP32_wifiStart(bool start){ 
  if (start)
    return esp_wifi_start();
  else 
    return esp_wifi_stop();
}

/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "adcPower",
 "generate"	: "jswrap_ESP32_adcPower",
 "params"	: [
   ["on", "bool", "true = power on , false = power off" ]
 ]
}
used to turn off adc to save power
*/
void jswrap_ESP32_adcPower(bool on){ 
  if (on)
    return adc_power_on();
  else 
    return adc_power_off();
}


/*JSON{
 "type"	: "staticmethod",
 "class"	: "ESP32",
 "ifdef" : "ESP32",
 "name"		: "setCPUfreq",
 "generate"	: "jswrap_ESP32_setCPUfreq",
 "params"	: [
   ["freqNo", "int", "0 = XTAL freq , 1 = 80M, 2 = 160M, 3 = 240M" ]
 ]
}
used to turn off adc to save power
*/
void jswrap_ESP32_setCPUfreq(int freqNo){ 
  if (freqNo == rtc_clk_cpu_freq_get()) return;
  if (freqNo<0 || freqNo >3) return;
  rtc_clk_cpu_freq_set(freqNo);
}
