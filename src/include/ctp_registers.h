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
 * @file ctp_registers.h
 * @brief Register map for the Goodix GT9271 capacitive touch controller.
 * @details This file defines the memory addresses and bitmasks used to
 * communicate with the GT9271. All addresses are based on the Rev.00 datasheet.
 */

#ifndef CTP_REGISTERS_H
#define CTP_REGISTERS_H

/* ==========================================================================
   SYSTEM & CONTROL REGISTERS
   ========================================================================== */

/** @brief Command register used to trigger internal chip actions. */
#define CTP_REG_COMMAND (0x8040U)
/** @brief ESD protection check register. */
#define CTP_REG_ESD_CHECK (0x8041U)

/* --- Command Values (Written to CTP_REG_COMMAND) --- */
/** @brief Command to enter sleep mode / turn off screen. */
#define CTP_COMMAND_OFF_SCREEN (0x05U)

/* ==========================================================================
   CONFIGURATION BLOCK
   ========================================================================== */

/** @brief Start address of the configuration data block. */
#define CTP_REG_CONFIG_DATA (0x8047U)
/** @brief End address of the configuration data block (Configuration Fresh
 * flag). */
#define CTP_REG_CONFIG_FRESH (0x8100U)

/* ==========================================================================
   DEVICE INFORMATION REGISTERS
   ========================================================================== */

/** @brief Product ID (4 bytes, ASCII). */
#define CTP_REG_PRODUCT_ID (0x8140U)
#define CTP_REG_PRODUCT_ID_SIZE (4U)

/** @brief Firmware version (2 bytes, HEX). */
#define CTP_REG_FW_VERSION (0x8144U)
#define CTP_REG_FW_VERSION_SIZE (2U)

/** @brief X-coordinate resolution (2 bytes, HEX). */
#define CTP_REG_X_CORD_RES (0x8146U)
#define CTP_REG_X_CORD_RES_SIZE (2U)

/** @brief Y-coordinate resolution (2 bytes, HEX). */
#define CTP_REG_Y_CORD_RES (0x8148U)
#define CTP_REG_Y_CORD_RES_SIZE (2U)

/** @brief Vendor identification ID. */
#define CTP_REG_VENDOR_ID (0x814AU)

/* ==========================================================================
   REAL-TIME STATUS & DATA REGISTERS
   ========================================================================== */

/**
 * @brief Status register containing touch and key flags.
 * @details This register must be cleared by writing CTP_STATUS_REG_CLEAR after
 * reading.
 */
#define CTP_REG_STATUS (0x814EU)

/* --- Status Register Bitmasks (CTP_REG_STATUS) --- */
/** @brief Data Ready Flag: 1 = New touch data available, 0 = No change. */
#define CTP_STATUS_DATA_FLAG (0x80U)
/** @brief Large Area Detection: 1 = Large area touch detected. */
#define CTP_STATUS_LARGE_DETECT (0x40U)
/** @brief Key Event Flag: 1 = A configured touch key was pressed. */
#define CTP_STATUS_HAVE_KEY (0x10U)
/** @brief Touch Count Mask: Extracts the number of active touch points (0-10).
 */
#define CTP_STATUS_TOUCH_MASK (0x0FU)
/** @brief Key Status Mask: Extracts the state of configured touch keys. */
#define CTP_STATUS_KEY_MASK (0x70U)

/** @brief Value written to CTP_REG_STATUS to clear the data ready flag. */
#define CTP_STATUS_REG_CLEAR (0x00U)

/**
 * @brief Start address of the touch coordinate data block.
 * @details Data is stored in blocks of 8 bytes per touch point.
 */
#define CTP_REG_TOUCH_DATA (0x814FU)

/** @brief Key value register for configured touch buttons. */
#define CTP_REG_KEY_VALUE (0x819FU)

/* ==========================================================================
   TOUCH POINT DATA LAYOUT (Per Point)
   ========================================================================== */

/**
 * The GT9271 stores each touch point as an 8-byte record.
 * Layout:
 * [TrackID][X_Low][X_High][Y_Low][Y_High][Size_Low][Size_High][Reserved]
 */
#define CTP_TOUCH_DATA_TRACK_ID_OFFSET (0U)
#define CTP_TOUCH_DATA_X_LOW_OFFSET (1U)
#define CTP_TOUCH_DATA_X_HIGH_OFFSET (2U)
#define CTP_TOUCH_DATA_Y_LOW_OFFSET (3U)
#define CTP_TOUCH_DATA_Y_HIGH_OFFSET (4U)
#define CTP_TOUCH_DATA_SIZE_LOW_OFFSET (5U)
#define CTP_TOUCH_DATA_SIZE_HIGH_OFFSET (6U)
#define CTP_TOUCH_DATA_RESERVED_OFFSET (7U)

#endif  // CTP_REGISTERS_H
