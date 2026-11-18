#ifndef CTP_TYPES_H
#define CTP_TYPES_H

#include <gt9271/ctp_defs.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum ctp_error_t {
  CTP_ERROR_NONE = 0,
  CTP_ERROR_HANDLER_IS_NULL,
  CTP_ERROR_PLAT_IS_NULL,
  CTP_ERROR_NULL_PTR,
  CTP_ERROR_NOT_INITIALIZED,
  CTP_ERROR_ATTEMPT_WRITE_READ_ONLY_REG,
  CTP_ERROR_ATTEMPT_READ_WRITE_ONLY_REG,
  CTP_ERROR_CONFIG_NOT_SENT,
  CTP_ERROR_CONFIG_INVALID,
  CTP_ERROR_INDEX_OOB_ATTEMPT, ///< user provided function input that would have
                               ///< exceded the bounds of an array
  CTP_ERROR_I2C,
  CTP_ERROR_TIMEOUT,
  CTP_WARNING_LEN_IS_ZERO, ///< function input has lead to non-critical early
                           ///< function exit
} ctp_error_t;

typedef enum ctp_event_t {
  CTP_EVENT_NONE = 0,
  CTP_EVENT_START,
  CTP_EVENT_MOVE,
  CTP_EVENT_RELEASE
} ctp_event_t;

typedef struct ctp_touch_t {
  uint16_t track_id;
  uint16_t x;
  uint16_t y;
  uint16_t size;
  bool is_touched;
  ctp_event_t event;
} ctp_touch_t;

typedef struct ctp_touch_list_t {
  ctp_touch_t touch[CTP_MAX_TOUCH_POINTS];
  uint_least8_t count;
  uint_least8_t key_status;
} ctp_touch_list_t;

#endif // CTP_TYPES_H
