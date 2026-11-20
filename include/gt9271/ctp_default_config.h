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
 * @file ctp_default_config.h
 * @brief Manufacturer-provided configuration for GT9271 panels.
 */

#ifndef CTP_DEFAULT_CONFIG_H
#define CTP_DEFAULT_CONFIG_H

#include <stdint.h>

/**
 * @brief Default manufacturer configuration that must be written to GT9271 on
 * boot
 *
 * @details This array contains the calibrated parameters for a specific LCD
 * panel (800x480). These values include sensitivity thresholds, noise filters,
 * and coordinate scaling factors.
 *
 * @warning Modifying this array without referring to the GT9271 register map
 * may result in reduced touch accuracy, increased ghost touches, or complete
 * device failure. Many of the valaues are reserved such that adjustments would
 * have unknown consequences. If adjusting for a different panel, ensure the
 * checksum at the end of the block is updated accordingly.
 */
extern const uint_least8_t g_ctp_default_config[];

/** @brief Length of the default manufacturer configuration array in bytes. */
extern const uint16_t g_ctp_default_config_len;

#endif  // CTP_DEFAULT_CONFIG_H
