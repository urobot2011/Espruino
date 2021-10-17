/**
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2020 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * Bluetooth Apple Notifications Handler
 * ----------------------------------------------------------------------------
 */

#include "nrf_ble_ancs_c.h"

/// Perform the given action for the current notification (positive/negative)
void ble_ancs_action(uint32_t uid, bool positive);
// Request the attributes for notification identified by uid
void ble_ancs_request(uint32_t uid);

// These functions are called from bluetooth.c
void ble_ancs_init();
void ble_ancs_get_adv_uuid(ble_uuid_t *p_adv_uuids);
void ble_ancs_bonding_succeeded(uint16_t conn_handle);
void ble_ancs_on_ble_evt(ble_evt_t * p_ble_evt);

/** Handle the events (called outside of IRQ by Espruino) - will poke the relevant events in */
void ble_ancs_handle_notif(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif);
void ble_ancs_handle_attr(BLEPending blep, ble_ancs_c_evt_notif_t *p_notif);