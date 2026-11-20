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
 * @file ctp_defs.h
 * @brief Global constants and hardware limits for the GT9271 driver.
 * @details This file contains the fixed constraints of the GT9271 hardware,
 * panel-specific geometry, and software tuning parameters used for noise
 * filtering.
 */

#ifndef CTP_DEFS_H
#define CTP_DEFS_H

/* ==========================================================================
   HARDWARE CONSTANTS (Fixed by GT9271 Specification)
   ========================================================================== */

/** @brief Required configuration length based on register range 0x8047 to
 * 0x8100. */
#define CTP_CONFIG_REQUIRED_LEN (186U)

/** @brief Maximum number of simultaneous touch points supported by the
 * hardware. */
#define CTP_MAX_TOUCH_POINTS (10U)

/** @brief Size in bytes of a single touch point record in the status registers.
 */
#define CTP_BYTES_PER_TOUCH (8U)

/** @brief Total buffer size required to read all possible touch points. */
#define CTP_MAX_TOUCH_DATA_BYTES (CTP_MAX_TOUCH_POINTS * CTP_BYTES_PER_TOUCH)

/* ==========================================================================
   PANEL PARAMETERS (Specific to the 800x480 LCD Panel)
   ========================================================================== */

/** @brief Physical X resolution of the touch panel. Used for boundary
 * validation. */
#define CTP_MAX_X (800U)

/** @brief Physical Y resolution of the touch panel. Used for boundary
 * validation. */
#define CTP_MAX_Y (480U)

/* ==========================================================================
   TUNING HEURISTICS (Adjustable for Noise and Sensitivity)
   ========================================================================== */

/**
 * @brief Minimum acceptable touch size.
 * Points smaller than this are rejected as electrical noise.
 */
#define CTP_MIN_TOUCH_SIZE (5U)

/**
 * @brief Maximum acceptable touch size.
 * Points larger than this are rejected as invalid or "palm" touches.
 */
#define CTP_MAX_TOUCH_SIZE (100U)

/**
 * @brief Movement deadband threshold in pixels.
 * @details To prevent coordinate flicker (jitter) when a finger is stationary,
 * a touch is only marked as 'MOVE' if the distance changed exceeds this value.
 */
#define CTP_TOUCH_MOVEMENT_THRESHOLD (5U)

#endif  // CTP_DEFS_H
