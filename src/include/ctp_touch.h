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
 * @file ctp_touch.h
 * @brief Internal utility functions for touch point manipulation and
 * validation.
 * @details This header provides the logic for managing the lifecycle of a touch
 * event, including noise filtering (validation) and state comparison. These
 * functions are intended for internal use by the driver core and should not be
 * exposed in the public API.
 */

#ifndef CTP_TOUCH_H
#define CTP_TOUCH_H

#include <gt9271/ctp_types.h>

/**
 * @brief Resets a single touch point to its default, inactive state.
 *
 * This function zeros out all coordinate and size data and marks the touch as
 * not active. It is used primarily during initialization or when a release
 * event is processed.
 *
 * @param[out] touch Pointer to the touch point structure to be cleared.
 * @pre touch must not be NULL.
 * @post touch->is_touched will be false and all numeric values set to 0.
 */
void ctp_clear_touch(ctp_touch_t* const touch);

/**
 * @brief Clears the entire list of touch points and resets the active count.
 *
 * Iterates through the full capacity of the touch list and calls
 * ctp_clear_touch() on every slot. This ensures no stale data remains from
 * previous sessions.
 *
 * @param[out] touch_list Pointer to the touch list structure to be cleared.
 * @pre touch_list must not be NULL.
 * @post touch_list->count will be 0 and all slots will be inactive.
 */
void ctp_clear_touch_event_list(ctp_touch_list_t* const touch_list);

/**
 * @brief Validates whether a touch point is physically plausible.
 *
 * This function acts as a software noise filter. It rejects points that:
 * 1. Have a track ID outside the supported range.
 * 2. Have a size smaller than CTP_MIN_TOUCH_SIZE or larger than
 * CTP_MAX_TOUCH_SIZE.
 * 3. Fall outside the defined screen boundaries (CTP_MAX_X, CTP_MAX_Y).
 * 4. Report coordinates at exactly (0,0), which is a common failure mode for
 *    noise-induced "ghost touches" on this hardware.
 *
 * @param[in] touch Pointer to the touch point to validate.
 * @return true if the touch point meets all validity criteria; false otherwise.
 */
bool is_touch_valid(const ctp_touch_t* const touch);

/**
 * @brief Determines if two touch points are identical in value and identity.
 *
 * Used to detect movement or size changes between polling cycles. A touch is
 * considered equal only if the track ID, X coordinate, Y coordinate, and
 * touch size are all identical. X and Y will be considered the same if the
 * delta is less than @ref CTP_TOUCH_MOVEMENT_THRESHOLD
 *
 * @param[in] a Pointer to the first touch point (usually the cached state).
 * @param[in] b Pointer to the second touch point (usually the new hardware
 * read).
 * @return true if no changes were detected; false if any value has shifted.
 */
bool are_touches_equal(const ctp_touch_t* const a, const ctp_touch_t* const b);

/**
 * @brief Copies coordinate and size data from one touch point to another.
 *
 * This function performs a shallow copy of the physical properties (ID, X, Y,
 * Size). It does NOT copy state flags such as is_touched or event, as these are
 * managed by the driver's state machine logic in ctp_check_for_change().
 *
 * @param[out] dest Pointer to the destination touch point.
 * @param[in] src Pointer to the source touch point containing new data.
 * @pre Both dest and src must not be NULL.
 */
void copy_touch(ctp_touch_t* const dest, const ctp_touch_t* const src);

#endif  // CTP_TOUCH_H
