#ifndef CTP_TYPES_H
#define CTP_TYPES_H

#include <gt9271/ctp_defs.h>
#include <gt9271/ctp_platform.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum ctp_error_t
{
  CTP_ERROR_NONE = 0,
  CTP_ERROR_HANDLER_IS_NULL,
  CTP_ERROR_PLAT_IS_NULL,
  CTP_ERROR_NULL_PTR,
  CTP_ERROR_NOT_INITIALIZED,
  CTP_ERROR_ATTEMPT_WRITE_READ_ONLY_REG,
  CTP_ERROR_ATTEMPT_READ_WRITE_ONLY_REG,
  CTP_ERROR_CONFIG_NOT_SENT,
  CTP_ERROR_CONFIG_INVALID,
  CTP_ERROR_CONFIG_RAW_CHECK_FAIL,
  CTP_ERROR_DEFS_INVALID,
  CTP_ERROR_COMM_TEST_FAIL,
  CTP_ERROR_EXCEED_MAX_TOUCH_COUNT,
  CTP_ERROR_TOUCH_COUNT_ZERO,
  CTP_ERROR_INDEX_OOB_ATTEMPT, ///< user provided function input that would have
                               ///< exceded the bounds of an array
  CTP_ERROR_I2C,
  CTP_ERROR_TIMEOUT,
  CTP_WARNING_LEN_IS_ZERO, ///< function input has lead to non-critical early
                           ///< function exit
} ctp_error_t;

typedef enum ctp_event_t
{
  CTP_EVENT_NONE = 0,
  CTP_EVENT_START,
  CTP_EVENT_MOVE,
  CTP_EVENT_RELEASE
} ctp_event_t;

typedef struct ctp_touch_t
{
  uint16_t track_id;
  uint16_t x;
  uint16_t y;
  uint16_t size;
  bool is_touched;
  ctp_event_t event;
} ctp_touch_t;

typedef struct ctp_touch_list_t
{
  ctp_touch_t touch[CTP_MAX_TOUCH_POINTS];
  uint_least8_t count;
  uint_least8_t key_status;
} ctp_touch_list_t;

typedef struct ctp_config_t
{
  const ctp_platform_interface_t *const plat;
  const uint_least8_t *const config_array;
  const uint16_t config_array_length;
  volatile bool *const isr_flag;
  const uint_least8_t default_max_touch_points;
  bool use_alt_i2c_addr;
} ctp_config_t;

/**
 * @brief Device context contains platform interface and cached device states.
 * Used to address specific device
 */
typedef struct ctp_device_t
{
  const ctp_config_t *config;
  bool is_initialized;                            ///< @ref ctp_start has been called
  uint_least8_t i2c_addr;                         ///< i2c write address, set on initialize
  uint_least8_t max_touch_points;                 ///< number of active touches to register
  ctp_touch_list_t touch_list;                    ///< All touch event data
  uint_least8_t rx_buf[CTP_MAX_TOUCH_DATA_BYTES]; ///< touch registers read buffer
} ctp_device_t;

#endif // CTP_TYPES_H
