#!/bin/false
# This file is part of Espruino, a JavaScript interpreter for Microcontrollers
#
# Copyright (C) 2013 Gordon Williams <gw@pur3.co.uk>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# ----------------------------------------------------------------------------------------
# This file contains information for a specific board - the available pins, and where LEDs,
# Buttons, and other in-built peripherals are. It is used to build documentation as well
# as various source and header files for Espruino.
# ----------------------------------------------------------------------------------------

import pinutils;


info = {
 'name' : "nRF52840 Dongle",
 'link' :  [ "https://www.nordicsemi.com/Software-and-Tools/Development-Kits/nRF52840-Dongle" ],
 'espruino_page_link' : 'nRF52840DONGLE',
  # This is the PCA10059
 'default_console' : "EV_SERIAL1",
 'default_console_tx' : "D15",
 'default_console_rx' : "D13",
 'default_console_baudrate' : "9600",
 'variables' : 12500, # How many variables are allocated for Espruino to use. RAM will be overflowed if this number is too high and code won't compile.
# 'bootloader' : 1,
 'binary_name' : 'espruino_%v_nrf52840_dongle.hex',
 'build' : {
   'optimizeflags' : '-Os',
   'libraries' : [
     'BLUETOOTH',
     'NET',
     'GRAPHICS',
#     'NFC',
     'NEOPIXEL'
   ],
   'makefile' : [
     'DEFINES += -DCONFIG_NFCT_PINS_AS_GPIOS', # Allow us to use NFC pins as GPIO
     'DEFINES += -DESPR_LSE_ENABLE ', # Ensure low speed external osc enabled
     'DEFINES += -DNRF_SDH_BLE_GATT_MAX_MTU_SIZE=131', # 23+x*27 rule as per https://devzone.nordicsemi.com/f/nordic-q-a/44825/ios-mtu-size-why-only-185-bytes
     'LDFLAGS += -Xlinker --defsym=LD_APP_RAM_BASE=0x2ec0', # set RAM base to match MTU
     'DEFINES += -DESPR_DCDC_ENABLE=1', # Use DC/DC converter
     'ESPR_BLUETOOTH_ANCS=1', # Enable ANCS (Apple notifications) support
     'DEFINES += -DCONFIG_GPIO_AS_PINRESET', # Allow the reset pin to work
     'DEFINES += -DBOARD_PCA10059',          # nRF52840-QIAA
     'DEFINES += -DNRF_USB=1 -DUSB',
     'DEFINES += -DPIN_NAMES_DIRECT=1', # Package skips out some pins, so we can't assume each port starts from 0
     'NRF_SDK15=1'
   ]
 }
};


chip = {
  'part' : "NRF52840",
  'family' : "NRF52",
  'package' : "QFN48",
  'ram' : 256,
  'flash' : 1024,
  'speed' : 64,
  'usart' : 2,
  'spi' : 3,
  'i2c' : 2,
  'adc' : 1,
  'dac' : 0,
  'saved_code' : {
    'address' : ((223 - 10) * 4096), # Dongle - default USB bootloader starts at 0xE0000
    'page_size' : 4096,
    'pages' : 10,
    'flash_available' : 1024 - ((31 + 30 + 2 + 10)*4) # Softdevice uses 31 pages of flash, USB bootloader 30, FS 2, code 10. Each page is 4 kb.
  },
};
# STORAGE: 966656 (page 236) -> 1007616 (page 246)
#       flashmap: 0xec000 == 966656 - 0xed854 == 972884 6228byte
# CODE: 155648 -> 617792 (462144 bytes)


# dongle bootloader: 0xe0000 == 917504 -> 0xFE000 == 1040384
# dongle FLASH_SIZE=0xdf000 913408 to make sure that there is room for the bootloader at the end of the flash.
#  ==> page 223 must be the end of the stored code area

devices = {
  'BTN1' : { 'pin' : 'D38' }, # the button is at P1.06
  'LED1' : { 'pin' : 'D6' },  # green LED
  'LED2' : { 'pin' : 'D8' },   # RGB LED / red   P0.08
  'LED3' : { 'pin' : 'D41' },  # RGB LED / green P1.09
  'LED4' : { 'pin' : 'D12' },  # RGB LED / blue  P0.12
  'RX_PIN_NUMBER' : { 'pin' : 'D13'},
  'TX_PIN_NUMBER' : { 'pin' : 'D15'},
};

# left-right, or top-bottom order
# pins on "top" edge:
# 0.10 PD10
# 0.09 PD9
# 1.00 PD32
# 0.24 PD24
# 0.22 PD22
# 0.20 PD20
# 0.17 PD17
# 0.15 PD15
# 0.13 PD13
# pins on "bottom" edge:
# 1.10 PD42
# 1.13 PD45
# 1.15 PD47
# 0.02 PD2
# 0.29 PD29
# 0.31 PD31
# test points on the bottom of board:
# 1.01 PD33
# 1.02 PD32
# 1.04 PD36
# 1.07 PD39
# 1.11 PD43
# 0.04 PD4
# 0.11 PD11
# 0.14 PD14
# 0.26 PD26
board = {
  'top' : [ 'GND', 'D10', 'D9', 'D32','D24','D22','D20','D17','D15','D13'],
  'bottom' : ['GND', 'D43', 'D45', 'D47', 'D2','D29','D31', 'GND','VDD_OUT','VBUS',''],
  '_notes' : {
    'D13' : "Serial console RX",
    'D15' : "Serial console TX"
  }
};
board["_css"] = """
#board {
  width: 528px;
  height: 800px;
  top: 0px;
  left : 200px;
  background-image: url(img/NRF528DK.jpg);
}
#boardcontainer {
  height: 900px;
}

#left {
    top: 219px;
    right: 466px;
}
#right {
    top: 150px;
    left: 466px;
}

.leftpin { height: 17px; }
.rightpin { height: 17px; }
""";

def get_pins():
  pins = []
  pinutils.findpin(pins, "PD0", False)["functions"]["XL1"]=0;
  pinutils.findpin(pins, "PD1", False)["functions"]["XL2"]=0;
  pinutils.findpin(pins, "PD2", False)["functions"]["ADC1_IN0"]=0;
  # 0.3 not connected pin
  pinutils.findpin(pins, "PD4", False)["functions"]["ADC1_IN2"]=0;
  # 0.5 not connected pin
  pinutils.findpin(pins, "PD6", False)["functions"]["NEGATED"]=0; # LED1 (G)
  # 0.7 not connected pin
  pinutils.findpin(pins, "PD8", False)["functions"]["NEGATED"]=0; # RGB LED / red  P0.08
  pinutils.findpin(pins, "PD9", False)
  pinutils.findpin(pins, "PD10", False)
  pinutils.findpin(pins, "PD11", False)
  pinutils.findpin(pins, "PD12", False)["functions"]["NEGATED"]=0; # RGB LED / blue  P0.12
  pinutils.findpin(pins, "PD13", False)["functions"]["RXD"]=0;
  pinutils.findpin(pins, "PD14", False)
  pinutils.findpin(pins, "PD15", False)["functions"]["TXD"]=0;
  # 0.16 not connected pin
  pinutils.findpin(pins, "PD17", False)
  pinutils.findpin(pins, "PD18", False)
  # SW2 is also connected to P0.19, P0.21, P0.23, and P0.25. This is done to simplify PCB routing. These
  # GPIOs should not be used and should be left as input with no pull or be disconnected by firmware.
  # pinutils.findpin(pins, "PD19", False)
  pinutils.findpin(pins, "PD20", False)
  # pinutils.findpin(pins, "PD21", False)
  pinutils.findpin(pins, "PD22", False)
  # pinutils.findpin(pins, "PD23", False)
  pinutils.findpin(pins, "PD24", False)
  # pinutils.findpin(pins, "PD25", False)
  pinutils.findpin(pins, "PD26", False)
  # 0.27 not connected pin
  # 0.28 not connected pin
  pinutils.findpin(pins, "PD29", False)["functions"]["ADC1_IN5"]=0;
  # 0.30 not connected pin
  pinutils.findpin(pins, "PD31", False)["functions"]["ADC1_IN7"]=0;
  pinutils.findpin(pins, "PD32", False)
  pinutils.findpin(pins, "PD33", False)
  pinutils.findpin(pins, "PD34", False)
  # 1.3 not connected pin
  pinutils.findpin(pins, "PD36", False)
  # 1.5 not connected pin
  pinutils.findpin(pins, "PD38", False)["functions"]["NEGATED"]=0; # the button is at P1.06
  pinutils.findpin(pins, "PD39", False)
  # 1.8 not connected pin
  pinutils.findpin(pins, "PD41", False)["functions"]["NEGATED"]=0; # RGB LED / green P1.09
  pinutils.findpin(pins, "PD42", False)
  pinutils.findpin(pins, "PD43", False)
  # 1.12 not connected pin
  pinutils.findpin(pins, "PD45", False)
  # 1.14 not connected pin
  pinutils.findpin(pins, "PD47", False)

  # # pins = pinutils.generate_pins(0,47) # 48 General Purpose I/O Pins.

  # pinutils.findpin(pins, "PD0", False)["functions"]["XL1"]=0;
  # pinutils.findpin(pins, "PD1", False)["functions"]["XL2"]=0;
  # pinutils.findpin(pins, "PD2", False)["functions"]["ADC1_IN0"]=0;
  # #  NC
  # pinutils.findpin(pins, "PD4", False)["functions"]["ADC1_IN2"]=0;
  # # pinutils.findpin(pins, "PD5", False)["functions"]["ADC1_IN3"]=0; # NC
  # pinutils.findpin(pins, "PD6", False)["functions"]["NEGATED"]=0; # LED1 (G)
  # # pinutils.findpin(pins, "PD28", False)["functions"]["ADC1_IN4"]=0; # NC
  # pinutils.findpin(pins, "PD29", False)["functions"]["ADC1_IN5"]=0;
  # # pinutils.findpin(pins, "PD30", False)["functions"]["ADC1_IN6"]=0; # NC
  # pinutils.findpin(pins, "PD31", False)["functions"]["ADC1_IN7"]=0;

  # pinutils.findpin(pins, "PD13", False)["functions"]["RXD"]=0;
  # pinutils.findpin(pins, "PD15", False)["functions"]["TXD"]=0;

  # # Make button and LEDs negated

  # pinutils.findpin(pins, "PD8", False)["functions"]["NEGATED"]=0;
  # pinutils.findpin(pins, "PD41", False)["functions"]["NEGATED"]=0;
  # pinutils.findpin(pins, "PD12", False)["functions"]["NEGATED"]=0;
  # pinutils.findpin(pins, "PD38", False)["functions"]["NEGATED"]=0;


  # # pins.remove(pinutils.findpin(pins, "PD3", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD5", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD7", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD16", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD26", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD27", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD30", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD35", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD37", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD40", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD44", False))  # Not connected pin
  # # pins.remove(pinutils.findpin(pins, "PD46", False))  # Not connected pin

  # # # Remove these pins, because:
  # # # SW2 is also connected to P0.19, P0.21, P0.23, and P0.25. This is done to simplify PCB routing. These
  # # # GPIOs should not be used and should be left as input with no pull or be disconnected by firmware.
  # # pins.remove(pinutils.findpin(pins, "PD19", False))
  # # pins.remove(pinutils.findpin(pins, "PD21", False))
  # # pins.remove(pinutils.findpin(pins, "PD23", False))
  # # pins.remove(pinutils.findpin(pins, "PD25", False))

  # everything is non-5v tolerant
  for pin in pins:
    pin["functions"]["3.3"]=0;
  print('pins:', pins)
  return pins
  