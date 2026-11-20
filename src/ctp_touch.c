/*
 * Copyright (c) 2026 Adaptive Circuits LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the same language governing permissions and
 * limitations under the License.
 */

/**
 * @file ctp_touch.c
 * @brief Implementation of touch point utility functions.
 * @details This file handles the low-level manipulation and validation of touch
 * data. It ensures that only physically plausible touch points are processed by
 * the driver, effectively acting as a software noise filter for the GT9271
 * hardware.
 */

#include "include/ctp_touch.h"

#include <stddef.h>
#include <stdlib.h>

void ctp_clear_touch(ctp_touch_t* const touch) {
  if (touch == NULL) {
    return;
  }

  touch->track_id = 0;
  touch->x = 0;
  touch->y = 0;
  touch->size = 0;
  touch->is_touched = false;
  touch->event = CTP_EVENT_NONE;
}

void ctp_clear_touch_event_list(ctp_touch_list_t* const touch_list) {
  if (touch_list == NULL) {
    return;
  }

  for (uint_least8_t i = 0; i < CTP_MAX_TOUCH_POINTS; i++) {
    ctp_clear_touch(&touch_list->touch[i]);
  }

  touch_list->count = 0;
  touch_list->key_status = 0;
}

bool is_touch_valid(const ctp_touch_t* const touch) {
  if (touch == NULL) {
    return false;
  }

  /* IDs must be within the range of our allocated slot array */
  if (touch->track_id >= CTP_MAX_TOUCH_POINTS) {
    return false;
  }

  /* Filter out points with implausible sizes (too small = noise, too large =
   * error) */
  if ((touch->size < CTP_MIN_TOUCH_SIZE) ||
      (touch->size > CTP_MAX_TOUCH_SIZE)) {
    return false;
  }

  /* Ensure coordinates are within the physical boundaries of the screen */
  if ((touch->x > CTP_MAX_X) || (touch->y > CTP_MAX_Y)) {
    return false;
  }

  /**
   * GHOST TOUCH FILTER:
   * The GT9271 occasionally reports a touch at (0,0) due to electrical noise
   * or power-on transients. Since it is physically improbable for a user
   * to touch the exact top-left pixel consistently, we reject these as invalid.
   */
  if ((touch->x == 0) && (touch->y == 0)) {
    return false;
  }

  return true;
}

bool are_touches_equal(const ctp_touch_t* const a, const ctp_touch_t* const b) {
  /* We check each field explicitly to allow for easier debugging with
   * breakpoints */
  if (a->track_id != b->track_id) {
    return false;
  }

  const uint16_t diff_x = (uint16_t)(abs((int16_t)a->x - (int16_t)b->x));
  const uint16_t diff_y = (uint16_t)(abs((int16_t)a->y - (int16_t)b->y));

  if ((diff_x > CTP_TOUCH_MOVEMENT_THRESHOLD) ||
      (diff_y > CTP_TOUCH_MOVEMENT_THRESHOLD)) {
    return false;
  }
  if (a->size != b->size) {
    return false;
  }

  return true;
}

void copy_touch(ctp_touch_t* const dest, const ctp_touch_t* const src) {
  /* Only copy physical properties. State flags (is_touched, event)
     are managed by the driver's state machine in ctp_check_for_change(). */
  dest->track_id = src->track_id;
  dest->x = src->x;
  dest->y = src->y;
  dest->size = src->size;
}
