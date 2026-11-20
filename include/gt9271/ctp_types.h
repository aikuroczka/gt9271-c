/**
 * @file ctp_types.h
 * @brief Type definitions and error codes for the GT9271 driver.
 * @details This file defines the core data structures used throughout the library,
 * including the device context, touch point records, and a comprehensive set of
 * error codes for robust failure analysis.
 */

#ifndef CTP_TYPES_H
#define CTP_TYPES_H

#include <gt9271/ctp_defs.h>
#include <gt9271/ctp_platform.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Driver error codes for precise failure diagnosis.
 */
typedef enum ctp_error_t
{
  CTP_ERROR_NONE = 0,                    ///< Operation completed successfully
  CTP_ERROR_HANDLER_IS_NULL,             ///< Device context pointer is NULL
  CTP_ERROR_CONFIG_IS_NULL,              ///< Device config pointer is NULL
  CTP_ERROR_PLAT_IS_NULL,                ///< Platform interface pointer is NULL
  CTP_ERROR_NULL_PTR,                    ///< A required data buffer pointer is NULL
  CTP_ERROR_NOT_INITIALIZED,             ///< Function called before ctp_initialize()
  CTP_ERROR_ATTEMPT_WRITE_READ_ONLY_REG, ///< Attempted to write to a read-only register
  CTP_ERROR_ATTEMPT_READ_WRITE_ONLY_REG, ///< Attempted to read from a write-only register
  CTP_ERROR_CONFIG_NOT_SENT,             ///< Configuration was not uploaded to the device
  CTP_ERROR_CONFIG_INVALID,              ///< Provided config array is NULL or wrong length
  CTP_ERROR_CONFIG_RAW_CHECK_FAIL,       ///< Read-back verification of config failed
  CTP_ERROR_DEFS_INVALID,                ///< Internal definition mismatch (e.g., buffer size)
  CTP_ERROR_COMM_TEST_FAIL,              ///< Device did not respond to basic communication test
  CTP_ERROR_EXCEED_MAX_TOUCH_COUNT,      ///< Requested touch count exceeds hardware limit
  CTP_ERROR_TOUCH_COUNT_ZERO,            ///< Touch count cannot be set to zero
  CTP_ERROR_INDEX_OOB_ATTEMPT,           ///< Index provided is out of array bounds
  CTP_ERROR_I2C,                         ///< Low-level I2C communication failure (NACK/Timeout)
  CTP_ERROR_TIMEOUT,                     ///< Hardware failed to respond within the time limit
  CTP_WARNING_LEN_IS_ZERO,               ///< Input length was zero; function exited without error
} ctp_error_t;

/**
 * @brief Touch event types for state-machine tracking.
 */
typedef enum ctp_event_t
{
  CTP_EVENT_NONE = 0, ///< No change in touch state
  CTP_EVENT_START,    ///< Finger just touched the screen
  CTP_EVENT_MOVE,     ///< Existing finger shifted position or size
  CTP_EVENT_RELEASE   ///< Finger was lifted from the screen
} ctp_event_t;

/**
 * @brief Individual touch point data.
 */
typedef struct ctp_touch_t
{
  uint16_t track_id; ///< Unique hardware ID assigned to this touch point
  uint16_t x;        ///< X coordinate in pixels
  uint16_t y;        ///< Y coordinate in pixels
  uint16_t size;     ///< Touch area size (used for noise filtering)
  bool is_touched;   ///< Current state: true if finger is present, false otherwise
  ctp_event_t event; ///< The transition event detected in the current cycle
} ctp_touch_t;

/**
 * @brief Collection of all active touch points and global status.
 */
typedef struct ctp_touch_list_t
{
  ctp_touch_t touch[CTP_MAX_TOUCH_POINTS]; ///< Array of touch point records
  uint_least8_t count;                     ///< Number of active touches reported by hardware
  uint_least8_t key_status;                ///< Raw bitmask of configured touch keys
} ctp_touch_list_t;

/**
 * @brief Configuration parameters used during device initialization.
 */
typedef struct ctp_config_t
{
  const ctp_platform_interface_t *const plat;   ///< Pointer to the PAL implementation
  const uint_least8_t *const config_array;      ///< Pointer to manufacturer config blob
  const uint16_t config_array_length;           ///< Length of the config array in bytes
  volatile bool *const isr_flag;                ///< Pointer to external ISR trigger flag
  const uint_least8_t default_max_touch_points; ///< Initial touch point limit (1-10)
  bool use_alt_i2c_addr;                        ///< true = 0x14, false = 0x5D
} ctp_config_t;

/**
 * @brief Device context containing platform interface and cached device states.
 * @details This structure acts as the primary handle for the driver. It stores
 * the current state of the hardware and the results of the most recent poll.
 */
typedef struct ctp_device_t
{
  const ctp_config_t *config;                     ///< Reference to the configuration used at init
  bool is_initialized;                            ///< True if ctp_initialize() completed successfully
  uint_least8_t i2c_addr;                         ///< Active 7-bit I2C address
  uint_least8_t max_touch_points;                 ///< Current active touch point limit
  uint_least8_t event_iterator_idx;               ///< Tracks the next touch point to yield
  ctp_touch_list_t touch_list;                    ///< Cached touch data and events
  uint_least8_t rx_buf[CTP_MAX_TOUCH_DATA_BYTES]; ///< Internal buffer for I2C reads
} ctp_device_t;

#endif // CTP_TYPES_H
